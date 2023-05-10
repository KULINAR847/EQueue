#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
class Settings;
}

class ConnectionSettings;
class NetWorker;
class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr, NetWorker *network=nullptr);
    ~Settings();

    // Сохранение настроек соединения в файл
    void saveSettings();
    // Загрузка настроек соединения из файла
    void loadSettings();

private slots:
    // Нажатие кнопки Инициализация
    void on_pbInit_clicked();
    // Нажатие кнопки Закрыть
    void on_pbClose_clicked();
    // Получение токена от сервера
    void onTokenReceived(const QString token);

signals:

private:
    ConnectionSettings *s = nullptr;
    NetWorker *network = nullptr;

    Ui::Settings *ui;
};

#endif // SETTINGS_H
