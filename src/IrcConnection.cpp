#include "IrcConnection.h"
#include <QDebug>
#include <QRegularExpression>

IrcConnection::IrcConnection(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
    , m_port(6667)
{
    connect(m_socket, &QTcpSocket::connected, this, &IrcConnection::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &IrcConnection::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &IrcConnection::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &IrcConnection::onSocketError);
}

IrcConnection::~IrcConnection()
{
    if (m_socket->isOpen()) {
        disconnect();
    }
}

void IrcConnection::connectToServer(const QString &host, quint16 port)
{
    m_server = host;
    m_port = port;
    
    qDebug() << "Connecting to" << host << ":" << port;
    m_socket->connectToHost(host, port);
}

void IrcConnection::disconnect()
{
    if (m_socket->isOpen()) {
        sendRawMessage("QUIT :Leaving");
        m_socket->disconnectFromHost();
    }
}

bool IrcConnection::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void IrcConnection::sendRawMessage(const QString &message)
{
    if (!isConnected()) {
        qWarning() << "Not connected to server";
        return;
    }
    
    QString msg = message + "\r\n";
    qDebug() << ">> " << message;
    m_socket->write(msg.toUtf8());
    m_socket->flush();
}

void IrcConnection::setNickname(const QString &nick)
{
    m_nickname = nick;
    sendRawMessage(QString("NICK %1").arg(nick));
    sendRawMessage(QString("USER %1 0 * :%1").arg(nick));
}

void IrcConnection::joinChannel(const QString &channel)
{
    sendRawMessage(QString("JOIN %1").arg(channel));
}

void IrcConnection::partChannel(const QString &channel)
{
    sendRawMessage(QString("PART %1").arg(channel));
}

void IrcConnection::sendMessage(const QString &target, const QString &message)
{
    sendRawMessage(QString("PRIVMSG %1 :%2").arg(target, message));
}

void IrcConnection::sendPrivateMessage(const QString &user, const QString &message)
{
    sendMessage(user, message);
}

void IrcConnection::onConnected()
{
    qDebug() << "Connected to server";
    emit connected();
}

void IrcConnection::onDisconnected()
{
    qDebug() << "Disconnected from server";
    emit disconnected();
}

void IrcConnection::onReadyRead()
{
    while (m_socket->canReadLine()) {
        QString line = QString::fromUtf8(m_socket->readLine()).trimmed();
        
        if (!line.isEmpty()) {
            qDebug() << "<< " << line;
            parseIrcMessage(line);
        }
    }
}

void IrcConnection::onSocketError(QAbstractSocket::SocketError error)
{
    QString errorStr = m_socket->errorString();
    qWarning() << "Socket error:" << errorStr;
    emit connectionError(errorStr);
}

QString IrcConnection::extractNickname(const QString &prefix)
{
    int exclamationPos = prefix.indexOf('!');
    if (exclamationPos > 0) {
        return prefix.left(exclamationPos);
    }
    return prefix;
}

void IrcConnection::parseIrcMessage(const QString &line)
{
    // IRC message format: [:prefix] command [params] [:trailing]
    QString message = line;
    QString prefix;
    
    // Extract prefix if present
    if (message.startsWith(':')) {
        int spacePos = message.indexOf(' ');
        prefix = message.mid(1, spacePos - 1);
        message = message.mid(spacePos + 1).trimmed();
    }
    
    // Split into command and parameters
    QStringList parts = message.split(' ');
    if (parts.isEmpty()) return;
    
    QString command = parts.takeFirst();
    
    // Extract trailing parameter if present
    QString trailing;
    for (int i = 0; i < parts.size(); ++i) {
        if (parts[i].startsWith(':')) {
            trailing = parts.mid(i).join(' ').mid(1);
            parts = parts.mid(0, i);
            break;
        }
    }
    
    // Handle PING (server keepalive)
    if (command == "PING") {
        sendRawMessage(QString("PONG %1").arg(trailing.isEmpty() ? parts.join(' ') : trailing));
        return;
    }
    
    // Handle numeric replies
    bool isNumeric = false;
    int numericCode = command.toInt(&isNumeric);
    if (isNumeric) {
        QStringList params = parts;
        if (!trailing.isEmpty()) {
            params.append(trailing);
        }
        handleNumericReply(numericCode, params);
        return;
    }
    
    // Handle text commands
    if (command == "PRIVMSG") {
        if (parts.size() >= 1) {
            QString target = parts[0];
            QString sender = extractNickname(prefix);
            emit messageReceived(sender, target, trailing);
        }
    }
    else if (command == "NOTICE") {
        QString sender = extractNickname(prefix);
        emit noticeReceived(sender, trailing);
    }
    else if (command == "JOIN") {
        QString channel = trailing.isEmpty() ? parts[0] : trailing;
        QString user = extractNickname(prefix);
        emit joinedChannel(channel, user);
    }
    else if (command == "PART") {
        if (parts.size() >= 1) {
            QString channel = parts[0];
            QString user = extractNickname(prefix);
            emit partedChannel(channel, user);
        }
    }
    else if (command == "NICK") {
        QString oldNick = extractNickname(prefix);
        QString newNick = trailing.isEmpty() ? parts[0] : trailing;
        emit nickChanged(oldNick, newNick);
    }
}

void IrcConnection::handleNumericReply(int code, const QStringList &params)
{
    switch (code) {
        case 1: // RPL_WELCOME
            emit serverMessageReceived(params.join(' '));
            break;
            
        case 332: // RPL_TOPIC
            if (params.size() >= 2) {
                QString channel = params[1];
                QString topic = params.size() > 2 ? params.mid(2).join(' ') : "";
                emit topicReceived(channel, topic);
            }
            break;
            
        case 353: // RPL_NAMREPLY (user list)
            if (params.size() >= 3) {
                QString channel = params[2];
                QString userList = params.size() > 3 ? params.mid(3).join(' ') : "";
                QStringList users = userList.split(' ', Qt::SkipEmptyParts);
                emit userListReceived(channel, users);
            }
            break;
            
        case 366: // RPL_ENDOFNAMES
            // End of NAMES list
            break;
            
        default:
            // Generic server message
            emit serverMessageReceived(QString::number(code) + " " + params.join(' '));
            break;
    }
}
