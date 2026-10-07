#ifndef BOARDWIRE_H
#define BOARDWIRE_H

#include <QByteArray>

#include "peerremote.h"

// Formato de transporte usado pelo BoardStub/BoardSkeleton para levar uma
// chamada de metodo da interface PeerRemote de um processo a outro.
//
// Isso e o equivalente a serializacao de objetos que o RMI faz escondido por
// tras do stub/skeleton gerados automaticamente a partir da interface -- aqui
// fazemos essa parte a mao, em binario (nao JSON/texto), porque o metodo
// chamado e identificado por um numero (MethodId), nao por uma string de
// "tipo" que precisaria ser comparada em um if/else.
enum class MethodId : quint8 {
    Hello = 0,
    StartGame = 1,
    Move = 2,
    Chat = 3,
    Resign = 4,
};

QByteArray encodeHello(const QString &playerName);
QByteArray encodeStartGame(bool hostPlaysFirst);
QByteArray encodeMove(const std::vector<MoveStepDTO> &steps);
QByteArray encodeChat(const QString &text);

QString decodeHello(const QByteArray &payload);
bool decodeStartGame(const QByteArray &payload);
std::vector<MoveStepDTO> decodeMove(const QByteArray &payload);
QString decodeChat(const QByteArray &payload);

#endif // BOARDWIRE_H
