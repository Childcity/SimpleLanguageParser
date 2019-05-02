#ifndef REVERSEPOLISHNOTATIONBUILDER_H
#define REVERSEPOLISHNOTATIONBUILDER_H

#include "cmath"
#include "ASTree/astnode.h"

#include <QJsonDocument>

namespace Gorod {

class ReversePolishNotationBuilder{
    using Token = LangTokens::Token;
public:
    ReversePolishNotationBuilder(ASTNode::SharedPtr astTree)
        : tree(astTree)
    {}

    QVariantList Generate(){
        QList<ASTNode::SharedPtr> subRuleLst;
        forEachSubElements(tree, subRuleLst);
        return polishTree;
    }

    QByteArray toRawJson() const{
        return QJsonDocument::fromVariant(polishTree).toJson(QJsonDocument::JsonFormat::Indented);
    }

private:
    void forEachSubElements(ASTNode::SharedPtr subRule, QList<ASTNode::SharedPtr> &subRuleLst, QString tab = QString())
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
                                               , {"result",
                                                  (ch->getType() == Token::NUMBER)
                                                  ? ch->getText()
                                                  : (ch->getType() == Token::IDENT) ? ch->getText()/*Should take value of Variable from table*/: "0" }
                                           });
                childs += ch->getText() + "(" + ch->getUniqueName().split("node")[1]/*LangTokens::GetToken(ch->getType())*/ + ") ";
            }

            polishTree += QVariantMap({
                                          {  "sub_rules", subTokenLst}
                                          , {"rule", subRule->getText()}
                                          , {"id", subRule->getUniqueName().split("node")[1]}
                                          , {"polish", RPNForRule(subRule, newSubRulesLst)}
                                          , {"result", countMathExpr(subRule, newSubRulesLst)}
                                       });
            qDebug().noquote() <<tab <<"Childrens: " <<childs;
            qDebug().noquote() <<tab<<"Was Children OF:" <<subRule->getText()  << "(" <<subRule->getUniqueName().split("node")[1] << ") "<<endl;
        }

        subRuleLst.append(subRule);

    }

    QString RPNForRule(ASTNode::SharedPtr &subToken, QList<ASTNode::SharedPtr> &subRulesLst){
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
                return " " + exprRPN + " m"+id+"УПХ " + blockRPN + "m"+id+": ";
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
                QString id1 = subToken->getUniqueName().split("node")[1];           //rj
                QString id2 = subRulesLst.at(0)->getUniqueName().split("node")[1];  //rj+1
                QString id3 = subRulesLst.at(1)->getUniqueName().split("node")[1];  //rj+2
                QString id4 = subRulesLst.at(2)->getUniqueName().split("node")[1];  //mi
                QString id5 = subRulesLst.at(3)->getUniqueName().split("node")[1];  //mi+1
                QString id6 = subRulesLst.at(4)->getUniqueName().split("node")[1];  //mi+2

                QString itV = subRulesLst.at(0)->getChild(0)->getText();
                QString assigmentRPN = findRecurseRPN(subRulesLst.at(0));
                QString expr1RPN = findRecurseRPN(subRulesLst.at(1));
                QString expr2RPN = findRecurseRPN(subRulesLst.at(2));
                QString logicExprRPN = findRecurseRPN(subRulesLst.at(3));
                QString blockRPN = findRecurseRPN(subRulesLst.at(4));
                return " " + assigmentRPN + " r" + id1 + " 1= m" + id4 + ": r" +id3 + " " + expr1RPN + "= r" + id2 + " " + expr2RPN + "= r"
                        + id1 + " 0= m" + id5 + "УПХ " + itV + itV + "r" + id2 + "+= m" + id5 + ": r" + id1 + " 0= "
                        + itV + "r" + id3 + "- r" + id2 + "* 0<= " + logicExprRPN + "&&" + " m"
                        + id6 + "УПХ  " + blockRPN + "  m" + id4 + "БП m" + id6 + ":";
                //return " " + assigmentRPN + " " + expr1RPN + " " + expr2RPN + " " + logicExprRPN + " " + blockRPN + " ";
            }
        default:
            return QString();
        }
    }

    QString findRecurseRPN(ASTNode::SharedPtr rule){
        if(rule->getType() == Token::IDENT || rule->getType() == Token::NUMBER)
            return rule->getText();

        for(const auto &child : polishTree){
            if(child.toMap()["id"] == rule->getUniqueName().split("node")[1])
                return child.toMap()["polish"].toString();
        }
        return "NOT_FOUND";
    }

    QString countMathExpr(ASTNode::SharedPtr &subToken, QList<ASTNode::SharedPtr> &subRulesLst){
        Token subRuleType = subToken->getType();

        if(subRulesLst.size() != 2) //if operators are not 2 this is error? so just return 0s
            return QString();

        if(subRulesLst.at(0)->getType() == Token::IDENT
                || subRulesLst.at(1)->getType() == Token::IDENT)
            return subRulesLst.at(0)->getText()+subToken->getText()+subRulesLst.at(1)->getText();

        int leftOperRPN = findRecurseRPNRes(subRulesLst.at(0));
        int rightOperRPN = findRecurseRPNRes(subRulesLst.at(1));
        switch (subRuleType) {
            case Token::Add:
                return QString::number(leftOperRPN + rightOperRPN);
            case Token::Sub:
                return QString::number(leftOperRPN - rightOperRPN);
            case Token::Mul:
                return QString::number(leftOperRPN * rightOperRPN);
            case Token::Div:
                return QString::number(leftOperRPN / rightOperRPN);
            case Token::Power:
                return QString::number(pow(leftOperRPN, rightOperRPN));
            default:
                return QString();
        }
    }

    int findRecurseRPNRes(ASTNode::SharedPtr rule){
        if(rule->getType() == Token::NUMBER)
            return rule->getText().toInt();

        for(const auto &child : polishTree){
            if(child.toMap()["id"] == rule->getUniqueName().split("node")[1])
                return child.toMap()["result"].toInt();
        }
        return 0;
    }

private:
    QVariantList polishTree;
    ASTNode::SharedPtr tree;
};

}

#endif // REVERSEPOLISHNOTATIONBUILDER_H
