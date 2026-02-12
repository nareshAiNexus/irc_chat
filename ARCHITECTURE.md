# IRC Client Architecture Guide

## Component Breakdown

### 1. IrcConnection (Network Layer)
**File**: `src/IrcConnection.cpp`, `include/IrcConnection.h`

**Responsibilities**:
- Manages TCP socket connection to IRC server
- Implements IRC protocol (message parsing, formatting)
- Handles server PING/PONG keepalive
- Emits Qt signals for UI to react to events

**Key Methods**:
```cpp
void connectToServer(host, port)    // Establish connection
void setNickname(nick)               // Set user nickname  
void joinChannel(channel)            // Join IRC channel
void sendMessage(target, message)    // Send PRIVMSG
void parseIrcMessage(line)           // Parse IRC protocol
```

**Signals Emitted**:
- `messageReceived(sender, target, message)` - Chat message
- `joinedChannel(channel, user)` - Someone joined
- `userListReceived(channel, users)` - Channel user list
- `topicReceived(channel, topic)` - Channel topic
- `connected()` / `disconnected()` - Connection status

### 2. ChatWidget (UI Component)
**File**: `src/ChatWidget.cpp`, `include/ChatWidget.h`

**Responsibilities**:
- Display chat messages for ONE channel/conversation
- Show user list on the right side
- Display topic bar at top
- Handle user input (message box at bottom)

**Key Components**:
```
┌────────────────────────────────────────┐
│ Topic: Welcome to #channel             │ ← Topic label
├──────────────────────────┬─────────────┤
│ [12:34] <Alice> Hello    │  @ops       │ ← Chat display + user list
│ [12:35] <Bob> Hi!        │  +voice     │
│ [12:36] * You joined     │  Alice      │
│                          │  Bob        │
├──────────────────────────┴─────────────┤
│ [Type message...]          [Send]      │ ← Input area
└────────────────────────────────────────┘
```

### 3. MainWindow (Application Controller)
**File**: `src/MainWindow.cpp`, `include/MainWindow.h`

**Responsibilities**:
- Create and manage multiple ChatWidget tabs
- Connect UI events to IrcConnection
- Handle menus (Connect, Join Channel, etc.)
- Route messages to correct chat tab

**Structure**:
```
MainWindow
├── QTabWidget (m_tabWidget)
│   ├── Tab 0: Server (always present)
│   ├── Tab 1: #channel1
│   ├── Tab 2: #channel2
│   └── Tab 3: PrivateMsg with Alice
├── IrcConnection (m_ircConnection)
└── QMap<QString, ChatWidget*> (m_chatWidgets)
```

## Data Flow Examples

### Example 1: User Sends a Message

```
User types "Hello" and presses Enter
        ↓
ChatWidget::onSendMessage()
        ↓
emit messageSent("Hello")
        ↓
MainWindow::onChatMessageSent()
        ↓
IrcConnection::sendMessage("#channel", "Hello")
        ↓
QTcpSocket sends: "PRIVMSG #channel :Hello\r\n"
        ↓
IRC Server receives message
        ↓
Server broadcasts to all channel members
```

### Example 2: Receiving a Message

```
IRC Server sends: ":Alice!user@host PRIVMSG #channel :Hi there\r\n"
        ↓
QTcpSocket::readyRead() signal
        ↓
IrcConnection::onReadyRead()
        ↓
IrcConnection::parseIrcMessage(line)
        ↓
emit messageReceived("Alice", "#channel", "Hi there")
        ↓
MainWindow::onMessageReceived()
        ↓
Find or create ChatWidget for #channel
        ↓
ChatWidget::addMessage("Alice", "Hi there")
        ↓
Display: "[12:34] <Alice> Hi there"
```

### Example 3: Joining a Channel

```
User clicks "Join Channel" menu
        ↓
MainWindow::onJoinChannelAction()
        ↓
Show input dialog for channel name
        ↓
IrcConnection::joinChannel("#linux")
        ↓
Socket sends: "JOIN #linux\r\n"
        ↓
Server responds:
  ":user!host JOIN #linux"
  ":server 332 #linux :Welcome to #linux"
  ":server 353 #linux :@ops +voice alice bob"
        ↓
IrcConnection parses and emits:
  - joinedChannel("#linux", "user")
  - topicReceived("#linux", "Welcome...")
  - userListReceived("#linux", ["@ops", "alice", "bob"])
        ↓
MainWindow creates new ChatWidget tab
        ↓
ChatWidget displays topic and user list
```

## IRC Protocol Basics

### Message Format
```
:prefix COMMAND params :trailing
```

Examples:
```
:nick!user@host PRIVMSG #channel :Hello world
└─────┬────────┘ └──┬──┘ └──┬───┘ └────┬─────┘
   prefix      command  param   trailing
```

### Common Commands

**Client → Server**:
- `NICK mynick` - Set nickname
- `USER mynick 0 * :Real Name` - Register user
- `JOIN #channel` - Join channel
- `PRIVMSG #channel :message` - Send message
- `PART #channel` - Leave channel
- `QUIT :reason` - Disconnect

**Server → Client**:
- `PING :server` - Keepalive (respond with PONG)
- `:nick!user@host PRIVMSG #channel :msg` - Message
- `:server 001 nick :Welcome message` - Connected
- `:server 332 #channel :Topic` - Channel topic
- `:server 353 #channel :user1 user2` - User list

### Numeric Replies
- `001` - RPL_WELCOME (connection successful)
- `332` - RPL_TOPIC (channel topic)
- `353` - RPL_NAMREPLY (user list)
- `366` - RPL_ENDOFNAMES (end of user list)

## Qt Signal/Slot Mechanism

Qt uses signals and slots for event-driven programming:

```cpp
// Object A emits a signal
class IrcConnection {
    signals:
        void messageReceived(QString sender, QString target, QString msg);
};

// Object B connects and handles the signal
connect(ircConnection, &IrcConnection::messageReceived,
        mainWindow, &MainWindow::onMessageReceived);

// When signal is emitted, slot is automatically called
emit messageReceived("Alice", "#channel", "Hello");
// → MainWindow::onMessageReceived() gets called
```

Benefits:
- Loose coupling between components
- Type-safe callbacks
- Automatic connection management
- Thread-safe communication

## Building Blocks Explained

### QTcpSocket
Qt's TCP networking class:
```cpp
QTcpSocket *socket = new QTcpSocket();
socket->connectToHost("irc.libera.chat", 6667);
connect(socket, &QTcpSocket::readyRead, this, &MyClass::onDataReady);
socket->write("NICK myname\r\n");
```

### QTabWidget
Manages multiple tabs:
```cpp
QTabWidget *tabs = new QTabWidget();
tabs->addTab(widget1, "Tab 1");
tabs->addTab(widget2, "Tab 2");
tabs->setCurrentIndex(0);  // Switch to first tab
```

### QTextEdit
Rich text display:
```cpp
QTextEdit *display = new QTextEdit();
display->setReadOnly(true);
display->append("<b>Bold text</b>");
display->append("<span style='color: red;'>Red text</span>");
```

## Extension Ideas

### 1. Add SSL/TLS Support
```cpp
// Replace QTcpSocket with QSslSocket
QSslSocket *m_socket = new QSslSocket(this);
m_socket->connectToHostEncrypted("irc.libera.chat", 6697);
```

### 2. Add Notifications
```cpp
// In MainWindow::onMessageReceived()
if (message.contains(m_currentNickname)) {
    QSystemTrayIcon::showMessage("IRC", 
        QString("%1 mentioned you").arg(sender));
}
```

### 3. Add Chat Logging
```cpp
// In ChatWidget::addMessage()
QFile logFile("chat_" + m_channelName + ".log");
logFile.open(QIODevice::Append);
logFile.write(formattedMessage.toUtf8());
```

### 4. Add Auto-Reconnect
```cpp
// In IrcConnection::onDisconnected()
QTimer::singleShot(5000, this, [this]() {
    connectToServer(m_server, m_port);
});
```

## Testing Checklist

- [ ] Connect to server
- [ ] Set nickname
- [ ] Join channel
- [ ] Send message to channel
- [ ] Receive messages from others
- [ ] See user list update
- [ ] See topic display
- [ ] Open multiple channel tabs
- [ ] Send private message
- [ ] Close channel tab (sends PART)
- [ ] Disconnect from server
- [ ] Reconnect to server
- [ ] Test IRC commands (/join, /part, /msg)

## Common Issues & Solutions

**Issue**: Messages not appearing
- Check Server tab for connection status
- Verify channel name starts with #
- Check if you actually joined the channel

**Issue**: Can't connect
- Firewall blocking port 6667
- Server requires registration
- Wrong server address

**Issue**: Build errors
- Missing Qt development packages
- Wrong CMake Qt path
- Incompatible Qt version (need 5.x or 6.x)

**Issue**: Segmentation fault
- Check null pointer before using ChatWidget
- Ensure socket is connected before sending
- Verify signal/slot connections are valid
