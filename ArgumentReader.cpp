#include "ArgumentReader.hh"
#include <utility>
#include <vector>
#include <list>
#include <string>
#include <cstring>
#include <iostream>
#include <cassert>
#include <algorithm>

ArgumentReader::ArgumentReader()
{
}

void ArgumentReader::setKeyHandler(std::unique_ptr<KeyHandler> handler)
{
    m_keyHandler = std::move(handler);
}

void ArgumentReader::setMessageStrategy(std::unique_ptr<MessageStrategy> strategy)
{
    m_messageStrategy = std::move(strategy);
}

template<typename T>
inline static T & nullRef()
{
    return *static_cast<T*>(nullptr);
}

bool ArgumentReader::read(char * argv[])
{
    std::list<std::string> values;
    Keys::iterator lastKey = m_keys.end();

    const auto handleLongKey = [&](string const & sKey) -> bool
    {
        auto it = m_keys.find(sKey);
        if (it == m_keys.end())
        {
            echo(MSG_UNKNOWN_KEY, sKey);
            return false;
        }

        auto const & v = getValue(it, nullRef<Value>());
        assert(&v != nullptr); // Да, я знаю, что так делать нельзя и что это UB

        if (v.hasValue)
        {
            lastKey = it;
        }
        else
        {
            int const msg = handleKey(it->second, sKey);
            if (msg != -1)
            {
                echo(it->second, sKey);
                return false;
            }
        }
        return true;
    };

    const auto handleShortKey = [&](char const * sKey) -> bool
    {
        size_t const sKeyLen = strlen(sKey);

        for (size_t j = 0; j < sKeyLen; ++j)
        {
            std::string const k(1, sKey[j]);

            auto it = m_keys.find(k);
            if (it == m_keys.end())
            {
                echo(MSG_UNKNOWN_KEY, k);
                return false;
            }

            auto const & v = getValue(it, nullRef<Value>());
            assert(&v != nullptr);

            if (v.hasValue)
            {
                if (j < sKeyLen - 1)
                {
                    echo(MSG_NO_VALUE, it->first);
                    return false;
                }
                lastKey = it;
            }
            else
            {
                int const msg = handleKey(it->second, k);
                if (msg != -1)
                {
                    echo(it->second, k);
                    return false;
                }
            }
        }
        return true;
    };

    auto handleValue = [&](char const * value) -> bool
    {
        if (lastKey == m_keys.end())
        {
            values.push_back(value);
        }
        else
        {
            int const msg = handleKey(lastKey->second, lastKey->first, {value});
            if (msg != -1)
            {
                echo(lastKey->second, lastKey->first, {value});
                return false;
            }
            lastKey = m_keys.end();
        }
        return true;
    };

    for (int i = 1; argv[i] != nullptr; ++i)
    {
        char const * const curr = argv[i];

        if (curr[0] == '-')
        {
            if (curr[1] == '-')
            {
                if (!handleLongKey(&curr[2]))
                    return false;
            }
            else if (!handleShortKey(&curr[1]))
            {
                return false;
            }
        }
        else if (!handleValue(curr))
        {
            return false;
        }
    }

    if (lastKey != m_keys.end())
    {
        echo(MSG_NO_VALUE, lastKey->first, {});
        return false;
    }
    else if (!values.empty())
    {
        int const msg = handleKey(KEY_DEFAULT, {}, values);
        if (msg != -1)
        {
            echo(KEY_DEFAULT, {}, values);
            return false;
        }
    }
    return true;
}

void ArgumentReader::addKey(int keyId, const string & key, bool hasValue, const string & description)
{
    assert(!key.empty());
    addKeys(keyId, {key}, hasValue, description);
}

void ArgumentReader::addKeys(int keyId, const std::initializer_list<string> & keys, bool hasValue,
                             const string & description)
{
    assert(keys.size() != 0);

    auto it = m_entries.lower_bound(keyId);
    if (it == m_entries.end() || it->first != keyId)
        m_entries.emplace_hint(it, keyId, Value{hasValue, description});
    else
        assert(!"");

    for (auto const & k : keys)
    {
        assert(!k.empty());
        if (m_keys.count(k) == 0)
            m_keys.emplace(k, keyId);
        else
            assert(!"");
    }
}

void ArgumentReader::setHeader(const string & header)
{
    m_header = header;
}

void ArgumentReader::addHelp(const string & description)
{
    addKeys(KEY_HELP, {"h", "help"}, false, description);
}

void ArgumentReader::echo(int msg, const string & key, const list<string> & values)
{
    if (msg == MSG_HELP)
    {
        list<Entries::iterator> uEntries;
        for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
        {
            if (it->first < KEY_USER_DEFINED)
                continue;

            auto pos = std::find_if(uEntries.begin(), uEntries.end(), [&it](auto const & item) -> bool
            {
                return item->first > it->first;
            });
            uEntries.insert(pos, it);
        }

        auto it = m_entries.find(KEY_HELP);
        if (it != m_entries.end())
            uEntries.push_back(it);

        if (!m_header.empty())
            std::cout << m_header << std::endl;

        list<string> sKeys;
        for (auto const & e : uEntries)
        {
            for (auto const & k : m_keys)
            {
                if (e->first != k.second)
                    continue;

                auto const & sKey = k.first;
                auto pos = std::find_if(sKeys.begin(), sKeys.end(), [&sKey](auto const & item) -> bool
                {
                    return (sKey.size() == 1 && item.size() > 1
                            ? true
                            : (sKey.size() > 1 && item.size() == 1
                               ? false
                               : sKey < item));
                });
                sKeys.insert(pos, sKey);
            }
            assert(!sKeys.empty());

            bool isFirst = true;
            while (!sKeys.empty())
            {
                if (isFirst)
                {
                    isFirst = false;
                    std::cout << "    ";
                }
                else
                {
                    std::cout << ", ";
                }

                if (sKeys.front().size() > 1)
                    std::cout << '-';
                std::cout << '-' << sKeys.front();
                sKeys.pop_front();
            }
            std::cout << "\t\t" << e->second.description << std::endl;
        }
    }
    else
    {
        if (m_messageStrategy)
            m_messageStrategy->echo(msg, key, values);
    }
}

int ArgumentReader::handleKey(int keyId, string const & key, list<string> const & values)
{
    if (keyId == KEY_HELP)
        return MSG_HELP;

    if (m_keyHandler)
        return m_keyHandler->handle(keyId, key, values);

    return -1;
}

ArgumentReader::Value & ArgumentReader::getValue(Keys::iterator iter, ArgumentReader::Value & alt)
{
    auto it = m_entries.find(iter->second);
    if (it == m_entries.end())
        return alt;
    return it->second;
}
