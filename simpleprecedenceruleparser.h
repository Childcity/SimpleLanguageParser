#ifndef SIMPLEPRECEDENCERULEPARSER_H
#define SIMPLEPRECEDENCERULEPARSER_H

#include "main.h"
#include "gorodlangtokens.h"
#include "gorodlangexception.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>
#include <QFile>
#include <QStack>

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
    explicit SimplePrecedenceRuleParser(QString rulesRawJson)
        : rulesLst(QJsonDocument::fromJson(rulesRawJson.toLatin1()).toVariant().toList())
    {}

    void Parse(){
        // find all "=." relations
        for(const auto &curRule : rulesLst){
            std::function<void(const QVariantList&)> recursePutRules = [&](const QVariantList &ruleArr) mutable {
                for (auto ruleElemIt = ruleArr.cbegin(); ruleElemIt != ruleArr.cend(); ++ruleElemIt){
                    if(ruleElemIt->userType() == QMetaType::QVariantList){
                        recursePutRules(ruleElemIt->toList());
                        continue;
                    }

                    // if exist next rule
                    if((ruleElemIt + 1) != ruleArr.end() && (! (*(ruleElemIt + 1)).isNull())){
                        putRelation(getRuleElemValue(*ruleElemIt), {{getRuleElemValue(*(ruleElemIt + 1)), " ="}});
                    }
                }
            };

            recursePutRules(getRuleArr(curRule));
        }

        // find all "<." and ">." relations. (FIRST+/LAST+)
        for(const auto &curRule : rulesLst){

            QList<QString> currRuleMemory;

            std::function<void(const QVariantList, bool)> recursePutRules = [&](const QVariantList ruleArr, bool isFindFirst){
                for (auto ruleElemIt = ruleArr.cbegin(); ruleElemIt != ruleArr.cend(); ++ruleElemIt){

                    if(ruleElemIt->userType() == QMetaType::QVariantList){
                        recursePutRules(ruleElemIt->toList(), isFindFirst);
                        continue;
                    }

                    // if First/Last is not terminal -> recursive go into
                    const auto firstOrLastArrElem = isFindFirst ? ruleArr.first().toMap() : ruleArr.last().toMap();
                    QString subRuleName = firstOrLastArrElem["value"].toString();
                    putRelation(getRuleName(curRule), {{subRuleName, isFindFirst ? "F" : "L"}});

                    if(isTerminal(firstOrLastArrElem)){
                        currRuleMemory <<subRuleName;
                        return;
                    }else{
                        if(checkOnInfiniteRecursion(subRuleName, currRuleMemory)){
                            continue;
                        }

                        currRuleMemory <<subRuleName;
                        recursePutRules(findRuleValueByName(subRuleName), isFindFirst);
                    }
                }
            };

            recursePutRules(getRuleArr(curRule), true); //Find FIRST+
            transformToFirstPlus();
            recursePutRules(getRuleArr(curRule), false); //Find LAST+
            transformToLastPlus();
        }
    }

    QString toJson() const {
        return QJsonDocument::fromVariant(relationsLst).toJson(QJsonDocument::JsonFormat::Indented);
    }

    QString toCSVTable() {
        QStringList tokenRuleColLst;
        QByteArray buff;
        QTextStream csv(&buff, QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
        csv.setFieldAlignment(QTextStream::AlignCenter);
        csv.setFieldWidth(0);

        for(const auto &curRule : rulesLst){
            tokenRuleColLst <<getRuleName(curRule);
        }
        for(const auto &curToken : Gorod::LangTokens::GetToken()){
            tokenRuleColLst <<curToken;
        }

        std::sort(relationsLst.begin(), relationsLst.end(), [this](const QVariant &l,const QVariant &r){return getRuleName(l)<getRuleName(r);});
        tokenRuleColLst.sort();

        // Build table HEADER
        csv << right << ";";
        for(const auto &curRuleToken : tokenRuleColLst){
            csv << (curRuleToken == ";" ? "!" : curRuleToken) << right << ";";
        }
        csv << endl;

        for(const auto &curRule : relationsLst){
            QString curRuleName = getRuleName(curRule);
            csv << left << (curRuleName == ";" ? "!" : curRuleName) << right << ";";

            const QVariantMap curRulMap = curRule.toMap().begin().value().toMap();
            for(auto it : tokenRuleColLst){
                if(curRulMap.contains(it)){
                    csv << curRulMap[it].toString().remove("F").remove("L");
                }
                csv << right << ";";
            }
            csv << endl;
        }

        return buff;
    }

    QVariantList getRelations() const {
        return relationsLst;
    }

private:
    QString getRuleName(const QVariant &ruleObj) const {
        return ruleObj.toMap().firstKey();
    }

    QVariantList getRuleArr(const QVariant &ruleObj) const {
        return ruleObj.toMap().first().toList();
    }

    bool isTerminal(const QVariant &ruleElem) const {
        return (ruleElem.toMap()["type"] == "term");
//                || (ruleElem.toMap()["}isTerminal"].toBool() == true);
    }

    bool isTerminal(const QString &ruleName) const {
        //ruleNamesFromColIt[0].isUpper() && ruleNamesFromColIt != "Ident" && ruleNamesFromColIt != "Number"
        for(const auto &curRule : rulesLst){
            if(getRuleName(curRule) == ruleName){
                return false;
            }
        }

        return true;
    }

    QString getRuleElemValue(const QVariant &ruleElem) const {
        return ruleElem.toMap()["value"].toString();
    }

    void putRelation(const QString &rule, const QVariantMap &relation){
        // find relation
        for(auto &ruleIt : relationsLst){
            if(getRuleName(ruleIt) == rule){
                //DEBUGRl(QJsonDocument::fromVariant(ruleIt).toJson(QJsonDocument::JsonFormat::Indented));
                auto existedRuleReletion = static_cast<QVariantMap*>(
                                // get link to first element in map and convert it to QVarianMap
                                static_cast<QVariantMap*>(ruleIt.data())->first().data()
                            );

                // each new relation we add to existed map
                for (auto relElemIt = relation.cbegin(); relElemIt != relation.cend(); ++relElemIt){
                    QString newValue;
                    if(! existedRuleReletion->contains(relElemIt.key())){
                        newValue = relElemIt->toString();
                    }else{
                        //if exist end str != str
                        QString existedRuleReletionVal = existedRuleReletion->value(relElemIt.key()).toString();
                        QString newReletionVal = relElemIt->toString();
                        if(existedRuleReletionVal.contains(newReletionVal)){
                            continue;
                        }

                        newValue = existedRuleReletionVal + newReletionVal;
                    }

                    existedRuleReletion->insert(relElemIt.key(), newValue);
                }
                //bDEBUGRl(QJsonDocument::fromVariant(ruleIt).toJson(QJsonDocument::JsonFormat::Indented));
                return;
            }
        }

        // create new rule, if we can't find rule in list
        relationsLst += QVariantMap({
                                        {rule, relation}
//                                        {"}isTerminal", isTerminal}
                                    });
    }

    QVariantList findRuleValueByName(const QString &ruleToFind){
        for(const auto &curRule : rulesLst){
            if(getRuleName(curRule) == ruleToFind){
                return getRuleArr(curRule);
            }
        }

        throw std::logic_error("ERROR: " + ruleToFind.toStdString() + " not found!");
    }

    bool checkOnInfiniteRecursion(const QString &rule, const QList<QString> &memory){
        if(memory.size() >= 4){
            //DEBUGRl(memory.at(memory.size() - 1) <<memory.at(memory.size() - 2)<< memory);
            if((memory.at(memory.size() - 2) == "Expr")
                    && (memory.at(memory.size() - 1) == "CondExpr")
                    && (rule == "Expr")){
                return true;
            }
        }
        return false;
    }

    QVariantMap matchRuleRelByName(const QString &ruleName) const {
        for(const auto &ruleIt : relationsLst){
            if(getRuleName(ruleIt) == ruleName){
                return ruleIt.toMap();
            }
        }
        return QVariantMap();
    }


    QStringList findFirstPlusReletions(const QString &ruleName){
        QStringList firstPlus;
        QStringList currRuleMemory;

        std::function<void(const QVariantList)> recurseFindFirst = [&](const QVariantList ruleArr){
            for (auto ruleElemIt = ruleArr.cbegin(); ruleElemIt != ruleArr.cend(); ++ruleElemIt){

                if(ruleElemIt->userType() == QMetaType::QVariantList){
                    recurseFindFirst(ruleElemIt->toList());
                    continue;
                }

                const auto firstArrElem = ruleArr.first().toMap();
                QString subRuleName = firstArrElem["value"].toString();
                firstPlus << subRuleName;
                //putRelation(getRuleName(curRule), {{subRuleName, isFindFirst ? "F" : "L"}});

                // if First is not terminal -> recursive go into
                if(isTerminal(firstArrElem)){
                    currRuleMemory << subRuleName;
                    return;
                }else{
                    if(checkOnInfiniteRecursion(subRuleName, currRuleMemory)){
                        continue;
                    }

                    currRuleMemory <<subRuleName;
                    recurseFindFirst(findRuleValueByName(subRuleName));
                }
            }
        };

        recurseFindFirst(findRuleValueByName(ruleName));

        return firstPlus;
    }

    void transformToFirstPlus(){
        // for each rule we try to find FIRST+ relation
        for(auto &ruleIt : relationsLst){
            auto existedRuleReletion = static_cast<QVariantMap*>(
                            // get link to first element in map and convert it to QVarianMap
                            static_cast<QVariantMap*>(ruleIt.data())->first().data()
                        );

            QStringList ruleNamesFromCol;

            // find for current rule all subrules, that is not a terminal and append them to 'nonTerminalsFromCol' list
            for (auto relElemIt = existedRuleReletion->cbegin(); relElemIt != existedRuleReletion->cend(); ++relElemIt){
                QString relElemKey = relElemIt.key();
                QString relElemVal = relElemIt.value().toString();
                if(relElemVal.contains("="))// && relElemKey[0].isUpper() && relElemKey != "Ident" && relElemKey != "Number")
                    ruleNamesFromCol <<relElemKey;
            }

            // for each element from column, that is in 'ruleNamesFromCol' we find rule in 'relationsLst'
            for (const auto &ruleNamesFromColIt : ruleNamesFromCol){

                // in subrule of founded rule we find relation 'First+(subrule)' if exist
                QVariantMap mathedRule = matchRuleRelByName(ruleNamesFromColIt);
                if(mathedRule.isEmpty())
                    continue;

                QVariantMap currRuleReletionMap = mathedRule.first().toMap();

                for (auto relElemIt = currRuleReletionMap.cbegin(); relElemIt != currRuleReletionMap.cend(); ++relElemIt){
                    QString relElemKey = relElemIt.key();
                    QString relElemVal = relElemIt.value().toString();
                    QString existedRelElemVal = existedRuleReletion->value(relElemKey).toString();

                    if(relElemVal.contains("F") && (! existedRelElemVal.contains("<"))){
                        // add "<" relation to relationsLst
                        QString newRelationValue = existedRelElemVal + "<";
                        //DEBUGRl(getRuleName(ruleIt) <<relElemKey<<relElemVal <<"OldCell:"<<existedRuleReletion->key(relElemKey) <<"NewCell:" <<newRelationValue);
                        existedRuleReletion->insert(relElemKey, newRelationValue);
                        //DEBUGRl(existedRuleReletion->keys().first() <<currReletionMap.keys().first());
                    }
                }
            }

        }
    }

    void transformToLastPlus(){
        // for each rule we try to find LAST+ relation
        for(auto &ruleIt : relationsLst){
            auto existedRuleReletion = static_cast<QVariantMap*>(
                            // get link to first element in map and convert it to QVarianMap
                            static_cast<QVariantMap*>(ruleIt.data())->first().data()
                        );

            QStringList ruleNamesFromCol, ruleNamesWithLast;

            // find for current rule all subrules, that have relation '=' and 'L'
            for(auto relElemIt = existedRuleReletion->cbegin(); relElemIt != existedRuleReletion->cend(); ++relElemIt){
                QString relElemKey = relElemIt.key();
                QString relElemVal = relElemIt.value().toString();
                if(relElemVal.contains("="))
                    ruleNamesFromCol <<relElemKey;
                if(relElemVal.contains("L"))
                    ruleNamesWithLast <<relElemKey;
            }

            for(const auto &ruleNamesWithLastIt : ruleNamesWithLast){
                for(const auto &ruleNamesFromColIt : ruleNamesFromCol){

                    if(isTerminal(ruleNamesFromColIt)){
                        // for terminal. (LAST+(R) '>' S, S - terminal)
                        putRelation(ruleNamesWithLastIt, {{ruleNamesFromColIt, ">"}});
                    }else{
                        // for non terminal. (LAST+(R) '>' FIRST+(S), S - non terminal)
                        for(const auto &firstPlusIt : findFirstPlusReletions(ruleNamesFromColIt)){
                            putRelation(ruleNamesWithLastIt, {{firstPlusIt, ">"}});
                        }
                    }

                }
            }

        }
    }

private:
    QVariantList rulesLst;
    QVariantList relationsLst;
};

}

#endif // SIMPLEPRECEDENCERULEPARSER_H
