#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "piece.h"

class ConnectPage;
class GameController;
class GamePage;
class HomePage;
class NetworkSession;
class QStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onSessionReady(NetworkSession *session, Player localPlayer, Player firstTurn,
                         const QString &localName, const QString &remoteName);
    void onGameOver(Player winner, const QString &reason);

private:
    QStackedWidget *m_stack;
    HomePage *m_homePage;
    ConnectPage *m_connectPage;
    GamePage *m_gamePage = nullptr;
    GameController *m_controller = nullptr;
    Player m_localPlayer = Player::None;
};
#endif // MAINWINDOW_H
