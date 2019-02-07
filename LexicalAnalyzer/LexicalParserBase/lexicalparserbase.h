#ifndef PARSERBASE_H
#define PARSERBASE_H

#include "main.h"
#include "gorodlangexception.h"
#include <QStringList>

namespace Gorod {


class LexicalParserBaseException : public Exception {
public:
    explicit LexicalParserBaseException(const QString msg, int line, int pos)
        : Exception(msg)
        , line_(line)
        , pos_(pos){}

    LexicalParserBaseException( const LexicalParserBaseException & ) = default;

    virtual ~LexicalParserBaseException() override;

    QString what() const override{
        return QString("Error[Lexical]:: %1").arg(msg_);
    }

    int getPos() const override {return pos_;}

    int getLine() const override {return line_;}

protected:
    int line_;
    int pos_;
};

class LexicalParserBase {
public:
    // незначащие символы - пробельные символы по умолчанию
    const QString DEFAULT_WHITESPACES = " \n\r\t";

public:
    explicit LexicalParserBase(const QString &source)
        : source_(source)
    {}

    virtual ~LexicalParserBase();

    QString getSource() const { return source_; }

    int getPos() const { return pos_; }

    int getPosInLine(int pos = -1) const { return (pos > 0 ? pos : pos_) - source_.midRef(0, (pos > 0 ? pos : pos_)).lastIndexOf('\n'); }

    int getLine() const { return line_; }

    // символ в текущей позиции указателя
    const QChar getCurrentSymbol() const { return at(pos_);}

    bool isEnd() const { return getCurrentSymbol() == QChar(0) || getCurrentSymbol() == ""; }

    // передвигает указатель на один символ
    void next() {
        if (! isEnd())
            pos_++;
    }

    // пропускает незначащие (пробельные) символы
    virtual void skip() {
        while (DEFAULT_WHITESPACES.indexOf(at(pos_)) >= 0){
            if(at(pos_) == '\n'){
                ++line_;
            }
            next();
        }
    }

    // проверяет, можно ли в текущей позиции указателя, распознать
    // одну из строк; указатель не смещается
    bool isMatch(const QStringList &terms) {
        int pos = pos_;
        const QString result = matchNoExcept(terms);
        pos_ = pos;
        return ! result.isNull();
    }

    bool isMatch(const QString &term) {
        return isMatch(QStringList() <<term);
    }

    bool isMatch(const QString &term1, const QString &term2) {
        return isMatch(QStringList() <<term1 <<term2);
    }

    // распознает одну из строк; при этом указатель смещается и
    // пропускаются незначащие символы; если ни одну из строк
    // распознать нельзя, то выбрасывается исключение
    QString match(const QStringList &terms) {
        int pos = pos_;
        const QString result = matchNoExcept(terms);
        if (result.isNull()) {
            QString message = "Ожидалась одна из строк: ";
            bool first = true;
            for (auto str : terms) {
                if (! first){
                    message += ", ";
                }
                message += QString("\"%1\"").arg(str);
                first = false;
            }
            DEBUGlex("match fail"<<getCurrentSymbol())
            throw LexicalParserBaseException(QString("%1 (pos=%2)").arg(message).arg(getPosInLine(pos))
                                             , getLine(), getPosInLine(pos));
        }
        return result;
    }

    // то же, что и Match(params string[] a), для удобства
    QString match(const QString &term1, const QString &term2) {
        return match(QStringList() <<term1 <<term2);
    }

    // то же, что и Match(params string[] a), для удобства
    QString match(const QString &s) {
    int pos = pos_;
        try {
            return match(QStringList()<<s);
        }catch(...){
        DEBUGlex("match fail (wait on symb/str)"<<getCurrentSymbol())
            throw LexicalParserBaseException(QString("%1: '%2' (pos=%3)")
                        .arg(s.length() == 1 ? "Ожидался символ" : "Ожидалась строка")
                        .arg(s).arg(getPosInLine(pos))
                    , getLine(), getPosInLine(pos)
                );
        }
    }

protected:
    // предотвращает возникновение ошибки обращения за пределы
    // массива; в этом случае возвращает (char) 0,
    // что означает конец входной строки
    const QChar at(int index) const {
        return index < source_.length() ? source_[index] : QChar(0);
    }

    // распознает одну из строк; при этом указатель смещается и
    // пропускаются незначащие символы;
    // если ни одну из строк распознать нельзя, то возвращается Null
    QString matchNoExcept(const QStringList &terms) {
        int pos = pos_;
        for (auto str : terms) {
            bool match = true;

            for (auto ch : str){
                if (getCurrentSymbol() != ch){
                    pos_ = pos;
                    match = false;
                    break;
                }

                next();
            }

            if (match) {
                // после разбора терминала пропускаем незначащие символы
                skip();
                return str;
            }
        }

        return QString();
    }

private:

// разбираемая строка
QString source_;

// позиция указателя
// (указывает на первый символ неразобранной части вход. строки)
int pos_ = 0;

// счетчик текущей строки
int line_ = 1;

};
}

#endif // PARSERBASE_H
