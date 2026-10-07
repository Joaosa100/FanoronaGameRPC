#include "boardskeleton.h"

#include <QDataStream>
#include <QTcpSocket>

#include "boardwire.h"
#include "peerremote.h"

BoardSkeleton::BoardSkeleton(QTcpSocket *socket, QObject *parent)
    : QObject(parent)
    , m_socket(socket)
{
    connect(m_socket, &QTcpSocket::readyRead, this, &BoardSkeleton::onReadyRead);
}

void BoardSkeleton::setTarget(PeerRemote *target)
{
    m_target = target;
}

void BoardSkeleton::onReadyRead()
{
    m_buffer += m_socket->readAll();

    // Framing binario por tamanho: le 4 bytes de cabecalho (tamanho do
    // restante do frame) e so processa quando o frame inteiro ja chegou.
    // Resolve o mesmo problema do delimitador '\n' na versao em sockets
    // (TCP pode entregar varios frames grudados ou um frame partido em
    // pedacos), so que usando tamanho explicito em vez de um caractere
    // separador -- necessario porque aqui o payload e binario.
    while (true) {
        if (m_buffer.size() < 4)
            return;

        QDataStream header(m_buffer);
        quint32 frameLen = 0;
        header >> frameLen;

        if (static_cast<quint32>(m_buffer.size()) < 4 + frameLen)
            return; // frame ainda incompleto, espera o resto chegar

        const quint8 methodId = static_cast<quint8>(m_buffer.at(4));
        const QByteArray payload = m_buffer.mid(5, frameLen - 1);
        m_buffer.remove(0, 4 + frameLen);

        dispatch(methodId, payload);
    }
}

void BoardSkeleton::dispatch(quint8 methodId, const QByteArray &payload)
{
    if (!m_target)
        return;

    // Unico ponto de "roteamento": por numero de metodo, nao por string.
    // Cada case invoca diretamente o metodo correspondente da interface no
    // objeto local -- e a parte que, no RMI real, e gerada automaticamente.
    switch (static_cast<MethodId>(methodId)) {
    case MethodId::Hello:
        m_target->remoteHello(decodeHello(payload));
        break;
    case MethodId::StartGame:
        m_target->remoteStartGame(decodeStartGame(payload));
        break;
    case MethodId::Move:
        m_target->remoteMove(decodeMove(payload));
        break;
    case MethodId::Chat:
        m_target->remoteChat(decodeChat(payload));
        break;
    case MethodId::Resign:
        m_target->remoteResign();
        break;
    }
}
