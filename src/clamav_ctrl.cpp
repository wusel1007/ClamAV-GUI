#include "clamav_ctrl.h"
#include "ui_clamav_ctrl.h"
#include "qscreen.h"

clamav_ctrl::clamav_ctrl(QWidget *parent) : QWidget(parent), ui(new Ui::clamav_ctrl)
{
    ui->setupUi(this);
    initWidget();
}

void clamav_ctrl::initWidget(){
QScreen *screen = QGuiApplication::primaryScreen();
QRect  screenGeometry = screen->geometry();
    int height = screenGeometry.height();
    int width = screenGeometry.width();


//QRect   rec = QApplication::desktop()->screenGeometry();
//int     height = rec.height();
//int     width = rec.width();

    this->setStyleSheet("background:transparent;");
    this->setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowStaysOnTopHint);
    this->move(width-this->width()-10,height-this->height()-10);
    this->setAcceptDrops(true);
}

clamav_ctrl::~clamav_ctrl()
{
    delete ui;
}

void clamav_ctrl::dragEnterEvent(QDragEnterEvent * event){
    event->acceptProposedAction();
}

void clamav_ctrl::dropEvent(QDropEvent * event){
QStringList list;
QList <QUrl> tempList;
QString tempString;

    if (event && event->mimeData()){
        const QMimeData * mimeData;
        mimeData = event->mimeData();
        if (mimeData->hasUrls()){
            tempList = mimeData->urls();
            for (int i = 0; i < tempList.count(); i++){
                tempString = tempList.at(i).toString();
                if (tempString.indexOf("file://") == 0){
                    list << tempString.mid(7);
                }
            }
            if (list.count() > 0) emit scanRequest(list);
        }
    }
}

