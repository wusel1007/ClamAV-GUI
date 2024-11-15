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

#ifndef CLAMAV_GUI_H
#define CLAMAV_GUI_H

#include <QSystemTrayIcon>
#include <QTextStream>
#include <QStringList>
#include <QPixmap>
#include <QProcess>
#include <QWidget>
#include <QAction>
#include <QTimer>
#include <QDebug>
#include <QScreen>
#include <QTimer>
#include <QMenu>
#include <QList>
#include <QDir>
#include "setupfilehandler.h"
#include "optionsdialog.h"
#include "clamav_ctrl.h"
#include "freshclamsetter.h"
#include "scantab.h"
#include "setuptab.h"
#include "scheduler.h"
#include "infodialog.h"
#include "schedulescanobject.h"
#include "logviewerobject.h"
#include "profilemanager.h"
#include "clamdmanager.h"

namespace Ui {
class clamav_gui;
}

class clamav_gui : public QWidget
{
    Q_OBJECT

public:
    explicit clamav_gui(QWidget *parent = 0);
    void closeEvent(QCloseEvent *);
    void changeEvent(QEvent *event);

    ~clamav_gui();

private:
    Ui::clamav_gui          *ui;
    QSystemTrayIcon         * trayIcon;
    QMenu                   * trayIconMenu;
    QAction                 * actionQuit;
    QAction                 * actionShowHideDropZone;
    QAction                 * actionShowHideMainWindow;
    setupFileHandler        * setupFile;
    QProcess                * scanProcess;
    QProcess                * sudoGUIProcess;
    QString                   guisudoapp;
    scanTab                 * scannerTab;
    setupTab                * setUpTab;
    QTimer                  * mainWindowTimer;
    QTimer                  * logoTimer;
    QTimer                  * showLogoTimer;
    optionsDialog           * optionTab;
    scanLimitsTab           * scanLimitTab;
    freshclamsetter         * freshclamTab;
    clamdManager            * clamdTab;
    ProfileManager          * profileManagerTab;
    scheduler               * schedulerTab;
    infoDialog              * infoTab;
    logViewerObject         * logTab;
    clamav_ctrl             * dropZone;
    QLabel                  * startLogoLabel;
    bool                      error;
    void createTrayIcon();
    void createDropZone();

private slots:
    void slot_setMainWindowState(bool);
    void slot_actionShowHideMainWindowTriggered();
    void slot_systemTrayIconActivated(QSystemTrayIcon::ActivationReason);
    void slot_actionShowHideDropZoneTriggered();
    void slot_hideWindow();
    void slot_scanRequest(QStringList);
    void slot_mainWinTimerTimeout();
    void slot_scanProcessHasStdOutput();
    void slot_scanProcessHasErrOutput();
    void slot_scanProcessFinished(int,QProcess::ExitStatus);
    void slot_abortScan();
    void slot_showDropZone();
    void slot_receiveScanJob(QString, QStringList);
    void slot_setTrayIconBalloonMessage(int,QString, QString);
    void slot_closeRequest(QWidget*);
    void slot_logoTimerTimeout();
    void slot_showLogoTimerTimeout();
    void slot_errorReporter();
    void slot_updateDatabase();
    void slot_startclamd();
    void slot_sudoGUIProcessFinished();
    void slot_switchActiveTab(int index);
    void slot_quitApplication();

signals:
    void showHideDropZoneTriggered();
    void setScannerForm(bool);
    void scanJobFinished();
    void startDatabaseUpdate();
};

#endif // CLAMAV_GUI_H
