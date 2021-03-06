#ifndef MAIN_H
#define MAIN_H

#include <QDebug>

//#define DEBUGAST(msg) qDebug()<<__FILE__<<" ["<<__LINE__<<"]: "<<msg;
#define DEBUGAST(msg){}

#define DEBUGM(msg) qDebug()<<__FILE__<<" ["<<__LINE__<<"]: "<<msg;

#define DEBUGRl(msg) qDebug().noquote()<<__FILE__<<" ["<<__LINE__<<"]: "<<msg;
//#define DEBUGRl(msg){}

//#define DEBUGlex(msg) qDebug()<<__FILE__<<" ["<<__LINE__<<"]: "<<msg;
#define DEBUGlex(msg){}

//#define DEBUGSYNTX(msg) qDebug()<<__FILE__<<" ["<<__LINE__<<"]: "<<msg;
#define DEBUGSYNTX(msg){}

#define DEBUGEXE(msg) qDebug()<<__FILE__<<" ["<<__LINE__<<"]: "<<msg;
//#define DEBUGEXE(msg){}

#endif // MAIN_H
