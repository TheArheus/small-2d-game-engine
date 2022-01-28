#include <stdlib.h>
#include <stdio.h>
#include <string>

enum MessageType
{
    MessageType_Message,
    MessageType_Error,
};

#define CreateMessage___(String, MessageType, Line) logger Log_##MessageType_##Line(String, sizeof(String), MessageType);
#define CreateMessage__(Time, Line, FunctionName, String, MessageType) CreateMessage___(FunctionName ##"(line: "#Line ##") - " ##Time ##" - " ##String, MessageType, Line)
#define CreateMessage_(String, MessageType, Line) CreateMessage__(__TIME__, Line, __FUNCTION__, String, MessageType)
#define CreateMessage(String, MessageType) CreateMessage_(String, MessageType, __LINE__)

#define MessageLog(String) CreateMessage(String, MessageType_Message);
#define MessageErr(String) CreateMessage(String, MessageType_Error);

struct logger
{
    const char* Message;
    int MessageSize;
    int MessageLine;
    char* FunctionName;
    MessageType Type;

    logger(const char* Message_, int MessageSize_, MessageType Type_)
    {
        Message = Message_;
        MessageSize = MessageSize_;
        Type = Type_;
    }

    logger(std::string Message_, int MessageSize_, MessageType Type_)
    {
        Message = Message_.c_str();
        MessageSize = int(Message_.size()) + 1;
        Type = Type_;
    }

    ~logger()
    {
        char Buffer[512];
        snprintf(Buffer, MessageSize, Message);

        switch(Type)
        {
            case MessageType_Message:
            {
                printf("\033[90m");
                fprintf(stdout, "[LOG] %s\n", Buffer);
                printf("\033[0m");
            } break;
            case MessageType_Error:
            {
                printf("\033[31m");
                fprintf(stderr, "[ERR] %s\n", Buffer);
                printf("\033[0m");
            } break;
        }
    }
};

