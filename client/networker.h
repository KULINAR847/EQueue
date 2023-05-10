#ifndef NETWORKER_H
#define NETWORKER_H

#include <QObject>

struct ConnectionSettings
{
    QString token;
    QString host;
    int number;
};

struct Ticket
{
    int id = -1;
    int number;
    int type;
    QString order;
    int queueCount;
    int resCount;
};

class QNetworkAccessManager;
class QNetworkReply;
class NetWorker : public QObject
{
    Q_OBJECT
public:
    explicit NetWorker(QObject *parent = nullptr);
    ~NetWorker();    

    void setSettings(QString host, int number, QString token="");
    // Подсказка пользователю о возникшей проблеме
    void showMessage(QString header, QString body);
    // Очистим правильно manager запроса
    void clearManager();

public slots:
    // Запрос на получение токена
    void getTokenRequest();
    // Ответ на получение токена
    void getTokenResponse(QNetworkReply *reply);

    // Запрос на вызов следующего посетителя
    void inviteNextVisitorRequest();
    // Ответ на вызов следующего посетителя
    void inviteNextVisitorResponse(QNetworkReply *reply);

    // Запрос на изменение статуса на Обслужен для посетителя
    void servedCashierRequest(int idTicket);
    // Ответ на изменение статуса на Обслужен для посетителя
    void servedCashierResponse(QNetworkReply *reply);
private:
    QNetworkAccessManager *manager = nullptr;
    ConnectionSettings *settings = nullptr;

signals:
    // Сигнализируем о получении токена от сервера
    void tokenSettings(const QString token);
    // Сигнализируем о получении данных следующего посетителя
    void nextVisitor(const Ticket &t);
    // Сигнализируем о смене статуса текущего посетителя
    void visitorChangedStatus(bool changed);
};

#endif // NETWORKER_H
