#include "gamecontroller.h"

#include "boardgeometry.h"
#include "chatwidget.h"
#include "gamepage.h"
#include "move.h"
#include "moverules.h"
#include "networksession.h"

GameController::GameController(NetworkSession *session,
                                 GamePage *gamePage,
                                 Player localPlayer,
                                 Player firstTurn,
                                 const QString &localName,
                                 const QString &remoteName,
                                 QObject *parent)
    : QObject(parent)
    , m_session(session)
    , m_gamePage(gamePage)
    , m_localPlayer(localPlayer)
    , m_localName(localName)
    , m_remoteName(remoteName)
{
    m_session->setParent(this);
    // A partir de agora, chamadas remotas de move/chat/resign caem direto
    // nos metodos desta classe (ver remoteMove/remoteChat/remoteResign).
    m_session->setLocalBoard(this);

    const QString nameA = localPlayer == Player::PlayerA ? m_localName : m_remoteName;
    const QString nameB = localPlayer == Player::PlayerB ? m_localName : m_remoteName;
    m_gamePage->setPlayerNames(nameA, nameB);
    m_gamePage->setLocalPlayer(localPlayer);
    m_gamePage->startGame(firstTurn);

    connect(m_gamePage, &GamePage::moveFinished, this, &GameController::onLocalMoveFinished);
    connect(m_gamePage, &GamePage::gameOver, this, &GameController::onGamePageGameOver);
    connect(m_gamePage, &GamePage::resignRequested, this, &GameController::onResignRequested);
    connect(m_gamePage->chat(), &ChatWidget::messageSubmitted, this, &GameController::onChatSubmitted);

    connect(m_session, &NetworkSession::disconnected, this, &GameController::onNetworkDisconnected);

    m_gamePage->chat()->appendSystemMessage(tr("Partida iniciada. %1 joga primeiro.")
                                                 .arg(firstTurn == Player::PlayerA ? nameA : nameB));
}

void GameController::onLocalMoveFinished(const Move &move, Player /*mover*/)
{
    if (m_gameEnded)
        return;

    std::vector<MoveStepDTO> dtoSteps;
    for (const auto &step : move.steps) {
        MoveStepDTO dto;
        dto.from = step.from;
        dto.to = step.to;
        dto.captureType = step.captureType;
        dto.captured = step.captured;
        dtoSteps.push_back(dto);
    }
    // Invoca diretamente o metodo no tabuleiro remoto -- "o j1 move sua peça
    // no seu tabuleiro e invoca a função no tabuleiro do j2", sem montar
    // mensagem nem indicar tipo.
    m_session->remote()->remoteMove(dtoSteps);
}

Move GameController::reconstructRemoteMove(const std::vector<MoveStepDTO> &steps) const
{
    Board scratch = m_gamePage->currentBoard();
    const Player mover = opponent(m_localPlayer);

    Move move;
    for (const auto &dto : steps) {
        const auto options = MoveRules::optionsFor(scratch, dto.from, dto.to, mover);
        MoveOption chosen = options.front();
        for (const auto &option : options) {
            if (option.type == dto.captureType) {
                chosen = option;
                break;
            }
        }

        MoveStep step;
        step.from = dto.from;
        step.to = dto.to;
        step.direction = BoardGeometry::directionBetween(dto.from, dto.to).value_or(Direction::N);
        step.captureType = chosen.type;
        step.captured = chosen.captured;

        Move singleStep;
        singleStep.steps.push_back(step);
        MoveRules::applyMove(scratch, singleStep);

        move.steps.push_back(step);
    }
    return move;
}

void GameController::remoteMove(const std::vector<MoveStepDTO> &steps)
{
    if (m_gameEnded)
        return;
    m_endReason = tr("captura de todas as peças");
    const Move move = reconstructRemoteMove(steps);
    m_gamePage->applyExternalMove(move);
}

void GameController::remoteChat(const QString &text)
{
    m_gamePage->chat()->appendRemoteMessage(m_remoteName, text);
}

void GameController::remoteResign()
{
    if (m_gameEnded)
        return;
    m_endReason = tr("%1 desistiu").arg(m_remoteName);
    m_gamePage->forceGameOver(m_localPlayer);
}

void GameController::remoteHello(const QString &)
{
    // Handshake ja concluido pela ConnectPage antes do GameController existir.
}

void GameController::remoteStartGame(bool)
{
    // Idem.
}

void GameController::onChatSubmitted(const QString &text)
{
    m_session->remote()->remoteChat(text);
    m_gamePage->chat()->appendLocalMessage(m_localName, text);
}

void GameController::onResignRequested()
{
    if (m_gameEnded)
        return;
    m_session->remote()->remoteResign();
    m_endReason = tr("%1 desistiu").arg(m_localName);
    m_gamePage->forceGameOver(opponent(m_localPlayer));
}

void GameController::onGamePageGameOver(Player winner)
{
    if (m_gameEnded)
        return;
    m_gameEnded = true;
    if (m_endReason.isEmpty())
        m_endReason = tr("captura de todas as peças");
    emit gameOver(winner, m_endReason);
}

void GameController::onNetworkDisconnected()
{
    if (!m_gameEnded)
        m_gamePage->chat()->appendSystemMessage(tr("O oponente desconectou."));
}
