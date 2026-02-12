#include "MainWindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QStatusBar>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ircConnection(new IrcConnection(this))
    , m_serverWidget(nullptr)
{
    setupUi();
    setupMenuBar();
    
    // Connect IRC signals
    connect(m_ircConnection, &IrcConnection::connected, 
            this, &MainWindow::onConnected);
    connect(m_ircConnection, &IrcConnection::disconnected, 
            this, &MainWindow::onDisconnected);
    connect(m_ircConnection, &IrcConnection::connectionError, 
            this, &MainWindow::onConnectionError);
    connect(m_ircConnection, &IrcConnection::messageReceived, 
            this, &MainWindow::onMessageReceived);
    connect(m_ircConnection, &IrcConnection::joinedChannel, 
            this, &MainWindow::onJoinedChannel);
    connect(m_ircConnection, &IrcConnection::partedChannel, 
            this, &MainWindow::onPartedChannel);
    connect(m_ircConnection, &IrcConnection::userListReceived, 
            this, &MainWindow::onUserListReceived);
    connect(m_ircConnection, &IrcConnection::topicReceived, 
            this, &MainWindow::onTopicReceived);
    connect(m_ircConnection, &IrcConnection::serverMessageReceived, 
            this, &MainWindow::onServerMessageReceived);
    
    updateWindowTitle();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    setWindowTitle(tr("IRC Client"));
    resize(900, 600);
    
    // Central widget with tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setTabsClosable(true);
    m_tabWidget->setMovable(true);
    
    connect(m_tabWidget, &QTabWidget::tabCloseRequested, 
            this, &MainWindow::onTabCloseRequested);
    
    setCentralWidget(m_tabWidget);
    
    // Status bar
    statusBar()->showMessage(tr("Not connected"));
    
    // Create server tab
    createServerTab();
}

void MainWindow::setupMenuBar()
{
    QMenuBar *menuBar = new QMenuBar(this);
    
    // Server menu
    QMenu *serverMenu = menuBar->addMenu(tr("&Server"));
    
    m_connectAction = serverMenu->addAction(tr("&Connect..."));
    connect(m_connectAction, &QAction::triggered, this, &MainWindow::onConnectAction);
    
    m_disconnectAction = serverMenu->addAction(tr("&Disconnect"));
    m_disconnectAction->setEnabled(false);
    connect(m_disconnectAction, &QAction::triggered, this, &MainWindow::onDisconnectAction);
    
    serverMenu->addSeparator();
    
    QAction *quitAction = serverMenu->addAction(tr("&Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuitAction);
    
    // Channel menu
    QMenu *channelMenu = menuBar->addMenu(tr("&Channel"));
    
    m_joinChannelAction = channelMenu->addAction(tr("&Join Channel..."));
    m_joinChannelAction->setEnabled(false);
    connect(m_joinChannelAction, &QAction::triggered, this, &MainWindow::onJoinChannelAction);
    
    setMenuBar(menuBar);
}

void MainWindow::createServerTab()
{
    m_serverWidget = new ChatWidget("Server", this);
    m_tabWidget->addTab(m_serverWidget, "Server");
    m_tabWidget->setTabsClosable(true);
    
    // Server tab should not be closable
    m_tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr);
}

ChatWidget* MainWindow::getOrCreateChatWidget(const QString &channelName)
{
    if (m_chatWidgets.contains(channelName)) {
        return m_chatWidgets[channelName];
    }
    
    ChatWidget *chatWidget = new ChatWidget(channelName, this);
    m_chatWidgets[channelName] = chatWidget;
    
    connect(chatWidget, &ChatWidget::messageSent, 
            this, &MainWindow::onChatMessageSent);
    
    int index = m_tabWidget->addTab(chatWidget, channelName);
    m_tabWidget->setCurrentIndex(index);
    
    return chatWidget;
}

ChatWidget* MainWindow::getCurrentChatWidget()
{
    return qobject_cast<ChatWidget*>(m_tabWidget->currentWidget());
}

void MainWindow::showConnectionDialog()
{
    bool ok;
    QString server = QInputDialog::getText(this, tr("Connect to Server"),
                                          tr("Server address:"), 
                                          QLineEdit::Normal,
                                          "irc.libera.chat", &ok);
    if (!ok || server.isEmpty()) {
        return;
    }
    
    QString nickname = QInputDialog::getText(this, tr("Set Nickname"),
                                            tr("Nickname:"), 
                                            QLineEdit::Normal,
                                            "QtIRCUser", &ok);
    if (!ok || nickname.isEmpty()) {
        return;
    }
    
    m_currentNickname = nickname;
    m_currentServer = server;
    
    m_serverWidget->addSystemMessage(QString("Connecting to %1...").arg(server));
    m_ircConnection->connectToServer(server);
}

void MainWindow::showJoinChannelDialog()
{
    bool ok;
    QString channel = QInputDialog::getText(this, tr("Join Channel"),
                                           tr("Channel name (e.g., #general):"), 
                                           QLineEdit::Normal,
                                           "#general", &ok);
    if (ok && !channel.isEmpty()) {
        if (!channel.startsWith('#')) {
            channel = "#" + channel;
        }
        m_ircConnection->joinChannel(channel);
        m_serverWidget->addSystemMessage(QString("Joining %1...").arg(channel));
    }
}

void MainWindow::updateWindowTitle()
{
    QString title = "IRC Client";
    if (!m_currentServer.isEmpty()) {
        title += QString(" - %1@%2").arg(m_currentNickname, m_currentServer);
    }
    setWindowTitle(title);
}

void MainWindow::onConnectAction()
{
    showConnectionDialog();
}

void MainWindow::onDisconnectAction()
{
    m_ircConnection->disconnect();
}

void MainWindow::onJoinChannelAction()
{
    showJoinChannelDialog();
}

void MainWindow::onQuitAction()
{
    QApplication::quit();
}

void MainWindow::onConnected()
{
    m_serverWidget->addSystemMessage("Connected to server!");
    statusBar()->showMessage(tr("Connected to %1").arg(m_currentServer));
    
    // Set nickname
    m_ircConnection->setNickname(m_currentNickname);
    
    // Update UI
    m_connectAction->setEnabled(false);
    m_disconnectAction->setEnabled(true);
    m_joinChannelAction->setEnabled(true);
    
    updateWindowTitle();
}

void MainWindow::onDisconnected()
{
    m_serverWidget->addSystemMessage("Disconnected from server");
    statusBar()->showMessage(tr("Not connected"));
    
    // Update UI
    m_connectAction->setEnabled(true);
    m_disconnectAction->setEnabled(false);
    m_joinChannelAction->setEnabled(false);
    
    // Clear channel tabs (keep server tab)
    while (m_tabWidget->count() > 1) {
        m_tabWidget->removeTab(1);
    }
    m_chatWidgets.clear();
}

void MainWindow::onConnectionError(const QString &error)
{
    m_serverWidget->addSystemMessage(QString("Connection error: %1").arg(error));
    QMessageBox::warning(this, tr("Connection Error"), error);
}

void MainWindow::onMessageReceived(const QString &sender, const QString &target, const QString &message)
{
    // Determine which widget to display the message in
    ChatWidget *widget = nullptr;
    
    if (target.startsWith('#')) {
        // Channel message
        widget = m_chatWidgets.value(target, nullptr);
    } else if (target == m_currentNickname) {
        // Private message to us
        widget = getOrCreateChatWidget(sender);
    }
    
    if (widget) {
        widget->addMessage(sender, message);
    } else {
        m_serverWidget->addMessage(sender, QString("[%1] %2").arg(target, message));
    }
}

void MainWindow::onJoinedChannel(const QString &channel, const QString &user)
{
    ChatWidget *widget = getOrCreateChatWidget(channel);
    
    if (user == m_currentNickname) {
        widget->addSystemMessage(QString("You have joined %1").arg(channel));
    } else {
        widget->addSystemMessage(QString("%1 has joined").arg(user));
        widget->addUser(user);
    }
}

void MainWindow::onPartedChannel(const QString &channel, const QString &user)
{
    ChatWidget *widget = m_chatWidgets.value(channel, nullptr);
    if (!widget) return;
    
    if (user == m_currentNickname) {
        widget->addSystemMessage(QString("You have left %1").arg(channel));
        // Optionally close the tab
    } else {
        widget->addSystemMessage(QString("%1 has left").arg(user));
        widget->removeUser(user);
    }
}

void MainWindow::onUserListReceived(const QString &channel, const QStringList &users)
{
    ChatWidget *widget = m_chatWidgets.value(channel, nullptr);
    if (widget) {
        widget->setUserList(users);
    }
}

void MainWindow::onTopicReceived(const QString &channel, const QString &topic)
{
    ChatWidget *widget = m_chatWidgets.value(channel, nullptr);
    if (widget) {
        widget->setTopic(topic);
        widget->addSystemMessage(QString("Topic: %1").arg(topic));
    }
}

void MainWindow::onServerMessageReceived(const QString &message)
{
    m_serverWidget->addSystemMessage(message);
}

void MainWindow::onChatMessageSent(const QString &message)
{
    ChatWidget *sender = qobject_cast<ChatWidget*>(QObject::sender());
    if (!sender) return;
    
    QString target = sender->getChannelName();
    
    // Handle IRC commands
    if (message.startsWith('/')) {
        QStringList parts = message.mid(1).split(' ', Qt::SkipEmptyParts);
        if (parts.isEmpty()) return;
        
        QString command = parts[0].toUpper();
        
        if (command == "JOIN" && parts.size() > 1) {
            QString channel = parts[1];
            if (!channel.startsWith('#')) channel = "#" + channel;
            m_ircConnection->joinChannel(channel);
        }
        else if (command == "PART" || command == "LEAVE") {
            m_ircConnection->partChannel(target);
        }
        else if (command == "QUIT") {
            m_ircConnection->disconnect();
        }
        else if (command == "MSG" && parts.size() > 2) {
            QString recipient = parts[1];
            QString msg = parts.mid(2).join(' ');
            m_ircConnection->sendMessage(recipient, msg);
            sender->addMessage(m_currentNickname, msg);
        }
        else {
            // Raw IRC command
            m_ircConnection->sendRawMessage(message.mid(1));
        }
    } else {
        // Regular message
        m_ircConnection->sendMessage(target, message);
        sender->addMessage(m_currentNickname, message);
    }
}

void MainWindow::onTabCloseRequested(int index)
{
    if (index == 0) {
        // Don't close server tab
        return;
    }
    
    ChatWidget *widget = qobject_cast<ChatWidget*>(m_tabWidget->widget(index));
    if (widget) {
        QString channelName = widget->getChannelName();
        
        // Part the channel if it's a channel (starts with #)
        if (channelName.startsWith('#')) {
            m_ircConnection->partChannel(channelName);
        }
        
        // Remove from our map and delete the widget
        m_chatWidgets.remove(channelName);
        m_tabWidget->removeTab(index);
        widget->deleteLater();
    }
}
