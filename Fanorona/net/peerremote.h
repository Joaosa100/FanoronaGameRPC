#ifndef PEERREMOTE_H
#define PEERREMOTE_H

#include <vector>

#include <QString>

#include "move.h"

// Um passo de movimento como atravessa a chamada remota: from/to/captureType
// descrevem o movimento; 'captured' viaja so como checagem de sanidade -- quem
// recebe a chamada NUNCA deve confiar cegamente nele, e sim recomputar com
// MoveRules (ver GameController::reconstructRemoteMove).
struct MoveStepDTO {
    int from = -1;
    int to = -1;
    CaptureType captureType = CaptureType::None;
    std::vector<int> captured;
};

// Interface remota -- equivalente a InverterItf.java (que estende Remote).
// Define as acoes que um lado pode invocar diretamente no "tabuleiro" do
// outro processo, como um objeto remoto RMI: nao existe aqui nenhum campo de
// "tipo de mensagem" -- cada acao e um metodo proprio da interface. Quem
// recebe a chamada (BoardSkeleton) invoca o metodo correspondente direto no
// objeto local, sem switch/case sobre string nem parsing de JSON.
class PeerRemote {
public:
    virtual ~PeerRemote() = default;

    virtual void remoteHello(const QString &playerName) = 0;
    virtual void remoteStartGame(bool hostPlaysFirst) = 0;
    virtual void remoteMove(const std::vector<MoveStepDTO> &steps) = 0;
    virtual void remoteChat(const QString &text) = 0;
    virtual void remoteResign() = 0;
};

#endif // PEERREMOTE_H
