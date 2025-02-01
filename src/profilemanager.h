#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QWidget>
#include "setupfilehandler.h"
#include "profilewizarddialog.h"

namespace Ui {
class ProfileManager;
}

class ProfileManager : public QWidget
{
    Q_OBJECT

public:
    explicit ProfileManager(QWidget *parent = 0);
    ~ProfileManager();

private:
    Ui::ProfileManager  *ui;
    setupFileHandler    *setupFile;
    ProfileWizardDialog *profileWizard;
    void getProfileList();
    void readProfileSettings();

private slots:
    void slot_readProfileSettings();
    void slot_addProfileButtonClicked();
    void slot_eraseProfileButtonClicked();
    void slot_editProfileButtonClicked();
    void slot_profileSaved();

signals:
    void triggerProfilesChanged();
};

#endif // PROFILEMANAGER_H
