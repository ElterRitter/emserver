#pragma once

#include "emserveradapterqt.h"

#include <QWidget>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(clientwgt)


namespace Ui {
class clientwgt;
}

class ClientWgt : public QWidget
{
    Q_OBJECT

public:
    explicit ClientWgt(QWidget *parent = nullptr);
    ~ClientWgt();

private slots:
    void onBtnConnectClicked(bool needConnect);
    void onBtnReqCapsClicked();


    void onClientConnected(emserverproxy::ConnectedState state);
    void onClientCapabilities(emserverproxy::Sensors caps);

private:
    Ui::clientwgt *ui;
    std::unique_ptr<EmserverAdapterQt> m_processor;

    void connectUI();
    void connectTransport();
};
