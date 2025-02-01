#ifndef SCANOPTIONYN_H
#define SCANOPTIONYN_H

#include <QWidget>
#include "setupfilehandler.h"
#include "translator.h"

namespace Ui {
class scanoptionyn;
}

class scanoptionyn : public QWidget
{
    Q_OBJECT

public:
    explicit scanoptionyn(QWidget *parent = nullptr, QString setupFileName = "", QString section = "", bool checked = false, QString label = "", QString comment = "");
    ~scanoptionyn();

private:
    Ui::scanoptionyn *ui;
    setupFileHandler * setupFile;
    translator       * trans;
    QString setupFileSection;
    QString option;
    QString com;
    QString languageset;

private slots:
    void slot_checkboxClicked();
    void slot_comboboxChanged(QString value);
};

#endif // SCANOPTIONYN_H
