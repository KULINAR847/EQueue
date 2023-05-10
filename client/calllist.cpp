#include "calllist.h"
#include "jsonworker.h"
#include "ui_calllist.h"
#include <QString>
#include <QByteArray>
#include <QJsonArray>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrlQuery>
#include <QMessageBox>
#include <QFont>


CallList::CallList(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CallList)
{
    ui->setupUi(this);
    setWindowFlags(windowType()|Qt::WindowStaysOnTopHint);
    token = "";
    ip = "";
    setWindowTitle("Вызов из списка");

    timer = new QTimer(this);
    timer->setInterval(10000);
    connect(timer, &QTimer::timeout, this, &CallList::RefreshData);
    timer->start();

    FocusOnRes = 0;
}

void CallList::RefreshData()
{
    getVisitorsList();
}

CallList::~CallList()
{


    delete ui;
}

void CallList::closeEvent(QCloseEvent *event)
{
    if(manager)
        delete manager;

    if(timer) delete timer;
}

void CallList::getVisitorsList()
{
    QString reqText = "http://"+ ip + "/visitors";
    qDebug() << reqText;

    if (manager)
    {
        delete manager;
        manager = nullptr;
    }

    QNetworkRequest request((QUrl(reqText)));
    request.setRawHeader("Authorization", token.toLocal8Bit());
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(getResponse(QNetworkReply*)));

    manager->get(request);
}

void CallList::setParams(QString ip, int number, QString token)
{
    this->ip = ip;
    this->number = number;
    this->token = token;
}

void CallList::getResponse(QNetworkReply *reply)
{
    JsonWorker jw;

    //qDebug() << reply->readAll();
    jw.readJsonByteArray(reply->readAll());

    QJsonArray visitors = jw.getJsonValue("visitors").toArray();
    qDebug() << visitors;
    int nReserved = 0;
    int nQueue = 0;

    foreach (const QJsonValue &info, visitors) {
        if (jw.getJsonValue("type", info.toObject()).toInt() == 0)
            nQueue++;
        if (jw.getJsonValue("type", info.toObject()).toInt() == 1)
            nReserved++;
    }

    ui->twReservation->setRowCount(nReserved);
    ui->twQueue->setRowCount(nQueue);

    int rowReserved = 0;
    int rowQueue = 0;

    foreach (const QJsonValue &info, visitors) {
        //qDebug() << jw.getJsonValue("id", info.toObject());
        //qDebug() << jw.getJsonValue("number", info.toObject());
        //qDebug() << jw.getJsonValue("type", info.toObject());
        //qDebug() << jw.getJsonValue("created_at", info.toObject());

        if (jw.getJsonValue("type", info.toObject()).toInt() == 0)
        {
            QTableWidgetItem *itemNum = new QTableWidgetItem(QString::number(jw.getJsonValue("number", info.toObject()).toInt()));
            itemNum->setFlags(itemNum->flags() ^ Qt::ItemIsEditable);
            itemNum->setWhatsThis(QString::number(jw.getJsonValue("id", info.toObject()).toInt()));
            ui->twQueue->setItem(rowQueue, 0, itemNum);
            QTableWidgetItem *itemTime = new QTableWidgetItem(jw.getJsonValue("created_at", info.toObject()).toString().mid(11,5));
            itemTime->setFlags(itemTime->flags() ^ Qt::ItemIsEditable);
            ui->twQueue->setItem(rowQueue, 1, itemTime);
            rowQueue++;
        }
        if (jw.getJsonValue("type", info.toObject()).toInt() == 1)
        {
            QTableWidgetItem *itemNum = new QTableWidgetItem(QString::number(jw.getJsonValue("number", info.toObject()).toInt()));
            itemNum->setFlags(itemNum->flags() ^ Qt::ItemIsEditable);
            itemNum->setWhatsThis(QString::number(jw.getJsonValue("id", info.toObject()).toInt()));
            ui->twReservation->setItem(rowReserved, 0, itemNum);
            QTableWidgetItem *itemTime = new QTableWidgetItem(jw.getJsonValue("created_at", info.toObject()).toString().mid(11,5));
            itemTime->setFlags(itemTime->flags() ^ Qt::ItemIsEditable);
            ui->twReservation->setItem(rowReserved, 1, itemTime);
            rowReserved++;
        }
    }

    //qDebug() << rowQueue;
    qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    setWindowTitle(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString());
}

void CallList::on_pbCancel_clicked()
{
    close();
}

void CallList::on_pbInvite_clicked()
{
    qDebug() << "Reservation focused = " << ui->twReservation->hasFocus();
    qDebug() << "Queue focused = " << ui->twQueue->hasFocus();
    int id = -1;
    if (FocusOnRes == 1)
        id = ui->twReservation->item(ui->twReservation->selectedItems()[0]->row(), 0)->whatsThis().toInt();
    if (FocusOnRes == 2)
        id = ui->twQueue->item(ui->twQueue->selectedItems()[0]->row(), 0)->whatsThis().toInt();

    if (id >= 0)
    {
        if (manager)
        {
            delete manager;
            manager = nullptr;
        }

        qDebug() << "id = " << id;

        QString reqText = "http://" + ip + "/cashier/" + QString::number(number) + "/invite";
        qDebug() << reqText;
        QNetworkRequest request((QUrl(reqText)));
        request.setRawHeader("Authorization", token.toLocal8Bit());

        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QUrlQuery params;
        params.addQueryItem("visitor_id", QString::number(id));

        manager = new QNetworkAccessManager(this);
        connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(callResponse(QNetworkReply*)));

        manager->post(request, params.query().toUtf8());
    }

}

void CallList::on_twQueue_itemSelectionChanged()
{
    ui->twReservation->clearSelection();
    FocusOnRes = 2;
    //ui->twQueue->selectRow(ui->twQueue->currentRow());
}

void CallList::on_twReservation_itemSelectionChanged()
{
    ui->twQueue->clearSelection();
    FocusOnRes = 1;
    //ui->twReservation->selectRow(ui->twReservation->currentRow());
}

void CallList::callResponse(QNetworkReply *reply)
{
    QString res = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    if (res == "200")
    {
        result = reply;
        setWindowTitle(res + " - call");
        accept();
    }
    if (res == "400")
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Внимание");
        QFont font("TimesNewRoman", 12);
        msgBox.setFont(font);
        msgBox.setText("Внимание, сессия истекла или кто-то случайно переподключился под номером Вашей кассы!!!");
        msgBox.exec();
    }
}
