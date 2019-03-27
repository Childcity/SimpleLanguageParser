#ifndef SYNTACTICALANALYZER_H
#define SYNTACTICALANALYZER_H

#include "main.h"
#include "SyntacticalParserBase/syntacticalparserbase.h"
#include "ASTree/astnode.h"

#include <QJsonDocument>
#include <QMap>

namespace Gorod {
static QVariantList synataxTree;
static QString rpn;

class SyntacticalException : public SyntacticalParserBaseException {
public:
    explicit SyntacticalException(const QString msg, int line, int pos)
        : SyntacticalParserBaseException(msg, line, pos){}

    SyntacticalException( const SyntacticalException & ) = default;

    ~SyntacticalException() override;

    QString what() const override{
        return SyntacticalParserBaseException::what();
    }
};

class SyntacticalAnalyzer : public SyntacticalParserBase{
    using Token = LangTokens::Token;
public:
    explicit SyntacticalAnalyzer(const QVariantList &source)
        : SyntacticalParserBase(source)
    {}

    virtual ~SyntacticalAnalyzer();

    // Number = [ "-" ] digit{digit} .
    ASTNode::SharedPtr Number() {
        DEBUGSYNTX("-> Number"<<lexemeValue())

        QString number = lexemeValue();
        next();

        DEBUGSYNTX("<- Number. Next lexeme"<<lexemeValue())
        return ASTNode::GetNewInstance(Token::NUMBER, number);
    }

    //Ident = Letter {letter | digit | "_"} .
    ASTNode::SharedPtr Ident() {
        DEBUGSYNTX("-> Ident"<<lexemeValue()<<lexeme()["varType"].toString())

        if(lexeme()["varType"].toString().isNull()){
            throw SyntacticalException( QString("\"%1\" Необьявленный идентификатор!")
                                        .arg(lexemeValue())
                                        , lexemeLine(), lexemePosInLine()
                );
        }

        QString identifier = lexemeValue();
        next();

        DEBUGSYNTX("<- Ident. Next lexeme"<<lexemeValue())
        return ASTNode::GetNewInstance(Token::IDENT, identifier);
    }

    //Group = "(" Add ")"  | Number | Ident .
    ASTNode::SharedPtr Group() {
        DEBUGSYNTX("-> Group"<<lexemeValue())
        if (isMatch("(")) { // выбираем альтернативу
            match("(");
            // это выражение в скобках
            auto result = Add();
            match(")");
            return result;
        } else if (lexeme()["type"] == "constant") {
            return Number(); // число
        } else
            return Ident(); // это идентификатор
    }

    //Power = Group ["^" Group] .
    ASTNode::SharedPtr Power() {
        DEBUGSYNTX("-> Power"<<lexemeValue())
        auto result = Group();
        if (isMatch("^")) { // обнаружино поднесение в степень
            QString oper = match("^");
            auto temp = Power();
            result = ASTNode::GetNewInstance(Token::Power, result, temp);
        }
        DEBUGSYNTX("<- Power. Next lexeme"<<lexemeValue())
        return result;
    }

    //Mult = Power [( "*" | "/" ) Power] .
    ASTNode::SharedPtr Mult() {
        DEBUGSYNTX("-> Mult"<<lexemeValue())
        auto result = Power();
        while (isMatch("*", "/")) { // повторяем нужное кол-во раз
            // здесь выбор альтернативы
            QString oper = match("*", "/");
            auto temp = Power();
            result = (oper == "*") ? ASTNode::GetNewInstance(Token::Mul, result, temp)
                                    : ASTNode::GetNewInstance(Token::Div, result, temp);
        }
        DEBUGSYNTX("<- Mult. Next lexeme"<<lexemeValue())
        return result;
    }

    //Add = Mult [( "+" | "-" ) Mult] .
    ASTNode::SharedPtr Add() { // аналогично Mult
        DEBUGSYNTX("-> Add"<<lexemeValue())
        auto result = Mult();
        while (isMatch("+", "-")) {
            QString oper = match("+", "-");
            auto temp = Mult();
            result = (oper == "+") ? ASTNode::GetNewInstance(Token::Add, result, temp)
                                    : ASTNode::GetNewInstance(Token::Sub, result, temp);
        }
        DEBUGSYNTX("<- Add. Next lexeme"<<lexemeValue())
        return result;
    }

    //LogicExpr = Ident ("<" | ">") Add .
    ASTNode::SharedPtr LogicExpr() {
        DEBUGSYNTX("-> LogicExpr"<<lexemeValue())
        auto result = Ident();
        while (isMatch("<", ">")) {
            QString oper = match("<", ">");
            auto temp = Add();
            result = (oper == "<") ? ASTNode::GetNewInstance(Token::Less, result, temp)
                                    : ASTNode::GetNewInstance(Token::More, result, temp);
        }
        DEBUGSYNTX("<- LogicExpr. Next lexeme"<<lexemeValue())
        return result;
    }

    //Assignment = Ident "=" Expr .
     ASTNode::SharedPtr Assignment() {
         DEBUGSYNTX("-> Assignment")
         auto identifier = Ident();
         match("=");
         auto value = Add();
         match(";");
         DEBUGSYNTX("<- Assignment. Next lexeme"<<lexemeValue())
         return ASTNode::GetNewInstance(Token::Assign, identifier, value);
     }

    // ....
    ASTNode::SharedPtr Statement() {
        DEBUGSYNTX("-> Statement"<<lexemeValue())

        // выбираем альтернативу
        if(isMatch(";")){ //пустое выражение
            match(";");
            return ASTNode::GetNewInstance(Token::DotPoint);

        } else if (isMatch("write")) { // это вывод данных
            match("write"); match("(");
            auto value = Add();
            match(")"); match(";");
            return ASTNode::GetNewInstance(Token::Write, value);

        } else if (isMatch("read")) {// это ввод данных
            //DEBUGSYNTX(getSource().mid(getPos(), 10))
            match("read"); match("(");
            auto identifier = Ident();
            match(")"); match(";");
            return ASTNode::GetNewInstance(Token::Read, identifier);

        } else if(isMatch("if")){ //условие
            DEBUGSYNTX("-> if")
            match("if"); match("(");
            auto logicExpr = LogicExpr();
            match(")");
            auto block = Block();
            DEBUGSYNTX("<- if")
            return ASTNode::GetNewInstance(Token::If, logicExpr, block);

        } else if(isMatch("for")){ //for цикл
            DEBUGSYNTX("-> for")
            match("for"); auto assignment =  Assignment();
            match("to"); auto to = Add();
            match("by"); auto by = Add();
            match("while"); match("("); auto logicExpr = LogicExpr(); match(")");
            auto block = Block();
            match("rof"); match(";");

            auto forResult = ASTNode::GetNewInstance(Token::For);
            forResult->addChild(assignment);
            forResult->addChild(to);
            forResult->addChild(by);
            forResult->addChild(logicExpr);
            forResult->addChild(block);
            DEBUGSYNTX("<- for")
            return forResult;

        } else { // это операция присвоения значения
            return Assignment();
        }
        DEBUGSYNTX("<- Statement. Next lexeme"<<lexemeValue());

        throw SyntacticalException( QString("Неизвестное выражение! (ошибка в коде) (pos=%1) [%2]")
                                       .arg(lexemePosInLine())
                                       .arg(lexemeValue())
                                   , lexemeLine(), lexemePosInLine()
            );
    }

    ASTNode::SharedPtr VarInit(){
        if(isMatch("int")){
            DEBUGSYNTX("-> int");
            match("int");
            auto integers = ASTNode::GetNewInstance(Token::Int);

            bool isDefinitionEnds = false;

            do {
                defineEachCurrentVar();
                integers->addChild(Ident());

                if(isMatch(",")){ //повторяем пока встречаются ","
                    match(",");
                }else{
                    isDefinitionEnds = true;
                }

            } while (! isDefinitionEnds);

            match(";");
            DEBUGSYNTX("<- int");
            return integers;
        }/*else if(isMatch("double")){

        }*/else{
            throw SyntacticalException( QString("Неверно указан тип (ошибка в коде!) (pos=%1) [%2]")
                                        .arg(lexemePosInLine())
                                        .arg(lexemeValue())
                                    , lexemeLine(), lexemePosInLine()
             );
        }
    }

    ASTNode::SharedPtr Gorod(){
        auto progNode = ASTNode::GetNewInstance(Token::GOROD, "TP-62: Gorodetskiy Program");
        progNode->addChild(Block());
        return progNode;
    }

    ASTNode::SharedPtr Block(){
        DEBUGSYNTX("-> {}")
        match("{");
        auto block = ASTNode::GetNewInstance(Token::LeftBraket, "{}");

        while(isMatch("int")){ //инициализация (in future: double, unsigned int)
            block->addChild(VarInit());
        }

        //while (! ) { // повторяем до конца входной строки
        while(lexemeValue() != "}"){
            if(isEnd())
                break;
            block->addChild(Statement());
        }
        match("}");
        DEBUGSYNTX("<- {}")
        return block;
    }

    ASTNode::SharedPtr Parse(){

        auto gorodProg = Gorod();

        if(! isEnd()){
            DEBUGSYNTX("ParseLang fail")
            throw SyntacticalException(QString("Лишний символ '%1'").arg(lexemeValue())
                                  , lexemeLine(), lexemePosInLine()
           );
        }

        return gorodProg;
    }




    static ASTNode::SharedPtr Parse(const QVariantList &source){
        SyntacticalAnalyzer ASTResult(source);
        return ASTResult.Parse();
    }

    static void GenerateReversePolishNotation(ASTNode::SharedPtr tree){
        QList<ASTNode::SharedPtr> subRuleLst;
        forEachSubElements(tree, subRuleLst);
        qDebug().noquote() <<QJsonDocument::fromVariant(synataxTree).toJson(QJsonDocument::JsonFormat::Indented);
        qDebug().noquote() <<rpn;
    }

    static void forEachSubElements(ASTNode::SharedPtr subRule, QList<ASTNode::SharedPtr> &subRuleLst, QString tab = QString())
    {
        if(subRule->getChildsCount() > 0){
            QList<ASTNode::SharedPtr> newSubRulesLst;

            for (int i = 0; i < subRule->getChildsCount(); i++) {
                forEachSubElements(subRule->getChild(i), newSubRulesLst, tab + " ");
            }

            QString childs;
            QVariantList subTokenLst;
            for(auto ch : newSubRulesLst){
                subTokenLst += QVariantMap({
                                               {"lexem", ch->getText()+" (" + LangTokens::GetToken(ch->getType()) + ")"}
                                               //, {"token", LangTokens::GetToken(ch->getType())}
                                               , {"id", ch->getUniqueName().split("node")[1]}
                                               , {"polish", findRecurseRPN(ch)}
                                           });
                childs += ch->getText() + "(" + ch->getUniqueName().split("node")[1]/*LangTokens::GetToken(ch->getType())*/ + ") ";
            }
            synataxTree += QVariantMap({
                                           {  "sub_rules", subTokenLst}
                                           , {"rule", subRule->getText()}
                                           , {"id", subRule->getUniqueName().split("node")[1]}
                                           , {"polish", RPNForRule(subRule, newSubRulesLst)}
                                       });
            qDebug().noquote() <<tab <<"Childrens: " <<childs;
            qDebug().noquote() <<tab<<"Was Children OF:" <<subRule->getText()  << "(" <<subRule->getUniqueName().split("node")[1] << ") "<<endl;

            rpn += RPNForRule(subRule, newSubRulesLst);
            subRuleLst.append(subRule);
        }else{
            rpn += subRule->getText() + " ";
            if(subRule->getType() == Token::IDENT) rpn += "\n";
            //qDebug()<<subRule->getText();
            subRuleLst.append(subRule);
        }
    }
    static QString RPNForRule(ASTNode::SharedPtr &subToken, QList<ASTNode::SharedPtr> &subRulesLst){
        QString ruleRpn;
        Token subRuleType = subToken->getType();
        switch (subRuleType) {
            case Token::Add:
            case Token::Sub:
            case Token::Mul:
            case Token::Div:
            case Token::Power:
            case Token::Assign:
            case Token::Less:
            case Token::More:{
                QString leftOperRPN = findRecurseRPN(subRulesLst.at(0));
                QString rightOperRPN = findRecurseRPN(subRulesLst.at(1));
                return leftOperRPN + rightOperRPN + subToken->getText();
            }
            case Token::Int:{
                for(const auto &child : subRulesLst)
                    ruleRpn += child->getText()+"#INT";
                return ruleRpn;
            }
            case Token::If:{
                QString id = subToken->getUniqueName().split("node")[1];
                QString exprRPN = findRecurseRPN(subRulesLst.at(0));
                QString blockRPN = findRecurseRPN(subRulesLst.at(1));
                return " " + exprRPN + "m"+id+"УПХ" + blockRPN + "m"+id+": ";
            }
            // Block Rule
            case Token::LeftBraket:{
                for(const auto &child : subRulesLst)
                    ruleRpn += findRecurseRPN(child) + " ";
                return ruleRpn;
            }
            case Token::Read:{
                return " " + subRulesLst.at(0)->getText() + "#READ";
            }
            case Token::Write:{
                for(const auto &child : subRulesLst)
                    ruleRpn += findRecurseRPN(child);
                return " " + ruleRpn + "#WRITE";
            }
        case Token::For:{
            QString id = subToken->getUniqueName().split("node")[1];
            QString assigmentRPN = findRecurseRPN(subRulesLst.at(0));
            QString expr1RPN = findRecurseRPN(subRulesLst.at(1));
            QString expr2RPN = findRecurseRPN(subRulesLst.at(2));
            QString logicExprRPN = findRecurseRPN(subRulesLst.at(3));
            QString blockRPN = findRecurseRPN(subRulesLst.at(4));
            return " " + assigmentRPN + " " + expr1RPN + " " + expr2RPN + " " + logicExprRPN + " " + blockRPN + " ";
        }
        }

        return QString();
    }
//    static QString RPNForRule(ASTNode::SharedPtr &subToken, QList<ASTNode::SharedPtr> &subRulesLst){
//        QString ruleRpn;
//        Token subRuleType = subToken->getType();
//        switch (subRuleType) {
//        case Token::Add:
//        case Token::Sub:
//        case Token::Mul:
//        case Token::Div:
//        case Token::Power:
//        case Token::Assign:
//        case Token::Less:
//        case Token::More:
//            return subToken->getText() + "\n";
//        }

//        return QString();
//    }

    static QString findRecurseRPN(ASTNode::SharedPtr rule){
        if(rule->getType() == Token::IDENT || rule->getType() == Token::NUMBER)
            return rule->getText();

        for(const auto &child : synataxTree){
            if(child.toMap()["id"] == rule->getUniqueName().split("node")[1])
                return child.toMap()["polish"].toString();
        }
        return "NOT_FOUND";
    }

private:
    void defineEachCurrentVar(){
        for(auto &var : getSourceRef()){
            QVariantMap varMap = var.toMap();
            if(varMap["value"] == lexemeValue()){//get not defined variable
                varMap["varType"] = "int";
                var = varMap; //set type of variable, so it is defined
                //DEBUGSYNTX(lexemeValue() <<var.toMap()["varType"])
            }
        }
    }

};

}

#endif // SYNTACTICALANALYZER_H
