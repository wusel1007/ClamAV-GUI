#include "scanoptionyn.h"
#include "ui_scanoptionyn.h"

scanoptionyn::scanoptionyn(QWidget *parent, QString setupFileName, QString section, bool checked, QString label, QString comment) :
    QWidget(parent),
    ui(new Ui::scanoptionyn)
{
    ui->setupUi(this);
    setupFile = new setupFileHandler(setupFileName);
    setupFileSection = section;
    setupFileHandler * baseSetup = new setupFileHandler(QDir::homePath() + "/.clamav-gui/settings.ini");
    languageset = baseSetup->getSectionValue("Setup","language");
    trans = new translator(languageset);

    option = label.mid(0,label.indexOf("<equal>"));
    com = trans->translateit(comment);
    ui->checkBox->setChecked(checked);
    ui->comboBox->setEnabled(checked);
    if (checked == true) {
        this->setStyleSheet("background-color:lightgreen");
    }
    ui->checkBox->setText(com);
    ui->checkBox->setToolTip(option);
    if (label.indexOf("<equal>yes") != -1) ui->comboBox->setCurrentText("yes");else ui->comboBox->setCurrentText("no");
}

scanoptionyn::~scanoptionyn()
{
    delete ui;
}

void scanoptionyn::slot_checkboxClicked()
{
    if (ui->checkBox->isChecked() == false) {
        setupFile->removeKeyword(setupFileSection,option + "<equal>" + ui->comboBox->currentText());
        ui->comboBox->setEnabled(false);
        this->setStyleSheet("");
    } else {
        setupFile->setSectionValue(setupFileSection,option + "<equal>" + ui->comboBox->currentText(), com);
        ui->comboBox->setEnabled(true);
        this->setStyleSheet("background-color:lightgreen");
    }
}

void scanoptionyn::slot_comboboxChanged(QString value)
{
    if (ui->checkBox->isChecked() == true) {
        setupFile->removeKeyword(setupFileSection,option + "<equal>yes");
        setupFile->removeKeyword(setupFileSection,option + "<equal>no");
        setupFile->setSectionValue(setupFileSection,option + "<equal>" + value ,com);
    }
}
