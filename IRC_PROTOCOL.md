# IRC Protocol Quick Reference

## Message Format

Every IRC message follows this structure:
```
[:prefix] COMMAND [param1] [param2] ... [:trailing parameter]
```

- **prefix** (optional): Identifies the message source (`:nick!user@host` or `:server`)
- **COMMAND**: IRC command or 3-digit numeric reply
- **params**: Space-separated parameters
- **trailing**: Last parameter, can contain spaces (prefixed with `:`)
- **Line ending**: `\r\n` (CRLF)

## Essential Client Commands

### Connection & Registration
```
NICK mynickname                    # Set your nickname
USER myname 0 * :Real Name        # Register user (after NICK)
PASS password                      # Server password (if required, before NICK/USER)
QUIT :Goodbye message             # Disconnect from server
```

### Channel Operations
```
JOIN #channel                      # Join a channel
JOIN #chan1,#chan2                # Join multiple channels
PART #channel :Leaving            # Leave a channel
PART #channel                     # Leave without message
TOPIC #channel :New topic         # Set channel topic
TOPIC #channel                    # View channel topic
NAMES #channel                    # Get user list
```

### Sending Messages
```
PRIVMSG #channel :Hello everyone  # Send to channel
PRIVMSG nick :Private message     # Send to user
NOTICE #channel :Announcement     # Send notice (no auto-reply)
```

### Server Communication
```
PING :argument                     # Server sends this
PONG :argument                     # Client must respond
```

### User Information
```
WHOIS nickname                     # Get user info
WHO #channel                       # Get channel user info
MODE #channel                      # Get channel modes
MODE #channel +i                   # Set channel invite-only
MODE nickname +i                   # Set user invisible
```

## Numeric Replies (Server → Client)

### Connection Messages
```
001    RPL_WELCOME           Welcome to the network
002    RPL_YOURHOST          Your host is...
003    RPL_CREATED           Server created on...
004    RPL_MYINFO            Server info
```

### Channel Information
```
332    RPL_TOPIC             #channel :Topic text
333    RPL_TOPICWHOTIME      #channel nick timestamp
353    RPL_NAMREPLY          = #channel :nick1 nick2 nick3
366    RPL_ENDOFNAMES        #channel :End of NAMES list
```

### User Modes & Prefixes
```
@      Channel operator (op)
+      Voice
%      Half-op (some servers)
~      Owner (some servers)
&      Admin (some servers)
```

### Error Codes
```
401    ERR_NOSUCHNICK        No such nickname
403    ERR_NOSUCHCHANNEL     No such channel
433    ERR_NICKNAMEINUSE     Nickname already in use
442    ERR_NOTONCHANNEL      You're not on that channel
461    ERR_NEEDMOREPARAMS    Not enough parameters
```

## Message Events (Server → Client)

### User Actions
```
:nick!user@host JOIN #channel              # User joined
:nick!user@host PART #channel :reason      # User left
:nick!user@host QUIT :reason               # User disconnected
:nick!user@host NICK newnick               # User changed nick
:nick!user@host KICK #channel target :reason  # User was kicked
```

### Messages
```
:nick!user@host PRIVMSG #channel :message  # Channel message
:nick!user@host PRIVMSG you :message       # Private message
:nick!user@host NOTICE #channel :message   # Channel notice
```

### Channel Modes
```
:nick!user@host MODE #channel +o target    # Give operator
:nick!user@host MODE #channel -o target    # Remove operator
:nick!user@host MODE #channel +v target    # Give voice
:nick!user@host MODE #channel +b mask      # Ban user
:nick!user@host MODE #channel +l 50        # Set user limit
:nick!user@host MODE #channel +k password  # Set channel key
```

## Parsing Examples

### Example 1: Channel Message
```
Raw: :alice!user@host.com PRIVMSG #general :Hello world!

Parsed:
  prefix:   alice!user@host.com
  command:  PRIVMSG
  params:   ["#general"]
  trailing: "Hello world!"
  
Extract:
  nickname: "alice" (from prefix before !)
  target:   "#general"
  message:  "Hello world!"
```

### Example 2: User Joined
```
Raw: :bob!bob@192.168.1.5 JOIN #general

Parsed:
  prefix:   bob!bob@192.168.1.5
  command:  JOIN
  params:   ["#general"]
  
Extract:
  nickname: "bob"
  channel:  "#general"
```

### Example 3: Server Welcome
```
Raw: :irc.server.com 001 mynick :Welcome to the IRC Network

Parsed:
  prefix:   irc.server.com
  command:  001 (numeric)
  params:   ["mynick"]
  trailing: "Welcome to the IRC Network"
```

### Example 4: User List
```
Raw: :server 353 mynick = #general :@alice +bob charlie

Parsed:
  prefix:   server
  command:  353 (RPL_NAMREPLY)
  params:   ["mynick", "=", "#general"]
  trailing: "@alice +bob charlie"
  
Extract:
  channel: "#general"
  users:   ["@alice", "+bob", "charlie"]
  ops:     ["alice"]
  voiced:  ["bob"]
  normal:  ["charlie"]
```

## Connection Sequence

```
1. Client → Server:  NICK alice
2. Client → Server:  USER alice 0 * :Alice Smith
3. Server → Client:  :server 001 alice :Welcome...
4. Server → Client:  :server 002 alice :Your host...
5. Server → Client:  :server 003 alice :Created...
6. Server → Client:  :server 004 alice :Server info...
   ...additional welcome messages...
   
   [Now connected and registered]
   
7. Server → Client:  PING :server
8. Client → Server:  PONG :server
   
   [Keepalive ping/pong continues periodically]
```

## Joining a Channel Sequence

```
1. Client → Server:  JOIN #general
2. Server → Client:  :alice!user@host JOIN #general
3. Server → Client:  :server 332 alice #general :Welcome to #general!
4. Server → Client:  :server 353 alice = #general :@op1 +voice1 alice bob
5. Server → Client:  :server 366 alice #general :End of NAMES list
   
   [Now in channel with topic and user list]
```

## Sending a Message Sequence

```
1. Client → Server:  PRIVMSG #general :Hello everyone!
   
   [Server broadcasts to all channel members]
   
2. Server → Others:  :alice!user@host PRIVMSG #general :Hello everyone!
   
   [Your own client doesn't receive it back - display locally]
```

## Special Characters & Formatting

Some IRC clients support formatting:
```
\x02 = Bold
\x1D = Italic  
\x1F = Underline
\x0F = Reset formatting
\x03 = Color (followed by color code)
```

Example with color:
```
PRIVMSG #channel :\x0304Red text\x0F normal text
```

## Channel Naming Rules

- **#channel** - Normal channel
- **&channel** - Local channel (server-specific)
- **+channel** - Modeless channel
- **!channel** - Safe channel (unique ID)

Most commonly used: `#channel`

## Best Practices

1. **Always respond to PING** - Server will disconnect you otherwise
2. **Rate limiting** - Don't send messages too fast (typically 1-2/sec max)
3. **Handle numerics** - Many servers use numeric replies instead of named commands
4. **Parse prefix carefully** - Format is `nick!user@host`
5. **Escape special chars** - Null, CR, LF, space need special handling
6. **UTF-8 encoding** - Modern IRC uses UTF-8
7. **Maximum message length** - Typically 512 bytes including `\r\n`
8. **Handle nick collisions** - Append numbers if nickname taken

## Testing Commands

Try these when connected:
```
/join #test              # Join test channel
/msg NickServ HELP       # Get help from nickname services
/whois YourNick          # See your own info
/topic #channel          # View channel topic
/names #channel          # List users
/part #channel           # Leave channel
/quit Goodbye!           # Disconnect
```

## Debugging Tips

### Enable Raw IRC Message Logging
```cpp
// In IrcConnection::parseIrcMessage()
qDebug() << "<<" << line;  // All incoming

// In IrcConnection::sendRawMessage()  
qDebug() << ">>" << message;  // All outgoing
```

### Common Parse Failures
- Forgot to strip `\r\n`
- Not handling missing prefix
- Assuming trailing always present
- Not extracting nickname from `nick!user@host`
- Hardcoding parameter positions

## Resources

- **Modern IRC Docs**: https://modern.ircdocs.horse/
- **RFC 1459**: Original IRC specification
- **RFC 2812**: Updated IRC protocol
- **IRCv3**: Modern extensions

## Quick Implementation Checklist

- [ ] Connect via TCP socket
- [ ] Send NICK and USER
- [ ] Respond to PING with PONG
- [ ] Parse `:prefix COMMAND params :trailing`
- [ ] Handle PRIVMSG (messages)
- [ ] Handle JOIN/PART (users joining/leaving)
- [ ] Handle numeric 353/366 (user list)
- [ ] Handle numeric 332 (topic)
- [ ] Extract nickname from prefix
- [ ] Send QUIT on disconnect
