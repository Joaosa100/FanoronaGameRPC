#include "boardstub.h"

#include <QAbstractSocket>
#include <QDataStream>
#include <QTcpSocket>

#include "boardwire.h"

namespace {

void sendFrame(QTcpSocket *socket, MethodId id, const QByteArray &payload)
{
    if (!socket || socket->state() != QAbstractSocket::ConnectedState)
        return;

    // Framing binario por tamanho (em vez do delimitador '\n' usado na
    // versao em sockets com JSON): 4 bytes de tamanho + 1 byte de MethodId +
    // o payload. Precisa de um numero de bytes explicito aqui porque o
    // payload e binario e pode conter qualquer valor, inclusive um que por
    // coincidencia seja igual ao delimitador usado no protocolo antigo.
    QByteArray frame;
    QDataStream header(&frame, QIODevice::WriteOnly);
    header << quint32(payload.size() + 1);
    frame.append(char(static_cast<quint8>(id)));
    frame.append(payload);

    socket->write(frame);
}

} // namespace

BoardStub::BoardStub(QTcpSocket *socket)
    : m_socket(socket)
{
}

void BoardStub::remoteHello(const QString &playerName)
{
    sendFrame(m_socket, MethodId::Hello, encodeHello(playerName));
}

void BoardStub::remoteStartGame(bool hostPlaysFirst)
{
    sendFrame(m_socket, MethodId::StartGame, encodeStartGame(hostPlaysFirst));
}

void BoardStub::remoteMove(const std::vector<MoveStepDTO> &steps)
{
    sendFrame(m_socket, MethodId::Move, encodeMove(steps));
}

void BoardStub::remoteChat(const QString &text)
{
    sendFrame(m_socket, MethodId::Chat, encodeChat(text));
}

void BoardStub::remoteResign()
{
    sendFrame(m_socket, MethodId::Resign, QByteArray());
}
