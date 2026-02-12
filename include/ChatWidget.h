#ifndef CHATWIDGET_H
#define CHATWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>

class ChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChatWidget(const QString &channelName, QWidget *parent = nullptr);
    
    QString getChannelName() const { return m_channelName; }
    void addMessage(const QString &sender, const QString &message);
    void addSystemMessage(const QString &message);
    void setUserList(const QStringList &users);
    void addUser(const QString &user);
    void removeUser(const QString &user);
    void setTopic(const QString &topic);

signals:
    void messageSent(const QString &message);

private slots:
    void onSendMessage();

private:
    void setupUi();
    QString formatMessage(const QString &sender, const QString &message);
    QString getCurrentTime();

    QString m_channelName;
    QTextEdit *m_chatDisplay;
    QLineEdit *m_inputLine;
    QListWidget *m_userList;
    QLabel *m_topicLabel;
};

#endif // CHATWIDGET_H
