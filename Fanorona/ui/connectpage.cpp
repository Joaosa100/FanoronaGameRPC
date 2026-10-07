#include "connectpage.h"

#include <QHostAddress>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkInterface>
#include <QPushButton>
#include <QRandomGenerator>
#include <QStackedWidget>
#include <QVBoxLayout>

#include "networksession.h"

ConnectPage::ConnectPage(QWidget *parent)
    : QWidget(parent)
{
    m_stack = new QStackedWidget(this);

    // Pagina 0: hospedando.
    auto *hostPage = new QWidget(this);
    m_hostInfoLabel = new QLabel(hostPage);
    m_hostInfoLabel->setWordWrap(true);
    m_hostInfoLabel->setAlignment(Qt::AlignCenter);
    auto *hostLayout = new QVBoxLayout(hostPage);
    hostLayout->addStretch();
    hostLayout->addWidget(m_hostInfoLabel);
    hostLayout->addStretch();

    // Pagina 1: entrando.
    auto *joinPage = new QWidget(this);
    m_hostEdit = new QLineEdit(joinPage);
    m_hostEdit->setPlaceholderText(tr("IP do host, ex: 192.168.0.10"));
    m_portEdit = new QLineEdit(joinPage);
    m_portEdit->setText(QString::number(kDefaultPort));
    m_connectButton = new QPushButton(tr("Conectar"), joinPage);
    m_joinStatusLabel = new QLabel(joinPage);
    m_joinStatusLabel->setWordWrap(true);
    auto *joinLayout = new QVBoxLayout(joinPage);
    joinLayout->addStretch();
    joinLayout->addWidget(new QLabel(tr("IP do host:"), joinPage));
    joinLayout->addWidget(m_hostEdit);
    joinLayout->addWidget(new QLabel(tr("Porta:"), joinPage));
    joinLayout->addWidget(m_portEdit);
    joinLayout->addWidget(m_connectButton);
    joinLayout->addWidget(m_joinStatusLabel);
    joinLayout->addStretch();

    m_stack->addWidget(hostPage);
    m_stack->addWidget(joinPage);

    auto *cancelButton = new QPushButton(tr("Cancelar"), this);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(200, 40, 200, 40);
    layout->addWidget(m_stack, 1);
    layout->addWidget(cancelButton);

    connect(m_connectButton, &QPushButton::clicked, this, &ConnectPage::onConnectClicked);
    connect(cancelButton, &QPushButton::clicked, this, &ConnectPage::onCancelClicked);
}

QString ConnectPage::localAddressesText() const
{
    QStringList addresses;
    for (const QHostAddress &addr : QNetworkInterface::allAddresses()) {
        if (addr.protocol() == QAbstractSocket::IPv4Protocol && !addr.isLoopback())
            addresses << addr.toString();
    }
    return addresses.isEmpty() ? tr("(nenhum endereço de rede encontrado)") : addresses.join(QStringLiteral(", "));
}

void ConnectPage::resetSession()
{
    if (m_session) {
        m_session->disconnect(this);
        m_session->deleteLater();
        m_session = nullptr;
    }
}

void ConnectPage::startHosting(const QString &localName)
{
    resetSession();
    m_isHost = true;
    m_localName = localName;
    m_remoteName.clear();

    m_session = new NetworkSession(this);
    connect(m_session, &NetworkSession::connected, this, &ConnectPage::onSessionConnected);
    connect(m_session, &NetworkSession::errorOccurred, this, &ConnectPage::onSessionError);
    // A partir de agora, chamadas remotas de hello/start_game caem direto
    // nos metodos desta classe (ver remoteHello/remoteStartGame abaixo).
    m_session->setLocalBoard(this);

    QString error;
    if (!m_session->startHosting(kDefaultPort, &error)) {
        m_hostInfoLabel->setText(tr("Erro ao hospedar: %1").arg(error));
    } else {
        m_hostInfoLabel->setText(tr("Aguardando outro jogador conectar...\n\n"
                                     "Seu endereço: %1\nPorta: %2")
                                      .arg(localAddressesText())
                                      .arg(kDefaultPort));
    }
    m_stack->setCurrentIndex(0);
}

void ConnectPage::startJoining(const QString &localName)
{
    resetSession();
    m_isHost = false;
    m_localName = localName;
    m_remoteName.clear();
    m_joinStatusLabel->clear();
    m_stack->setCurrentIndex(1);
}

void ConnectPage::onConnectClicked()
{
    const QString host = m_hostEdit->text().trimmed();
    bool portOk = false;
    const quint16 port = m_portEdit->text().trimmed().toUShort(&portOk);
    if (host.isEmpty() || !portOk || port == 0) {
        m_joinStatusLabel->setText(tr("Informe um IP e uma porta válidos."));
        return;
    }

    resetSession();
    m_session = new NetworkSession(this);
    connect(m_session, &NetworkSession::connected, this, &ConnectPage::onSessionConnected);
    connect(m_session, &NetworkSession::errorOccurred, this, &ConnectPage::onSessionError);
    m_session->setLocalBoard(this);

    m_joinStatusLabel->setText(tr("Conectando..."));
    m_session->connectToHost(host, port);
}

void ConnectPage::onSessionConnected()
{
    // Invoca diretamente o metodo no "tabuleiro" remoto -- nao ha string de
    // tipo nem objeto JSON sendo montado aqui, so a chamada da interface.
    m_session->remote()->remoteHello(m_localName);
    if (m_isHost)
        m_hostInfoLabel->setText(tr("Jogador conectado! Sincronizando..."));
    else
        m_joinStatusLabel->setText(tr("Conectado! Aguardando início da partida..."));
}

void ConnectPage::remoteHello(const QString &playerName)
{
    m_remoteName = playerName;
    if (!m_isHost)
        return;

    const bool hostFirst = QRandomGenerator::global()->bounded(2) == 0;
    m_session->remote()->remoteStartGame(hostFirst);
    const Player firstTurn = hostFirst ? Player::PlayerA : Player::PlayerB;
    NetworkSession *session = m_session;
    m_session = nullptr; // dono agora é o GameController (ver reparent em GameController)
    emit sessionReady(session, Player::PlayerA, firstTurn, m_localName, m_remoteName);
}

void ConnectPage::remoteStartGame(bool hostPlaysFirst)
{
    if (m_isHost)
        return;

    const Player firstTurn = hostPlaysFirst ? Player::PlayerA : Player::PlayerB;
    NetworkSession *session = m_session;
    m_session = nullptr;
    emit sessionReady(session, Player::PlayerB, firstTurn, m_localName, m_remoteName);
}

void ConnectPage::remoteMove(const std::vector<MoveStepDTO> &)
{
    // Nao ocorre durante o handshake; o jogo so comeca depois de sessionReady.
}

void ConnectPage::remoteChat(const QString &)
{
    // Idem.
}

void ConnectPage::remoteResign()
{
    // Idem.
}

void ConnectPage::onSessionError(const QString &message)
{
    if (m_isHost)
        m_hostInfoLabel->setText(tr("Erro de conexão: %1").arg(message));
    else
        m_joinStatusLabel->setText(tr("Erro de conexão: %1").arg(message));
}

void ConnectPage::onCancelClicked()
{
    resetSession();
    emit cancelled();
}
