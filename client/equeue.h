#ifndef EQUEUE_H
#define EQUEUE_H

#include <QWidget>

#include <QVector>
#include <QCloseEvent>
#include <QString>

namespace Ui {
class EQueue;
}

class NetWorker;
class Ticket;
class EQueue : public QWidget
{
    Q_OBJECT

public:
    explicit EQueue(QWidget *parent = nullptr);
    ~EQueue();

    void closeEvent(QCloseEvent *ev);
    // Подсказка кассиру об ошибке
    void ShowMessage(QString message);

    // Меняем доступность кнопок управления [Неявка, Обслужен, Повтор]
    void controlButtonsEnable(bool flag);
    // Меняем доступность кнопок вызова [Следующий, Вызов из списка]
    void nextButtonsEnable(bool flag);

    // Очистим информацию о текущем вызванном посетителе
    void clearVisitorInfo();

public slots:
   // Обновляем информацию о вызванном посетителе
   void refreshVisitorInfo(const Ticket &t);
   // Получаем новый токен
   void onTokenReceived(QString token);
   // Отлавливаем события смены статуса посетителя, после нажатия кнопок управления, кроме Повтора
   void onVisitorStatusChanged(bool changed);
   // void EnableNextButton();

signals:

private slots:
    // Нажатие кнопки Следующий
    void on_pbNextVisitor_clicked();
    // Нажатие кнопки Обслужен
    void on_pbServedVisitor_clicked();
    // Нажатие кнопки Настройки
    void on_tbSettings_clicked();

private:
    // Работа с сетью
    NetWorker *network = nullptr;
    // Текущий посетитель
    int currentIdTicket;

    Ui::EQueue *ui;
};

#endif // EQUEUE_H
