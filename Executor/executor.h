#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "cmath"
#include "ASTree/astnode.h"

namespace Gorod {

class Executor: public QObject {
    Q_OBJECT
    using Token = LangTokens::Token;
    QMap<QString, qint64> globalVarMap;

signals:
    void sigReadText(qint64 &);

public:

    explicit Executor();

    void exec(ASTNode::SharedPtr subTree){
        if(subTree->getChildsCount() <= 0)
            return;

        for (int i = 0; i < subTree->getChildsCount(); i++) {
            ASTNode::SharedPtr child = subTree->getChild(i);
            switch (child->getType()) {
                case Token::LeftBraket: exec(child); break;
                case Token::Int: execVarInit(child); break;
                case Token::Assign: execAssignment(child); break;
                case Token::If: execIf(child); break;
                case Token::Read: execRead(child); break;
                default:
                  break;
            }
        }
    }

private:
    void execVarInit(ASTNode::SharedPtr variablesTree){
        for (int i = 0; i < variablesTree->getChildsCount(); i++) {
            globalVarMap[variablesTree->getChild(i)->getText()] = 0;
        }
        showVariables();
    }

    void execAssignment(ASTNode::SharedPtr assignTree){
        globalVarMap[assignTree->getChild(0)->getText()] = countExpr(assignTree->getChild(1)).toLongLong();
        showVariables();
    }

    void execIf(ASTNode::SharedPtr ifTree){
        if(countExpr(ifTree->getChild(0)).toBool()){
            exec(ifTree->getChild(1));
        }
    }

    void execRead(ASTNode::SharedPtr readTree){
        qint64 number;
        emit sigReadText(number);
        globalVarMap[readTree->getChild(0)->getText()] = number;
    }







    void showVariables(){
        QString vars;
        for (auto it = globalVarMap.constKeyValueBegin(); it != globalVarMap.constKeyValueEnd(); it++) {
            vars.append(it.base().key() + ":" + QString("%1 ").arg(it.base().value()));
        }
        DEBUGEXE(vars);
    }

    QVariant countExpr(ASTNode::SharedPtr exprTree){
        Token opType = exprTree->getType();

        if(opType == Token::More){
            return globalVarMap[exprTree->getChild(0)->getText()] > countMathExpr(exprTree->getChild(1));
        }else if(opType == Token::Less){
            return globalVarMap[exprTree->getChild(0)->getText()] < countMathExpr(exprTree->getChild(1));
        }else{
            return countMathExpr(exprTree);
        }
    }

    qint64 countMathExpr(ASTNode::SharedPtr mathTree){
        Token opType = mathTree->getType();

        if(opType == Token::NUMBER){
            return mathTree->getText().toLongLong();
        }else if(opType == Token::IDENT){
            return globalVarMap[mathTree->getText()];
        }

        if(mathTree->getChildsCount() != 2){ //if operators are not 2 this is error? so just return 0s
            DEBUGEXE("!!! More then 2 childrens !!!");
            for (int i = 0; i < mathTree->getChildsCount(); i++) {
                DEBUGEXE(mathTree->getChild(i)->getText());
            }
            return 0;
        }

        qint64 leftOperRPN = countMathExpr(mathTree->getChild(0));
        qint64 rightOperRPN = countMathExpr(mathTree->getChild(1));
        switch (opType) {
            case Token::Add:
                return leftOperRPN + rightOperRPN;
            case Token::Sub:
                return leftOperRPN - rightOperRPN;
            case Token::Mul:
                return leftOperRPN * rightOperRPN;
            case Token::Div:
                return leftOperRPN / rightOperRPN;
            case Token::Power:
                return pow(leftOperRPN, rightOperRPN);
            default:
                DEBUGEXE("!!! Unrecognized Math Operation !!!");
                return 0;
        }
    }
};

}

#endif // EXECUTOR_H
