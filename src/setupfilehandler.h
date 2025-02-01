/***************************************************************************
 *   Copyright (C) 2015 by Joerg Zopes                                     *
 *   joerg.zopes@gmx.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef SETUPFILEHANDLER_H
#define SETUPFILEHANDLER_H

#include <QMessageBox>
#include <QFileDevice>
#include <QTextStream>
#include <QStringList>
#include <QString>
#include <QFile>
#include <QDir>

class setupFileHandler
{
public:
    setupFileHandler();
    setupFileHandler(QString filename = "");
    void setSetupFileName(QString filename);
    void setSectionValue(QString section, QString keyword, const char* value, bool overwrite = true);
    void setSectionValue(QString section, QString keyword, QString value, bool overwrite = true);
    void setSectionValue(QString section, QString keyword, bool tempValue, bool overwrite = true);
    void setSectionValue(QString section, QString keyword, qint64 tempValue, bool overwrite = true);
    void setSectionValue(QString section, QString keyword, double tempValue, bool overwrite = true);
    void setSectionValue(QString section, QString keyword, int tempValue, bool overwrite = true);
    void removeSection(QString section);
    void removeKeyword(QString section, QString keyword);
    void clearSetupFile();
    QStringList getSectionNames();
    QStringList getSectionNames(QStringList excludeList);
    QStringList getSectionNames(QString excludeString);
    QStringList getKeywords(QString section);
    QString getSetupFileName();
    QString getSectionValue(QString section, QString keyword);
    bool getSectionBoolValue(QString section, QString keyword);
    qint64 getSectionIntValue(QString section, QString keyword);
    double getSectionDoubleValue(QString section, QString keyword);
    QString getFreeFloaterValue(QString keyword);
    void setFreeFloaterValue(QString keyword, QString value, bool overwrite = true);
    bool sectionExists(QString);
    bool keywordExists(QString section, QString keyword);
    bool freeFloaterExists(QString keyword);
    QString getSingleLineValue(QString keyword);
    void setSingleLineValue(QString keyword, QString value);
    void addSingleLineValue(QString keyword, QString value);
    void removeSingleLine(QString keyword, QString value);
    bool singleLineExists(QString keyword);

private:
    QString setupFileName;
    QString setupFileContent;
    QString getSection(QString sectionID);
    void readSetupFile();
    void writeSetupFile();
};

#endif // SETUPFILEHANDLER_H
