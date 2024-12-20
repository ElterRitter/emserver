#include "clientwgt.h"
#include "ui_clientwgt.h"

Q_LOGGING_CATEGORY(clientwgt, "NativeClient")
#define Debug qDebug(clientwgt)
#define Info qInfo(clientwgt)
#define Warn qWarning(clientwgt)

ClientWgt::ClientWgt(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::clientwgt)
    , m_processor{nullptr}
{
    ui->setupUi(this);
    connectUI();
    connectTransport();
}

ClientWgt::~ClientWgt()
{
    if(m_processor)
        m_processor->connectionClose();

    delete ui;
}

void ClientWgt::onBtnConnectClicked(bool needConnect)
{
    if(needConnect)
    {
        QString ipAddr = ui->m_editIP->text();
        quint16 port = ui->m_spinPort->value();
        Debug << "Try to connect to " << ipAddr << ":" << port;
        m_processor = std::unique_ptr<EmserverAdapterQt>( new EmserverAdapterQt() );
        m_processor->connectionOpen(ipAddr, port);
        connectTransport();
    }
    else
    {
        m_processor->connectionClose();
        m_processor.reset();
    }
}

void ClientWgt::onBtnReqCapsClicked()
{
    if(!m_processor)
        return;

    m_processor->sendRequestCapabilities();
}



void ClientWgt::onClientConnected(emserverproxy::ConnectedState state)
{
    ui->m_lblRemoteIP->setText(QString::fromStdString(state.remotePoint.address));
    ui->m_lblRemotePort->setText(QString::number(state.remotePoint.port));
    bool needVisible = !state.remotePoint.address.empty();
    ui->m_lblRemoteIP->setVisible(needVisible);
    ui->m_lblRemotePort->setVisible(needVisible);
    ui->m_lblRemoteSeparator->setVisible(needVisible);

    needVisible = !state.localPoint.address.empty();
    ui->m_lblLocalIP->setText(QString::fromStdString(state.localPoint.address));
    ui->m_lblLocalIP->setVisible(needVisible);
    ui->m_lblLocalPort->setText(QString::number(state.localPoint.port));
    ui->m_lblLocalPort->setVisible(needVisible);
    ui->m_lblLocalSeparator->setVisible(needVisible);
}

void ClientWgt::onClientCapabilities(emserverproxy::Sensors caps)
{

}


void ClientWgt::connectUI()
{
    connect(ui->m_pbConnect, &QPushButton::toggled, this, &ClientWgt::onBtnConnectClicked);
    connect(ui->m_pbRequestCap, &QPushButton::clicked, this, &ClientWgt::onBtnReqCapsClicked);
}

void ClientWgt::connectTransport()
{
    connect(m_processor.get(), &EmserverAdapterQt::clientConnectedStateChanged, this, &ClientWgt::onClientConnected, Qt::QueuedConnection);
//    connect(m_processor.get(), &EmserverAdapterQt::currentCapabilities, this, &ClientWgt::onClientCapabilities, Qt::QueuedConnection);
}

