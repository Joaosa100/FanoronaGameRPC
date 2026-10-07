#ifndef BOARDSKELETON_H
#define BOARDSKELETON_H

#include <QByteArray>
#include <QObject>

class QTcpSocket;
class PeerRemote;

// Skeleton: do lado de quem recebe, le os frames binarios que chegam do
// socket, identifica qual metodo da interface PeerRemote foi chamado (por um
// numero, o MethodId -- nunca uma string de "tipo") e invoca esse metodo
// diretamente no objeto alvo local. E o equivalente ao que a maquina do RMI
// faz escondida atras do stub remoto gerado automaticamente a partir da
// interface.
class BoardSkeleton : public QObject {
    Q_OBJECT

public:
    explicit BoardSkeleton(QTcpSocket *socket, QObject *parent = nullptr);

    // Objeto local que efetivamente executa as chamadas recebidas (o
    // "Servidor" de verdade). Pode ser trocado em tempo de execucao -- e
    // assim que a troca da ConnectPage para o GameController acontece
    // quando a partida comeca.
    void setTarget(PeerRemote *target);

private slots:
    void onReadyRead();

private:
    void dispatch(quint8 methodId, const QByteArray &payload);

    QTcpSocket *m_socket;
    PeerRemote *m_target = nullptr;
    QByteArray m_buffer;
};

#endif // BOARDSKELETON_H
