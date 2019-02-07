#ifndef GORODLANGEXCEPTION_H
#define GORODLANGEXCEPTION_H

#include <QString>

namespace Gorod {

class Exception{
public:
    explicit Exception(const QString &msg)
        : msg_(msg){}

    Exception( const Exception & ) = default;

    virtual ~Exception();

    virtual QString what() const { return msg_; }

    virtual int getPos() const {return -1;}

    virtual int getLine() const {return -1;}

protected:
    QString msg_;
};

}




#endif // GORODLANGEXCEPTION_H
