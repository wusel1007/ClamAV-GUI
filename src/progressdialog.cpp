#include "progressdialog.h"
#include "ui_progressdialog.h"

progressDialog::progressDialog(QWidget *parent) : QWidget(parent), ui(new Ui::progressDialog)
{
    ui->setupUi(this);
    progressBar = new QRoundProgressBar(this);
    progressBar->setGeometry((this->width() - 80) / 2,20,100,100);
    progressBar->setMaximum(100);
    progressBar->setMinimum(0);
    progressBar->setValue(0);
    progressBar->setBarStyle(QRoundProgressBar::StyleDonut);
    progressBar->setFormat("");
    progressBar->show();
}

progressDialog::~progressDialog()
{
    delete ui;
}

void progressDialog::setText(QString text){
QString output = text.replace("\n","");

    ui->progressText->setText(output);
}

void progressDialog::setProgressBarMaxValue(double value) {
    progressBar->setMaximum(value);
}

void progressDialog::setProgressBarValue(double value) {
    progressBar->setValue(value);
}
