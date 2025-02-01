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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <QButtonGroup>
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QDateTime>
#include <QWidget>
#include <QTimer>
#include <QDir>
#include "setupfilehandler.h"
#include "logviewobject.h"

namespace Ui {
class scheduler;
}

class scheduler : public QWidget
{
    Q_OBJECT

public:
    explicit scheduler(QWidget *parent = 0);
    ~scheduler();

private:
    Ui::scheduler *ui;
    setupFileHandler    *setupFile;
    QButtonGroup        *removeButtonGroup;
    QButtonGroup        *scanNowButtonGroup;
    QButtonGroup        *logButtonGroup;
    QTimer              *checkTimer;
    void updateScheduleList();
    void startScanJob(QString);

private slots:
    void slot_addDailyScanJobButtonClicked();
    void slot_addWeeklyScanJobButtonClicked();
    void slot_addMonthlyScanJobButtonClicked();
    void slot_updateProfiles();
    void slot_scanButtonClicked(int);
    void slot_removeButtonClicked(int);
    void slot_logButtonClicked(int);
    void slot_checkTimerTimeout();
    void slot_profileSelectionChanged();
    void slot_logChanged();
    void slot_disableScheduler();

signals:
    void triggerScanJob(QString name, QStringList parameters);
    void logChanged();
};

#endif // SCHEDULER_H
