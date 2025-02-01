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

#ifndef CLAMAV_CTRL_H
#define CLAMAV_CTRL_H

#include <QSystemTrayIcon>
#include <QDragEnterEvent>
#include <QMessageBox>
#include <QDropEvent>
#include <QMimeData>
#include <QWidget>
#include <QMenu>
#include <QUrl>
#include <QDir>

namespace Ui {
class clamav_ctrl;
}

class clamav_ctrl : public QWidget
{
    Q_OBJECT

public:
    explicit clamav_ctrl(QWidget *parent = 0);
    ~clamav_ctrl();

private:
    Ui::clamav_ctrl *ui;
    void initWidget();
    void dragEnterEvent(QDragEnterEvent * event);
    void dropEvent(QDropEvent * event);

signals:
    void scanRequest(QStringList);
};

#endif // CLAMAV_CTRL_H
