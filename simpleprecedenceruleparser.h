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
        : rulesLst(rulesEBNF.replace("\t","").replace("\n","").replace("\r","").trimmed().simplified().split("."))
    {

        for(auto it : rulesLst){
            if(it.isEmpty())
                continue;

            rulesMap[it.mid(0, it.indexOf(" =")).trimmed()] = it.mid(it.indexOf(" =") + 3).trimmed();
        }

        auto it = QMapIterator<QString, QString>(rulesMap);
        while (it.hasNext()) {
            it.next();
            DEBUGRl(it.key()<<it.value());
        }
    }

private:
    QStringList rulesLst;
    QMap<QString, QString> rulesMap;
};

}

#endif // SIMPLEPRECEDENCERULEPARSER_H
