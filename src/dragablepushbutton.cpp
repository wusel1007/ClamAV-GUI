#include "dragablepushbutton.h"
#include "ui_dragablepushbutton.h"

dragablePushButton::dragablePushButton(QWidget *parent) : QPushButton(parent), ui(new Ui::dragablePushButton)
{
    //ui->setupUi(this);
}

dragablePushButton::dragablePushButton(QIcon icon, QString text, QWidget *parent, QString dragText) : QPushButton(icon, text, parent)
{
    dragDropText = dragText;
}

dragablePushButton::~dragablePushButton()
{
    //delete ui;
}

void dragablePushButton::mouseMoveEvent(QMouseEvent *event) {
QUrl url("file://" + dragDropText);
QList <QUrl> urls;
    event = event;

    urls << url;
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setText("file://" + dragDropText);
    mimeData->setUrls(urls);
    drag->setMimeData(mimeData);
    drag->setPixmap(QPixmap(":/icons/icons/media.png"));
    emit dragStarted();
    drag->exec();
}
