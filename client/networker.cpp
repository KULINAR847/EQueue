
#include "networker.h"
#include "jsonworker.h"
#include "networker.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpMultiPart>
#include <QUrlQuery>

#include <QMessageBox>
#include <QSettings>
#include <QFile>

NetWorker::NetWorker(QObject *parent)
    : QObject{parent}
{
    settings = new ConnectionSettings();

    // Прочитаем файл настроек, если он существует и заполним параметры подключения
    if (QFile::exists("conection.ini"))
    {
        QSettings *fsettings = new QSettings("conection.ini", QSettings::IniFormat);
        settings->token = fsettings->value("token", "").toString();
        settings->host = fsettings->value("host", "").toString();
        settings->number = fsettings->value("number", 0).toInt();
        delete fsettings;
    }
}

NetWorker::~NetWorker() {
    if (settings)
    {
        delete settings;
        settings = nullptr;
    }
    clearManager();
}

void NetWorker::setSettings(QString host, int number, QString token)
{
    if(settings)
    {
        settings->token = token;
        settings->host = host;
        settings->number = number;
    }
}

// Подсказка пользователю о возникшей проблеме
void NetWorker::showMessage(QString header, QString body)
{
    QMessageBox msgBox;
    msgBox.setWindowFlags(msgBox.windowFlags()|Qt::WindowStaysOnTopHint);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle(header);
    QFont font("TimesNewRoman", 12);
    msgBox.setFont(font);
    msgBox.setText(body);
    msgBox.exec();
}

// Очистим правильно manager запроса
void NetWorker::clearManager()
{    
    if(manager)
    {
        manager->disconnect();
        manager->deleteLater();
        manager = nullptr;
    }
}

// Запрос на получение токена
void NetWorker::getTokenRequest()
{
    if(settings)
    {
        QString reqText = QString("http://%1/cashier/%2").arg(settings->host, QString::number(settings->number));
        qDebug() << reqText;
        QNetworkRequest request((QUrl(reqText)));
        QByteArray body = QString(QString("number=%1").arg(settings->number)).toLocal8Bit();

        request.setHeader(QNetworkRequest::ContentTypeHeader,
                          QStringLiteral("text/plain; charset=utf-8"));

        manager = new QNetworkAccessManager(this);
        connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(getTokenResponse(QNetworkReply*)));

        manager->post(request, body);
    }
}

// Ответ на получение токена
void NetWorker::getTokenResponse(QNetworkReply *reply)
{
    QString resCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();

    if (resCode == "200")
    {
        JsonWorker jw(reply->readAll());
        settings->token = jw.getJsonValue("token").toString();
    }
    else
    {
        //qDebug() << "resCode = " << resCode;
        if (resCode == "")
            showMessage("Информация", "Сервер " + settings->host + " не отвечает! Проверьте включен ли он!");
        else
            showMessage("Информация", "Ответ от сервера с кодом " + resCode);
    }
    emit tokenSettings(settings->token);
    clearManager();
}

// Запрос на вызов следующего посетителя
void NetWorker::inviteNextVisitorRequest()
{
    if (settings->token != "")
    {
        QString reqText = QString("http://%1/cashier/%2/invite").arg(settings->host, QString::number(settings->number));
        qDebug() << reqText;
        QNetworkRequest request((QUrl(reqText)));
        request.setRawHeader("Authorization", settings->token.toLocal8Bit());

        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        QUrlQuery params;
        //params.addQueryItem("visitor_id", QString::number(currentID));

        manager = new QNetworkAccessManager(this);
        //managersList.append(manager);
        connect(manager, SIGNAL(finished(QNetworkReply*)),
                this, SLOT(inviteNextVisitorResponse(QNetworkReply*)));

        manager->post(request, params.query().toUtf8());
    }
}

// Ответ на вызов следующего посетителя
void NetWorker::inviteNextVisitorResponse(QNetworkReply *reply)
{
    QString resCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();
    Ticket t;

    if (resCode == "200")
    {
        JsonWorker jw(reply->readAll());

        t.id = jw.getJsonValue("id", jw.getJsonValue("visitor").toObject()).toInt();
        t.number = jw.getJsonValue("number", jw.getJsonValue("visitor").toObject()).toInt();
        t.type = jw.getJsonValue("type", jw.getJsonValue("visitor").toObject()).toInt();
        t.order = jw.getJsonValue("order_number", jw.getJsonValue("visitor").toObject()).toVariant().toString();
    }
    else
    {        
        if (resCode == "")
            showMessage("Информация", "Сервер " + settings->host + " не отвечает! Проверьте включен ли он!");
        else
            showMessage("Информация", "Ответ от сервера с кодом " + resCode);
    }
    emit nextVisitor(t);
    clearManager();
}

// Запрос на изменение статуса на Обслужен для посетителя
void NetWorker::servedCashierRequest(int idTicket)
{
    if (settings->token != "")
    {
        qDebug() << "served start";
        if (idTicket >= 0)
        {
            //lockControlButtons();

            QString reqText = QString("http://%1/visitor/%2").arg(settings->host, QString::number(idTicket));
            qDebug() << reqText;

            QNetworkRequest request((QUrl(reqText)));
            request.setRawHeader("Authorization", settings->token.toLocal8Bit());
            manager = new QNetworkAccessManager(this);
            //managersList.append(manager);
            connect(manager, SIGNAL(finished(QNetworkReply*)),
                    this, SLOT(servedCashierResponse(QNetworkReply*)));

            request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
            QUrlQuery params;
            params.addQueryItem("status", QString::number(2)); // завершил обслуживание

            manager->sendCustomRequest(request, "PATCH", params.query().toUtf8());
        }
    }
}

// Ответ на изменение статуса на Обслужен для посетителя
void NetWorker::servedCashierResponse(QNetworkReply *reply)
{
    QString resCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString();

    qDebug() << "servedCashierResponse = " << resCode;

    if(resCode != "200")
        showMessage("Информация", "Внимание! Зайдите в настройки и инициализируйте подключение по новой! ");

    emit visitorChangedStatus(resCode == "200");
    clearManager();
}

