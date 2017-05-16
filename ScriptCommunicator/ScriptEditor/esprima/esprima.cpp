/*
  Copyright (C) 2012 Ariya Hidayat <ariya.hidayat@gmail.com>
  Copyright (C) 2012 Mathias Bynens <mathias@qiwi.be>
  Copyright (C) 2012 Joost-Wim Boekesteijn <joost-wim@boekesteijn.nl>
  Copyright (C) 2012 Kris Kowal <kris.kowal@cixar.com>
  Copyright (C) 2012 Yusuke Suzuki <utatane.tea@gmail.com>
  Copyright (C) 2012 Arpad Borsos <arpad.borsos@googlemail.com>
  Copyright (C) 2011 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "esprima.h"
#include <stdarg.h>
#include <sstream>
#include <set>
#include <map>
#undef EOF

using namespace esprima;

Poolable::Poolable(Pool &pool) : poolable(pool.first) {
    pool.first = this;
}

namespace Token {
    enum {
        BooleanLiteral = 1,
        EOF = 2,
        Identifier = 3,
        Keyword = 4,
        NullLiteral = 5,
        NumericLiteral = 6,
        Punctuator = 7,
        StringLiteral = 8
    };
}

namespace PropertyKind {
    enum {
        Data = 1,
        Get = 2,
        Set = 4
    };
}

namespace Messages {
    static const char *UnexpectedToken = "Unexpected token %0";
    static const char *UnexpectedNumber = "Unexpected number";
    static const char *UnexpectedString = "Unexpected string";
    static const char *UnexpectedIdentifier = "Unexpected identifier";
    static const char *UnexpectedReserved = "Unexpected reserved word";
    static const char *UnexpectedEOS = "Unexpected end of input";
    static const char *NewlineAfterThrow = "Illegal newline after throw";
    // static const char *InvalidRegExp = "Invalid regular expression";
    static const char *UnterminatedRegExp = "Invalid regular expression: missing /";
    static const char *InvalidLHSInAssignment = "Invalid left-hand side in assignment";
    static const char *InvalidLHSInForIn = "Invalid left-hand side in for-in";
    static const char *MultipleDefaultsInSwitch = "More than one default clause in switch statement";
    static const char *NoCatchOrFinally = "Missing catch or finally after try";
    static const char *UnknownLabel = "Undefined label '%0'";
    static const char *Redeclaration = "%0 '%1' has already been declared";
    static const char *IllegalContinue = "Illegal continue statement";
    static const char *IllegalBreak = "Illegal break statement";
    static const char *IllegalReturn = "Illegal return statement";
    static const char *StrictModeWith = "Strict mode code may not include a with statement";
    static const char *StrictCatchVariable = "Catch variable may not be eval or arguments in strict mode";
    static const char *StrictVarName = "Variable name may not be eval or arguments in strict mode";
    static const char *StrictParamName = "Parameter name eval or arguments is not allowed in strict mode";
    static const char *StrictParamDupe = "Strict mode function may not have duplicate parameter names";
    static const char *StrictFunctionName = "Function name may not be eval or arguments in strict mode";
    static const char *StrictOctalLiteral = "Octal literals are not allowed in strict mode.";
    static const char *StrictDelete = "Delete of an unqualified identifier in strict mode.";
    static const char *StrictDuplicateProperty = "Duplicate data property in object literal not allowed in strict mode";
    static const char *AccessorDataProperty = "Object literal may not have data and accessor property with the same name";
    static const char *AccessorGetSet = "Object literal may not have multiple get/set accessors with the same name";
    static const char *StrictLHSAssignment = "Assignment to eval or arguments is not allowed in strict mode";
    static const char *StrictLHSPostfix = "Postfix increment/decrement may not have eval or arguments operand in strict mode";
    static const char *StrictLHSPrefix = "Prefix increment/decrement may not have eval or arguments operand in strict mode";
    static const char *StrictReservedWord = "Use of future reserved word in strict mode";
}

static std::string format(std::string format, const std::string &arg0) {
    size_t index = format.find("%0");
    assert(index != std::string::npos);
    format.replace(index, 2, arg0);
    return format;
}

static std::string format(std::string format, const std::string &arg0, const std::string &arg1) {
    size_t index = format.find("%0");
    assert(index != std::string::npos);
    format.replace(index, 2, arg0);
    index = format.find("%1");
    assert(index != std::string::npos);
    format.replace(index, 2, arg1);
    return format;
}

struct EsprimaToken : Poolable {
    int type;
    std::string stringValue;
    double doubleValue;
    bool octal;
    int lineNumber;
    int lineStart;
    int range[2];

    EsprimaToken(Pool &pool) : Poolable(pool), type(), doubleValue(), octal(), lineNumber(), lineStart() { range[0] = range[1] = 0; }
};

struct EsprimaParser {
    static bool isDecimalDigit(int ch) {
        return (ch >= 48 && ch <= 57);   // 0..9
    }

    static bool isHexDigit(int ch) {
        return std::string("0123456789abcdefABCDEF").find(ch) != std::string::npos;
    }

    static bool isOctalDigit(int ch) {
        return std::string("01234567").find(ch) != std::string::npos;
    }

    // 7.2 White Space

    static bool isWhiteSpace(int ch) {
        return (ch == 32) ||  // space
            (ch == 9) ||      // tab
            (ch == 0xB) ||
            (ch == 0xC) ||
            (ch == 0xA0);
    }

    // 7.3 Line Terminators

    static bool isLineTerminator(int ch) {
        return (ch == 10) || (ch == 13) || (ch == 0x2028) || (ch == 0x2029);
    }

    // 7.6 Identifier Names and Identifiers

    static bool isIdentifierStart(int ch) {
        return (ch == 36) || (ch == 95) ||  // $ (dollar) and _ (underscore)
            (ch >= 65 && ch <= 90) ||         // A..Z
            (ch >= 97 && ch <= 122) ||        // a..z
            (ch == 92);                      // \ (backslash)
    }

    static bool isIdentifierPart(int ch) {
        return (ch == 36) || (ch == 95) ||  // $ (dollar) and _ (underscore)
            (ch >= 65 && ch <= 90) ||         // A..Z
            (ch >= 97 && ch <= 122) ||        // a..z
            (ch >= 48 && ch <= 57) ||         // 0..9
            (ch == 92);                      // \ (backslash)
    }

    // 7.6.1.2 Future Reserved Words

    static bool isFutureReservedWord(const std::string &id) {
        return
            id == "class" ||
            id == "enum" ||
            id == "export" ||
            id == "extends" ||
            id == "import" ||
            id == "super";
    }

    static bool isStrictModeReservedWord(const std::string &id) {
        return
            id == "implements" ||
            id == "interface" ||
            id == "package" ||
            id == "private" ||
            id == "protected" ||
            id == "public" ||
            id == "static" ||
            id == "yield" ||
            id == "let";
    }

    static bool isRestrictedWord(const std::string &id) {
        return id == "eval" || id == "arguments";
    }

    // 7.6.1.1 Keywords

    bool isKeyword(const std::string &id) {
        if (strict && isStrictModeReservedWord(id)) {
            return true;
        }

        // 'const' is specialized as Keyword in V8.
        // 'yield' and 'let' are for compatiblity with SpiderMonkey and ES.next.
        // Some others are from future reserved words.

        return
            (id == "if") ||
            (id == "in") ||
            (id == "do") ||
            (id == "var") ||
            (id == "for") ||
            (id == "new") ||
            (id == "try") ||
            (id == "let") ||
            (id == "this") ||
            (id == "else") ||
            (id == "case") ||
            (id == "void") ||
            (id == "with") ||
            (id == "enum") ||
            (id == "while") ||
            (id == "break") ||
            (id == "catch") ||
            (id == "throw") ||
            (id == "const") ||
            (id == "yield") ||
            (id == "class") ||
            (id == "super") ||
            (id == "return") ||
            (id == "typeof") ||
            (id == "delete") ||
            (id == "switch") ||
            (id == "export") ||
            (id == "import") ||
            (id == "default") ||
            (id == "finally") ||
            (id == "extends") ||
            (id == "function") ||
            (id == "continue") ||
            (id == "debugger") ||
            (id == "instanceof");
    }

    // 7.4 Comments

    void skipComment() {
        int ch;
        bool blockComment;
        bool lineComment;

        blockComment = false;
        lineComment = false;

        while (index < length) {
            ch = source[index];

            if (lineComment) {
                ++index;
                if (isLineTerminator(ch)) {
                    lineComment = false;
                    if (ch == 13 && source[index] == 10) {
                        ++index;
                    }
                    ++lineNumber;
                    lineStart = index;
                }
            } else if (blockComment) {
                if (isLineTerminator(ch)) {
                    if (ch == 13 && source[index + 1] == 10) {
                        ++index;
                    }
                    ++lineNumber;
                    ++index;
                    lineStart = index;
                    if (index >= length) {
                        throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
                    }
                } else {
                    ch = source[index++];
                    if (index >= length) {
                        throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
                    }
                    // Block comment ends with '*/' (char #42, char #47).
                    if (ch == 42) {
                        ch = source[index];
                        if (ch == 47) {
                            ++index;
                            blockComment = false;
                        }
                    }
                }
            } else if (ch == 47) {
                ch = source[index + 1];
                // Line comment starts with '//' (char #47, char #47).
                if (ch == 47) {
                    index += 2;
                    lineComment = true;
                } else if (ch == 42) {
                    // Block comment starts with '/*' (char #47, char #42).
                    index += 2;
                    blockComment = true;
                    if (index >= length) {
                        throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
                    }
                } else {
                    break;
                }
            } else if (isWhiteSpace(ch)) {
                ++index;
            } else if (isLineTerminator(ch)) {
                ++index;
                if (ch == 13 && source[index] == 10) {
                    ++index;
                }
                ++lineNumber;
                lineStart = index;
            } else {
                break;
            }
        }
    }

    int scanHexEscape(int prefix) {
        int i, len, ch, code = 0;

        len = (prefix == 'u') ? 4 : 2;
        for (i = 0; i < len; ++i) {
            if (index < length && isHexDigit(source[index])) {
                ch = source[index++];
                code = code * 16 + std::string("0123456789abcdef").find(tolower(ch));
            } else {
                return -1;
            }
        }
        return code;
    }

    std::string getEscapedIdentifier() {
        int ch;
        std::string id;

        ch = source[index++];
        id = std::string(1, ch);

        // '\u' (char #92, char #117) denotes an escaped character.
        if (ch == 92) {
            if (source[index] != 117) {
                throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
            }
            ++index;
            ch = scanHexEscape('u');
            if (ch == -1 || ch == '\\' || !isIdentifierStart(ch)) {
                throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
            }
            id = ch;
        }

        while (index < length) {
            ch = source[index];
            if (!isIdentifierPart(ch)) {
                break;
            }
            ++index;
            id += ch;

            // '\u' (char #92, char #117) denotes an escaped character.
            if (ch == 92) {
                id = id.substr(0, id.size() - 1);
                if (source[index] != 117) {
                    throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
                }
                ++index;
                ch = scanHexEscape('u');
                if (ch == -1 || ch == '\\' || !isIdentifierPart(ch)) {
                    throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
                }
                id += ch;
            }
        }

        return id;
    }

    std::string getIdentifier() {
        int start, ch;

        start = index++;
        while (index < length) {
            ch = source[index];
            if (ch == 92) {
                // Blackslash (char #92) marks Unicode escape sequence.
                index = start;
                return getEscapedIdentifier();
            }
            if (isIdentifierPart(ch)) {
                ++index;
            } else {
                break;
            }
        }

        return source.substr(start, index - start);
    }

    EsprimaToken *scanIdentifier() {
        int start, type;
        std::string id;

        start = index;

        // Backslash (char #92) starts an escaped character.
        id = (source[index] == 92) ? getEscapedIdentifier() : getIdentifier();

        // There is no keyword or literal with only one character.
        // Thus, it must be an identifier.
        if (id.size() == 1) {
            type = Token::Identifier;
        } else if (isKeyword(id)) {
            type = Token::Keyword;
        } else if (id == "null") {
            type = Token::NullLiteral;
        } else if (id == "true" || id == "false") {
            type = Token::BooleanLiteral;
        } else {
            type = Token::Identifier;
        }

        EsprimaToken *token = new EsprimaToken(pool);
        token->type = type;
        token->stringValue = id;
        token->lineNumber = lineNumber;
        token->lineStart = lineStart;
        token->range[0] = start; token->range[1] = index;
        return token;
    }


    // 7.7 Punctuators

    EsprimaToken *scanPunctuator() {
        EsprimaToken *token;
        int start = index;
        int code = source[index];
        int code2;
        int ch1 = source[index];
        int ch2;
        int ch3;
        int ch4;

        switch (code) {

        // Check for most common single-character punctuators.
        case 46:   // . dot
        case 40:   // ( open bracket
        case 41:   // ) close bracket
        case 59:   // ; semicolon
        case 44:   // , comma
        case 123:  // { open curly brace
        case 125:  // } close curly brace
        case 91:   // [
        case 93:   // ]
        case 58:   // :
        case 63:   // ?
        case 126:  // ~
            ++index;
            token = new EsprimaToken(pool);
            token->type = Token::Punctuator;
            token->stringValue = std::string(1, code);
            token->lineNumber = lineNumber;
            token->lineStart = lineStart;
            token->range[0] = start; token->range[1] = index;
            return token;

        default:
            code2 = source[index + 1];

            // '=' (char #61) marks an assignment or comparison operator.
            if (code2 == 61) {
                switch (code) {
                case 37:  // %
                case 38:  // &
                case 42:  // *:
                case 43:  // +
                case 45:  // -
                case 47:  // /
                case 60:  // <
                case 62:  // >
                case 94:  // ^
                case 124: // |
                    index += 2;
                    token = new EsprimaToken(pool);
                    token->type = Token::Punctuator;
                    token->stringValue = std::string(1, code) + std::string(1, code2);
                    token->lineNumber = lineNumber;
                    token->lineStart = lineStart;
                    token->range[0] = start; token->range[1] = index;
                    return token;

                case 33: // !
                case 61: // =
                    index += 2;

                    // != and ==
                    if (source[index] == 61) {
                        ++index;
                    }
                    token = new EsprimaToken(pool);
                    token->type = Token::Punctuator;
                    token->stringValue = source.substr(start, index - start);
                    token->lineNumber = lineNumber;
                    token->lineStart = lineStart;
                    token->range[0] = start; token->range[1] = index;
                    return token;

                default:
                    break;
                }
            }
            break;
        }

        // Peek more characters.

        ch2 = source[index + 1];
        ch3 = source[index + 2];
        ch4 = source[index + 3];

        // 4-character punctuator: >>>=

        if (ch1 == '>' && ch2 == '>' && ch3 == '>') {
            if (ch4 == '=') {
                index += 4;
                token = new EsprimaToken(pool);
                token->type = Token::Punctuator;
                token->stringValue = ">>>=";
                token->lineNumber = lineNumber;
                token->lineStart = lineStart;
                token->range[0] = start; token->range[1] = index;
                return token;
            }
        }

        // 3-character punctuators: == != >>> <<= >>=

        if (ch1 == '>' && ch2 == '>' && ch3 == '>') {
            index += 3;
            token = new EsprimaToken(pool);
            token->type = Token::Punctuator;
            token->stringValue = ">>>";
            token->lineNumber = lineNumber;
            token->lineStart = lineStart;
            token->range[0] = start; token->range[1] = index;
            return token;
        }

        if (ch1 == '<' && ch2 == '<' && ch3 == '=') {
            index += 3;
            token = new EsprimaToken(pool);
            token->type = Token::Punctuator;
            token->stringValue = "<<=";
            token->lineNumber = lineNumber;
            token->lineStart = lineStart;
            token->range[0] = start; token->range[1] = index;
            return token;
        }

        if (ch1 == '>' && ch2 == '>' && ch3 == '=') {
            index += 3;
            token = new EsprimaToken(pool);
            token->type = Token::Punctuator;
            token->stringValue = ">>=";
            token->lineNumber = lineNumber;
            token->lineStart = lineStart;
            token->range[0] = start; token->range[1] = index;
            return token;
        }

        // Other 2-character punctuators: ++ -- << >> && ||

        if (ch1 == ch2 && std::string("+-<>&|").find(ch1) != std::string::npos) {
            index += 2;
            token = new EsprimaToken(pool);
            token->type = Token::Punctuator;
            token->stringValue = std::string(1, ch1) + std::string(1, ch2);
            token->lineNumber = lineNumber;
            token->lineStart = lineStart;
            token->range[0] = start; token->range[1] = index;
            return token;
        }

        if (std::string("<>=!+-*%&|^/").find(ch1) != std::string::npos) {
            ++index;
            token = new EsprimaToken(pool);
            token->type = Token::Punctuator;
            token->stringValue = std::string(1, ch1);
            token->lineNumber = lineNumber;
            token->lineStart = lineStart;
            token->range[0] = start; token->range[1] = index;
            return token;
        }

        throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
    }

    // 7.8.3 Numeric Literals

    EsprimaToken *scanHexLiteral(int start) {
        std::string number = "";

        while (index < length) {
            if (!isHexDigit(source[index])) {
                break;
            }
            number += source[index++];
        }

        if (number.size() == 0) {
            throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
        }

        if (isIdentifierStart(source[index])) {
            throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
        }

        EsprimaToken *token = new EsprimaToken(pool);
        token->type = Token::NumericLiteral;
        std::stringstream(number) >> std::hex >> token->doubleValue;
        token->lineNumber = lineNumber;
        token->lineStart = lineStart;
        token->range[0] = start; token->range[1] = index;
        return token;
    }

    EsprimaToken *scanOctalLiteral(int start) {
        std::string number = std::string(1, source[index++]);
        while (index < length) {
            if (!isOctalDigit(source[index])) {
                break;
            }
            number += source[index++];
        }

        if (isIdentifierStart(source[index]) || isDecimalDigit(source[index])) {
            throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
        }

        EsprimaToken *token = new EsprimaToken(pool);
        token->type = Token::NumericLiteral;
        std::stringstream(number) >> std::oct >> token->doubleValue;
        token->octal = true;
        token->lineNumber = lineNumber;
        token->lineStart = lineStart;
        token->range[0] = start; token->range[1] = index;
        return token;
    }

    EsprimaToken *scanNumericLiteral() {
        std::string number;
        int start, ch;

        ch = source[index];
        assert(isDecimalDigit(ch) || (ch == '.') &&
            "Numeric literal must start with a decimal digit or a decimal point");

        start = index;
        number = "";
        if (ch != '.') {
            number = std::string(1, source[index++]);
            ch = source[index];

            // Hex number starts with '0x'.
            // Octal number starts with '0'.
            if (number == "0") {
                if (ch == 'x' || ch == 'X') {
                    ++index;
                    return scanHexLiteral(start);
                }
                if (isOctalDigit(ch)) {
                    return scanOctalLiteral(start);
                }

                // decimal number starts with '0' such as '09' is illegal.
                if (ch && isDecimalDigit(ch)) {
                    throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
                }
            }

            while (isDecimalDigit(source[index])) {
                number += source[index++];
            }
            ch = source[index];
        }

        if (ch == '.') {
            number += source[index++];
            while (isDecimalDigit(source[index])) {
                number += source[index++];
            }
            ch = source[index];
        }

        if (ch == 'e' || ch == 'E') {
            number += source[index++];

            ch = source[index];
            if (ch == '+' || ch == '-') {
                number += source[index++];
            }
            if (isDecimalDigit(source[index])) {
                while (isDecimalDigit(source[index])) {
                    number += source[index++];
                }
            } else {
                throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
            }
        }

        if (isIdentifierStart(source[index])) {
            throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
        }

        EsprimaToken *token = new EsprimaToken(pool);
        token->type = Token::NumericLiteral;
        std::stringstream(number) >> token->doubleValue;
        token->lineNumber = lineNumber;
        token->lineStart = lineStart;
        token->range[0] = start; token->range[1] = index;
        return token;
    }

    // 7.8.4 String Literals

    EsprimaToken *scanStringLiteral() {
        std::string str = "";
        int quote, start, ch, code, unescaped, restore;
        bool octal = false;

        quote = source[index];
        assert((quote == '\'' || quote == '"') &&
            "String literal must starts with a quote");

        start = index;
        ++index;

        while (index < length) {
            ch = source[index++];

            if (ch == quote) {
                quote = 0;
                break;
            } else if (ch == '\\') {
                ch = source[index++];
                if (!ch || !isLineTerminator(ch)) {
                    switch (ch) {
                    case 'n':
                        str += '\n';
                        break;
                    case 'r':
                        str += '\r';
                        break;
                    case 't':
                        str += '\t';
                        break;
                    case 'u':
                    case 'x':
                        restore = index;
                        unescaped = scanHexEscape(ch);
                        if (unescaped != -1) {
                            str += unescaped;
                        } else {
                            index = restore;
                            str += ch;
                        }
                        break;
                    case 'b':
                        str += '\b';
                        break;
                    case 'f':
                        str += '\f';
                        break;
                    case 'v':
                        str += '\v';
                        break;

                    default:
                        if (isOctalDigit(ch)) {
                            code = ch - '0';

                            // \0 is not octal escape sequence
                            if (code != 0) {
                                octal = true;
                            }

                            if (index < length && isOctalDigit(source[index])) {
                                octal = true;
                                code = code * 8 + source[index++] - '0';

                                // 3 digits are only allowed when string starts
                                // with 0, 1, 2, 3
                                if (std::string(1, '0123').find(ch) != std::string::npos &&
                                        index < length &&
                                        isOctalDigit(source[index])) {
                                    code = code * 8 + source[index++] - '0';
                                }
                            }
                            str += std::string(1, code);
                        } else {
                            str += ch;
                        }
                        break;
                    }
                } else {
                    ++lineNumber;
                    if (ch ==  '\r' && source[index] == '\n') {
                        ++index;
                    }
                }
            } else if (isLineTerminator(ch)) {
                break;
            } else {
                str += ch;
            }
        }

        if (quote != 0) {
            throwError(NULL, format(Messages::UnexpectedToken, "ILLEGAL"));
        }

        EsprimaToken *token = new EsprimaToken(pool);
        token->type = Token::StringLiteral;
        token->stringValue = str;
        token->octal = octal;
        token->lineNumber = lineNumber;
        token->lineStart = lineStart;
        token->range[0] = start; token->range[1] = index;
        return token;
    }

    struct EsprimaRegExp : Poolable {
        std::string literal, pattern, flags;
        int range[2];

        EsprimaRegExp(Pool& pool, const std::string &literal, const std::string &pattern, const std::string &flags, int rangeMin, int rangeMax)
                : Poolable(pool), literal(literal), pattern(pattern), flags(flags) {
            range[0] = rangeMin;
            range[1] = rangeMax;
        }
    };

    EsprimaRegExp *scanRegExp() {
        std::string str;
        int ch, start;
        std::string pattern, flags;
        bool classMarker = false;
        int restore;
        bool terminated = false;

        lookahead = NULL;
        skipComment();

        start = index;
        ch = source[index];
        assert(ch == '/' && "Regular expression literal must start with a slash");
        str = source[index++];

        while (index < length) {
            ch = source[index++];
            str += ch;
            if (classMarker) {
                if (ch == ']') {
                    classMarker = false;
                }
            } else {
                if (ch == '\\') {
                    ch = source[index++];
                    // ECMA-262 7.8.5
                    if (isLineTerminator(ch)) {
                        throwError(NULL, Messages::UnterminatedRegExp);
                    }
                    str += ch;
                } else if (ch == '/') {
                    terminated = true;
                    break;
                } else if (ch == '[') {
                    classMarker = true;
                } else if (isLineTerminator(ch)) {
                    throwError(NULL, Messages::UnterminatedRegExp);
                }
            }
        }

        if (!terminated) {
            throwError(NULL, Messages::UnterminatedRegExp);
        }

        // Exclude leading and trailing slash.
        pattern = str.substr(1, str.size() - 2);

        flags = "";
        while (index < length) {
            ch = source[index];
            if (!isIdentifierPart(ch)) {
                break;
            }

            ++index;
            if (ch == '\\' && index < length) {
                ch = source[index];
                if (ch == 'u') {
                    ++index;
                    restore = index;
                    ch = scanHexEscape('u');
                    if (ch != -1) {
                        flags += ch;
                        for (str += "\\u"; restore < index; ++restore) {
                            str += source[restore];
                        }
                    } else {
                        index = restore;
                        flags += 'u';
                        str += "\\u";
                    }
                } else {
                    str += '\\';
                }
            } else {
                flags += ch;
                str += ch;
            }
        }

        peek();

        return new EsprimaRegExp(pool,
            str,
            pattern,
            flags,
            start, index
        );
    }

    bool isIdentifierName(EsprimaToken *token) {
        return token->type == Token::Identifier ||
            token->type == Token::Keyword ||
            token->type == Token::BooleanLiteral ||
            token->type == Token::NullLiteral;
    }

    EsprimaToken *advance() {
        int ch;

        skipComment();

        if (index >= length) {
            EsprimaToken *token = new EsprimaToken(pool);
            token->type = Token::EOF;
            token->lineNumber = lineNumber;
            token->lineStart = lineStart;
            token->range[0] = index; token->range[1] = index;
            return token;
        }

        ch = source[index];

        // Very common: ( and ) and ;
        if (ch == 40 || ch == 41 || ch == 58) {
            return scanPunctuator();
        }

        // String literal starts with single quote (#39) or double quote (#34).
        if (ch == 39 || ch == 34) {
            return scanStringLiteral();
        }

        if (isIdentifierStart(ch)) {
            return scanIdentifier();
        }

        // Dot (.) char #46 can also start a floating-point number, hence the need
        // to check the next character.
        if (ch == 46) {
            if (isDecimalDigit(source[index + 1])) {
                return scanNumericLiteral();
            }
            return scanPunctuator();
        }

        if (isDecimalDigit(ch)) {
            return scanNumericLiteral();
        }

        return scanPunctuator();
    }

    EsprimaToken *lex() {
        EsprimaToken *token;

        token = lookahead;
        index = token->range[1];
        lineNumber = token->lineNumber;
        lineStart = token->lineStart;

        lookahead = advance();

        index = token->range[1];
        lineNumber = token->lineNumber;
        lineStart = token->lineStart;

        return token;
    }

    void peek() {
        int pos, line, start;

        pos = index;
        line = lineNumber;
        start = lineStart;
        lookahead = advance();
        index = pos;
        lineNumber = line;
        lineStart = start;
    }

    struct SyntaxTreeDelegate {
        EsprimaParser &parser;
        SyntaxTreeDelegate(EsprimaParser &parser) : parser(parser) {}

        void postProcess(Node *node) {
        }

        ArrayExpression *createArrayExpression(const std::vector<Node *> &elements) {
            ArrayExpression *node = new ArrayExpression(parser.pool);
            node->elements = elements;
            return node;
        }

        AssignmentExpression *createAssignmentExpression(const std::string &operator_, Node *left, Node *right) {
            AssignmentExpression *node = new AssignmentExpression(parser.pool);
            node->operator_ = operator_;
            node->left = left;
            node->right = right;
            return node;
        }

        Node *createBinaryExpression(const std::string &operator_, Node *left, Node *right) {
            if (operator_ == "||" || operator_ == "&&") {
                LogicalExpression *node = new LogicalExpression(parser.pool);
                node->operator_ = operator_;
                node->left = left;
                node->right = right;
                return node;
            } else {
                BinaryExpression *node = new BinaryExpression(parser.pool);
                node->operator_ = operator_;
                node->left = left;
                node->right = right;
                return node;
            }
        }

        BlockStatement *createBlockStatement(const std::vector<Node *> &body) {
            BlockStatement *node = new BlockStatement(parser.pool);
            node->body = body;
            return node;
        }

        BreakStatement *createBreakStatement(Identifier *label) {
            BreakStatement *node = new BreakStatement(parser.pool);
            node->label = label;
            return node;
        }

        CallExpression *createCallExpression(Node *callee, const std::vector<Node *> &args) {
            CallExpression *node = new CallExpression(parser.pool);
            node->callee = callee;
            node->arguments = args;
            return node;
        }

        CatchClause *createCatchClause(Node *param, BlockStatement *body) {
            CatchClause *node = new CatchClause(parser.pool);
            node->param = param;
            node->body = body;
            return node;
        }

        ConditionalExpression *createConditionalExpression(Node *test, Node *consequent, Node *alternate) {
            ConditionalExpression *node = new ConditionalExpression(parser.pool);
            node->test = test;
            node->consequent = consequent;
            node->alternate = alternate;
            return node;
        }

        ContinueStatement *createContinueStatement(Identifier *label) {
            ContinueStatement *node = new ContinueStatement(parser.pool);
            node->label = label;
            return node;
        }

        DebuggerStatement *createDebuggerStatement() {
            DebuggerStatement *node = new DebuggerStatement(parser.pool);
            return node;
        }

        DoWhileStatement *createDoWhileStatement(Node *body, Node *test) {
            DoWhileStatement *node = new DoWhileStatement(parser.pool);
            node->body = body;
            node->test = test;
            return node;
        }

        EmptyStatement *createEmptyStatement() {
            EmptyStatement *node = new EmptyStatement(parser.pool);
            return node;
        }

        ExpressionStatement *createExpressionStatement(Node *expression) {
            ExpressionStatement *node = new ExpressionStatement(parser.pool);
            node->expression = expression;
            return node;
        }

        ForStatement *createForStatement(Node *init, Node *test, Node *update, Node *body) {
            ForStatement *node = new ForStatement(parser.pool);
            node->init = init;
            node->test = test;
            node->update = update;
            node->body = body;
            return node;
        }

        ForInStatement *createForInStatement(Node *left, Node *right, Node *body) {
            ForInStatement *node = new ForInStatement(parser.pool);
            node->left = left;
            node->right = right;
            node->body = body;
            return node;
        }

        FunctionDeclaration *createFunctionDeclaration(Identifier *id, const std::vector<Identifier *> &params, BlockStatement *body) {
            FunctionDeclaration *node = new FunctionDeclaration(parser.pool);
            node->id = id;
            node->params = params;
            node->body = body;
            return node;
        }

        FunctionExpression *createFunctionExpression(Identifier *id, const std::vector<Identifier *> &params, BlockStatement *body) {
            FunctionExpression *node = new FunctionExpression(parser.pool);
            node->id = id;
            node->params = params;
            node->body = body;
            return node;
        }

        Identifier *createIdentifier(const std::string &name) {
            Identifier *node = new Identifier(parser.pool);
            node->name = name;
            return node;
        }

        IfStatement *createIfStatement(Node *test, Node *consequent, Node *alternate) {
            IfStatement *node = new IfStatement(parser.pool);
            node->test = test;
            node->consequent = consequent;
            node->alternate = alternate;
            return node;
        }

        LabeledStatement *createLabeledStatement(Identifier *label, Node *body) {
            LabeledStatement *node = new LabeledStatement(parser.pool);
            node->label = label;
            node->body = body;
            return node;
        }

        Literal *createLiteral(EsprimaToken *token) {
            if (token->type == Token::NumericLiteral) {
                NumericLiteral *node = new NumericLiteral(parser.pool);
                node->value = token->doubleValue;
                return node;
            } else if (token->type == Token::NullLiteral) {
                NullLiteral *node = new NullLiteral(parser.pool);
                return node;
            } else if (token->type == Token::StringLiteral) {
                StringLiteral *node = new StringLiteral(parser.pool);
                node->value = token->stringValue;
                return node;
            } else if (token->type == Token::BooleanLiteral) {
                BooleanLiteral *node = new BooleanLiteral(parser.pool);
                node->value = token->stringValue == "true";
                return node;
            } else {
                assert(false);
            }
        }

        Literal *createLiteral(EsprimaRegExp *token) {
            RegExpLiteral *node = new RegExpLiteral(parser.pool);
            node->pattern = token->pattern;
            node->flags = token->flags;
            return node;
        }

        MemberExpression *createMemberExpression(int accessor, Node *object, Node *property) {
            MemberExpression *node = new MemberExpression(parser.pool);
            node->computed = accessor == '[';
            node->object = object;
            node->property = property;
            return node;
        }

        NewExpression *createNewExpression(Node *callee, const std::vector<Node *> &args) {
            NewExpression *node = new NewExpression(parser.pool);
            node->callee = callee;
            node->arguments = args;
            return node;
        }

        ObjectExpression *createObjectExpression(const std::vector<Property *> &properties) {
            ObjectExpression *node = new ObjectExpression(parser.pool);
            node->properties = properties;
            return node;
        }

        UpdateExpression *createPostfixExpression(const std::string &operator_, Node *argument) {
            UpdateExpression *node = new UpdateExpression(parser.pool);
            node->operator_ = operator_;
            node->argument = argument;
            node->prefix = false;
            return node;
        }

        Program *createProgram(const std::vector<Node *> &body) {
            Program *node = new Program(parser.pool);
            node->body = body;
            return node;
        }

        Property *createProperty(const std::string &kind, Node *key, Node *value) {
            Property *node = new Property(parser.pool);
            node->key = key;
            node->value = value;
            node->kind = kind;
            return node;
        }

        ReturnStatement *createReturnStatement(Node *argument) {
            ReturnStatement *node = new ReturnStatement(parser.pool);
            node->argument = argument;
            return node;
        }

        SequenceExpression *createSequenceExpression(const std::vector<Node *> &expressions) {
            SequenceExpression *node = new SequenceExpression(parser.pool);
            node->expressions = expressions;
            return node;
        }

        SwitchCase *createSwitchCase(Node *test, const std::vector<Node *> &consequent) {
            SwitchCase *node = new SwitchCase(parser.pool);
            node->test = test;
            node->consequent = consequent;
            return node;
        }

        SwitchStatement *createSwitchStatement(Node *discriminant, const std::vector<SwitchCase *> &cases) {
            SwitchStatement *node = new SwitchStatement(parser.pool);
            node->discriminant = discriminant;
            node->cases = cases;
            return node;
        }

        ThisExpression *createThisExpression() {
            ThisExpression *node = new ThisExpression(parser.pool);
            return node;
        }

        ThrowStatement *createThrowStatement(Node *argument) {
            ThrowStatement *node = new ThrowStatement(parser.pool);
            node->argument = argument;
            return node;
        }

        TryStatement *createTryStatement(BlockStatement *block, CatchClause *handler, BlockStatement *finalizer) {
            TryStatement *node = new TryStatement(parser.pool);
            node->block = block;
            node->handler = handler;
            node->finalizer = finalizer;
            return node;
        }

        Node *createUnaryExpression(const std::string &operator_, Node *argument) {
            if (operator_ == "++" || operator_ == "--") {
                UpdateExpression *node = new UpdateExpression(parser.pool);
                node->operator_ = operator_;
                node->argument = argument;
                node->prefix = true;
                return node;
            } else {
                UnaryExpression *node = new UnaryExpression(parser.pool);
                node->operator_ = operator_;
                node->argument = argument;
                return node;
            }
        }

        VariableDeclaration *createVariableDeclaration(const std::vector<VariableDeclarator *> &declarations, const std::string &kind) {
            VariableDeclaration *node = new VariableDeclaration(parser.pool);
            node->declarations = declarations;
            node->kind = kind;
            return node;
        }

        VariableDeclarator *createVariableDeclarator(Identifier *id, Node *init) {
            VariableDeclarator *node = new VariableDeclarator(parser.pool);
            node->id = id;
            node->init = init;
            return node;
        }

        WhileStatement *createWhileStatement(Node *test, Node *body) {
            WhileStatement *node = new WhileStatement(parser.pool);
            node->test = test;
            node->body = body;
            return node;
        }

        WithStatement *createWithStatement(Node *object, Node *body) {
            WithStatement *node = new WithStatement(parser.pool);
            node->object = object;
            node->body = body;
            return node;
        }
    };

    // Return true if there is a line terminator before the next token.

    bool peekLineTerminator() {
        int pos, line, start, found;

        pos = index;
        line = lineNumber;
        start = lineStart;
        skipComment();
        found = lineNumber != line;
        index = pos;
        lineNumber = line;
        lineStart = start;

        return found;
    }

    // Throw an exception

    void throwError(EsprimaToken *token, const std::string &msg) __attribute__((noreturn)) {
        ParseError error;

        if (token) {
            std::stringstream ss;
            ss << "Line " << token->lineNumber << ": " << msg;
            error.description = ss.str();
            error.index = token->range[0];
            error.lineNumber = token->lineNumber;
            error.column = token->range[0] - lineStart + 1;
        } else {
            std::stringstream ss;
            ss << "Line " << lineNumber << ": " << msg;
            error.description = ss.str();
            error.index = index;
            error.lineNumber = lineNumber;
            error.column = index - lineStart + 1;
        }

        throw error;
    }


    // Throw an exception because of the token.

    void throwUnexpected(EsprimaToken *token) __attribute__((noreturn)) {
        if (token->type == Token::EOF) {
            throwError(token, Messages::UnexpectedEOS);
        }

        if (token->type == Token::NumericLiteral) {
            throwError(token, Messages::UnexpectedNumber);
        }

        if (token->type == Token::StringLiteral) {
            throwError(token, Messages::UnexpectedString);
        }

        if (token->type == Token::Identifier) {
            throwError(token, Messages::UnexpectedIdentifier);
        }

        if (token->type == Token::Keyword) {
            if (isFutureReservedWord(token->stringValue)) {
                throwError(token, Messages::UnexpectedReserved);
            } else if (strict && isStrictModeReservedWord(token->stringValue)) {
                throwError(token, Messages::StrictReservedWord);
            }
            throwError(token, format(Messages::UnexpectedToken, token->stringValue.c_str()));
        }

        // BooleanLiteral, NullLiteral, or Punctuator.
        throwError(token, format(Messages::UnexpectedToken, token->stringValue.c_str()));
    }


    // Expect the next token to match the specified punctuator.
    // If not, an exception will be thrown.

    void expect(const std::string &value) {
        EsprimaToken *token = lex();
        if (token->type != Token::Punctuator || token->stringValue != value) {
            throwUnexpected(token);
        }
    }

    // Expect the next token to match the specified keyword.
    // If not, an exception will be thrown.

    void expectKeyword(const std::string &keyword) {
        EsprimaToken *token = lex();
        if (token->type != Token::Keyword || token->stringValue != keyword) {
            throwUnexpected(token);
        }
    }

    // Return true if the next token matches the specified punctuator.

    bool match(const std::string &value) {
        return lookahead->type == Token::Punctuator && lookahead->stringValue == value;
    }

    // Return true if the next token matches the specified keyword

    bool matchKeyword(const std::string &keyword) {
        return lookahead->type == Token::Keyword && lookahead->stringValue == keyword;
    }

    // Return true if the next token is an assignment operator

    bool matchAssign() {
        std::string op;

        if (lookahead->type != Token::Punctuator) {
            return false;
        }
        op = lookahead->stringValue;
        return op == "=" ||
            op == "*=" ||
            op == "/=" ||
            op == "%=" ||
            op == "+=" ||
            op == "-=" ||
            op == "<<=" ||
            op == ">>=" ||
            op == ">>>=" ||
            op == "&=" ||
            op == "^=" ||
            op == "|=";
    }

    void consumeSemicolon() {
        int line;

        // Catch the very common case first: immediately a semicolon (char #59).
        if (source[index] == 59) {
            lex();
            return;
        }

        line = lineNumber;
        skipComment();
        if (lineNumber != line) {
            return;
        }

        if (match(";")) {
            lex();
            return;
        }

        if (lookahead->type != Token::EOF && !match("}")) {
            throwUnexpected(lookahead);
        }
    }

    // Return true if provided expression is LeftHandSideExpression

    bool isLeftHandSide(Node *expr) {
        return expr->is<Identifier>() || expr->is<MemberExpression>();
    }

    // 11.1.4 Array Initialiser

    ArrayExpression *parseArrayInitialiser() {
        std::vector<Node *> elements;

        expect("[");

        while (!match("]")) {
            if (match(",")) {
                lex();
                elements.push_back(NULL);
            } else {
                elements.push_back(parseAssignmentExpression());

                if (!match("]")) {
                    expect(",");
                }
            }
        }

        expect("]");

        return delegate.createArrayExpression(elements);
    }

    // 11.1.5 Object Initialiser

    FunctionExpression *parsePropertyFunction(const std::vector<Identifier *> &param, EsprimaToken *first) {
        WrapTrackingFunction wtf(*this);
        bool previousStrict;
        BlockStatement *body;

        previousStrict = strict;
        body = parseFunctionSourceElements();
        if (first && strict && isRestrictedWord(param[0]->name)) {
            throwError(first, Messages::StrictParamName);
        }
        strict = previousStrict;
        return wtf.close(delegate.createFunctionExpression(NULL, param, body));
    }

    Node *parseObjectPropertyKey() {
        WrapTrackingFunction wtf(*this);
        EsprimaToken *token = lex();

        // Note: This function is called only from parseObjectProperty(), where
        // EOF and Punctuator tokens are already filtered out.

        if (token->type == Token::StringLiteral || token->type == Token::NumericLiteral) {
            if (strict && token->octal) {
                throwError(token, Messages::StrictOctalLiteral);
            }
            return wtf.close(delegate.createLiteral(token));
        }

        return wtf.close(delegate.createIdentifier(token->stringValue));
    }

    Property *parseObjectProperty() {
        WrapTrackingFunction wtf(*this);
        EsprimaToken *token;
        Node *key;
        Node *id, *value;
        std::vector<Identifier *> param;

        token = lookahead;

        if (token->type == Token::Identifier) {

            id = parseObjectPropertyKey();

            // Property Assignment: Getter and Setter.

            if (token->stringValue == "get" && !match(":")) {
                key = parseObjectPropertyKey();
                expect("(");
                expect(")");
                value = parsePropertyFunction(param, NULL);
                return wtf.close(delegate.createProperty("get", key, value));
            }
            if (token->stringValue == "set" && !match(":")) {
                key = parseObjectPropertyKey();
                expect("(");
                token = lookahead;
                if (token->type != Token::Identifier) {
                    throwUnexpected(lex());
                }
                param.push_back(parseVariableIdentifier());
                expect(")");
                value = parsePropertyFunction(param, token);
                return wtf.close(delegate.createProperty("set", key, value));
            }
            expect(":");
            value = parseAssignmentExpression();
            return wtf.close(delegate.createProperty("init", id, value));
        }
        if (token->type == Token::EOF || token->type == Token::Punctuator) {
            throwUnexpected(token);
        } else {
            key = parseObjectPropertyKey();
            expect(":");
            value = parseAssignmentExpression();
            return wtf.close(delegate.createProperty("init", key, value));
        }
    }

    ObjectExpression *parseObjectInitialiser() {
        std::vector<Property *> properties;
        Property *property;
        std::string name;
        int kind;
        std::map<std::string, int> map;

        expect("{");

        while (!match("}")) {
            property = parseObjectProperty();

            if (property->key->is<Identifier>()) {
                name = property->key->as<Identifier>()->name;
            } else if (property->key->is<StringLiteral>()) {
                name = property->key->as<StringLiteral>()->value;
            } else {
                assert(property->key->is<NumericLiteral>());
                std::stringstream ss;
                ss << property->key->as<NumericLiteral>()->value;
                name = ss.str();
            }
            kind = (property->kind == "init") ? PropertyKind::Data : (property->kind == "get") ? PropertyKind::Get : PropertyKind::Set;

            if (map.count(name)) {
                if (map[name] == PropertyKind::Data) {
                    if (strict && kind == PropertyKind::Data) {
                        throwError(NULL, Messages::StrictDuplicateProperty);
                    } else if (kind != PropertyKind::Data) {
                        throwError(NULL, Messages::AccessorDataProperty);
                    }
                } else {
                    if (kind == PropertyKind::Data) {
                        throwError(NULL, Messages::AccessorDataProperty);
                    } else if (map[name] & kind) {
                        throwError(NULL, Messages::AccessorGetSet);
                    }
                }
                map[name] |= kind;
            } else {
                map[name] = kind;
            }

            properties.push_back(property);

            if (!match("}")) {
                expect(",");
            }
        }

        expect("}");

        return delegate.createObjectExpression(properties);
    }

    // 11.1.6 The Grouping Operator

    Node *parseGroupExpression() {
        WrapTrackingFunction wtf(*this);
        return wtf.close(trackGroupExpression());

        /*
        Node *expr;

        expect("(");

        expr = parseExpression();

        expect(")");

        return expr;
        */
    }

    // 11.1 Primary Expressions

    Node *parsePrimaryExpression() {
        WrapTrackingFunction wtf(*this);
        int type;
        EsprimaToken *token;

        type = lookahead->type;

        if (type == Token::Identifier) {
            return wtf.close(delegate.createIdentifier(lex()->stringValue));
        }

        if (type == Token::StringLiteral || type == Token::NumericLiteral) {
            if (strict && lookahead->octal) {
                throwError(lookahead, Messages::StrictOctalLiteral);
            }
            return wtf.close(delegate.createLiteral(lex()));
        }

        if (type == Token::Keyword) {
            if (matchKeyword("this")) {
                lex();
                return wtf.close(delegate.createThisExpression());
            }

            if (matchKeyword("function")) {
                return wtf.close(parseFunctionExpression());
            }
        }

        if (type == Token::BooleanLiteral) {
            token = lex();
            return wtf.close(delegate.createLiteral(token));
        }

        if (type == Token::NullLiteral) {
            token = lex();
            return wtf.close(delegate.createLiteral(token));
        }

        if (match("[")) {
            return wtf.close(parseArrayInitialiser());
        }

        if (match("{")) {
            return wtf.close(parseObjectInitialiser());
        }

        if (match("(")) {
            return wtf.close(parseGroupExpression());
        }

        if (match("/") || match("/=")) {
            return wtf.close(delegate.createLiteral(scanRegExp()));
        }

        throwUnexpected(lex());
    }

    // 11.2 Left-Hand-Side Expressions

    std::vector<Node *> parseArguments() {
        std::vector<Node *> args;

        expect("(");

        if (!match(")")) {
            while (index < length) {
                args.push_back(parseAssignmentExpression());
                if (match(")")) {
                    break;
                }
                expect(",");
            }
        }

        expect(")");

        return args;
    }

    Identifier *parseNonComputedProperty() {
        WrapTrackingFunction wtf(*this);
        EsprimaToken *token = lex();

        if (!isIdentifierName(token)) {
            throwUnexpected(token);
        }

        return wtf.close(delegate.createIdentifier(token->stringValue));
    }

    Identifier *parseNonComputedMember() {
        expect(".");

        return parseNonComputedProperty();
    }

    Node *parseComputedMember() {
        WrapTrackingFunction wtf(*this);
        Node *expr;

        expect("[");

        expr = parseExpression();

        expect("]");

        return wtf.close(expr);
    }

    NewExpression *parseNewExpression() {
        WrapTrackingFunction wtf(*this);
        Node *callee;
        std::vector<Node *> args;

        expectKeyword("new");
        callee = parseLeftHandSideExpression();
        if (match("(")) args = parseArguments();

        return wtf.close(delegate.createNewExpression(callee, args));
    }

    Node *parseLeftHandSideExpressionAllowCall() {
        WrapTrackingFunction wtf(*this);
        return wtf.close(trackLeftHandSideExpressionAllowCall());

        /*
        Node *expr;
        std::vector<Node *> args;
        Node *property;

        expr = matchKeyword("new") ? parseNewExpression() : parsePrimaryExpression();

        while (match(".") || match("[") || match("(")) {
            if (match("(")) {
                args = parseArguments();
                expr = delegate.createCallExpression(expr, args);
            } else if (match("[")) {
                property = parseComputedMember();
                expr = delegate.createMemberExpression('[', expr, property);
            } else {
                property = parseNonComputedMember();
                expr = delegate.createMemberExpression('.', expr, property);
            }
        }

        return expr;
        */
    }

    Node *parseLeftHandSideExpression() {
        WrapTrackingFunction wtf(*this);
        return wtf.close(trackLeftHandSideExpression());

        /*
        Node *expr;
        Node *property;

        expr = matchKeyword("new") ? parseNewExpression() : parsePrimaryExpression();

        while (match(".") || match("[")) {
            if (match("[")) {
                property = parseComputedMember();
                expr = delegate.createMemberExpression('[', expr, property);
            } else {
                property = parseNonComputedMember();
                expr = delegate.createMemberExpression('.', expr, property);
            }
        }

        return expr;
        */
    }

    // 11.3 Postfix Expressions

    Node *parsePostfixExpression() {
        Node *expr = parseLeftHandSideExpressionAllowCall();
        EsprimaToken *token;

        if (lookahead->type != Token::Punctuator) {
            return expr;
        }

        if ((match("++") || match("--")) && !peekLineTerminator()) {
            // 11.3.1, 11.3.2
            if (strict && expr->is<Identifier>() && isRestrictedWord(expr->as<Identifier>()->name)) {
                throwError(NULL, Messages::StrictLHSPostfix);
            }

            if (!isLeftHandSide(expr)) {
                throwError(NULL, Messages::InvalidLHSInAssignment);
            }

            token = lex();
            expr = delegate.createPostfixExpression(token->stringValue, expr);
        }

        return expr;
    }

    // 11.4 Unary Operators

    Node *parseUnaryExpression() {
        WrapTrackingFunction wtf(*this);
        EsprimaToken *token;
        Node *expr;

        if (lookahead->type != Token::Punctuator && lookahead->type != Token::Keyword) {
            return wtf.close(parsePostfixExpression());
        }

        if (match("++") || match("--")) {
            token = lex();
            expr = parseUnaryExpression();
            // 11.4.4, 11.4.5
            if (strict && expr->is<Identifier>() && isRestrictedWord(expr->as<Identifier>()->name)) {
                throwError(NULL, Messages::StrictLHSPrefix);
            }

            if (!isLeftHandSide(expr)) {
                throwError(NULL, Messages::InvalidLHSInAssignment);
            }

            return wtf.close(delegate.createUnaryExpression(token->stringValue, expr));
        }

        if (match("+") || match("-") || match("~") || match("!")) {
            token = lex();
            expr = parseUnaryExpression();
            return wtf.close(delegate.createUnaryExpression(token->stringValue, expr));
        }

        if (matchKeyword("delete") || matchKeyword("void") || matchKeyword("typeof")) {
            token = lex();
            expr = parseUnaryExpression();
            expr = delegate.createUnaryExpression(token->stringValue, expr);
            if (strict && expr->as<UnaryExpression>()->operator_ == "delete" && expr->as<UnaryExpression>()->argument->is<Identifier>()) {
                throwError(NULL, Messages::StrictDelete);
            }
            return wtf.close(expr);
        }

        return wtf.close(parsePostfixExpression());
    }

    int binaryPrecedence(EsprimaToken *token, bool allowIn) {
        int prec = 0;

        if (token->type != Token::Punctuator && token->type != Token::Keyword) {
            return 0;
        }

        const std::string &value = token->stringValue;

        if (value == "||") {
            prec = 1;
        }

        else if (value == "&&") {
            prec = 2;
        }

        else if (value == "|") {
            prec = 3;
        }

        else if (value == "^") {
            prec = 4;
        }

        else if (value == "&") {
            prec = 5;
        }

        else if (value == "==" || value == "!=" || value == "===" || value == "!==") {
            prec = 6;
        }

        else if (value == "<" || value == ">" || value == "<=" || value == ">=" || value == "instanceof") {
            prec = 7;
        }

        else if (value == "in") {
            prec = allowIn ? 7 : 0;
        }

        else if (value == "<<" || value == ">>" || value == ">>>") {
            prec = 8;
        }

        else if (value == "+" || value == "-") {
            prec = 9;
        }

        else if (value == "*" || value == "/" || value == "%") {
            prec = 11;
        }

        return prec;
    }

    // 11.5 Multiplicative Operators
    // 11.6 Additive Operators
    // 11.7 Bitwise Shift Operators
    // 11.8 Relational Operators
    // 11.9 Equality Operators
    // 11.10 Binary Bitwise Operators
    // 11.11 Binary Logical Operators

    Node *parseBinaryExpression() {
        WrapTrackingFunction wtf(*this);
        Node *expr;
        EsprimaToken *token;
        int prec;
        bool previousAllowIn;
        std::vector<int> precedenceStack;
        std::vector<EsprimaToken *> operatorStack;
        std::vector<Node *> expressionStack;
        Node *right;
        std::string operator_;
        Node *left;
        int i;

        previousAllowIn = state.allowIn;
        state.allowIn = true;

        expr = parseUnaryExpression();

        token = lookahead;
        prec = binaryPrecedence(token, previousAllowIn);
        if (prec == 0) {
            return wtf.close(expr);
        }
        lex();

        expressionStack.push_back(expr);
        precedenceStack.push_back(prec);
        operatorStack.push_back(token);
        expressionStack.push_back(parseUnaryExpression());

        while ((prec = binaryPrecedence(lookahead, previousAllowIn)) > 0) {

            // Reduce: make a binary expression from the three topmost entries.
            while ((expressionStack.size() > 1) && (prec <= precedenceStack[precedenceStack.size() - 1])) {
                right = expressionStack[expressionStack.size() - 1]; expressionStack.pop_back();
                operator_ = operatorStack[operatorStack.size() - 1]->stringValue; operatorStack.pop_back(); precedenceStack.pop_back();
                left = expressionStack[expressionStack.size() - 1]; expressionStack.pop_back();
                expressionStack.push_back(delegate.createBinaryExpression(operator_, left, right));
            }

            // Shift.
            token = lex();
            precedenceStack.push_back(prec);
            operatorStack.push_back(token);
            expressionStack.push_back(parseUnaryExpression());
        }

        state.allowIn = previousAllowIn;

        // Final reduce to clean-up the stack.
        i = expressionStack.size() - 1;
        expr = expressionStack[i];
        while (i > 0) {
            expr = delegate.createBinaryExpression(operatorStack[i - 1]->stringValue, expressionStack[i - 1], expr);
            i--;
        }
        return wtf.close(expr);
    }

    // 11.12 Conditional Operator

    Node *parseConditionalExpression() {
        WrapTrackingFunction wtf(*this);
        Node *expr;
        bool previousAllowIn;
        Node *consequent, *alternate;

        expr = parseBinaryExpression();

        if (match("?")) {
            lex();
            previousAllowIn = state.allowIn;
            state.allowIn = true;
            consequent = parseAssignmentExpression();
            state.allowIn = previousAllowIn;
            expect(":");
            alternate = parseAssignmentExpression();

            expr = delegate.createConditionalExpression(expr, consequent, alternate);
        }

        return wtf.close(expr);
    }

    // 11.13 Assignment Operators

    Node *parseAssignmentExpression() {
        WrapTrackingFunction wtf(*this);
        EsprimaToken *token;
        Node *left, *right;

        token = lookahead;
        left = parseConditionalExpression();

        if (matchAssign()) {
            // LeftHandSideExpression
            if (!isLeftHandSide(left)) {
                throwError(NULL, Messages::InvalidLHSInAssignment);
            }

            // 11.13.1
            if (strict && left->is<Identifier>() && isRestrictedWord(left->as<Identifier>()->name)) {
                throwError(token, Messages::StrictLHSAssignment);
            }

            token = lex();
            right = parseAssignmentExpression();
            return wtf.close(delegate.createAssignmentExpression(token->stringValue, left, right));
        }

        return wtf.close(left);
    }

    // 11.14 Comma Operator

    Node *parseExpression() {
        WrapTrackingFunction wtf(*this);
        Node *expr = parseAssignmentExpression();

        if (match(",")) {
            std::vector<Node *> expressions;
            expressions.push_back(expr);

            while (index < length) {
                if (!match(",")) {
                    break;
                }
                lex();
                expressions.push_back(parseAssignmentExpression());
            }

            expr = delegate.createSequenceExpression(expressions);
        }
        return wtf.close(expr);
    }

    // 12.1 Block

    std::vector<Node *> parseStatementList() {
        std::vector<Node *> list;
        Node *statement;

        while (index < length) {
            if (match("}")) {
                break;
            }
            statement = parseSourceElement();
            if (statement == NULL) {
                break;
            }
            list.push_back(statement);
        }

        return list;
    }

    BlockStatement *parseBlock() {
        WrapTrackingFunction wtf(*this);
        std::vector<Node *> block;

        expect("{");

        block = parseStatementList();

        expect("}");

        return wtf.close(delegate.createBlockStatement(block));
    }

    // 12.2 Variable Node

    Identifier *parseVariableIdentifier() {
        WrapTrackingFunction wtf(*this);
        EsprimaToken *token = lex();

        if (token->type != Token::Identifier) {
            throwUnexpected(token);
        }

        return wtf.close(delegate.createIdentifier(token->stringValue));
    }

    VariableDeclarator *parseVariableDeclaration(const std::string &kind) {
        WrapTrackingFunction wtf(*this);
        Identifier *id = parseVariableIdentifier();
        Node *init = NULL;

        // 12.2.1
        if (strict && isRestrictedWord(id->name)) {
            throwError(NULL, Messages::StrictVarName);
        }

        if (kind == "const") {
            expect("=");
            init = parseAssignmentExpression();
        } else if (match("=")) {
            lex();
            init = parseAssignmentExpression();
        }

        return wtf.close(delegate.createVariableDeclarator(id, init));
    }

    std::vector<VariableDeclarator *> parseVariableDeclarationList(const std::string &kind) {
        std::vector<VariableDeclarator *> list;

        do {
            list.push_back(parseVariableDeclaration(kind));
            if (!match(",")) {
                break;
            }
            lex();
        } while (index < length);

        return list;
    }

    VariableDeclaration *parseVariableStatement() {
        std::vector<VariableDeclarator *> declarations;

        expectKeyword("var");

        declarations = parseVariableDeclarationList("var");

        consumeSemicolon();

        return delegate.createVariableDeclaration(declarations, "var");
    }

    // kind may be `const` or `let`
    // Both are experimental and not in the specification yet.
    // see http://wiki.ecmascript.org/doku.php?id=harmony:const
    // and http://wiki.ecmascript.org/doku.php?id=harmony:let
    VariableDeclaration *parseConstLetDeclaration(const std::string &kind) {
        WrapTrackingFunction wtf(*this);
        std::vector<VariableDeclarator *> declarations;

        expectKeyword(kind);

        declarations = parseVariableDeclarationList(kind);

        consumeSemicolon();

        return wtf.close(delegate.createVariableDeclaration(declarations, kind));
    }

    // 12.3 Empty Node

    EmptyStatement *parseEmptyStatement() {
        expect(";");
        return delegate.createEmptyStatement();
    }

    // 12.4 Node Node

    ExpressionStatement *parseExpressionStatement() {
        Node *expr = parseExpression();
        consumeSemicolon();
        return delegate.createExpressionStatement(expr);
    }

    // 12.5 If statement

    IfStatement *parseIfStatement() {
        Node *test;
        Node *consequent, *alternate;

        expectKeyword("if");

        expect("(");

        test = parseExpression();

        expect(")");

        consequent = parseStatement();

        if (matchKeyword("else")) {
            lex();
            alternate = parseStatement();
        } else {
            alternate = NULL;
        }

        return delegate.createIfStatement(test, consequent, alternate);
    }

    // 12.6 Iteration Statements

    DoWhileStatement *parseDoWhileStatement() {
        Node *body;
        Node *test;
        bool oldInIteration;

        expectKeyword("do");

        oldInIteration = state.inIteration;
        state.inIteration = true;

        body = parseStatement();

        state.inIteration = oldInIteration;

        expectKeyword("while");

        expect("(");

        test = parseExpression();

        expect(")");

        if (match(";")) {
            lex();
        }

        return delegate.createDoWhileStatement(body, test);
    }

    WhileStatement *parseWhileStatement() {
        Node *test;
        Node *body;
        bool oldInIteration;

        expectKeyword("while");

        expect("(");

        test = parseExpression();

        expect(")");

        oldInIteration = state.inIteration;
        state.inIteration = true;

        body = parseStatement();

        state.inIteration = oldInIteration;

        return delegate.createWhileStatement(test, body);
    }

    VariableDeclaration *parseForVariableDeclaration() {
        WrapTrackingFunction wtf(*this);
        EsprimaToken *token = lex();
        std::vector<VariableDeclarator *> declarations = parseVariableDeclarationList("var");

        return wtf.close(delegate.createVariableDeclaration(declarations, token->stringValue));
    }

    Node *parseForStatement() {
        Node *init;
        Node *test, *update;
        Node *left;
        Node *right;
        Node *body;
        bool oldInIteration;
        bool leftIsUndefined = true;

        init = test = update = NULL;

        expectKeyword("for");

        expect("(");

        if (match(";")) {
            lex();
        } else {
            if (matchKeyword("var") || matchKeyword("let")) {
                state.allowIn = false;
                init = parseForVariableDeclaration();
                state.allowIn = true;

                if (init->as<VariableDeclaration>()->declarations.size() == 1 && matchKeyword("in")) {
                    lex();
                    left = init;
                    leftIsUndefined = false;
                    right = parseExpression();
                    init = NULL;
                }
            } else {
                state.allowIn = false;
                init = parseExpression();
                state.allowIn = true;

                if (matchKeyword("in")) {
                    // LeftHandSideExpression
                    if (!isLeftHandSide(init)) {
                        throwError(NULL, Messages::InvalidLHSInForIn);
                    }

                    lex();
                    left = init;
                    leftIsUndefined = false;
                    right = parseExpression();
                    init = NULL;
                }
            }

            if (leftIsUndefined) {
                expect(";");
            }
        }

        if (leftIsUndefined) {

            if (!match(";")) {
                test = parseExpression();
            }
            expect(";");

            if (!match(")")) {
                update = parseExpression();
            }
        }

        expect(")");

        oldInIteration = state.inIteration;
        state.inIteration = true;

        body = parseStatement();

        state.inIteration = oldInIteration;

        return (leftIsUndefined) ?
                static_cast<Node *>(delegate.createForStatement(init, test, update, body)) :
                static_cast<Node *>(delegate.createForInStatement(left, right, body));
    }

    // 12.7 The continue statement

    ContinueStatement *parseContinueStatement() {
        Identifier *label = NULL;

        expectKeyword("continue");

        // Optimize the most common form: 'continue;'.
        if (source[index] == 59) {
            lex();

            if (!state.inIteration) {
                throwError(NULL, Messages::IllegalContinue);
            }

            return delegate.createContinueStatement(NULL);
        }

        if (peekLineTerminator()) {
            if (!state.inIteration) {
                throwError(NULL, Messages::IllegalContinue);
            }

            return delegate.createContinueStatement(NULL);
        }

        if (lookahead->type == Token::Identifier) {
            label = parseVariableIdentifier();

            if (!state.labelSet.count(label->name)) {
                throwError(NULL, format(Messages::UnknownLabel, label->name.c_str()));
            }
        }

        consumeSemicolon();

        if (label == NULL && !state.inIteration) {
            throwError(NULL, Messages::IllegalContinue);
        }

        return delegate.createContinueStatement(label);
    }

    // 12.8 The break statement

    BreakStatement *parseBreakStatement() {
        Identifier *label = NULL;

        expectKeyword("break");

        // Catch the very common case first: immediately a semicolon (char #59).
        if (source[index] == 59) {
            lex();

            if (!(state.inIteration || state.inSwitch)) {
                throwError(NULL, Messages::IllegalBreak);
            }

            return delegate.createBreakStatement(NULL);
        }

        if (peekLineTerminator()) {
            if (!(state.inIteration || state.inSwitch)) {
                throwError(NULL, Messages::IllegalBreak);
            }

            return delegate.createBreakStatement(NULL);
        }

        if (lookahead->type == Token::Identifier) {
            label = parseVariableIdentifier();

            if (!state.labelSet.count(label->name)) {
                throwError(NULL, format(Messages::UnknownLabel, label->name.c_str()));
            }
        }

        consumeSemicolon();

        if (label == NULL && !(state.inIteration || state.inSwitch)) {
            throwError(NULL, Messages::IllegalBreak);
        }

        return delegate.createBreakStatement(label);
    }

    // 12.9 The return statement

    ReturnStatement *parseReturnStatement() {
        Node *argument = NULL;

        expectKeyword("return");

        if (!state.inFunctionBody) {
            throwError(NULL, Messages::IllegalReturn);
        }

        // 'return' followed by a space and an identifier is very common.
        if (source[index] == 32) {
            if (isIdentifierStart(source[index + 1])) {
                argument = parseExpression();
                consumeSemicolon();
                return delegate.createReturnStatement(argument);
            }
        }

        if (peekLineTerminator()) {
            return delegate.createReturnStatement(NULL);
        }

        if (!match(";")) {
            if (!match("}") && lookahead->type != Token::EOF) {
                argument = parseExpression();
            }
        }

        consumeSemicolon();

        return delegate.createReturnStatement(argument);
    }

    // 12.10 The with statement

    WithStatement *parseWithStatement() {
        Node *object;
        Node *body;

        if (strict) {
            throwError(NULL, Messages::StrictModeWith);
        }

        expectKeyword("with");

        expect("(");

        object = parseExpression();

        expect(")");

        body = parseStatement();

        return delegate.createWithStatement(object, body);
    }

    // 12.10 The swith statement

    SwitchCase *parseSwitchCase() {
        WrapTrackingFunction wtf(*this);
        Node *test;
        std::vector<Node *> consequent;
        Node *statement;

        if (matchKeyword("default")) {
            lex();
            test = NULL;
        } else {
            expectKeyword("case");
            test = parseExpression();
        }
        expect(":");

        while (index < length) {
            if (match("}") || matchKeyword("default") || matchKeyword("case")) {
                break;
            }
            statement = parseStatement();
            consequent.push_back(statement);
        }

        return wtf.close(delegate.createSwitchCase(test, consequent));
    }

    SwitchStatement *parseSwitchStatement() {
        Node *discriminant;
        std::vector<SwitchCase *> cases;
        SwitchCase *clause;
        bool oldInSwitch, defaultFound;

        expectKeyword("switch");

        expect("(");

        discriminant = parseExpression();

        expect(")");

        expect("{");

        if (match("}")) {
            lex();
            return delegate.createSwitchStatement(discriminant, cases);
        }

        oldInSwitch = state.inSwitch;
        state.inSwitch = true;
        defaultFound = false;

        while (index < length) {
            if (match("}")) {
                break;
            }
            clause = parseSwitchCase();
            if (clause->test == NULL) {
                if (defaultFound) {
                    throwError(NULL, Messages::MultipleDefaultsInSwitch);
                }
                defaultFound = true;
            }
            cases.push_back(clause);
        }

        state.inSwitch = oldInSwitch;

        expect("}");

        return delegate.createSwitchStatement(discriminant, cases);
    }

    // 12.13 The throw statement

    ThrowStatement *parseThrowStatement() {
        Node *argument;

        expectKeyword("throw");

        if (peekLineTerminator()) {
            throwError(NULL, Messages::NewlineAfterThrow);
        }

        argument = parseExpression();

        consumeSemicolon();

        return delegate.createThrowStatement(argument);
    }

    // 12.14 The try statement

    CatchClause *parseCatchClause() {
        WrapTrackingFunction wtf(*this);
        Node *param;
        BlockStatement *body;

        expectKeyword("catch");

        expect("(");
        if (match(")")) {
            throwUnexpected(lookahead);
        }

        param = parseExpression();
        // 12.14.1
        if (strict && param->is<Identifier>() && isRestrictedWord(param->as<Identifier>()->name)) {
            throwError(NULL, Messages::StrictCatchVariable);
        }

        expect(")");
        body = parseBlock();
        return wtf.close(delegate.createCatchClause(param, body));
    }

    TryStatement *parseTryStatement() {
        BlockStatement *block;
        CatchClause *handler = NULL;
        BlockStatement *finalizer = NULL;

        expectKeyword("try");

        block = parseBlock();

        if (matchKeyword("catch")) {
            handler = parseCatchClause();
        }

        if (matchKeyword("finally")) {
            lex();
            finalizer = parseBlock();
        }

        if (handler == NULL && !finalizer) {
            throwError(NULL, Messages::NoCatchOrFinally);
        }

        return delegate.createTryStatement(block, handler, finalizer);
    }

    // 12.15 The debugger statement

    DebuggerStatement *parseDebuggerStatement() {
        expectKeyword("debugger");

        consumeSemicolon();

        return delegate.createDebuggerStatement();
    }

    // 12 Statements

    Node *parseStatement() {
        WrapTrackingFunction wtf(*this);
        int type = lookahead->type;
        Node *expr;
        Node *labeledBody;
        std::string key;

        if (type == Token::EOF) {
            throwUnexpected(lookahead);
        }

        if (type == Token::Punctuator) {
            switch (lookahead->stringValue[0]) {
            case ';':
                return wtf.close(parseEmptyStatement());
            case '{':
                return wtf.close(parseBlock());
            case '(':
                return wtf.close(parseExpressionStatement());
            default:
                break;
            }
        }

        if (type == Token::Keyword) {
            const std::string &value = lookahead->stringValue;
            if (value == "break")
                return wtf.close(parseBreakStatement());
            if (value == "continue")
                return wtf.close(parseContinueStatement());
            if (value == "debugger")
                return wtf.close(parseDebuggerStatement());
            if (value == "do")
                return wtf.close(parseDoWhileStatement());
            if (value == "for")
                return wtf.close(parseForStatement());
            if (value == "function")
                return wtf.close(parseFunctionDeclaration());
            if (value == "if")
                return wtf.close(parseIfStatement());
            if (value == "return")
                return wtf.close(parseReturnStatement());
            if (value == "switch")
                return wtf.close(parseSwitchStatement());
            if (value == "throw")
                return wtf.close(parseThrowStatement());
            if (value == "try")
                return wtf.close(parseTryStatement());
            if (value == "var")
                return wtf.close(parseVariableStatement());
            if (value == "while")
                return wtf.close(parseWhileStatement());
            if (value == "with")
                return wtf.close(parseWithStatement());
        }

        expr = parseExpression();

        // 12.12 Labelled Statements
        if (expr->is<Identifier>() && match(":")) {
            lex();

            key = expr->as<Identifier>()->name;
            if (state.labelSet.count(key)) {
                throwError(NULL, format(Messages::Redeclaration, "Label", key.c_str()));
            }

            state.labelSet.insert(key);
            labeledBody = parseStatement();
            state.labelSet.erase(key);
            return wtf.close(delegate.createLabeledStatement(expr->as<Identifier>(), labeledBody));
        }

        consumeSemicolon();

        return wtf.close(delegate.createExpressionStatement(expr));
    }

    // 13 Function Definition

    BlockStatement *parseFunctionSourceElements() {
        WrapTrackingFunction wtf(*this);
        Node *sourceElement;
        std::vector<Node *> sourceElements;
        EsprimaToken *token;
        std::string directive;
        EsprimaToken *firstRestricted;
        std::set<std::string> oldLabelSet;
        bool oldInIteration, oldInSwitch, oldInFunctionBody;

        expect("{");

        while (index < length) {
            if (lookahead->type != Token::StringLiteral) {
                break;
            }
            token = lookahead;

            sourceElement = parseSourceElement();
            sourceElements.push_back(sourceElement);
            if (!sourceElement->as<ExpressionStatement>()->expression->is<StringLiteral>()) {
                // this is not directive
                break;
            }
            directive = source.substr(token->range[0] + 1, token->range[1] - 1 - token->range[0]);
            if (directive == "use strict") {
                strict = true;
                if (firstRestricted) {
                    throwError(firstRestricted, Messages::StrictOctalLiteral);
                }
            } else {
                if (!firstRestricted && token->octal) {
                    firstRestricted = token;
                }
            }
        }

        oldLabelSet = state.labelSet;
        oldInIteration = state.inIteration;
        oldInSwitch = state.inSwitch;
        oldInFunctionBody = state.inFunctionBody;

        state.labelSet = std::set<std::string>();
        state.inIteration = false;
        state.inSwitch = false;
        state.inFunctionBody = true;

        while (index < length) {
            if (match("}")) {
                break;
            }
            sourceElement = parseSourceElement();
            if (sourceElement == NULL) {
                break;
            }
            sourceElements.push_back(sourceElement);
        }

        expect("}");

        state.labelSet = oldLabelSet;
        state.inIteration = oldInIteration;
        state.inSwitch = oldInSwitch;
        state.inFunctionBody = oldInFunctionBody;

        return wtf.close(delegate.createBlockStatement(sourceElements));
    }

    struct ParseParams : Poolable {
        std::vector<Identifier *> params;
        EsprimaToken *stricted;
        EsprimaToken *firstRestricted;
        std::string message;
        ParseParams(Pool& pool) : Poolable(pool){}
    };

    ParseParams *parseParams(EsprimaToken *firstRestricted) {
        Identifier *param;
        std::vector<Identifier *> params;
        EsprimaToken *token;
        EsprimaToken *stricted;
        std::set<std::string> paramSet;
        std::string message;
        expect("(");

        if (!match(")")) {
            paramSet = std::set<std::string>();
            while (index < length) {
                token = lookahead;
                param = parseVariableIdentifier();
                if (strict) {
                    if (isRestrictedWord(token->stringValue)) {
                        stricted = token;
                        message = Messages::StrictParamName;
                    }
                    if (paramSet.count(token->stringValue)) {
                        stricted = token;
                        message = Messages::StrictParamDupe;
                    }
                } else if (!firstRestricted) {
                    if (isRestrictedWord(token->stringValue)) {
                        firstRestricted = token;
                        message = Messages::StrictParamName;
                    } else if (isStrictModeReservedWord(token->stringValue)) {
                        firstRestricted = token;
                        message = Messages::StrictReservedWord;
                    } else if (paramSet.count(token->stringValue)) {
                        firstRestricted = token;
                        message = Messages::StrictParamDupe;
                    }
                }
                params.push_back(param);
                paramSet.insert(token->stringValue);
                if (match(")")) {
                    break;
                }
                expect(",");
            }
        }

        expect(")");

        ParseParams *result = new ParseParams(pool);
        result->params = params;
        result->stricted = stricted;
        result->firstRestricted = firstRestricted;
        result->message = message;
        return result;
    }

    FunctionDeclaration *parseFunctionDeclaration() {
        WrapTrackingFunction wtf(*this);
        Identifier *id;
        std::vector<Identifier *>params;
        BlockStatement *body;
        EsprimaToken *token, *stricted;
        ParseParams *tmp;
        EsprimaToken *firstRestricted;
        std::string message;
        bool previousStrict;

        expectKeyword("function");
        token = lookahead;
        id = parseVariableIdentifier();
        if (strict) {
            if (isRestrictedWord(token->stringValue)) {
                throwError(token, Messages::StrictFunctionName);
            }
        } else {
            if (isRestrictedWord(token->stringValue)) {
                firstRestricted = token;
                message = Messages::StrictFunctionName;
            } else if (isStrictModeReservedWord(token->stringValue)) {
                firstRestricted = token;
                message = Messages::StrictReservedWord;
            }
        }

        tmp = parseParams(firstRestricted);
        params = tmp->params;
        stricted = tmp->stricted;
        firstRestricted = tmp->firstRestricted;
        if (!tmp->message.empty()) {
            message = tmp->message;
        }

        previousStrict = strict;
        body = parseFunctionSourceElements();
        if (strict && firstRestricted) {
            throwError(firstRestricted, message);
        }
        if (strict && stricted) {
            throwError(stricted, message);
        }
        strict = previousStrict;

        return wtf.close(delegate.createFunctionDeclaration(id, params, body));
    }

    FunctionExpression *parseFunctionExpression() {
        WrapTrackingFunction wtf(*this);
        EsprimaToken *token;
        Identifier *id = NULL;
        EsprimaToken *stricted, *firstRestricted;
        std::string message;
        ParseParams *tmp;
        std::vector<Identifier *> params;
        BlockStatement *body;
        bool previousStrict;

        expectKeyword("function");

        if (!match("(")) {
            token = lookahead;
            id = parseVariableIdentifier();
            if (strict) {
                if (isRestrictedWord(token->stringValue)) {
                    throwError(token, Messages::StrictFunctionName);
                }
            } else {
                if (isRestrictedWord(token->stringValue)) {
                    firstRestricted = token;
                    message = Messages::StrictFunctionName;
                } else if (isStrictModeReservedWord(token->stringValue)) {
                    firstRestricted = token;
                    message = Messages::StrictReservedWord;
                }
            }
        }

        tmp = parseParams(firstRestricted);
        params = tmp->params;
        stricted = tmp->stricted;
        firstRestricted = tmp->firstRestricted;
        if (!tmp->message.empty()) {
            message = tmp->message;
        }

        previousStrict = strict;
        body = parseFunctionSourceElements();
        if (strict && firstRestricted) {
            throwError(firstRestricted, message);
        }
        if (strict && stricted) {
            throwError(stricted, message);
        }
        strict = previousStrict;

        return wtf.close(delegate.createFunctionExpression(id, params, body));
    }

    // 14 Program

    Node *parseSourceElement() {
        if (lookahead->type == Token::Keyword) {
            if (lookahead->stringValue == "const" || lookahead->stringValue == "let")
                return parseConstLetDeclaration(lookahead->stringValue);
            else if (lookahead->stringValue == "function")
                return parseFunctionDeclaration();
            else
                return parseStatement();
        }

        if (lookahead->type != Token::EOF) {
            return parseStatement();
        }
        return NULL;
    }

    std::vector<Node *> parseSourceElements() {
        Node *sourceElement;
        std::vector<Node *> sourceElements;
        EsprimaToken *token;
        std::string directive;
        EsprimaToken *firstRestricted;

        while (index < length) {
            token = lookahead;
            if (token->type != Token::StringLiteral) {
                break;
            }

            sourceElement = parseSourceElement();
            sourceElements.push_back(sourceElement);
            if (sourceElement->as<ExpressionStatement>()->expression->is<StringLiteral>()) {
                // this is not directive
                break;
            }
            directive = source.substr(token->range[0] + 1, token->range[1] - 1 - token->range[0]);
            if (directive == "use strict") {
                strict = true;
                if (firstRestricted) {
                    throwError(firstRestricted, Messages::StrictOctalLiteral);
                }
            } else {
                if (!firstRestricted && token->octal) {
                    firstRestricted = token;
                }
            }
        }

        while (index < length) {
            sourceElement = parseSourceElement();
            if (sourceElement == NULL) {
                break;
            }
            sourceElements.push_back(sourceElement);
        }
        return sourceElements;
    }

    Program *parseProgram() {
        WrapTrackingFunction wtf(*this);
        std::vector<Node *> body;
        strict = false;
        peek();
        body = parseSourceElements();
        return wtf.close(delegate.createProgram(body));
    }

    struct EsprimaMarker : Poolable{
        EsprimaParser &parser;
        int range[2];
        SourceLocation *loc;
        EsprimaMarker(Pool& pool, EsprimaParser &parser) : Poolable(pool), parser(parser), loc() { range[0] = range[1] = 0; }

        void end() {
            range[1] = parser.index;
            loc->end->line = parser.lineNumber;
            loc->end->column = parser.index - parser.lineStart;
        }

        void applyGroup(Node *node) {
            node->groupRange[0] = range[0]; node->groupRange[1] = range[1];
            node->groupLoc = loc;
            parser.delegate.postProcess(node);
        }

        void apply(Node *node) {
            node->range[0] = range[0]; node->range[1] = range[1];
            node->loc = loc;
            parser.delegate.postProcess(node);
        }
    };

    EsprimaMarker *createLocationMarker() {
        EsprimaMarker *marker = new EsprimaMarker(pool, *this);

        marker->loc = new SourceLocation(pool);
        marker->loc->start = new Position(pool);
        marker->loc->start->line = lineNumber;
        marker->loc->start->column = index - lineStart;
        marker->loc->end = new Position(pool);
        marker->loc->end->line = lineNumber;
        marker->loc->end->column = index - lineStart;

        return marker;
    }

    Node *trackGroupExpression() {
        EsprimaMarker *marker;
        Node *expr;

        skipComment();
        marker = createLocationMarker();
        expect("(");

        expr = parseExpression();

        expect(")");

        marker->end();
        marker->applyGroup(expr);

        return expr;
    }

    Node *trackLeftHandSideExpression() {
        EsprimaMarker *marker;
        Node *expr, *property;

        skipComment();
        marker = createLocationMarker();

        expr = matchKeyword("new") ? parseNewExpression() : parsePrimaryExpression();

        while (match(".") || match("[")) {
            if (match("[")) {
                property = parseComputedMember();
                expr = delegate.createMemberExpression('[', expr, property);
                marker->end();
                marker->apply(expr);
            } else {
                property = parseNonComputedMember();
                expr = delegate.createMemberExpression('.', expr, property);
                marker->end();
                marker->apply(expr);
            }
        }

        return expr;
    }

    Node *trackLeftHandSideExpressionAllowCall() {
        EsprimaMarker *marker;
        Node *expr;
        std::vector<Node *> args;
        Node *property;

        skipComment();
        marker = createLocationMarker();

        expr = matchKeyword("new") ? parseNewExpression() : parsePrimaryExpression();

        while (match(".") || match("[") || match("(")) {
            if (match("(")) {
                args = parseArguments();
                expr = delegate.createCallExpression(expr, args);
                marker->end();
                marker->apply(expr);
            } else if (match("[")) {
                property = parseComputedMember();
                expr = delegate.createMemberExpression('[', expr, property);
                marker->end();
                marker->apply(expr);
            } else {
                property = parseNonComputedMember();
                expr = delegate.createMemberExpression('.', expr, property);
                marker->end();
                marker->apply(expr);
            }
        }

        return expr;
    }

    void visitBinary(Node *node) {
        if (node->is<LogicalExpression>()) {
            LogicalExpression *expr = node->as<LogicalExpression>();
            visit(expr, expr->left, expr->right);
        }
        else if (node->is<BinaryExpression>()) {
            BinaryExpression *expr = node->as<BinaryExpression>();
            visit(expr, expr->left, expr->right);
        }
    }

    void visit(Node *node, Node *left, Node *right) {
        visitBinary(left);
        visitBinary(right);

        if (left->groupRange || right->groupRange) {
            int start = left->groupRange ? left->groupRange[0] : left->range[0];
            int end = right->groupRange ? right->groupRange[1] : right->range[1];
            node->range[0] = start; node->range[1] = end;
        } else if (!node->range[1]) {
            int start = left->range[0];
            int end = right->range[1];
            node->range[0] = start; node->range[1] = end;
        }

        if (left->groupLoc || right->groupLoc) {
            assert(left->groupLoc || left->loc);
            assert(right->groupLoc || right->loc);
            Position *start = left->groupLoc ? left->groupLoc->start : left->loc->start;
            Position *end = right->groupLoc ? right->groupLoc->end : right->loc->end;
            node->loc = new SourceLocation(pool);
            node->loc->start = start;
            node->loc->end = end;
            delegate.postProcess(node);
        } else if (node->loc == NULL) {
            assert(left->loc);
            assert(right->loc);
            node->loc = new SourceLocation(pool);
            node->loc->start = left->loc->start;
            node->loc->end = right->loc->end;
            delegate.postProcess(node);
        }
    }

    struct WrapTrackingFunction {
        EsprimaParser &parser;
        EsprimaMarker *marker;

        WrapTrackingFunction(EsprimaParser &parser) : parser(parser), marker() {
            parser.skipComment();
            marker = parser.createLocationMarker();
        }

        ~WrapTrackingFunction() {
            //delete marker;
        }

        template <typename T>
        T *close(T *node) {

            if (node->loc == NULL) {
                marker->apply(node);
            }

            marker->end();

            parser.visitBinary(node);

            return node;
        }
    };

    Program *parse(const std::string &code) {
        source = code;
        index = 0;
        lineNumber = (source.size() > 0) ? 1 : 0;
        lineStart = 0;
        length = source.size();
        lookahead = NULL;
        state = State();

        return parseProgram();
    }

    struct State {
        bool allowIn;
        std::set<std::string> labelSet;
        bool inFunctionBody;
        bool inIteration;
        bool inSwitch;
        State() : allowIn(true), inFunctionBody(), inIteration(), inSwitch() {}
    };

    Pool &pool;
    std::string source;
    bool strict;
    int index;
    int lineNumber;
    int lineStart;
    int length;
    SyntaxTreeDelegate delegate;
    EsprimaToken *lookahead;
    State state;

    EsprimaParser(Pool &pool) : pool(pool), strict(), index(), lineNumber(), lineStart(), length(), delegate(*this), lookahead() {}
};

Program *esprima::parse(Pool &pool, const std::string &code) {
    return EsprimaParser(pool).parse(code);
}

void visit(Visitor *visitor, Node *node) {
    if (node) node->accept(visitor);
}

template <typename T>
void visit(Visitor *visitor, const std::vector<T *> &nodes) {
    for (size_t i = 0; i < nodes.size(); i++) {
        visit(visitor, nodes[i]);
    }
}

void Visitor::visitChildren(Program *node) {
    ::visit(this, node->body);
}

void Visitor::visitChildren(Identifier *) {
}

void Visitor::visitChildren(BlockStatement *node) {
    ::visit(this, node->body);
}

void Visitor::visitChildren(EmptyStatement *) {
}

void Visitor::visitChildren(ExpressionStatement *node) {
    ::visit(this, node->expression);
}

void Visitor::visitChildren(IfStatement *node) {
    ::visit(this, node->test);
    ::visit(this, node->consequent);
    ::visit(this, node->alternate);
}

void Visitor::visitChildren(LabeledStatement *node) {
    ::visit(this, node->label);
    ::visit(this, node->body);
}

void Visitor::visitChildren(BreakStatement *node) {
    ::visit(this, node->label);
}

void Visitor::visitChildren(ContinueStatement *node) {
    ::visit(this, node->label);
}

void Visitor::visitChildren(WithStatement *node) {
    ::visit(this, node->object);
    ::visit(this, node->body);
}

void Visitor::visitChildren(SwitchCase *node) {
    ::visit(this, node->test);
    ::visit(this, node->consequent);
}

void Visitor::visitChildren(SwitchStatement *node) {
    ::visit(this, node->discriminant);
    ::visit(this, node->cases);
}

void Visitor::visitChildren(ReturnStatement *node) {
    ::visit(this, node->argument);
}

void Visitor::visitChildren(ThrowStatement *node) {
    ::visit(this, node->argument);
}

void Visitor::visitChildren(CatchClause *node) {
    ::visit(this, node->param);
    ::visit(this, node->body);
}

void Visitor::visitChildren(TryStatement *node) {
    ::visit(this, node->block);
    ::visit(this, node->handler);
    ::visit(this, node->finalizer);
}

void Visitor::visitChildren(WhileStatement *node) {
    ::visit(this, node->test);
    ::visit(this, node->body);
}

void Visitor::visitChildren(DoWhileStatement *node) {
    ::visit(this, node->body);
    ::visit(this, node->test);
}

void Visitor::visitChildren(ForStatement *node) {
    ::visit(this, node->init);
    ::visit(this, node->test);
    ::visit(this, node->update);
    ::visit(this, node->body);
}

void Visitor::visitChildren(ForInStatement *node) {
    ::visit(this, node->left);
    ::visit(this, node->right);
    ::visit(this, node->body);
}

void Visitor::visitChildren(DebuggerStatement *) {
}

void Visitor::visitChildren(FunctionDeclaration *node) {
    ::visit(this, node->id);
    ::visit(this, node->params);
    ::visit(this, node->body);
}

void Visitor::visitChildren(VariableDeclarator *node) {
    ::visit(this, node->id);
    ::visit(this, node->init);
}

void Visitor::visitChildren(VariableDeclaration *node) {
    ::visit(this, node->declarations);
}

void Visitor::visitChildren(ThisExpression *) {
}

void Visitor::visitChildren(ArrayExpression *node) {
    ::visit(this, node->elements);
}

void Visitor::visitChildren(Property *node) {
    ::visit(this, node->key);
    ::visit(this, node->value);
}

void Visitor::visitChildren(ObjectExpression *node) {
    ::visit(this, node->properties);
}

void Visitor::visitChildren(FunctionExpression *node) {
    ::visit(this, node->id);
    ::visit(this, node->params);
    ::visit(this, node->body);
}

void Visitor::visitChildren(SequenceExpression *node) {
    ::visit(this, node->expressions);
}

void Visitor::visitChildren(UnaryExpression *node) {
    ::visit(this, node->argument);
}

void Visitor::visitChildren(BinaryExpression *node) {
    ::visit(this, node->left);
    ::visit(this, node->right);
}

void Visitor::visitChildren(AssignmentExpression *node) {
    ::visit(this, node->left);
    ::visit(this, node->right);
}

void Visitor::visitChildren(UpdateExpression *node) {
    ::visit(this, node->argument);
}

void Visitor::visitChildren(LogicalExpression *node) {
    ::visit(this, node->left);
    ::visit(this, node->right);
}

void Visitor::visitChildren(ConditionalExpression *node) {
    ::visit(this, node->test);
    ::visit(this, node->consequent);
    ::visit(this, node->alternate);
}

void Visitor::visitChildren(NewExpression *node) {
    ::visit(this, node->callee);
    ::visit(this, node->arguments);
}

void Visitor::visitChildren(CallExpression *node) {
    ::visit(this, node->callee);
    ::visit(this, node->arguments);
}

void Visitor::visitChildren(MemberExpression *node) {
    ::visit(this, node->object);
    ::visit(this, node->property);
}

void Visitor::visitChildren(NullLiteral *) {
}

void Visitor::visitChildren(RegExpLiteral *) {
}

void Visitor::visitChildren(StringLiteral *) {
}

void Visitor::visitChildren(NumericLiteral *) {
}

void Visitor::visitChildren(BooleanLiteral *) {
}
