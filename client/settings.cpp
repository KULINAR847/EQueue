#include "settings.h"
#include "settings.h"
#include "ui_settings.h"
#include "networker.h"

#include <QDebug>
#include <QFile>
#include <QSettings>

Settings::Settings(QWidget *parent, NetWorker *network) :
    QDialog(parent), network(network),
    ui(new Ui::Settings)
{
    ui->setupUi(this);

    setWindowFlags(windowType()|Qt::WindowStaysOnTopHint);
    setWindowTitle("Настройки");

    s = new ConnectionSettings();

    // Считаем настройки из файла если он есть
    loadSettings();

    // Отобразим видимую информацию о хосте, к которому подключаемся и какой кассой
    ui->leHost->setText(s->host);
    ui->leNumber->setText(QString::number(s->number));

    // Получаем информацию от network о токене
    connect(network, SIGNAL(tokenSettings(QString)), this, SLOT(onTokenReceived(QString)));
}

Settings::~Settings()
{
    if(s) delete s;
    delete ui;
}

// Сохранение настроек соединения в файл
void Settings::saveSettings()
{
    QSettings *settings = new QSettings("conection.ini", QSettings::IniFormat);
    settings->setValue("token", s->token);
    settings->setValue("host", s->host);
    settings->setValue("number", s->number);
    settings->sync();
    delete settings;
}

// Загрузка настроек соединения из файла
void Settings::loadSettings()
{
    // Прочитаем файл настроек, если он существует и заполним параметры подключения
    if (QFile::exists("conection.ini"))
    {
        QSettings *settings = new QSettings("conection.ini", QSettings::IniFormat);        

        s->token = settings->value("token", "").toString();
        s->host = settings->value("host", "").toString();
        s->number = settings->value("number", 0).toInt();
        delete settings;
    }
    else
    {
        s->token = "";
        s->host = "localhost:5000";
        s->number = 1;
    }
}

// Нажатие кнопки Закрыть
void Settings::on_pbClose_clicked()
{
    reject();
}

// Получение токена от сервера
void Settings::onTokenReceived(const QString token)
{    
    if(token.length())
    {
        s->token = token;
        saveSettings();
        accept();
    }
}

// Нажатие кнопки Инициализация
void Settings::on_pbInit_clicked()
{
    // Обновим данные полученные из виджетов
    s->host = ui->leHost->text();
    s->number = ui->leNumber->text().toInt();
    network->setSettings(s->host, s->number);
    network->getTokenRequest();
}
