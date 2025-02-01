#ifndef SETUPTAB_H
#define SETUPTAB_H

#include <QFileSystemWatcher>
#include <QFileDialog>
#include <QProcess>
#include <QPixmap>
#include <QWidget>
#include <QMovie>
#include <QDir>
#include "setupfilehandler.h"
#include "highlighter.h"

namespace Ui {
class setupTab;
}

class setupTab : public QWidget
{
    Q_OBJECT

public:
    explicit setupTab(QWidget *parent = 0);
    ~setupTab();

private:
    Ui::setupTab *ui;
    setupFileHandler    * setupFile; // clamd && freshclam
    bool                  supressMessage;

private slots:
    void slot_selectedLanguageChanged();
    void slot_basicSettingsChanged();
    void slot_updateSystemInfo();
    void slot_clamdButtonClicked();
    void slot_clamonaccButtonClicked();
    void slot_freshclamButtonClicked();

signals:
    void switchActiveTab(int);
    void sendSystemInfo(QString);
};

#endif // SETUPTAB_H
