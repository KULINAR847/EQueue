#include "equeue.h"
#include "settings.h"
#include "calllist.h"

#include "ui_equeue.h"

#include <QDebug>
#include <QBuffer>
#include <QUrlQuery>
#include <QThread>
#include <QFont>
#include <QIcon>
#include <QTimer>
#include <QString>
#include <QJsonObject>
#include "networker.h"

EQueue::EQueue(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EQueue)
{
    ui->setupUi(this);

    setWindowFlags( windowType() | Qt::WindowStaysOnTopHint);
    setWindowTitle("Электронная очередь");

    // Создаём необходимые объекты
    network = new NetWorker(this);
    currentIdTicket = -1;

    // Очищаем информационные поля
    ui->lbCashierStatus->setText("Касса не активна");
    ui->lbNumber->setText("---");
    ui->lbType->setText("---");
    ui->lbReserved_2->setText("---");
    ui->lbQueue->setText("--");
    ui->lbReserved->setText("--");

    // Закрываем доступность всех кнопок
    ui->pbCallList->setEnabled(false);
    ui->pbFoldVisitor->setEnabled(false);
    ui->pbNextVisitor->setEnabled(false);
    ui->pbRepeatVisitor->setEnabled(false);
    ui->pbServedVisitor->setEnabled(false);
    ui->pbDisableCashier->setEnabled(false);

    // Связываем сигналы от модуля network с ui
    connect(network, SIGNAL(tokenSettings(QString)), this, SLOT(onTokenReceived(QString)));
    connect(network, SIGNAL(nextVisitor(const Ticket &)), this, SLOT(refreshVisitorInfo(const Ticket &)));
    connect(network, SIGNAL(visitorChangedStatus(bool)), this, SLOT(onVisitorStatusChanged(bool)));

    // Производим подключение
    network->getTokenRequest();
}

void EQueue::closeEvent(QCloseEvent *ev)
{
//    if (status == 1)
//    {
//        //on_pbDisableCashier_clicked();
//        //ShowMessage("Забыли выключить. Выключаю... ");

//        ev->accept();
//    }
    ev->accept();
}

EQueue::~EQueue()
{
    if(network) delete network;
    delete ui;
}

// Нажатие кнопки Следующий
void EQueue::on_pbNextVisitor_clicked()
{
    network->inviteNextVisitorRequest();
}

// Нажатие кнопки Обслужен
void EQueue::on_pbServedVisitor_clicked()
{
    if(currentIdTicket >= 0)
        network->servedCashierRequest(currentIdTicket);
}

// Нажатие кнопки Настройки
void EQueue::on_tbSettings_clicked()
{
    Settings s(nullptr, network);
    if (s.exec() == 1)
    {
        controlButtonsEnable(false);
        nextButtonsEnable(true);
    }
    else
    {
        controlButtonsEnable(false);
        nextButtonsEnable(false);
    }
}

// Меняем доступность кнопок управления [Неявка, Обслужен, Повтор]
void EQueue::controlButtonsEnable(bool flag)
{
    ui->pbFoldVisitor->setEnabled(flag);
    ui->pbServedVisitor->setEnabled(flag);
    ui->pbRepeatVisitor->setEnabled(flag);
}

// Меняем доступность кнопок вызова [Следующий, Вызов из списка]
void EQueue::nextButtonsEnable(bool flag)
{
    ui->pbNextVisitor->setEnabled(flag);
    ui->pbCallList->setEnabled(flag);
}

// Очистим информацию о текущем вызванном посетителе
void EQueue::clearVisitorInfo()
{
    ui->lbNumber->setText("---");
    ui->lbType->setText("---");
    ui->lbReserved_2->setText("---");
}

// Обновляем информацию о вызванном посетителе
void EQueue::refreshVisitorInfo(const Ticket &t)
{
    qDebug() << "refreshVisitorInfo " << t.id;
    if(t.id >= 0)
    {
        currentIdTicket = t.id;

        ui->lbNumber->setText( QString::number(t.number) );
        ui->lbType->setText(t.type == 1 ? "Выкуп брони" : "Посетитель" );
        ui->lbReserved_2->setText(t.type == 1 ? "Бронь " + t.order : "Очередь");

        controlButtonsEnable(true);
        nextButtonsEnable(false);
    }
    else
    {
        controlButtonsEnable(false);
        nextButtonsEnable(false);
    }
}

// Получаем новый токен
void EQueue::onTokenReceived(QString token)
{
    if(token != "")
    {
        controlButtonsEnable(false);
        nextButtonsEnable(true);
    }
}

// Отлавливаем события смены статуса посетителя, после нажатия кнопок управления, кроме Повтора
void EQueue::onVisitorStatusChanged(bool changed)
{
    qDebug() << "onVisitorStatusChanged = " << changed;
    if(changed)
    {
        controlButtonsEnable(false);
        nextButtonsEnable(true);

        clearVisitorInfo();
    }
    else
    {
        controlButtonsEnable(false);
        nextButtonsEnable(false);
    }
}

