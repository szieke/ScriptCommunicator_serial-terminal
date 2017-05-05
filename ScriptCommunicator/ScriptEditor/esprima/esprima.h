#ifndef __ESPRIMA_H__
#define __ESPRIMA_H__

#include <assert.h>
#include <string>
#include <vector>

// API from https://developer.mozilla.org/en-US/docs/SpiderMonkey/Parser_API

namespace esprima {

    struct Pool;

    struct Poolable {
        Poolable *poolable;
        Poolable(Pool &pool);
        virtual ~Poolable() {}
    };

    struct Pool {
        Poolable *first;
        Pool() : first() {}
        ~Pool() {
            while (first) {
                Poolable *poolable = first;
                first = first->poolable;
                delete poolable;
            }
        }
    };

    struct Position : Poolable {
        int line;
        int column;
        Position(Pool &pool) : Poolable(pool), line(), column() {}
    };

    struct SourceLocation : Poolable{
        Position *start;
        Position *end;
        SourceLocation(Pool &pool) : Poolable(pool), start(), end() {}
    };

    struct Program;
    struct Identifier;
    struct BlockStatement;
    struct EmptyStatement;
    struct ExpressionStatement;
    struct IfStatement;
    struct LabeledStatement;
    struct BreakStatement;
    struct ContinueStatement;
    struct WithStatement;
    struct SwitchCase;
    struct SwitchStatement;
    struct ReturnStatement;
    struct ThrowStatement;
    struct CatchClause;
    struct TryStatement;
    struct WhileStatement;
    struct DoWhileStatement;
    struct ForStatement;
    struct ForInStatement;
    struct DebuggerStatement;
    struct FunctionDeclaration;
    struct VariableDeclarator;
    struct VariableDeclaration;
    struct ThisExpression;
    struct ArrayExpression;
    struct Property;
    struct ObjectExpression;
    struct FunctionExpression;
    struct SequenceExpression;
    struct UnaryExpression;
    struct BinaryExpression;
    struct AssignmentExpression;
    struct UpdateExpression;
    struct LogicalExpression;
    struct ConditionalExpression;
    struct NewExpression;
    struct CallExpression;
    struct MemberExpression;
    struct NullLiteral;
    struct RegExpLiteral;
    struct StringLiteral;
    struct NumericLiteral;
    struct BooleanLiteral;

    struct Visitor {
        virtual void visit(Program *node) = 0;
        virtual void visit(Identifier *node) = 0;
        virtual void visit(BlockStatement *node) = 0;
        virtual void visit(EmptyStatement *node) = 0;
        virtual void visit(ExpressionStatement *node) = 0;
        virtual void visit(IfStatement *node) = 0;
        virtual void visit(LabeledStatement *node) = 0;
        virtual void visit(BreakStatement *node) = 0;
        virtual void visit(ContinueStatement *node) = 0;
        virtual void visit(WithStatement *node) = 0;
        virtual void visit(SwitchCase *node) = 0;
        virtual void visit(SwitchStatement *node) = 0;
        virtual void visit(ReturnStatement *node) = 0;
        virtual void visit(ThrowStatement *node) = 0;
        virtual void visit(CatchClause *node) = 0;
        virtual void visit(TryStatement *node) = 0;
        virtual void visit(WhileStatement *node) = 0;
        virtual void visit(DoWhileStatement *node) = 0;
        virtual void visit(ForStatement *node) = 0;
        virtual void visit(ForInStatement *node) = 0;
        virtual void visit(DebuggerStatement *node) = 0;
        virtual void visit(FunctionDeclaration *node) = 0;
        virtual void visit(VariableDeclarator *node) = 0;
        virtual void visit(VariableDeclaration *node) = 0;
        virtual void visit(ThisExpression *node) = 0;
        virtual void visit(ArrayExpression *node) = 0;
        virtual void visit(Property *node) = 0;
        virtual void visit(ObjectExpression *node) = 0;
        virtual void visit(FunctionExpression *node) = 0;
        virtual void visit(SequenceExpression *node) = 0;
        virtual void visit(UnaryExpression *node) = 0;
        virtual void visit(BinaryExpression *node) = 0;
        virtual void visit(AssignmentExpression *node) = 0;
        virtual void visit(UpdateExpression *node) = 0;
        virtual void visit(LogicalExpression *node) = 0;
        virtual void visit(ConditionalExpression *node) = 0;
        virtual void visit(NewExpression *node) = 0;
        virtual void visit(CallExpression *node) = 0;
        virtual void visit(MemberExpression *node) = 0;
        virtual void visit(NullLiteral *node) = 0;
        virtual void visit(RegExpLiteral *node) = 0;
        virtual void visit(StringLiteral *node) = 0;
        virtual void visit(NumericLiteral *node) = 0;
        virtual void visit(BooleanLiteral *node) = 0;

        void visitChildren(Program *node);
        void visitChildren(Identifier *node);
        void visitChildren(BlockStatement *node);
        void visitChildren(EmptyStatement *node);
        void visitChildren(ExpressionStatement *node);
        void visitChildren(IfStatement *node);
        void visitChildren(LabeledStatement *node);
        void visitChildren(BreakStatement *node);
        void visitChildren(ContinueStatement *node);
        void visitChildren(WithStatement *node);
        void visitChildren(SwitchCase *node);
        void visitChildren(SwitchStatement *node);
        void visitChildren(ReturnStatement *node);
        void visitChildren(ThrowStatement *node);
        void visitChildren(CatchClause *node);
        void visitChildren(TryStatement *node);
        void visitChildren(WhileStatement *node);
        void visitChildren(DoWhileStatement *node);
        void visitChildren(ForStatement *node);
        void visitChildren(ForInStatement *node);
        void visitChildren(DebuggerStatement *node);
        void visitChildren(FunctionDeclaration *node);
        void visitChildren(VariableDeclarator *node);
        void visitChildren(VariableDeclaration *node);
        void visitChildren(ThisExpression *node);
        void visitChildren(ArrayExpression *node);
        void visitChildren(Property *node);
        void visitChildren(ObjectExpression *node);
        void visitChildren(FunctionExpression *node);
        void visitChildren(SequenceExpression *node);
        void visitChildren(UnaryExpression *node);
        void visitChildren(BinaryExpression *node);
        void visitChildren(AssignmentExpression *node);
        void visitChildren(UpdateExpression *node);
        void visitChildren(LogicalExpression *node);
        void visitChildren(ConditionalExpression *node);
        void visitChildren(NewExpression *node);
        void visitChildren(CallExpression *node);
        void visitChildren(MemberExpression *node);
        void visitChildren(NullLiteral *node);
        void visitChildren(RegExpLiteral *node);
        void visitChildren(StringLiteral *node);
        void visitChildren(NumericLiteral *node);
        void visitChildren(BooleanLiteral *node);
    };

    struct Node : Poolable {
        SourceLocation *loc;
        SourceLocation *groupLoc;
        int range[2];
        int groupRange[2];
        Node(Pool &pool) : Poolable(pool), loc(), groupLoc(NULL) { range[0] = range[1] = groupRange[0] = groupRange[1] = 0; }
        virtual ~Node() {}
        virtual void accept(Visitor *visitor) = 0;
        template <class T> bool is() {
            return dynamic_cast<T *>(this) != NULL;
        }
        template <class T> T *as() {
            T *t = dynamic_cast<T *>(this);
            assert(t != NULL);
            return t;
        }
    };


    struct Program : Node {
        std::vector<Node *> body;
        Program(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct Identifier : Node {
        std::string name;
        Identifier(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct BlockStatement : Node {
        std::vector<Node *> body;
        BlockStatement(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    // This is a mixin, not a base class, and so isn't poolable
    struct Function {
        Identifier *id;
        std::vector<Identifier *> params;
        BlockStatement *body;
        Function() : id(), body() {}
    };

    struct EmptyStatement : Node {
        EmptyStatement(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ExpressionStatement : Node {
        Node *expression;
        ExpressionStatement(Pool &pool) : Node(pool), expression() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct IfStatement : Node {
        Node *test;
        Node *consequent;
        Node *alternate;
        IfStatement(Pool &pool) : Node(pool), test(), consequent(), alternate() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct LabeledStatement : Node {
        Identifier *label;
        Node *body;
        LabeledStatement(Pool &pool) : Node(pool), label(), body() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct BreakStatement : Node {
        Identifier *label;
        BreakStatement(Pool &pool) : Node(pool), label() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ContinueStatement : Node {
        Identifier *label;
        ContinueStatement(Pool &pool) : Node(pool), label() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct WithStatement : Node {
        Node *object;
        Node *body;
        WithStatement(Pool &pool) : Node(pool), object(), body() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct SwitchCase : Node {
        Node *test;
        std::vector<Node *> consequent;
        SwitchCase(Pool &pool) : Node(pool), test() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct SwitchStatement : Node {
        Node *discriminant;
        std::vector<SwitchCase *> cases;
        SwitchStatement(Pool &pool) : Node(pool), discriminant() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ReturnStatement : Node {
        Node *argument;
        ReturnStatement(Pool &pool) : Node(pool), argument() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ThrowStatement : Node {
        Node *argument;
        ThrowStatement(Pool &pool) : Node(pool), argument() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct CatchClause : Node {
        Node *param;
        BlockStatement *body;
        CatchClause(Pool &pool) : Node(pool), param(), body() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct TryStatement : Node {
        BlockStatement *block;
        CatchClause *handler;
        BlockStatement *finalizer;
        TryStatement(Pool &pool) : Node(pool), block(), handler(), finalizer() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct WhileStatement : Node {
        Node *test;
        Node *body;
        WhileStatement(Pool &pool) : Node(pool), test(), body() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct DoWhileStatement : Node {
        Node *body;
        Node *test;
        DoWhileStatement(Pool &pool) : Node(pool), body(), test() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ForStatement : Node {
        Node *init; // Either a VariableDeclaration or an Node
        Node *test;
        Node *update;
        Node *body;
        ForStatement(Pool &pool) : Node(pool), init(), test(), update(), body() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ForInStatement : Node {
        Node *left; // Either a VariableDeclaration or an Node
        Node *right;
        Node *body;
        ForInStatement(Pool &pool) : Node(pool), left(), right(), body() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct DebuggerStatement : Node {
        DebuggerStatement(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct FunctionDeclaration : Node, Function {
        FunctionDeclaration(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct VariableDeclarator : Node {
        Identifier *id;
        Node *init;
        VariableDeclarator(Pool &pool) : Node(pool), id(), init() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct VariableDeclaration : Node {
        std::vector<VariableDeclarator *> declarations;
        std::string kind;
        VariableDeclaration(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ThisExpression : Node {
        ThisExpression(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ArrayExpression : Node {
        std::vector<Node *> elements;
        ArrayExpression(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct Property : Node {
        std::string kind;
        Node *key; // Either a Literal or an Identifier
        Node *value;
        Property(Pool &pool) : Node(pool), key(), value() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ObjectExpression : Node {
        std::vector<Property *> properties;
        ObjectExpression(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct FunctionExpression : Node, Function {
        FunctionExpression(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct SequenceExpression : Node {
        std::vector<Node *> expressions;
        SequenceExpression(Pool &pool) : Node(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct UnaryExpression : Node {
        std::string operator_;
        bool prefix;
        Node *argument;
        UnaryExpression(Pool &pool) : Node(pool), prefix(), argument() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct BinaryExpression : Node {
        std::string operator_;
        Node *left;
        Node *right;
        BinaryExpression(Pool &pool) : Node(pool), left(), right() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct AssignmentExpression : Node {
        std::string operator_;
        Node *left;
        Node *right;
        AssignmentExpression(Pool &pool) : Node(pool), left(), right() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct UpdateExpression : Node {
        std::string operator_;
        Node *argument;
        bool prefix;
        UpdateExpression(Pool &pool) : Node(pool), argument(), prefix() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct LogicalExpression : Node {
        std::string operator_;
        Node *left;
        Node *right;
        LogicalExpression(Pool &pool) : Node(pool), left(), right() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ConditionalExpression : Node {
        Node *test;
        Node *alternate;
        Node *consequent;
        ConditionalExpression(Pool &pool) : Node(pool), test(), alternate(), consequent() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct NewExpression : Node {
        Node *callee;
        std::vector<Node *> arguments;
        NewExpression(Pool &pool) : Node(pool), callee() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct CallExpression : Node {
        Node *callee;
        std::vector<Node *> arguments;
        CallExpression(Pool &pool) : Node(pool), callee() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct MemberExpression : Node {
        Node *object;
        Node *property; // Identifier if computed == false
        bool computed;
        MemberExpression(Pool &pool) : Node(pool), object(), property(), computed() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct Literal : Node {
        Literal(Pool &pool) : Node(pool) {}
    };

    struct NullLiteral : Literal {
        NullLiteral(Pool &pool) : Literal(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct RegExpLiteral : Literal {
        std::string pattern;
        std::string flags;
        RegExpLiteral(Pool &pool) : Literal(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct StringLiteral : Literal {
        std::string value;
        StringLiteral(Pool &pool) : Literal(pool) {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct NumericLiteral : Literal {
        double value;
        NumericLiteral(Pool &pool) : Literal(pool), value() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct BooleanLiteral : Literal {
        bool value;
        BooleanLiteral(Pool &pool) : Literal(pool), value() {}
        void accept(Visitor *visitor) { visitor->visit(this); }
    };

    struct ParseError {
        std::string description;
        int index;
        int lineNumber;
        int column;
    };

    Program *parse(Pool &pool, const std::string &code);
}

#endif
