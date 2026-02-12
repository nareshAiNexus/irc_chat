# Qt IRC Client

A modern IRC client built with C++ and Qt, inspired by classic IRC clients like XChat.

## Features

- ✅ Connect to IRC servers
- ✅ Join multiple channels (tabbed interface)
- ✅ Send and receive messages in real-time
- ✅ User list display for channels
- ✅ Channel topics
- ✅ Private messaging
- ✅ IRC command support
- ✅ Timestamps on messages
- ✅ System notifications (joins/parts)

## Architecture

```
┌─────────────────────────────────────┐
│         MainWindow                  │
│  - Tab management                   │
│  - Menu bar & UI coordination       │
└──────────┬──────────────────────────┘
           │
    ┌──────┴───────┬───────────────┐
    │              │               │
┌───▼────────┐ ┌──▼─────────┐ ┌──▼─────────┐
│ChatWidget  │ │ChatWidget  │ │ChatWidget  │
│ #channel1  │ │ #channel2  │ │  Server    │
└───┬────────┘ └──┬─────────┘ └──┬─────────┘
    │              │               │
    └──────────────┴───────────────┘
                   │
           ┌───────▼────────┐
           │ IrcConnection  │
           │ - IRC Protocol │
           │ - QTcpSocket   │
           └───────┬────────┘
                   │
              ┌────▼─────┐
              │IRC Server│
              └──────────┘
```

## Project Structure

```
irc-client/
├── CMakeLists.txt          # Build configuration
├── README.md               # This file
├── include/                # Header files
│   ├── MainWindow.h        # Main application window
│   ├── IrcConnection.h     # IRC protocol & networking
│   └── ChatWidget.h        # Individual channel/chat view
└── src/                    # Implementation files
    ├── main.cpp            # Application entry point
    ├── MainWindow.cpp      # Main window implementation
    ├── IrcConnection.cpp   # IRC protocol parser
    └── ChatWidget.cpp      # Chat UI implementation
```

## Prerequisites

### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev
# Or for Qt5:
sudo apt install build-essential cmake qtbase5-dev
```

### Fedora:
```bash
sudo dnf install gcc-c++ cmake qt6-qtbase-devel
# Or for Qt5:
sudo dnf install gcc-c++ cmake qt5-qtbase-devel
```

### macOS:
```bash
brew install cmake qt@6
# Or for Qt5:
brew install cmake qt@5
```

### Windows:
1. Install Visual Studio 2019 or later
2. Install CMake from https://cmake.org/
3. Install Qt from https://www.qt.io/download-qt-installer

## Building the Application

### Linux/macOS:

```bash
cd irc-client
mkdir build
cd build
cmake ..
make
./IRCClient
```

### Windows (Visual Studio):

```bash
cd irc-client
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
.\Release\IRCClient.exe
```

## Usage

1. **Connect to a Server:**
   - Click `Server → Connect...`
   - Enter server address (e.g., `irc.libera.chat`)
   - Enter your desired nickname

2. **Join a Channel:**
   - Click `Channel → Join Channel...`
   - Enter channel name (e.g., `#general` or `#linux`)

3. **Send Messages:**
   - Type in the input box at the bottom
   - Press Enter or click Send

4. **IRC Commands:**
   - `/join #channel` - Join a channel
   - `/part` or `/leave` - Leave current channel
   - `/msg nickname message` - Send private message
   - `/quit` - Disconnect from server
   - Any other command starting with `/` is sent as raw IRC

5. **Close Channels:**
   - Click the X on the tab to leave and close a channel

## IRC Protocol Details

The client implements the following IRC protocol features:

### Commands Sent:
- `NICK` - Set nickname
- `USER` - User registration
- `JOIN` - Join channels
- `PART` - Leave channels
- `PRIVMSG` - Send messages
- `PONG` - Respond to server pings
- `QUIT` - Disconnect

### Server Replies Handled:
- `001` - Welcome message
- `332` - Channel topic
- `353` - Names list (user list)
- `366` - End of names
- `PING` - Server keepalive
- `PRIVMSG` - Messages
- `JOIN` - User joins
- `PART` - User leaves
- `NICK` - Nickname changes

## Testing

You can test the client with these public IRC servers:

- **Libera.Chat**: `irc.libera.chat` (port 6667)
  - Channels: `#linux`, `#programming`, `##chat`
  
- **OFTC**: `irc.oftc.net` (port 6667)
  - Channels: `#test`, `#oftc`

## Customization Ideas

### Easy Enhancements:
1. **Color Schemes**: Customize message colors in `ChatWidget::formatMessage()`
2. **Font Settings**: Change fonts in `ChatWidget::setupUi()`
3. **Sounds**: Add notification sounds for mentions
4. **Auto-join**: Save and auto-join favorite channels

### Advanced Features:
1. **SSL/TLS Support**: Add encrypted connections
2. **DCC File Transfer**: Implement file sharing
3. **Ignore List**: Block specific users
4. **Logging**: Save chat history to files
5. **Scripting**: Add plugin/script support
6. **Emoji Support**: Add emoji picker
7. **URL Detection**: Make links clickable

## Code Highlights

### IRC Message Parsing
The IRC protocol uses a simple text-based format:
```
:prefix COMMAND param1 param2 :trailing parameter
```

Example:
```
:nick!user@host PRIVMSG #channel :Hello world!
```

See `IrcConnection::parseIrcMessage()` for the parser implementation.

### Signal/Slot Architecture
Qt's signal-slot mechanism provides clean separation:

```cpp
// IrcConnection emits signals
emit messageReceived(sender, target, message);

// MainWindow receives signals
connect(m_ircConnection, &IrcConnection::messageReceived,
        this, &MainWindow::onMessageReceived);
```

## Troubleshooting

### "Cannot connect to server"
- Check your internet connection
- Try a different IRC server
- Some networks require registration (try Libera.Chat)

### "Build errors with Qt"
- Ensure Qt development packages are installed
- Check CMake finds Qt: `cmake .. -DCMAKE_PREFIX_PATH=/path/to/qt`

### "No messages appearing"
- Check the Server tab for connection status
- Ensure you've joined a channel
- Try `/msg NickServ HELP` to test messaging

## License

This is a tutorial/educational project. Feel free to modify and use as you wish.

## Learning Resources

- **IRC Protocol**: https://modern.ircdocs.horse/
- **Qt Documentation**: https://doc.qt.io/
- **Qt Network**: https://doc.qt.io/qt-6/qtnetwork-index.html
- **RFC 1459**: Original IRC protocol specification

## Contributing

This is an educational example, but improvements are welcome:
- Bug fixes
- Feature additions
- Code cleanup
- Documentation improvements
