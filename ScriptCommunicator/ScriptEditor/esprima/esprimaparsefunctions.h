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
    bool isArrayIndex;//True if the value is from an array index.
    QStringList additionalInformation;//Additional information.
};

static void parseEsprimaObjectExpression(esprima::ObjectExpression* objExp, ParsedEntry* parent, int tabIndex,
                                         QMap<QString, ParsedEntry>* objects = NULL);

static void getTypeFromNode(esprima::Node* node, ParsedEntry& subEntry);
static void parseEsprimaFunctionDeclaration(esprima::FunctionDeclaration* function, ParsedEntry *parent, ParsedEntry* entry, int tabIndex,
                                            QMap<QString, ParsedEntry>* objects = NULL);

static void parseEsprimaIfStatement(esprima::IfStatement* ifStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL);

static void parseEsprimaSwitchStatement(esprima::SwitchStatement* switchStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void parseEsprimaForStatement(esprima::ForStatement* forStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void parseEsprimaWhileStatement(esprima::WhileStatement* whileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void parseEsprimaDoWhileStatement(esprima::DoWhileStatement* doWhileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void parseEsprimaForInStatement(esprima::ForInStatement* forInStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects);

static void checkForControlStatements(esprima::Node* node, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::IfStatement* ifStatement = dynamic_cast<esprima::IfStatement*>(node);
    if(ifStatement)
    {
        parseEsprimaIfStatement(ifStatement, parent, tabIndex, objects);
    }

    esprima::SwitchStatement* switchStatement = dynamic_cast<esprima::SwitchStatement*>(node);
    if(switchStatement)
    {
       parseEsprimaSwitchStatement(switchStatement, parent, tabIndex, objects);
    }

    esprima::ForStatement* forStatement = dynamic_cast<esprima::ForStatement*>(node);
    if(forStatement)
    {
       parseEsprimaForStatement(forStatement, parent, tabIndex, objects);
    }

    esprima::WhileStatement* whileStatement = dynamic_cast<esprima::WhileStatement*>(node);
    if(whileStatement)
    {
       parseEsprimaWhileStatement(whileStatement, parent, tabIndex, objects);
    }

    esprima::DoWhileStatement* doWhileStatement = dynamic_cast<esprima::DoWhileStatement*>(node);
    if(doWhileStatement)
    {
       parseEsprimaDoWhileStatement(doWhileStatement, parent, tabIndex, objects);
    }
    esprima::ForInStatement* forInStatement = dynamic_cast<esprima::ForInStatement*>(node);
    if(forInStatement)
    {
       parseEsprimaForInStatement(forInStatement, parent, tabIndex, objects);
    }
}

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
            subEntry.isArrayIndex = false;
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
            checkForControlStatements(funcExp->body->body[j], *parent, tabIndex, objects);

            esprima::ExpressionStatement* expStatement = dynamic_cast<esprima::ExpressionStatement*>(funcExp->body->body[j]);
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
                    subEntry.isArrayIndex = false;

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

            esprima::FunctionDeclaration* funcDecl = dynamic_cast<esprima::FunctionDeclaration*>(funcExp->body->body[j]);
            if(funcDecl)
            {
                parseEsprimaFunctionDeclaration(funcDecl, parent ,&subEntry, tabIndex, objects);

                parent->subElements.append(subEntry);
            }
        }

    }
}


static void getTypeFromCallExpression(esprima::CallExpression* callExpression, ParsedEntry& subEntry);

static void getTypeFromMemberExpression(esprima::MemberExpression* memExpression, ParsedEntry& subEntry)
{
    esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->object);
    if(id)
    {
        subEntry.valueType += id->name.c_str();

        if(memExpression->computed)
        {
            subEntry.isArrayIndex = true;
        }
        else
        {
            id = dynamic_cast<esprima::Identifier*>(memExpression->property);
            if(id)
            {
                subEntry.valueType += QString(".") + id->name.c_str();
            }
        }
    }

    esprima::CallExpression* callExp = dynamic_cast<esprima::CallExpression*>(memExpression->object);
    if(callExp)
    {
        getTypeFromCallExpression(callExp, subEntry);

        id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
            subEntry.valueType += QString(".") + id->name.c_str();

        }
    }

    esprima::MemberExpression* mem = dynamic_cast<esprima::MemberExpression*>(memExpression->object);
    if(mem)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
            getTypeFromMemberExpression(mem, subEntry);

             subEntry.valueType += QString(".") + id->name.c_str();
        }
    }

    esprima::ThisExpression* thisExpr = dynamic_cast<esprima::ThisExpression*>(memExpression->object);
    if(thisExpr)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(memExpression->property);
        if(id)
        {
            int index;
            QString tmpCompleteName = subEntry.completeName;

            index = tmpCompleteName.lastIndexOf(".");
            if(index != -1)
            {
                tmpCompleteName.remove(index, tmpCompleteName.length() - index);
            }


            index = tmpCompleteName.lastIndexOf(".");
            if(index != -1)
            {
                subEntry.valueType = tmpCompleteName;
                subEntry.valueType.remove(index, subEntry.valueType.length() - index);
                subEntry.valueType += ".";
                tmpCompleteName = "";
            }


            if(!tmpCompleteName.isEmpty())
            {
                subEntry.valueType = tmpCompleteName + ".";
            }
            subEntry.valueType += id->name.c_str();
        }
    }
}

static void getTypeFromCallExpression(esprima::CallExpression* callExpression, ParsedEntry& subEntry)
{

    esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(callExpression->callee);
    if(id)
    {
        subEntry.valueType = id->name.c_str();

        if(subEntry.valueType == "Array")
        {
            if(callExpression->arguments.size() > 0)
            {
                esprima::StringLiteral* strLiteral = dynamic_cast<esprima::StringLiteral*>(callExpression->arguments[0]);
                if(strLiteral)
                {
                    subEntry.valueType = "Array<String>";
                }

                esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(callExpression->arguments[0]);
                if(numLiteral)
                {
                    subEntry.valueType = "Array<Number>";
                }
            }
        }
        else if((subEntry.valueType == "Date") || (subEntry.valueType == "String") ||
                (subEntry.valueType == "Number"))
        {

        }
        else
        {
            if(!subEntry.valueType.endsWith("()"))
            {
                subEntry.valueType+= "()";
            }
        }
    }

    esprima::MemberExpression* memExpression = dynamic_cast<esprima::MemberExpression*>(callExpression->callee);
    if(memExpression)
    {
        getTypeFromMemberExpression(memExpression, subEntry);
        if(!subEntry.valueType.endsWith("()"))
        {
            subEntry.valueType+= "()";
        }

        if(subEntry.valueType.endsWith("getWidget()"))
        {//Possibly ScriptTable.getWidget.

            for(auto el : callExpression->arguments)
            {
                esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(el);
                if(numLiteral)
                {
                    subEntry.additionalInformation.append(QString("%1").arg(numLiteral->value));
                }
            }
        }

    }
}

static void getTypeFromNode(esprima::Node* node, ParsedEntry& subEntry)
{
    esprima::StringLiteral* strLiteral = dynamic_cast<esprima::StringLiteral*>(node);
    if(strLiteral)
    {
        subEntry.valueType = "String";
    }

    esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(node);
    if(numLiteral)
    {
        subEntry.valueType = "Number";
    }

    esprima::BooleanLiteral* boolLiteral = dynamic_cast<esprima::BooleanLiteral*>(node);
    if(boolLiteral)
    {
        subEntry.valueType = "bool";
    }
    esprima::NewExpression* newExp = dynamic_cast<esprima::NewExpression*>(node);
    if(newExp)
    {
        esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(newExp->callee);
        if(id)
        {
            subEntry.valueType = id->name.c_str();
        }
    }

    esprima::Identifier* id = dynamic_cast<esprima::Identifier*>(node);
    if(id)
    {
        subEntry.valueType = id->name.c_str();
    }

    esprima::MemberExpression* memExpression = dynamic_cast<esprima::MemberExpression*>(node);
    if(memExpression)
    {
        getTypeFromMemberExpression(memExpression, subEntry);

    }

    esprima::BinaryExpression* binExp = dynamic_cast<esprima::BinaryExpression*>(node);
    if(binExp)
    {
        getTypeFromNode(binExp->left, subEntry);
    }

    esprima::ArrayExpression* arrayExp = dynamic_cast<esprima::ArrayExpression*>(node);
    if(arrayExp)
    {
        subEntry.valueType = "Array";

        if(arrayExp->elements.size() > 0)
        {
            esprima::StringLiteral* strLiteral = dynamic_cast<esprima::StringLiteral*>(arrayExp->elements[0]);
            if(strLiteral)
            {
                subEntry.valueType = "Array<String>";
            }

            esprima::NumericLiteral* numLiteral = dynamic_cast<esprima::NumericLiteral*>(arrayExp->elements[0]);
            if(numLiteral)
            {
                subEntry.valueType = "Array<Number>";
            }
        }

    }


    if(node == 0)
    {
        subEntry.valueType = "";
    }

    esprima::CallExpression* callExpression = dynamic_cast<esprima::CallExpression*>(node);
    if(callExpression)
    {
        subEntry.valueType = "";
        getTypeFromCallExpression(callExpression, subEntry);
    }

}

/**
 * Parses an (esprima) object expression.
 * @param objExp
 *      The object expression.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the expression belongs to.
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
        subEntry.isArrayIndex = false;
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

        parent->subElements.append(subEntry);
    }
}


static void parseEsprimaVariableDeclaration(esprima::VariableDeclaration* varDecl, ParsedEntry &parent, ParsedEntry& entry, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects = NULL);



/**
 * Parses an (esprima) function declaration.
 * @param function
 *      The function declaration.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the declaration belongs to.
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
                            subEntry.isArrayIndex = false;
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
 * @param newExp
 *      The new expression.
 * @param parent
 *      The parent entry.
 * @param tabIndex
 *      The tab index to which the expression belongs to.
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


static void parseEsprimaVariableDeclaration(esprima::VariableDeclaration* varDecl, ParsedEntry& parent, ParsedEntry& entry, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects )
{
    entry.line = varDecl->declarations[0]->loc->start->line - 1;
    entry.column = varDecl->declarations[0]->loc->start->column;
    entry.name = varDecl->declarations[0]->id->name.c_str();
    entry.isArrayIndex = false;
    entry.endLine = varDecl->declarations[0]->loc->end->line - 1;
    entry.completeName = parent.completeName.isEmpty() ? entry.name : parent.completeName + "." + entry.name;
    entry.params = QStringList();
    entry.tabIndex = tabIndex;
    entry.isArrayIndex = false;


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
                        entry.isArrayIndex = true;
                    }
                }
            }

        }
    }
}



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
        checkForControlStatements(blockStatement->body[i], parent, tabIndex,objects);
    }
}

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

static void parseEsprimaSwitchStatement(esprima::SwitchStatement* switchStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    for(auto el : switchStatement->cases)
    {
        parseEsprimaSwitchCase(el, parent, tabIndex, objects);
    }
}

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

static void parseEsprimaWhileStatement(esprima::WhileStatement* whileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( whileStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);
    }
}

static void parseEsprimaDoWhileStatement(esprima::DoWhileStatement* doWhileStatement, ParsedEntry& parent, int tabIndex,
                                      QMap<QString, ParsedEntry>* objects)
{
    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( doWhileStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);
    }
}

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
            entry.isArrayIndex = true;
        }

        parent.subElements.append(entry);

    }

    esprima::BlockStatement* blockStatement = dynamic_cast<esprima::BlockStatement*>( forInStatement->body);
    if(blockStatement)
    {
        parseEsprimaBlockStatement(blockStatement, parent, tabIndex, objects);
    }
}


static void checkForProrotypFunction(esprima::Node* node, int tabIndex, QMap<QString, ParsedEntry>* objects,
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


static getAllParsedTypes(QMap<QString, QString>& parsedTypes, ParsedEntry& entry, QString parentName)
{
    QString key = (parentName.isEmpty()) ? entry.name : parentName + "." + entry.name;

    parsedTypes[key] = entry.valueType;


    for(auto el : entry.subElements)
    {
        getAllParsedTypes(parsedTypes, el, key);
    }
}



#endif // ESPRIMAPARSEFUNCTIONS

