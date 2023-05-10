#include "jsonworker.h"
#include <QFile>
#include <QByteArray>
#include <QDebug>

JsonWorker::JsonWorker()
{

}

JsonWorker::JsonWorker(QString data, bool dataIsFileName)
{
    qDebug() << "data = " << data;
    if (dataIsFileName)
        readJsonFile(data);
    else
        m_document = QJsonDocument::fromJson(data.toUtf8());
}

void JsonWorker::readJsonFile(QString fileName)
{
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    m_document = QJsonDocument::fromJson(file.readAll());
    file.close();
}

void JsonWorker::readJsonByteArray(QByteArray array)
{
    m_document = QJsonDocument::fromJson(array);
}

QJsonValue JsonWorker::getJsonValue(QString keyName, QJsonObject obj)
{
    QJsonObject jsonObj = m_document.object();
    qDebug() << jsonObj.keys();
    if (obj != QJsonObject())
        jsonObj = obj;

    if (jsonObj.contains(keyName))
        return jsonObj.value(QString(keyName));
    return QJsonValue();
}

bool JsonWorker::setJsonValue(QString keyName, QJsonValue val)
{
    QJsonObject jsonObj = m_document.object();
    if (jsonObj.contains(keyName))
    {
        auto iterator = jsonObj.find(keyName);
        jsonObj.erase(iterator);
        jsonObj.insert(keyName, val);

        m_document.setObject(jsonObj);

        return true;
    }
    else
    {
        jsonObj.insert(keyName, val);
        m_document.setObject(jsonObj);
        return true;
    }
}

void JsonWorker::appendToArray(QString keyName, QJsonValue val)
{
    QJsonObject jsonObj = m_document.object();
    if (jsonObj.contains(keyName))
    {
        QJsonArray array = jsonObj[keyName].toArray();
        array.append(val);

        auto iterator = jsonObj.find(keyName);
        jsonObj.erase(iterator);
        jsonObj.insert(keyName, array);

        m_document.setObject(jsonObj);
    }
    else
    {
        QJsonArray array;
        array.append(val);
        jsonObj.insert(keyName, array);
        m_document.setObject(jsonObj);
    }
}

QJsonObject JsonWorker::createSimpleObject(QStringList keys, QJsonArray values)
{
    QJsonObject tempObj;
    for(int i = 0; i < keys.size(); i++)
        tempObj.insert(keys[i], values[i]);

    return tempObj;
}




QString JsonWorker::toJson()
{
    return m_document.toJson();
}

QJsonObject JsonWorker::toObject()
{
    return m_document.object();
}

bool JsonWorker::Save(QString fileName)
{
    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(m_document.toJson());
    file.close();

    return true;
}
