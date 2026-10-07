#ifndef BOARDSTUB_H
#define BOARDSTUB_H

#include "peerremote.h"

class QTcpSocket;

// Stub: a referencia que o lado local guarda para o "tabuleiro remoto" do
// adversario -- equivalente ao objeto devolvido por Naming.lookup() no
// exemplo em Java, que implementa a mesma interface remota (InverterItf) e
// esconde a rede por tras da chamada de metodo.
//
// Chamar qualquer metodo aqui (remoteMove, remoteChat, ...) parece uma
// chamada local, mas na verdade serializa a chamada (ver boardwire.h) e
// escreve os bytes no socket -- nao ha criacao de string nem de JSON no
// ponto de chamada, so a invocacao direta do metodo da interface.
class BoardStub : public PeerRemote {
public:
    explicit BoardStub(QTcpSocket *socket);

    void remoteHello(const QString &playerName) override;
    void remoteStartGame(bool hostPlaysFirst) override;
    void remoteMove(const std::vector<MoveStepDTO> &steps) override;
    void remoteChat(const QString &text) override;
    void remoteResign() override;

private:
    QTcpSocket *m_socket;
};

#endif // BOARDSTUB_H
