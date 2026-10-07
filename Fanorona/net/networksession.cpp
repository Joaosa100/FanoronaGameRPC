#include "networksession.h"

#include <QHostAddress>
#include <QTcpServer>
#include <QTcpSocket>

#include "boardskeleton.h"
#include "boardstub.h"

NetworkSession::NetworkSession(QObject *parent)
    : QObject(parent)
{
}

NetworkSession::~NetworkSession() = default;

bool NetworkSession::startHosting(quint16 port, QString *errorMessage)
{
    if (m_server) {
        if (errorMessage)
            *errorMessage = tr("Já está hospedando uma partida.");
        return false;
    }

    m_server = new QTcpServer(this);
    if (!m_server->listen(QHostAddress::Any, port)) {
        if (errorMessage)
            *errorMessage = m_server->errorString();
        m_server->deleteLater();
        m_server = nullptr;
        return false;
    }

    connect(m_server, &QTcpServer::newConnection, this, &NetworkSession::handleNewConnection);
    return true;
}

void NetworkSession::connectToHost(const QString &host, quint16 port)
{
    auto *socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, this, &NetworkSession::handleSocketConnected);
    connect(socket, &QAbstractSocket::errorOccurred, this, &NetworkSession::handleSocketError);
    attachSocket(socket);
    socket->connectToHost(host, port);
}

void NetworkSession::handleNewConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket)
        return;

    // Jogo de 2 jogadores: aceita a primeira conexao e para de escutar por outras.
    m_server->close();

    attachSocket(socket);
    emit connected();
}

void NetworkSession::attachSocket(QTcpSocket *socket)
{
    m_socket = socket;

    // Monta o par stub/skeleton sobre o mesmo socket: um para invocar
    // metodos no outro lado, outro para receber e executar as chamadas que
    // o outro lado faz aqui.
    m_stub = std::make_unique<BoardStub>(socket);
    m_skeleton = std::make_unique<BoardSkeleton>(socket);
    if (m_pendingLocalBoard)
        m_skeleton->setTarget(m_pendingLocalBoard);

    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkSession::handleDisconnected);
}

PeerRemote *NetworkSession::remote() const
{
    return m_stub.get();
}

void NetworkSession::setLocalBoard(PeerRemote *localBoard)
{
    m_pendingLocalBoard = localBoard;
    if (m_skeleton)
        m_skeleton->setTarget(localBoard);
}

void NetworkSession::handleSocketConnected()
{
    emit connected();
}

void NetworkSession::handleSocketError(QAbstractSocket::SocketError)
{
    if (m_socket)
        emit errorOccurred(m_socket->errorString());
}

void NetworkSession::handleDisconnected()
{
    emit disconnected();
}

bool NetworkSession::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}
