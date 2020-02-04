#include "ArgumentReader.hh"
#include <iostream>
#include <unordered_map>
#include <map>
#include <algorithm>
#include <cstring>
#include <cassert>

using namespace std;

enum KeyId
{
    KEY_TYPE = ArgumentReader::KEY_USER_DEFINED,
    KEY_CHARS,
    KEY_STYLE,
    KEY_LIME,
};

enum MessageId
{
    MSG_UNKNOWN_FORMAT = ArgumentReader::MSG_USER_DEFINED,
};

class MyKeyHandler: public KeyHandler
{
public:
    ~MyKeyHandler() override = default;
    int handle(int keyId, const string & key, const list<string> & values) override;
};

class MyMessageHandler: public MessageStrategy
{
public:
    ~MyMessageHandler() override = default;
    void echo(int msg, const string & key, const list<string> & values) override;
};

static
void echoValues(const list<string> & values);

static
int & foo(std::vector<int> & v, int & alt)
{
    auto it = std::find(v.begin(), v.end(), 3);
    if (it == v.end())
        return alt;
    return *it;
}

int main(int argc, char* argv[])
{
    std::vector<int> vec {1,2,3,4,5,6,7,8,9,10};
    auto & x = foo(vec, *static_cast<int*>(nullptr));
//    x += 1;
    assert(&x != nullptr);

    ArgumentReader parser;
    parser.setHeader("Super Mega Program (c)");
    parser.setKeyHandler(std::make_unique<MyKeyHandler>());
    parser.setMessageStrategy(std::make_unique<MyMessageHandler>());
    parser.addKey(KEY_TYPE, "type", true);
    parser.addKey(KEY_LIME, "lime");
    parser.addKeys(KEY_CHARS, {"g", "a", "f"}, true);
    parser.addKey(KEY_STYLE, "style");
    parser.addHelp("This help");
    if (!parser.read(argv))
        return 1;
    return 0;
}

int MyKeyHandler::handle(int keyId, const std::string & key, const list<string> & values)
{
//    using AP = ArgumentReader;
//    switch (keyId)
//    {
//    case KEY_CHARS:
//        break;
//    case AP::KEY_DEFAULT:
//        break;
//    }

//    if (keyId == KEY_CHARS)
//    {
//        std::cout << "SomeId" << std::endl;
//    }
//    else if (keyId == AP::KEY_DEFAULT)
//    {
//        std::cout << "DefaultKey";
//        echoValues(values);
//        std::cout << std::endl;
////        return MSG_UNKNOWN_FORMAT;
//    }
//    else
//    {
//    }

//    std::cout << "handleKey(): key is <" << key << ">, value is ";
//    echoValues(values);
//    std::cout << std::endl;

    return -1;
}

void MyMessageHandler::echo(int msg, const string & key, const list<string> & values)
{
    using AP = ArgumentReader::Message;
    switch (msg)
    {
    case AP::MSG_NO_VALUE:
        std::cout << "Key <" << key << "> has no value" << std::endl;
        break;
    case AP::MSG_UNKNOWN_KEY:
        std::cout << "Unknown key <" << key << '>' << std::endl;
        break;
    case MSG_UNKNOWN_FORMAT:
        std::cout << "Указано неверное значение: <" << values.back() << '>' << std::endl;
        break;
    default:
        break;
    }
}

void echoValues(const list<string> & values)
{
    bool isFirst = true;
    std::cout << '<';
    for (auto const & v : values)
    {
        if (isFirst)
            isFirst = false;
        else
            std::cout << ", ";
        std::cout << v;
    }
    std::cout << '>';
}
