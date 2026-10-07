#ifndef CONNECTPAGE_H
#define CONNECTPAGE_H

#include <QWidget>

#include "peerremote.h"
#include "piece.h"

class NetworkSession;
class QLabel;
class QLineEdit;
class QPushButton;
class QStackedWidget;

// Cuida da UI e do handshake de rede (troca de "hello" + sorteio de quem comeca) para
// os dois modos: hospedar (QTcpServer aguardando conexao) e entrar (conecta em
// IP:porta digitados). O handshake agora e feito invocando diretamente os metodos
// remoteHello()/remoteStartGame() da interface PeerRemote -- ConnectPage implementa
// PeerRemote para RECEBER essas chamadas enquanto a partida ainda nao comecou (ver
// setLocalBoard). Emite sessionReady() quando a partida esta pronta para comecar.
class ConnectPage : public QWidget, public PeerRemote {
    Q_OBJECT

public:
    static constexpr quint16 kDefaultPort = 47321;

    explicit ConnectPage(QWidget *parent = nullptr);

    void startHosting(const QString &localName);
    void startJoining(const QString &localName);

    // PeerRemote: chamadas recebidas do outro lado durante o handshake.
    void remoteHello(const QString &playerName) override;
    void remoteStartGame(bool hostPlaysFirst) override;
    void remoteMove(const std::vector<MoveStepDTO> &steps) override;
    void remoteChat(const QString &text) override;
    void remoteResign() override;

signals:
    void sessionReady(NetworkSession *session, Player localPlayer, Player firstTurn,
                       const QString &localName, const QString &remoteName);
    void cancelled();

private slots:
    void onConnectClicked();
    void onCancelClicked();
    void onSessionConnected();
    void onSessionError(const QString &message);

private:
    void resetSession();
    QString localAddressesText() const;

    QStackedWidget *m_stack;

    // Pagina "hospedando"
    QLabel *m_hostInfoLabel;

    // Pagina "entrando"
    QLineEdit *m_hostEdit;
    QLineEdit *m_portEdit;
    QPushButton *m_connectButton;
    QLabel *m_joinStatusLabel;

    NetworkSession *m_session = nullptr;
    bool m_isHost = false;
    QString m_localName;
    QString m_remoteName;
};

#endif // CONNECTPAGE_H
