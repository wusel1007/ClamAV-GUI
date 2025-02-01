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

#ifndef PARTIALLOGOBJECT_H
#define PARTIALLOGOBJECT_H

#include <QMessageBox>
#include <QWidget>
#include "highlighter.h"

namespace Ui {
class partialLogObject;
}

class partialLogObject : public QWidget
{
    Q_OBJECT

public:
    explicit partialLogObject(QWidget *parent = 0, QString logText = "");
    ~partialLogObject();
    QString getLogText();

private:
    Ui::partialLogObject *ui;
    highlighter       *logHighlighter;
    int                  start;
    int                  errorStart;
    long                 infectedStart;
    void setLogText(QString);

private slots:
    void slot_searchButtonClicked();
    void slot_searchLineEditChanged();
    void slot_clearLineEditButtonClicked();
    void slot_totalErrorButtonClicked();
    void slot_infectedFilesButtonClicked();
};

#endif // PARTIALLOGOBJECT_H
