#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include <QMap>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include "IrcConnection.h"
#include "ChatWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectAction();
    void onDisconnectAction();
    void onJoinChannelAction();
    void onQuitAction();
    
    // IRC connection handlers
    void onConnected();
    void onDisconnected();
    void onConnectionError(const QString &error);
    void onMessageReceived(const QString &sender, const QString &target, const QString &message);
    void onJoinedChannel(const QString &channel, const QString &user);
    void onPartedChannel(const QString &channel, const QString &user);
    void onUserListReceived(const QString &channel, const QStringList &users);
    void onTopicReceived(const QString &channel, const QString &topic);
    void onServerMessageReceived(const QString &message);
    
    // Chat widget handlers
    void onChatMessageSent(const QString &message);
    void onTabCloseRequested(int index);

private:
    void setupUi();
    void setupMenuBar();
    void createServerTab();
    ChatWidget* getOrCreateChatWidget(const QString &channelName);
    ChatWidget* getCurrentChatWidget();
    void showConnectionDialog();
    void showJoinChannelDialog();
    void updateWindowTitle();

    QTabWidget *m_tabWidget;
    IrcConnection *m_ircConnection;
    QMap<QString, ChatWidget*> m_chatWidgets;
    ChatWidget *m_serverWidget;
    
    QString m_currentNickname;
    QString m_currentServer;
    
    // Menu actions
    QAction *m_connectAction;
    QAction *m_disconnectAction;
    QAction *m_joinChannelAction;
};

#endif // MAINWINDOW_H
