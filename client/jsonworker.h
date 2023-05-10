#ifndef JSONWORKER_H
#define JSONWORKER_H

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QByteArray>
#include <QMap>

class JsonWorker
{

public:
    // Пустой JSON
    JsonWorker();
    // JSON из файла
    JsonWorker(QString data, bool dataIsFileName = false);
    // JSON из строки
    //JsonWorker(QByteArray bytes);
    QJsonDocument m_document;
    void readJsonFile(QString fileName);
    void readJsonByteArray(QByteArray array);
    QJsonValue getJsonValue(QString keyName, QJsonObject obj = QJsonObject());
    bool setJsonValue(QString keyName, QJsonValue val);
    void appendToArray(QString keyName, QJsonValue val);
    QJsonObject createSimpleObject(QStringList keys, QJsonArray values);

    QString toJson();
    QJsonObject toObject();
    bool Save(QString fileName);
};

#endif // JSONWORKER_H
