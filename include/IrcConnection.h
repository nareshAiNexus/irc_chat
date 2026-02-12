#ifndef IRCCONNECTION_H
#define IRCCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QStringList>

class IrcConnection : public QObject
{
    Q_OBJECT

public:
    explicit IrcConnection(QObject *parent = nullptr);
    ~IrcConnection();

    // Connection methods
    void connectToServer(const QString &host, quint16 port = 6667);
    void disconnect();
    bool isConnected() const;

    // IRC commands
    void sendRawMessage(const QString &message);
    void setNickname(const QString &nick);
    void joinChannel(const QString &channel);
    void partChannel(const QString &channel);
    void sendMessage(const QString &target, const QString &message);
    void sendPrivateMessage(const QString &user, const QString &message);

signals:
    // Connection signals
    void connected();
    void disconnected();
    void connectionError(const QString &error);

    // IRC message signals
    void messageReceived(const QString &sender, const QString &target, const QString &message);
    void noticeReceived(const QString &sender, const QString &message);
    void joinedChannel(const QString &channel, const QString &user);
    void partedChannel(const QString &channel, const QString &user);
    void userListReceived(const QString &channel, const QStringList &users);
    void topicReceived(const QString &channel, const QString &topic);
    void serverMessageReceived(const QString &message);
    void nickChanged(const QString &oldNick, const QString &newNick);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    void parseIrcMessage(const QString &line);
    void handleNumericReply(int code, const QStringList &params);
    QString extractNickname(const QString &prefix);

    QTcpSocket *m_socket;
    QString m_nickname;
    QString m_server;
    quint16 m_port;
    QString m_buffer;
};

#endif // IRCCONNECTION_H
