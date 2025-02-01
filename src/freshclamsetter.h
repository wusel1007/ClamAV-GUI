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

#ifndef FRESHCLAMSETTER_H
#define FRESHCLAMSETTER_H

#include <QWidget>
#include <QMovie>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileSystemWatcher>
#include <QProcess>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include "setupfilehandler.h"
#include "highlighter.h"
#include "progressdialog.h"

namespace Ui {
class freshclamsetter;
}

class freshclamsetter : public QWidget
{
    Q_OBJECT

public:
    explicit freshclamsetter(QWidget *parent = 0);
    ~freshclamsetter();

private:
    Ui::freshclamsetter *ui;
    bool                startup;
    setupFileHandler    *setupFile;
    setupFileHandler    *freshclamConf;
    bool                 lockFreshclamConf;
    QProcess            * getDBUserProcess;
    QProcess            * freshclamLocationProcess;
    QProcess            * clamscanLocationProcess;
    QString               freshclamlocationProcessOutput;
    QString               clamscanlocationProcessOutput;
    QProcess            *updater;
    QProcess            *startDeamonProcess;
    QProcess            *ps_process;
    QString              pidFile;
    QString              logFile;
    QString              sudoGUI;
    QTimer              *startDelayTimer;
    QTimer              *processWatcher;
    progressDialog      *busyLabel;
    QFileSystemWatcher  *pidFileWatcher;
    QFileSystemWatcher  *logFileWatcher;
    QFileSystemWatcher  *updateLogFileWatcher;
    highlighter         *updateLogHighLighter;
    highlighter         *freshclamLogHighLighter;
    int                 freshclamStartupCounter;
    void setForm(bool);
    void setUpdaterInfo();
    void checkDaemonRunning();
    void initFreshclamSettings();
    QString extractPureNumber(QString value);

private slots:
    void slot_updateNowButtonClicked();
    void slot_startStopDeamonButtonClicked();
    void slot_updaterFinished(int);
    void slot_fileSystemWatcherTriggered();
    void slot_logFileWatcherTriggered();
    void slot_updateFileWatcherTriggered();
    void slot_clearLogButtonClicked();
    void slot_clearDeamonLogButtonClicked();
    void slot_updaterHasOutput();
    void slot_startDeamonProcessFinished(int exitCode,QProcess::ExitStatus exitStatus);
    void slot_ps_processFinished(int rc);
    void slot_disableUpdateButtons();
    void slot_startDelayTimerExpired();
    void slot_runasrootCheckBoxChanged();
    void slot_writeFreshclamSettings();
    void slot_dbPathChanged(QString dbPath);
    void slot_getDBUserProcessFinished();
    void slot_pidFileSelectButtonClicked();
    void slot_freshclamLocationProcessFinished();
    void slot_freshclamLocationProcessHasOutput();
    void slot_clamscanLocationProcessFinished();
    void slot_clamscanLocationProcessHasOutput();
    void slot_setFreshclamsettingsFrameState(bool state);
    void slot_autoStartDaemon();
    void slot_onUpdateExecuteButtonClicked();
    void slot_onErrorExecuteButtonClicked();
    void slot_onOutdatedExecuteButtonClicked();
    void slot_processWatcherExpired();

signals:
    void setBallonMessage(int, QString,QString);
    void disableUpdateButtons();
    void reportError();
    void updateDatabase();
    void freshclamStarted();
    void systemStatusChanged();
    void quitApplication();

};

#endif // FRESHCLAMSETTER_H
