#pragma once

#include <utility>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>

using std::list;
using std::string;

class KeyHandler
{
public:
    virtual ~KeyHandler() = default;
    virtual int handle(int keyId, const string & key, const list<string> & values = {}) = 0;
};

class MessageStrategy
{
public:
    virtual ~MessageStrategy() = default;
    virtual void echo(int msg, const string & key, const list<string> & values = {}) = 0;
};

class ArgumentReader
{
public:
    enum KeyId
    {
        KEY_DEFAULT,
        KEY_HELP,
        KEY_USER_DEFINED = 1000,
    };

    enum Message
    {
        MSG_HELP,
        MSG_UNKNOWN_KEY,
        MSG_NO_VALUE,
        MSG_USER_DEFINED = 1000,
    };

    ArgumentReader();

    void setKeyHandler(std::unique_ptr<KeyHandler> handler);
    void setMessageStrategy(std::unique_ptr<MessageStrategy> strategy);

    bool read(char * argv[]);
    void addKey(int keyId, const string & key, bool hasValue = false,
                const string & description = {});
    void addKeys(int keyId, const std::initializer_list<std::string> & keys, bool hasValue = false,
                 const string & description = {});
    void setHeader(const string & header);
    void addHelp(const string & description);

private:
    void echo(int msg, const string & key, const list<string> & value = {});
    int handleKey(int keyId, const string & key, const list<string> & values = {});

    struct Value
    {
        bool hasValue;
        string description;
    };

    using Keys = std::unordered_map<string, int>;
    using Entries = std::map<int, Value>;

    Value & getValue(Keys::iterator iter, Value & alt);

    Entries m_entries;
    Keys m_keys;
    std::unique_ptr<KeyHandler> m_keyHandler;
    std::unique_ptr<MessageStrategy> m_messageStrategy;
    string m_header;
};
