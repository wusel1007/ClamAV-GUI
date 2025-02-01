#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QStringList>
#include <QString>

class translator
{
public:
    translator(QString lang);
    QString translateit(QString original);
    QString languageset;
};

#endif // TRANSLATOR_H
