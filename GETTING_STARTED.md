# IRC Client - Complete Tutorial Guide

## 🎯 What You've Got

A fully functional IRC client built with C++ and Qt, similar to classic XChat! This project includes:

- ✅ **Complete source code** - ~1000 lines of well-commented C++
- ✅ **CMake build system** - Works on Linux, macOS, Windows
- ✅ **IRC protocol implementation** - Full client-to-server communication
- ✅ **Modern Qt UI** - Tabbed interface with user lists
- ✅ **Documentation** - Architecture guide, protocol reference, README

## 📁 Project Structure

```
irc-client/
├── README.md              # Main documentation
├── ARCHITECTURE.md        # Detailed component breakdown
├── IRC_PROTOCOL.md        # IRC protocol reference
├── CMakeLists.txt         # Build configuration
├── build.sh               # Quick build script (Linux/macOS)
│
├── include/               # Header files (.h)
│   ├── MainWindow.h       # Main application window + tab management
│   ├── IrcConnection.h    # IRC protocol + TCP networking
│   └── ChatWidget.h       # Individual channel/chat display
│
└── src/                   # Implementation files (.cpp)
    ├── main.cpp           # Application entry point
    ├── MainWindow.cpp     # Main window logic
    ├── IrcConnection.cpp  # IRC message parser + network I/O
    └── ChatWidget.cpp     # Chat UI implementation
```

## 🚀 Quick Start

### Step 1: Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt update
sudo apt install build-essential cmake qtbase5-dev
```

**macOS:**
```bash
brew install cmake qt@5
```

**Windows:**
- Install Visual Studio 2019+
- Install CMake from https://cmake.org/
- Install Qt from https://www.qt.io/download

### Step 2: Build

**Linux/macOS:**
```bash
cd irc-client
chmod +x build.sh
./build.sh
```

**Or manually:**
```bash
mkdir build
cd build
cmake ..
make
```

**Windows:**
```bash
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
```

### Step 3: Run

```bash
./build/IRCClient        # Linux/macOS
.\build\Release\IRCClient.exe   # Windows
```

## 🎮 Using the Application

1. **Connect**: Menu → Server → Connect
   - Server: `irc.libera.chat`
   - Nickname: Choose any name

2. **Join Channel**: Menu → Channel → Join Channel
   - Try: `#test` or `#chat`

3. **Send Messages**: Type in the input box and press Enter

4. **IRC Commands**:
   - `/join #channel` - Join a channel
   - `/part` - Leave current channel
   - `/msg nickname Hi` - Private message
   - `/quit` - Disconnect

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────┐
│          MainWindow (Controller)        │
│  - Manages tabs                         │
│  - Routes messages between components   │
└────────┬────────────────────────────────┘
         │
    ┌────┴────┬───────────┬──────────┐
    │         │           │          │
┌───▼────┐ ┌─▼──────┐ ┌─▼──────┐ ┌─▼──────┐
│Server  │ │#chan1  │ │#chan2  │ │Private │
│Tab     │ │Tab     │ │Tab     │ │Msg Tab │
└────────┘ └────────┘ └────────┘ └────────┘
    Each tab is a ChatWidget
         │
         │ All tabs share one connection
         │
    ┌────▼──────────────┐
    │  IrcConnection    │
    │  - IRC Protocol   │
    │  - QTcpSocket     │
    └────┬──────────────┘
         │
    ┌────▼──────┐
    │IRC Server │
    └───────────┘
```

## 📚 Key Concepts

### 1. IRC Protocol
IRC is text-based. Every message looks like:
```
:nickname!user@host COMMAND param1 param2 :trailing text
```

Example:
```
:alice!alice@host.com PRIVMSG #general :Hello world!
```

**Our parser** (in `IrcConnection.cpp`):
- Splits on spaces
- Extracts prefix (`:alice!alice@host.com`)
- Gets command (`PRIVMSG`)
- Parses parameters (`#general`)
- Grabs trailing text (`:Hello world!`)

### 2. Qt Signals & Slots
Qt's event system connects components:

```cpp
// IrcConnection emits when message arrives
emit messageReceived("alice", "#general", "Hello");

// MainWindow listens and handles it
connect(ircConnection, &IrcConnection::messageReceived,
        this, &MainWindow::onMessageReceived);
```

This keeps components **loosely coupled** - IrcConnection doesn't need to know about MainWindow!

### 3. Tab Management
MainWindow creates ChatWidget tabs dynamically:

```cpp
// User joins #linux
ChatWidget *widget = new ChatWidget("#linux");
m_tabWidget->addTab(widget, "#linux");
m_chatWidgets["#linux"] = widget;  // Store for later
```

When messages arrive for #linux, we find the right tab:
```cpp
ChatWidget *widget = m_chatWidgets["#linux"];
widget->addMessage("alice", "Hello!");
```

## 🔧 How It Works - Step by Step

### Connecting to Server

1. User clicks "Connect" → enters server + nickname
2. `MainWindow` calls `IrcConnection::connectToServer()`
3. `IrcConnection` opens TCP socket:
   ```cpp
   m_socket->connectToHost("irc.libera.chat", 6667);
   ```
4. When connected, sends registration:
   ```
   NICK mynickname
   USER mynickname 0 * :Real Name
   ```
5. Server responds with welcome messages (001, 002, etc.)
6. Connection established! ✅

### Joining a Channel

1. User types channel name
2. Client sends: `JOIN #linux`
3. Server broadcasts to all:
   ```
   :mynick!user@host JOIN #linux
   ```
4. Server sends topic (332) and user list (353)
5. `IrcConnection` parses and emits signals
6. `MainWindow` creates new ChatWidget tab
7. ChatWidget displays topic and user list

### Sending a Message

1. User types "Hello" in #linux tab
2. ChatWidget emits `messageSent("Hello")`
3. MainWindow catches it:
   ```cpp
   m_ircConnection->sendMessage("#linux", "Hello");
   ```
4. IrcConnection sends to server:
   ```
   PRIVMSG #linux :Hello
   ```
5. Display message locally (we don't get it back from server)

### Receiving a Message

1. Server sends:
   ```
   :alice!user@host PRIVMSG #linux :Hi there!
   ```
2. TCP socket receives bytes
3. `IrcConnection::parseIrcMessage()`:
   - Extracts nickname: `alice`
   - Extracts target: `#linux`
   - Extracts message: `Hi there!`
4. Emits: `messageReceived("alice", "#linux", "Hi there!")`
5. MainWindow routes to #linux ChatWidget
6. ChatWidget displays: `[12:34] <alice> Hi there!`

## 🎨 Customization Ideas

### Easy Changes:

**Change Colors:**
```cpp
// In ChatWidget.cpp, formatMessage()
QString color = "darkblue";  // Change this!
```

**Change Font:**
```cpp
// In ChatWidget.cpp, setupUi()
m_chatDisplay->setFont(QFont("Arial", 12));
```

**Auto-Join Channels:**
```cpp
// In MainWindow::onConnected()
m_ircConnection->joinChannel("#general");
m_ircConnection->joinChannel("#help");
```

### Advanced Features:

1. **Add SSL/TLS** (port 6697):
   ```cpp
   QSslSocket *m_socket = new QSslSocket(this);
   m_socket->connectToHostEncrypted(host, 6697);
   ```

2. **Add Notifications**:
   ```cpp
   if (message.contains(m_currentNickname)) {
       // Show system notification
   }
   ```

3. **Save Chat History**:
   ```cpp
   QFile log(channelName + ".log");
   log.open(QIODevice::Append);
   log.write(message.toUtf8() + "\n");
   ```

4. **Clickable URLs**:
   ```cpp
   m_chatDisplay->setOpenExternalLinks(true);
   // Detect URLs and wrap in <a href="...">
   ```

## 🐛 Troubleshooting

**Build fails - Qt not found:**
```bash
cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/5.15.2/gcc_64
```

**Can't connect to server:**
- Check internet connection
- Try `irc.libera.chat` (port 6667)
- Check firewall settings

**Messages not showing:**
- Look at Server tab for errors
- Make sure you joined the channel (#channel)
- Try `/msg NickServ HELP` to test

**Crash on startup:**
- Ensure Qt libraries are in PATH
- On Linux: `export LD_LIBRARY_PATH=/path/to/qt/lib`

## 📖 Learning Path

**Beginner** - Understand the basics:
1. Read `README.md`
2. Build and run the application
3. Connect to a server and join channels
4. Read `IRC_PROTOCOL.md` to understand message format

**Intermediate** - Explore the code:
1. Read `ARCHITECTURE.md`
2. Trace a message from send to receive
3. Add debug prints to see data flow
4. Modify colors/fonts to customize UI

**Advanced** - Extend the client:
1. Add SSL/TLS support
2. Implement DCC file transfer
3. Add plugin system
4. Create custom IRC commands

## 🔍 Code Walkthrough

### Key Files to Study:

1. **Start here**: `src/main.cpp`
   - Simple! Just creates MainWindow and shows it

2. **Then**: `src/IrcConnection.cpp`
   - See how TCP socket connects
   - Study `parseIrcMessage()` function
   - Understand PING/PONG keepalive

3. **Next**: `src/ChatWidget.cpp`
   - See how messages are displayed
   - Understand user list management
   - Study Qt layouts

4. **Finally**: `src/MainWindow.cpp`
   - See how everything connects
   - Study signal/slot connections
   - Understand tab management

### Important Functions:

```cpp
// Parse IRC messages (in IrcConnection.cpp)
void IrcConnection::parseIrcMessage(const QString &line)

// Handle incoming messages (in MainWindow.cpp)
void MainWindow::onMessageReceived(sender, target, message)

// Display messages (in ChatWidget.cpp)
void ChatWidget::addMessage(sender, message)

// Send messages (in IrcConnection.cpp)
void IrcConnection::sendMessage(target, message)
```

## 🎓 Next Steps

1. **Build and test** the application
2. **Read the source code** - it's well-commented!
3. **Make modifications** - change colors, fonts, add features
4. **Study Qt documentation** - https://doc.qt.io/
5. **Read IRC specs** - https://modern.ircdocs.horse/
6. **Join IRC channels** - Talk to real users!

## 📝 Additional Resources

- **Qt Tutorial**: https://doc.qt.io/qt-5/qtwidgets-tutorials-notepad-example.html
- **Qt Network**: https://doc.qt.io/qt-5/qtnetwork-index.html
- **IRC Protocol**: https://modern.ircdocs.horse/
- **Qt Signals/Slots**: https://doc.qt.io/qt-5/signalsandslots.html

## 🏆 Challenge Yourself

Try implementing these features:

- [ ] SSL/TLS encrypted connections
- [ ] Save/load server list
- [ ] Auto-reconnect on disconnect
- [ ] Clickable URLs in chat
- [ ] User mentions highlighting
- [ ] Chat logging to files
- [ ] Custom themes/color schemes
- [ ] Sound notifications
- [ ] Away/back status
- [ ] Channel bookmarks

## 💡 Tips for Success

1. **Start small** - Get it building first
2. **Use debugging** - Add `qDebug()` statements
3. **Test incrementally** - Don't change too much at once
4. **Read Qt docs** - Qt has excellent documentation
5. **Ask for help** - Join #qt on IRC (use your own client!)

---

**You now have a complete, working IRC client!** 🎉

Study the code, make changes, break things, fix them, and learn. That's the best way to understand how it all works.

Happy coding! 🚀
