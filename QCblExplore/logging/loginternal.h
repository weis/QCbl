#ifndef LOGINTERNAL_H
#define LOGINTERNAL_H

#include <QMessageLogger>

class LogInternal
{
public:
    LogInternal();

private:
    bool logInit();
    bool checkDirectory(const QString& dir);

};

#endif // LOGINTERNAL_H
