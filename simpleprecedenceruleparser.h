#ifndef SIMPLEPRECEDENCERULEPARSER_H
#define SIMPLEPRECEDENCERULEPARSER_H

#include "main.h"
#include "gorodlangexception.h"

#include <QStringList>

namespace Gorod {

class SimplePrecedenceRuleParserEx : public Exception {
public:
    explicit SimplePrecedenceRuleParserEx(const QString msg)
        : Exception(msg)
    {}

    SimplePrecedenceRuleParserEx( const SimplePrecedenceRuleParserEx & ) = default;

    virtual ~SimplePrecedenceRuleParserEx() override;

    QString what() const override{
        return QString("Error[RuleParser]:: %1").arg(msg_);
    }
};

class SimplePrecedenceRuleParser{
public:
    explicit SimplePrecedenceRuleParser(QString rulesEBNF)
        : rulesLst(rulesEBNF.simplified().replace("\t","").replace("\n","").replace("\r","").split("."))
    {
        for(auto it : rulesLst){
            DEBUGRl(it.toLatin1().data())
        }
    }

private:
    QStringList rulesLst;
};

}

#endif // SIMPLEPRECEDENCERULEPARSER_H
