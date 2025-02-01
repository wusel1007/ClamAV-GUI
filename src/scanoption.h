#ifndef SCANOPTION_H
#define SCANOPTION_H

#include <QWidget>
#include "setupfilehandler.h"
#include "translator.h"

namespace Ui {
class scanoption;
}

class scanoption : public QWidget
{
    Q_OBJECT

public:
    explicit scanoption(QWidget *parent = nullptr, QString setupFileName = "", QString section = "", bool checked = false, QString label = "", QString comment = "");
    ~scanoption();

private:
    Ui::scanoption *ui;
    setupFileHandler * setupFile;
    translator       * trans;
    QString languageset;
    QString setupFileSection;
    QString option;
    QString com;

private slots:
    void slot_checkboxClicked();
};

#endif // SCANOPTION_H
