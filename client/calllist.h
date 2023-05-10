#ifndef CALLLIST_H
#define CALLLIST_H

#include <QWidget>
#include <QDialog>
#include <QTimer>
#include <QCloseEvent>


class QNetworkReply;
class QNetworkAccessManager;
//class QNetworkReply;

namespace Ui {
class CallList;
}

class CallList : public QDialog
{
    Q_OBJECT

public:
    explicit CallList(QWidget *parent = nullptr);
    virtual ~CallList();

    QString token;
    int number;
    QString ip;
    int FocusOnRes;

    QNetworkAccessManager *manager = nullptr;
    QTimer *timer = nullptr;
    QNetworkReply *result;
    void getVisitorsList();
    void setParams(QString ip, int number, QString token="");

    void closeEvent(QCloseEvent *event);
public slots:
    void RefreshData();
private slots:
    void getResponse(QNetworkReply *reply);
    void on_pbCancel_clicked();
    void on_pbInvite_clicked();
    void on_twQueue_itemSelectionChanged();
    void on_twReservation_itemSelectionChanged();
    void callResponse(QNetworkReply *reply);

private:
    Ui::CallList *ui;
};

#endif // CALLLIST_H
