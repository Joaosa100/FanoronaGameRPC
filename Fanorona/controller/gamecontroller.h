#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <vector>

#include <QObject>
#include <QString>

#include "peerremote.h"
#include "piece.h"

class GamePage;
class NetworkSession;

struct Move;

// Liga GamePage (UI+regras de interacao) a NetworkSession (RPC caseiro), aplicando a
// atribuicao de cores/turno inicial e traduzindo eventos locais em chamadas remotas
// (e chamadas remotas recebidas em acoes locais). GameController implementa PeerRemote
// para SER o "tabuleiro" que o lado remoto invoca -- equivalente ao Servidor.java do
// exemplo em aula, que implementa de fato a interface remota.
class GameController : public QObject, public PeerRemote {
    Q_OBJECT

public:
    GameController(NetworkSession *session,
                    GamePage *gamePage,
                    Player localPlayer,
                    Player firstTurn,
                    const QString &localName,
                    const QString &remoteName,
                    QObject *parent = nullptr);

    // PeerRemote: chamadas invocadas pelo lado remoto neste tabuleiro.
    void remoteHello(const QString &playerName) override;
    void remoteStartGame(bool hostPlaysFirst) override;
    void remoteMove(const std::vector<MoveStepDTO> &steps) override;
    void remoteChat(const QString &text) override;
    void remoteResign() override;

signals:
    void gameOver(Player winner, const QString &reason);

private slots:
    void onLocalMoveFinished(const Move &move, Player mover);
    void onChatSubmitted(const QString &text);
    void onResignRequested();
    void onGamePageGameOver(Player winner);
    void onNetworkDisconnected();

private:
    Move reconstructRemoteMove(const std::vector<MoveStepDTO> &steps) const;

    NetworkSession *m_session;
    GamePage *m_gamePage;
    Player m_localPlayer;
    QString m_localName;
    QString m_remoteName;
    QString m_endReason;
    bool m_gameEnded = false;
};

#endif // GAMECONTROLLER_H
