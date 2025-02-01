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

#ifndef DRAGABLEPUSHBUTTON_H
#define DRAGABLEPUSHBUTTON_H

#include <QPushButton>
#include <QMimeData>
#include <QUrl>
#include <QDrag>

namespace Ui {
class dragablePushButton;
}

class dragablePushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit dragablePushButton(QWidget *parent = 0);
    dragablePushButton(QIcon, QString, QWidget *, QString);
    ~dragablePushButton();
    void mouseMoveEvent(QMouseEvent *);

private:
    Ui::dragablePushButton *ui;
    QString dragDropText;

signals:
    void dragStarted();

};

#endif // DRAGABLEPUSHBUTTON_H
