#ifndef ESPRIMAPARSEFUNCTIONS
#define ESPRIMAPARSEFUNCTIONS

#include <QMap>
#include <QVector>
#include "esprima/esprima.h"

//The type of an parsed entry.
typedef enum
{
    PARSED_ENTRY_TYPE_VAR = 0,//Variable
    PARSED_ENTRY_TYPE_CONST,//Const value
    PARSED_ENTRY_TYPE_FUNCTION,//Function
    PARSED_ENTRY_TYPE_CLASS,//Class
    PARSED_ENTRY_TYPE_CLASS_FUNCTION,//Class function
    PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION,//Class this function
    PARSED_ENTRY_TYPE_CLASS_VAR,//Class variable
    PARSED_ENTRY_TYPE_MAP,//Map
    PARSED_ENTRY_TYPE_MAP_VAR,//Map variable
    PARSED_ENTRY_TYPE_MAP_FUNC,//Map function
    PARSED_ENTRY_TYPE_PARSE_ERROR,//Parse error
    PARSED_ENTRY_TYPE_PROTOTYPE_FUNC,//Prototype function.
    PARSED_ENTRY_TYPE_FILE//File

}ParsedEntryType;

///A parsed entry.
typedef struct ParsedEntry ParsedEntry;
struct ParsedEntry
{
    int line;//The line number.
    int endLine;//The end line.
    int column;//The column.
    QString name;//The entry name.
    QString completeName;//The complete name (inkludes the parents).
    bool findWithCase;//False if for this entry the case is ignored during search.
    bool findWholeWord;//True if for this entry the whole complete name shall be searched during search.
    ParsedEntryType type;//The type of this entry.
    QStringList params;//The parameters.
    int tabIndex;//The tab (dodument) to which this entry belongs to.
    QVector<ParsedEntry> subElements;//The sub entries.
    QString valueType;//The value type of the element (variable).
    bool isObjectArrayIndex;//True if the value is from an object array index.
    bool isFunctionArrayIndex;//True if the value is from an function array index.
    QStringList additionalInformation;//Additional information.
};


/********************************************Function declarations***************************************************/

/**
 * Parses an (esprima) object expression.
 *
 * @param objExp
 *      The object expression.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the expression belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaObjectExpression(esprima::ObjectExpression* objExp, ParsedEntry* parent, int tabIndex,
                                         QMap<QString, ParsedEntry>* objects = NULL);

/**
 * Reads the type from a node.
 *
 * @param node
 *      The node.
 * @param entry
 *      The parsed entry to which the type is written to.
 * @return
 *      True if a type was found.
 */
static bool getTypeFromNode(esprima::Node* node, ParsedEntry& subEntry);


/**
 * Parses an (esprima) function declaration.
 *
 * @param function
 *      The function declaration.
 * @param parent
 *      The parent entry.
 * @param entry
 *      The parsed entry.
 * @param tabIndex
 *      The tab index to which the declaration belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaFunctionDeclaration(esprima::FunctionDeclaration* function, ParsedEntry *parent, ParsedEntry* entry, int tabIndex,
                                            QMap<QString, ParsedEntry>* objects = NULL);

/**
 * This function parses if statements.
 *
 * @param ifStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaIfStatement(esprima::IfStatement* ifStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL);

/**
 * This function parses switch statements.
 *
 * @param switchStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaSwitchStatement(esprima::SwitchStatement* switchStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

/**
 * This function parses for statements.
 *
 * @param forStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaForStatement(esprima::ForStatement* forStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

/**
 * This function parses while statements.
 *
 * @param whileStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaWhileStatement(esprima::WhileStatement* whileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

/**
 * This function parses do while statements.
 *
 * @param doWhileStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaDoWhileStatement(esprima::DoWhileStatement* doWhileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

/**
 * This function parses for in statements.
 *
 * @param forInStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaForInStatement(esprima::ForInStatement* forInStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

/**
 * Reads the type from a call expression.
 *
 * @param callExpression
 *      The call expression.
 * @param entry
 *      The parsed entry to which the type is written to.
 * @return
 *      True if a type was found.
 */
static bool getTypeFromCallExpression(esprima::CallExpression* callExpression, ParsedEntry& subEntry);

/**
 * This function parses variable declarations.
 *
 * @param varDecl
 *      variable declaration.
 * @param parent
 *      The parent of parsed entry.
 * @param entry
 *      The parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaVariableDeclaration(esprima::VariableDeclaration* varDecl, ParsedEntry &parent, ParsedEntry& entry, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL);

/***********************************************************************************************************************************/


/**
 * Parses control statements
 *
 * @param node
 *      The node.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the expression belongs to.
 * @param objects
 *      The map which contains all objects.
 * @return
 *      Treu if an element was found.
 */
static bool checkForControlStatements(esprima::Node* node, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::IfStatement* ifStatement = dynamic_cast<esprima::IfStatement*>(node);
    if(ifStatement)
    {
        parseEsprimaIfStatement(ifStatement, parent, tabIndex, objects);
        return true;
    }

    esprima::SwitchStatement* switchStatement = dynamic_cast<esprima::SwitchStatement*>(node);
    if(switchStatement)
    {
       parseEsprimaSwitchStatement(switchStatement, parent, tabIndex, objects);
       return true;
    }

    esprima::ForStatement* forStatement = dynamic_cast<esprima::ForStatement*>(node);
    if(forStatement)
    {
       parseEsprimaForStatement(forStatement, parent, tabIndex, objects);
       return true;
    }

    esprima::WhileStatement* whileStatement = dynamic_cast<esprima::WhileStatement*>(node);
    if(whileStatement)
    {
       parseEsprimaWhileStatement(whileStatement, parent, tabIndex, objects);
       return true;
    }

    esprima::DoWhileStatement* doWhileStatement = dynamic_cast<esprima::DoWhileStatement*>(node);
    if(doWhileStatement)
    {
       parseEsprimaDoWhileStatement(doWhileStatement, parent, tabIndex, objects);
       return true;
    }
    esprima::ForInStatement* forInStatement = dynamic_cast<esprima::ForInStatement*>(node);
    if(forInStatement)
    {
       parseEsprimaForInStatement(forInStatement, parent, tabIndex, objects);
       return true;
    }

    return false;
}


/**
 * Parses an (esprima) function expression.
 *
 * @param funcExp
 *      The function expression.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the expression belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaFunctionExpression(esprima::FunctionExpression* funcExp, ParsedEntry* parent, int tabIndex,
                                           QMap<QString, ParsedEntry>* objects = NULL)


{
    for(int j = 0; j < funcExp->body->body.size(); j++)
    {
        ParsedEntry subEntry;

        esprima::VariableDeclaration* subVarDecl = dynamic_cast<esprima::VariableDeclaration*>(funcExp->body->body[j]);
        if(subVarDecl)
        {
            subEntry.line = subVarDecl->declarations[0]->loc->start->line - 1;
            subEntry.column = subVarDecl->declarations[0]->loc->start->column;
            subEntry.endLine = subVarDecl->declarations[0]->loc->end->line - 1;
            subEntry.name = subVarDecl->declarations[0]->id->name.c_str();
            subEntry.params = QStringList();
            subEntry.tabIndex = tabIndex;
            subEntry.isFunctionArrayIndex = false;
            subEntry.isObjectArrayIndex = false;
            subEntry.completeName = parent->name.isEmpty() ? subEntry.name : parent->completeName + "." + subEntry.name;

            esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(subVarDecl->declarations[0]->init);
            esprima::ObjectExpression* objExp = dynamic_cast<esprima::ObjectExpression*>(subVarDecl->declarations[0]->init);
            if(funcExp)
            {//Function

                subEntry.type = PARSED_ENTRY_TYPE_CLASS_FUNCTION;
                for(int j = 0; j < funcExp->params.size(); j++)
                {
                    subEntry.params.append(funcExp->params[j]->name.c_str());
                }

                parseEsprimaFunctionExpression(funcExp, &subEntry, tabIndex, objects);
            }
            else if(objExp)
            {//Map/Array

                subEntry.type = PARSED_ENTRY_TYPE_MAP;
                parseEsprimaObjectExpression(objExp, &subEntry, tabIndex,objects);
                if(objects)
                {
                    (*objects)[subEntry.completeName] = subEntry;
                }
            }
            else
            {//Variable

               subEntry.type = PARSED_ENTRY_TYPE_CLASS_VAR;

               getTypeFromNode(subVarDecl->declarations[0]->init, subEntry);
            }

            parent->subElements.append(subEntry);
        }
        else
        {
            if(!checkForControlStatements(funcExp->body->body[j], *parent, tabIndex, objects))
            {

                esprima::ExpressionStatement* expStatement = dynamic_cast<esprima::ExpressionStatement*>(funcExp->body->body[j]);
                esprima::FunctionDeclaration* funcDecl = dynamic_cast<esprima::FunctionDeclaration*>(funcExp->body->body[j]);
                if(expStatement)
                {
                    esprima::AssignmentExpression* assignmentStatement = dynamic_cast<esprima::AssignmentExpression*>(expStatement->expression);
                    if(assignmentStatement)
                    {
                        subEntry.line = assignmentStatement->loc->start->line - 1;
                        subEntry.column = assignmentStatement->loc->start->column;
                        subEntry.endLine = assignmentStatement->loc->end->line - 1;
                        subEntry.params = QStringList();
                        subEntry.tabIndex = tabIndex;
                        subEntry.isFunctionArrayIndex = false;
                        subEntry.isObjectArrayIndex = false;

                        esprima::MemberExpression* memExp = dynamic_cast<esprima::MemberExpression*>(assignmentStatement->left);
                        if(memExp)
                        {
                            esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(assignmentStatement->right);
                            esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExp->property);
                            if(funcExp && memExp && id)
                            {
                                subEntry.name = id->name.c_str();
                                subEntry.completeName = parent->name.isEmpty() ? subEntry.name : parent->completeName + "." + subEntry.name;
                                subEntry.type = PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION;

                                for(int j = 0; j < funcExp->params.size(); j++)
                                {
                                    subEntry.params.append(funcExp->params[j]->name.c_str());
                                }

                                parseEsprimaFunctionExpression(funcExp, &subEntry, tabIndex, objects);
                                parent->subElements.append(subEntry);
                            }
                        }
                    }
                }
                else if(funcDecl)
                {
                    parseEsprimaFunctionDeclaration(funcDecl, parent ,&subEntry, tabIndex, objects);

                    parent->subElements.append(subEntry);
                }

            }//if(!checkForControlStatements(funcExp->body->body[j], *parent, tabIndex, objects))
        }

    }
}

/**
 * Reads the type from a member expression.
 *
 * @param memExpression
 *      The member expression.
 * @param entry
 *      The parsed entry to which the type is written to.
 * @return
 *      Treu if a type was found.
 */
static bool getTypeFromMemberExpression(esprima::MemberExpression* memExpression, ParsedEntry& entry)
{
    esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->object);
    if(id)
    {
        entry.valueType += id->name.c_str();

        if(memExpression->computed)
        {
            entry.isObjectArrayIndex = true;
        }
        else
        {
            id = dynamic_cast<esprima::Identifier*>(memExpression->property);
            if(id)
            {
                entry.valueType += QString(".") + id->name.c_str();
            }
        }

        return true;
    }

    esprima::BinaryExpression* binExp = dynamic_cast<esprima::BinaryExpression*>(memExpression->object);
    if(binExp)
    {
        getTypeFromNode(binExp->left, entry);

        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
            entry.valueType += QString(".") + id->name.c_str();
        }

        return true;
    }

    esprima::CallExpression* callExp = dynamic_cast<esprima::CallExpression*>(memExpression->object);
    if(callExp)
    {
        getTypeFromCallExpression(callExp, entry);

        id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
            entry.valueType += QString(".") + id->name.c_str();

        }
        if(memExpression->computed)
        {
            entry.isFunctionArrayIndex = true;
        }

        return true;
    }

    esprima::MemberExpression* mem = dynamic_cast<esprima::MemberExpression*>(memExpression->object);
    if(mem)
    {
         getTypeFromMemberExpression(mem, entry);

        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
             entry.valueType += QString(".") + id->name.c_str();
             if(memExpression->computed)
             {
                 entry.isFunctionArrayIndex = true;
             }
        }
        else
        {
            if(memExpression->computed)
            {
                entry.isObjectArrayIndex = true;
            }

            return true;
        }

    }

    esprima::NewExpression* newExp = dynamic_cast<esprima::NewExpression*>(memExpression->object);
    if(newExp)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(newExp->callee);
        if(id)
        {
            entry.valueType = id->name.c_str();

            if((entry.valueType != "Date") && (entry.valueType != "String") &&
               (entry.valueType != "Number") && (entry.valueType != "RegExp"))
            {
                entry.valueType += "()";
            }

            id = dynamic_cast<esprima::Identifier*>(memExpression->property);
            if(id)
            {
                entry.valueType += QString(".") + id->name.c_str();
            }

            return true;
        }
    }

    esprima::ThisExpression* thisExpr = dynamic_cast<esprima::ThisExpression*>(memExpression->object);
    if(thisExpr)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
            int index;
            QString tmpCompleteName = entry.completeName;

            index = tmpCompleteName.lastIndexOf(".");
            if(index != -1)
            {
                tmpCompleteName.remove(index, tmpCompleteName.length() - index);
            }


            index = tmpCompleteName.lastIndexOf(".");
            if(index != -1)
            {//There is an upper context.

                entry.valueType = tmpCompleteName;
                entry.valueType.remove(index, entry.valueType.length() - index);
                entry.valueType += ".";
                entry.valueType += id->name.c_str();
            }
            else
            {
               entry.valueType = id->name.c_str();
            }

            return true;

        }
    }

    return false;
}

/**
 * Reads the type from a condional expression.
 *
 * @param condExp
 *      The condional expression.
 * @param entry
 *      The parsed entry to which the type is written to.
 */
static void getTypeFromConditionalExpression(esprima::ConditionalExpression* condExp, ParsedEntry& entry)
{
    getTypeFromNode(condExp->consequent, entry);
}

/**
 * Reads the type from a call expression.
 *
 * @param callExpression
 *      The call expression.
 * @param entry
 *      The parsed entry to which the type is written to.
 */
static bool getTypeFromCallExpression(esprima::CallExpression* callExpression, ParsedEntry& entry)
{

    esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(callExpression->callee);
    if(id)
    {
        entry.valueType = id->name.c_str();

        if(entry.valueType == "Array")
        {
            if(callExpression->arguments.size() > 0)
            {
                esprima::StringLiteral* strLiteral = dynamic_cast<esprima::StringLiteral*>(callExpression->arguments[0]);
                if(strLiteral)
                {
                    entry.valueType = "Array<String>";
                }

                esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(callExpression->arguments[0]);
                if(numLiteral)
                {
                    entry.valueType = "Array<Number>";
                }
            }

        }
        else if((entry.valueType == "Date") || (entry.valueType == "String") ||
                (entry.valueType == "Number") || (entry.valueType == "RegExp"))
        {

        }
        else
        {
            if(!entry.valueType.endsWith("()"))
            {
                entry.valueType+= "()";
            }
        }

        return true;
    }

    esprima::MemberExpression* memExpression = dynamic_cast<esprima::MemberExpression*>(callExpression->callee);
    if(memExpression)
    {
        getTypeFromMemberExpression(memExpression, entry);
        if(!entry.valueType.endsWith("()"))
        {
            entry.valueType+= "()";
        }

        if(entry.valueType.endsWith("getWidget()"))
        {//Possibly ScriptTable.getWidget.

            for(auto el : callExpression->arguments)
            {
                esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(el);
                if(numLiteral)
                {
                    entry.additionalInformation.append(QString("%1").arg(numLiteral->value));
                }
            }
        }

        return true;
    }

    return false;
}


/**
 * Reads the type from a node.
 *
 * @param node
 *      The node.
 * @param entry
 *      The parsed entry to which the type is written to.
 */
static bool getTypeFromNode(esprima::Node* node, ParsedEntry& entry)
{
    esprima::StringLiteral* strLiteral = dynamic_cast<esprima::StringLiteral*>(node);
    if(strLiteral)
    {
        entry.valueType = "String";
        return true;
    }

    esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(node);
    if(numLiteral)
    {
        entry.valueType = "Number";
        return true;
    }

    esprima::BooleanLiteral* boolLiteral = dynamic_cast<esprima::BooleanLiteral*>(node);
    if(boolLiteral)
    {
        entry.valueType = "bool";
        return true;
    }
    esprima::NewExpression* newExp = dynamic_cast<esprima::NewExpression*>(node);
    if(newExp)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(newExp->callee);
        if(id)
        {
            entry.valueType = id->name.c_str();
            return true;
        }
    }

    esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(node);
    if(id)
    {
        entry.valueType = id->name.c_str();
        return true;
    }

    esprima::MemberExpression* memExpression = dynamic_cast<esprima::MemberExpression*>(node);
    if(memExpression)
    {
        return getTypeFromMemberExpression(memExpression, entry);

    }

    esprima::BinaryExpression* binExp = dynamic_cast<esprima::BinaryExpression*>(node);
    if(binExp)
    {
        return getTypeFromNode(binExp->left, entry);
    }

    esprima::ArrayExpression* arrayExp = dynamic_cast<esprima::ArrayExpression*>(node);
    if(arrayExp)
    {
        entry.valueType = "Array";

        if(arrayExp->elements.size() > 0)
        {
            esprima::StringLiteral* strLiteral = dynamic_cast<esprima::StringLiteral*>(arrayExp->elements[0]);
            if(strLiteral)
            {
                entry.valueType = "Array<String>";
            }

            esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(arrayExp->elements[0]);
            if(numLiteral)
            {
                entry.valueType = "Array<Number>";
            }

            esprima::BooleanLiteral* boolLiteral = dynamic_cast<esprima::BooleanLiteral*>(arrayExp->elements[0]);
            if(boolLiteral)
            {
                entry.valueType = "Array<bool>";
            }
        }

        return true;
    }

    esprima::CallExpression* callExpression = dynamic_cast<esprima::CallExpression*>(node);
    if(callExpression)
    {
        entry.valueType = "";
        return getTypeFromCallExpression(callExpression, entry);
    }

    esprima::ConditionalExpression* condExp = dynamic_cast<esprima::ConditionalExpression*>(node);
    if(condExp)
    {
        entry.valueType = "";
        getTypeFromConditionalExpression(condExp, entry);
        return true;
    }

    return false;
}

/**
 * Parses an (esprima) object expression.
 *
 * @param objExp
 *      The object expression.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the expression belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaObjectExpression(esprima::ObjectExpression* objExp, ParsedEntry* parent, int tabIndex,
                                         QMap<QString, ParsedEntry>* objects)
{
    for(int j = 0; j < objExp->properties.size(); j++)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(objExp->properties[j]->key);
        esprima::StringLiteral* literal = dynamic_cast<esprima::StringLiteral*>(objExp->properties[j]->key);

        ParsedEntry subEntry;
        subEntry.line = objExp->properties[j]->loc->start->line - 1;
        subEntry.column = objExp->properties[j]->loc->start->column;
        subEntry.endLine = objExp->properties[j]->loc->end->line - 1;
        subEntry.isFunctionArrayIndex = false;
        subEntry.isObjectArrayIndex = false;
        if(id)
        {
            subEntry.name = id->name.c_str();
        }
        else if(literal)
        {
            subEntry.name = literal->value.c_str();
        }
        else
        {
        }

        subEntry.completeName = parent->name.isEmpty() ? subEntry.name : parent->completeName + "." + subEntry.name;

        subEntry.type = PARSED_ENTRY_TYPE_MAP_VAR;
        subEntry.params = QStringList();
        subEntry.tabIndex = tabIndex;

        esprima::ObjectExpression* subObjExp = dynamic_cast<esprima::ObjectExpression*>(objExp->properties[j]->value);
        if(subObjExp)
        {

            parseEsprimaObjectExpression(subObjExp, &subEntry, tabIndex, objects);

            if(objects)
            {
                (*objects)[subEntry.completeName] = subEntry;
            }
        }
        else
        {
            esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(objExp->properties[j]->value);
            if(funcExp)
            {//The current expression is a function.

                subEntry.type = PARSED_ENTRY_TYPE_MAP_FUNC;

                //Parse all arguments.
                for(int j = 0; j < funcExp->params.size(); j++)
                {
                    subEntry.params.append(funcExp->params[j]->name.c_str());
                }

                parseEsprimaFunctionExpression(funcExp, &subEntry, tabIndex, objects);
            }
            else
            {
                getTypeFromNode(objExp->properties[j]->value, subEntry);
            }
        }

        parent->subElements.append(subEntry);
    }
}


/**
 * Parses an (esprima) function declaration.
 *
 * @param function
 *      The function declaration.
 * @param parent
 *      The parent entry.
 * @param entry
 *      The parsed entry.
 * @param tabIndex
 *      The tab index to which the declaration belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaFunctionDeclaration(esprima::FunctionDeclaration* function, ParsedEntry* parent, ParsedEntry* entry, int tabIndex,
                                            QMap<QString, ParsedEntry>* objects)
{

    entry->line = function->id->loc->start->line - 1;
    entry->column = function->id->loc->start->column;
    entry->endLine = function->body->loc->end->line - 1;

    entry->name = function->id->name.c_str();
    entry->completeName = parent->completeName.isEmpty() ? entry->name : parent->completeName + "." + entry->name;
    entry->type = PARSED_ENTRY_TYPE_FUNCTION;
    entry->tabIndex = tabIndex;

    //Parse all arguments.
    for(int j = 0; j < function->params.size(); j++)
    {
        entry->params.append(function->params[j]->name.c_str());
    }

    //Parse the body of of the function.
    for(int j = 0; j < function->body->body.size(); j++)
    {
        ParsedEntry subEntry;
        esprima::VariableDeclaration* subVarDecl = dynamic_cast<esprima::VariableDeclaration*>(function->body->body[j]);
        if(subVarDecl)
        {
            parseEsprimaVariableDeclaration(subVarDecl, *entry, subEntry, tabIndex, objects);

            entry->subElements.append(subEntry);
        }
        else
        {
            checkForControlStatements(function->body->body[j], *entry, tabIndex, objects);

            esprima::ExpressionStatement* expStatement = dynamic_cast<esprima::ExpressionStatement*>(function->body->body[j]);
            if(expStatement)
            {
                esprima::AssignmentExpression* assignmentStatement = dynamic_cast<esprima::AssignmentExpression*>(expStatement->expression);
                if(assignmentStatement)
                {
                    esprima::MemberExpression* memExp = dynamic_cast<esprima::MemberExpression*>(assignmentStatement->left);
                    esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(assignmentStatement->right);
                    if(funcExp && memExp)
                    {
                        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExp->property);
                        if(funcExp && memExp && id)
                        {
                            subEntry.line = assignmentStatement->loc->start->line - 1;
                            subEntry.column = assignmentStatement->loc->start->column;
                            subEntry.endLine = assignmentStatement->loc->end->line - 1;
                            subEntry.name = id->name.c_str();
                            subEntry.isFunctionArrayIndex = false;
                            subEntry.isObjectArrayIndex = false;
                            subEntry.type = PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION;
                            subEntry.params = QStringList();
                            subEntry.tabIndex = tabIndex;
                            subEntry.completeName = entry->completeName.isEmpty() ? subEntry.name : entry->completeName + "." + subEntry.name;
                            for(int j = 0; j < funcExp->params.size(); j++)
                            {
                                subEntry.params.append(funcExp->params[j]->name.c_str());
                            }

                            parseEsprimaFunctionExpression(funcExp, &subEntry, tabIndex, objects);
                            entry->subElements.append(subEntry);
                        }
                    }
                }
            }
        }

    }
}


/**
 * Parses an (esprima) new expression.
 *
 * @param newExp
 *      The new expression.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the expression belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaNewExpression(esprima::NewExpression* newExp, ParsedEntry* parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL)
{
    esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(newExp->callee);
    if(funcExp)
    {
        parseEsprimaFunctionExpression(funcExp, parent, tabIndex, objects);
    }
}


/**
 * This function parses variable declarations.
 *
 * @param varDecl
 *      variable declaration.
 * @param parent
 *      The parent of parsed entry.
 * @param entry
 *      The parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaVariableDeclaration(esprima::VariableDeclaration* varDecl, ParsedEntry& parent, ParsedEntry& entry, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects )
{
    entry.line = varDecl->declarations[0]->loc->start->line - 1;
    entry.column = varDecl->declarations[0]->loc->start->column;
    entry.name = varDecl->declarations[0]->id->name.c_str();
    entry.isFunctionArrayIndex = false;
    entry.isObjectArrayIndex = false;
    entry.endLine = varDecl->declarations[0]->loc->end->line - 1;
    entry.completeName = parent.completeName.isEmpty() ? entry.name : parent.completeName + "." + entry.name;
    entry.params = QStringList();
    entry.tabIndex = tabIndex;


    esprima::NewExpression* newExp = dynamic_cast<esprima::NewExpression*>(varDecl->declarations[0]->init);
    esprima::ObjectExpression* objExp = dynamic_cast<esprima::ObjectExpression*>(varDecl->declarations[0]->init);

    if(newExp)
    {//Class.

        esprima::FunctionExpression* funcExpr = dynamic_cast<esprima::FunctionExpression*>(newExp->callee);
        if(funcExpr)
        {//Class

            entry.type = PARSED_ENTRY_TYPE_CLASS;
            parseEsprimaNewExpression(newExp, &entry, tabIndex, objects);
            (*objects)[entry.name] = entry;
        }
        else
        {//Variable
            entry.type = PARSED_ENTRY_TYPE_VAR;
            esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(newExp->callee);
            if(id)
            {
                entry.params.append(id->name.c_str());
            }
            getTypeFromNode(newExp->callee, entry);
        }

    }
    else if(objExp)
    {//Map/Array

        entry.type = PARSED_ENTRY_TYPE_MAP;
        parseEsprimaObjectExpression(objExp, &entry, tabIndex, objects);
        (*objects)[entry.name] = entry;

    }
    else
    {//Function, Const or variable.

        esprima::VariableDeclarator* decl = dynamic_cast<esprima::VariableDeclarator*>(varDecl->declarations[0]);
        if(decl)
        {
            esprima::FunctionExpression* funcExp = dynamic_cast<esprima::FunctionExpression*>(decl->init);
            if(funcExp)
            {
                entry.type = PARSED_ENTRY_TYPE_CLASS_THIS_FUNCTION;
                parseEsprimaFunctionExpression(funcExp, &entry, tabIndex, objects);
            }
            else
            {
                entry.type = (varDecl->kind == "const") ? PARSED_ENTRY_TYPE_CONST : PARSED_ENTRY_TYPE_VAR;
                esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(decl->init);
                if(id)
                {//The the declaration of the current variable has an assignement of another variable/object
                 //e.g. var map2 = map1;
                    entry.params.append(id->name.c_str());
                }

                getTypeFromNode(decl->init, entry);

                if(dynamic_cast<esprima::MemberExpression*>(decl->init))
                {
                    entry.params.append(entry.valueType);

                    if(dynamic_cast<esprima::MemberExpression*>(decl->init)->computed)
                    {
                        entry.isFunctionArrayIndex = true;
                    }
                }
            }

        }
    }
}


/**
 * This function parses block statements.
 *
 * @param blockStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaBlockStatement(esprima::BlockStatement* blockStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL)
{
    for(int i = 0; i < blockStatement->body.size(); i++)
    {
        esprima::VariableDeclaration* varDecl = dynamic_cast<esprima::VariableDeclaration*>(blockStatement->body[i]);
        if(varDecl)
        {
            ParsedEntry entry;
            parseEsprimaVariableDeclaration(varDecl, parent, entry, tabIndex, objects);

            //Append the current entry to the file result list.
            parent.subElements.append(entry);

        }
        else
        {
            checkForControlStatements(blockStatement->body[i], parent, tabIndex,objects);
        }
    }
}

/**
 * This function parses if statements.
 *
 * @param ifStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaIfStatement(esprima::IfStatement* ifStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( ifStatement->alternate);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);

    }

    blockStatement = dynamic_cast<esprima::BlockStatement*>( ifStatement->consequent);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);

    }
}


/**
 * This function parses switch case statements.
 *
 * @param switchCase
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaSwitchCase(esprima::SwitchCase* switchCase, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{

    for(auto el : switchCase->consequent)
    {
        esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( el);
        if(blockStatement)
        {
            parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);

        }
    }
}

/**
 * This function parses switch statements.
 *
 * @param switchStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaSwitchStatement(esprima::SwitchStatement* switchStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    for(auto el : switchStatement->cases)
    {
        parseEsprimaSwitchCase(el, parent, tabIndex, objects);
    }
}

/**
 * This function parses for statements.
 *
 * @param forStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaForStatement(esprima::ForStatement* forStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::VariableDeclaration* varDecl = dynamic_cast<esprima::VariableDeclaration*>(forStatement->init);
    if(varDecl)
    {
        ParsedEntry entry;
        parseEsprimaVariableDeclaration( varDecl, parent, entry, tabIndex, objects);

        parent.subElements.append(entry);

    }

    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( forStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);

    }
}

/**
 * This function parses while statements.
 *
 * @param whileStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaWhileStatement(esprima::WhileStatement* whileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( whileStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);
    }
}

/**
 * This function parses do while statements.
 *
 * @param doWhileStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaDoWhileStatement(esprima::DoWhileStatement* doWhileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( doWhileStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);
    }
}

/**
 * This function parses for in statements.
 *
 * @param forInStatement
 *      The statement.
 * @param parent
 *      The parent of parsed entry.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param objects
 *      The map which contains all objects.
 */
static void parseEsprimaForInStatement(esprima::ForInStatement* forInStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::VariableDeclaration* varDecl = dynamic_cast<esprima::VariableDeclaration*>(forInStatement->left);
    if(varDecl)
    {
        ParsedEntry entry;
        parseEsprimaVariableDeclaration( varDecl, parent, entry, tabIndex, objects);

        esprima::Identifier* type = dynamic_cast<esprima::Identifier*>(forInStatement->right);
        if(type)
        {

            entry.valueType = type->name.c_str();
            entry.isFunctionArrayIndex = true;
        }

        parent.subElements.append(entry);

    }

    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( forInStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);
    }
}


/**
 * This function checks for prototyp functions (e.g.
 * classSingelton1.prototype.getInfo = function(arg))
 *
 * @param node
 *      The esprima node.
 * @param tabIndex
 *      The tab index to which the node belongs to.
 * @param prototypeFunctions
 *      The map which contains all prototyp functions.
 */
static void checkForPrototypFunction(esprima::Node* node, int tabIndex, QMap<QString, ParsedEntry>* objects,
                                     QMap<QString, QVector<ParsedEntry>>& prototypeFunctions)
{
    esprima::ExpressionStatement* expStatement = dynamic_cast<esprima::ExpressionStatement*>(node);
    if(expStatement)
    {
        esprima::AssignmentExpression* assExp = dynamic_cast<esprima::AssignmentExpression*>(expStatement->expression);
        if(assExp)
        {
            esprima::MemberExpression* left = dynamic_cast<esprima::MemberExpression*>(assExp->left);
            esprima::FunctionExpression* right = dynamic_cast<esprima::FunctionExpression*>(assExp->right);
            if(left && right)
            {
                esprima::MemberExpression* object = dynamic_cast<esprima::MemberExpression*>(left->object);
                esprima::Identifier* property = dynamic_cast<esprima::Identifier*>(left->property);
                if(object && property)
                {
                    esprima::Identifier* subObject = dynamic_cast<esprima::Identifier*>(object->object);
                    if(subObject)
                    {//Prototyp function

                        ParsedEntry prot;
                        prot.line = assExp->loc->start->line -1;
                        prot.column = assExp->loc->start->column;
                        prot.endLine = assExp->loc->end->line -1;
                        prot.name = property->name.c_str();
                        prot.completeName = prot.name;
                        prot.params = QStringList();
                        prot.tabIndex = tabIndex;
                        prot.type = PARSED_ENTRY_TYPE_PROTOTYPE_FUNC;

                        for(int j = 0; j < right->params.size(); j++)
                        {
                            prot.params.append(right->params[j]->name.c_str());
                        }

                         parseEsprimaFunctionExpression(right, &prot, tabIndex, objects);

                        if(!prototypeFunctions.contains(subObject->name.c_str()))
                        {
                          prototypeFunctions[subObject->name.c_str()] = QVector<ParsedEntry>();
                        }
                        prototypeFunctions[subObject->name.c_str()].append(prot);
                    }
                }
            }
        }

    }
}


/**
 * Adds a parsed entry (and his subentries) to the auto completion list.
 *
 * @param entry
 *      The parsed entry.
 * @param autoCompletionEntries
 *      The auto completion list.
 * @param parentString
 *      The string of the parent in the auto completion list.
 * @param rootObjectName
 *      The name of the root parsed entry.
 */
static void addParsedEntiresToAutoCompletionList(const ParsedEntry& entry, QMap<QString, QStringList>& autoCompletionEntries,
                                                 QString parentString, QString rootObjectName)
{
    QString value = (parentString.isEmpty()) ? entry.name : parentString + "::" + entry.name;

    autoCompletionEntries[rootObjectName] << value;

    for(auto el : entry.subElements)
    {
        addParsedEntiresToAutoCompletionList(el, autoCompletionEntries, value, rootObjectName);
    }
}


static void getAllParsedTypes(QMap<QString, QString>& parsedTypes, ParsedEntry& entry, QString parentName)
{
    QString key = (parentName.isEmpty()) ? entry.name : parentName + "." + entry.name;

    parsedTypes[key] = entry.valueType;

    for(auto el : entry.subElements)
    {
        getAllParsedTypes(parsedTypes, el, key);
    }
}



#endif // ESPRIMAPARSEFUNCTIONS

