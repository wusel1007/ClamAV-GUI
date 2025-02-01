#ifndef PROFILEWIZARDDIALOG_H
#define PROFILEWIZARDDIALOG_H

#include <QFileSystemWatcher>
#include <QFileSystemModel>
#include <QButtonGroup>
#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QDialog>
#include <QDebug>
#include <QList>
#include <QDir>
#include <QFile>
#include "cfilesystemmodel.h"
#include "dragablepushbutton.h"
#include "setupfilehandler.h"
#include "scanoption.h"
#include "scanoptionyn.h"

namespace Ui {
class ProfileWizardDialog;
}

class ProfileWizardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProfileWizardDialog(QWidget *parent = 0, QString name = "");
    ~ProfileWizardDialog();

private:
    QString                  profileName;
    Ui::ProfileWizardDialog *ui;
    CFileSystemModel        * model;
    QProcess                * whoamiProcess;
    QProcess                *getClamscanParametersProcess;
    QString                  getClamscanProcessOutput;
    QFileSystemWatcher      * fileSystemWatcher;
    QButtonGroup            * deviceGroup;
    QString                   setupFileFilename;
    setupFileHandler        * setupFile;
    int                      buttonID;
    bool                     newProfile;
    QString                  username;
    QStringList              devices;
    void readSettings();
    void closeEvent(QCloseEvent *);

private slots:
    void slot_previousButtonClicked();
    void slot_nextButtonClicked();
    void slot_checkNavigationButtons();
    void slot_enablePUACheckBoxActivated();
    void slot_homeButtonClicked();
    void slot_rootButtonClicked();
    void slot_listChanged();
    void slot_loadAllSupportedDBButtonClicked();
    void slot_saveScanReportToFileButtonClicked();
    void slot_scanFromFileButtonClicked();
    void slot_createTempFilesButtonClicked();
    void slot_moveInfectedFilesButtonClicked();
    void slot_copyInfectedFilesButtonClicked();
    void slot_createButtonClicked();
    void slot_getClamscanProcessHasOutput();
    void slot_getClamscanProcessFinished();
    void slot_cancleButtonClicked();

signals:
    void signal_profileSaved();

};

#endif // PROFILEWIZARDDIALOG_H
