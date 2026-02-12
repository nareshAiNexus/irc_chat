#include "ChatWidget.h"
#include <QDateTime>
#include <QLabel>
#include <QPushButton>

ChatWidget::ChatWidget(const QString &channelName, QWidget *parent)
    : QWidget(parent)
    , m_channelName(channelName)
{
    setupUi();
}

void ChatWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Topic bar
    m_topicLabel = new QLabel(tr("No topic set"));
    m_topicLabel->setWordWrap(true);
    m_topicLabel->setStyleSheet("QLabel { background-color: #f0f0f0; padding: 5px; border-bottom: 1px solid #ccc; }");
    m_topicLabel->setMaximumHeight(50);
    mainLayout->addWidget(m_topicLabel);
    
    // Splitter for chat area and user list
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    
    // Chat display area
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setFont(QFont("Monospace", 10));
    splitter->addWidget(m_chatDisplay);
    
    // User list
    m_userList = new QListWidget();
    m_userList->setMaximumWidth(150);
    m_userList->setMinimumWidth(100);
    m_userList->setSortingEnabled(true);
    splitter->addWidget(m_userList);
    
    // Set splitter sizes (chat gets more space)
    splitter->setStretchFactor(0, 3);
    splitter->setStretchFactor(1, 1);
    
    mainLayout->addWidget(splitter);
    
    // Input area
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    m_inputLine = new QLineEdit();
    m_inputLine->setPlaceholderText(tr("Type a message..."));
    connect(m_inputLine, &QLineEdit::returnPressed, this, &ChatWidget::onSendMessage);
    
    QPushButton *sendButton = new QPushButton(tr("Send"));
    connect(sendButton, &QPushButton::clicked, this, &ChatWidget::onSendMessage);
    
    inputLayout->addWidget(m_inputLine);
    inputLayout->addWidget(sendButton);
    
    mainLayout->addLayout(inputLayout);
}

void ChatWidget::addMessage(const QString &sender, const QString &message)
{
    QString formattedMsg = formatMessage(sender, message);
    m_chatDisplay->append(formattedMsg);
}

void ChatWidget::addSystemMessage(const QString &message)
{
    QString timestamp = getCurrentTime();
    QString formattedMsg = QString("<span style='color: green;'>[%1] * %2</span>")
                          .arg(timestamp, message);
    m_chatDisplay->append(formattedMsg);
}

void ChatWidget::setUserList(const QStringList &users)
{
    m_userList->clear();
    
    for (const QString &user : users) {
        QString cleanUser = user;
        // Remove IRC user modes (@, +, etc.)
        if (!cleanUser.isEmpty() && (cleanUser[0] == '@' || cleanUser[0] == '+' || 
            cleanUser[0] == '%' || cleanUser[0] == '~' || cleanUser[0] == '&')) {
            m_userList->addItem(cleanUser);
        } else {
            m_userList->addItem(cleanUser);
        }
    }
}

void ChatWidget::addUser(const QString &user)
{
    if (m_userList->findItems(user, Qt::MatchExactly).isEmpty()) {
        m_userList->addItem(user);
    }
}

void ChatWidget::removeUser(const QString &user)
{
    QList<QListWidgetItem*> items = m_userList->findItems(user, Qt::MatchExactly);
    for (QListWidgetItem *item : items) {
        delete m_userList->takeItem(m_userList->row(item));
    }
}

void ChatWidget::setTopic(const QString &topic)
{
    if (topic.isEmpty()) {
        m_topicLabel->setText(tr("No topic set"));
    } else {
        m_topicLabel->setText(QString("Topic: %1").arg(topic));
    }
}

void ChatWidget::onSendMessage()
{
    QString message = m_inputLine->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    emit messageSent(message);
    m_inputLine->clear();
}

QString ChatWidget::formatMessage(const QString &sender, const QString &message)
{
    QString timestamp = getCurrentTime();
    QString color = sender == "SERVER" ? "blue" : "black";
    
    return QString("<span style='color: gray;'>[%1]</span> "
                  "<span style='color: %2; font-weight: bold;'>&lt;%3&gt;</span> %4")
           .arg(timestamp, color, sender, message.toHtmlEscaped());
}

QString ChatWidget::getCurrentTime()
{
    return QDateTime::currentDateTime().toString("HH:mm:ss");
}
