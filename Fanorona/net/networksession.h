#ifndef NETWORKSESSION_H
#define NETWORKSESSION_H

#include <memory>

#include <QAbstractSocket>
#include <QObject>

class QTcpServer;
class QTcpSocket;
class BoardStub;
class BoardSkeleton;
class PeerRemote;

// Uma sessao ponto-a-ponto entre os dois jogadores: o host abre um QTcpServer
// e aceita a primeira conexao recebida; o client conecta diretamente via
// IP:porta. Essa parte faz o papel simplificado do servico de nomes (SN) do
// RMI -- em vez de Naming.rebind()/Naming.lookup() por um nome registrado,
// aqui a "descoberta" do objeto remoto e o IP:porta digitado na ConnectPage.
//
// Depois de conectados, cada lado tem:
//  - um BoardStub (remote()), a referencia para invocar metodos no
//    "tabuleiro" do processo do outro jogador -- equivalente ao objeto
//    devolvido por Naming.lookup() no exemplo em Java;
//  - um BoardSkeleton interno, que recebe as chamadas feitas pelo lado
//    remoto e as invoca diretamente no objeto local passado a
//    setLocalBoard() -- equivalente ao "Servidor" que de fato implementa a
//    interface.
class NetworkSession : public QObject {
    Q_OBJECT

public:
    explicit NetworkSession(QObject *parent = nullptr);
    ~NetworkSession() override;

    bool startHosting(quint16 port, QString *errorMessage = nullptr);
    void connectToHost(const QString &host, quint16 port);

    // Referencia remota: chamar um metodo aqui executa a acao no tabuleiro
    // do processo do outro jogador. So valido depois do sinal connected().
    PeerRemote *remote() const;

    // Objeto local que recebe e executa as chamadas feitas pelo lado remoto.
    void setLocalBoard(PeerRemote *localBoard);

    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void errorOccurred(const QString &message);

private slots:
    void handleNewConnection();
    void handleSocketConnected();
    void handleSocketError(QAbstractSocket::SocketError socketError);
    void handleDisconnected();

private:
    void attachSocket(QTcpSocket *socket);

    QTcpServer *m_server = nullptr;
    QTcpSocket *m_socket = nullptr;
    std::unique_ptr<BoardStub> m_stub;
    std::unique_ptr<BoardSkeleton> m_skeleton;
    PeerRemote *m_pendingLocalBoard = nullptr;
};

#endif // NETWORKSESSION_H
