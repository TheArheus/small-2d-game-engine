#include <sstream>
#include <iomanip>
#include <ctime>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>

enum MessageType
{
    MessageType_Message,
    MessageType_Error,
};

#define TimeStamp() ({time_t retval;retval=time(NULL);auto retval=asctime(localtime(&TimeStamp));retval;})

#define CreateMessage___(String, MessageType, Line) logger Log_##MessageType_##Line(String, sizeof(String), MessageType);
#define CreateMessage__(Line, FunctionName, String, MessageType) CreateMessage___(FunctionName ##"(line: "#Line ##") - " ##String, MessageType, Line)
#define CreateMessage_(String, MessageType, Line) CreateMessage__(Line, __FUNCTION__, String, MessageType)
#define CreateMessage(String, MessageType) CreateMessage_(String, MessageType, __LINE__)

#define MessageLog(String) CreateMessage(String, MessageType_Message);
#define MessageErr(String) CreateMessage(String, MessageType_Error);

struct logger
{
    const char* Message;
    int MessageSize = 0;

    MessageType Type;

    int MessageLine;

    logger(const char* Message_, int MessageSize_, MessageType Type_)
    {
        Type = Type_;

        auto Time = std::time(NULL);
        auto TimeStamp = *std::localtime(&Time);
        std::ostringstream oss;
        oss << std::put_time(&TimeStamp, "%Hh %Mm %Ss - ");
        std::string OutTime = oss.str();

        char NewMessage[256] = {};
        strncat(NewMessage, OutTime.c_str(), OutTime.size());
        MessageSize += int(OutTime.size());
        strcat(NewMessage, Message_);
        MessageSize += int(MessageSize_ - 1);
        strcat(NewMessage, "\0");
        MessageSize += 1;
        Message = NewMessage;
    }

    logger(std::string Message_, int MessageSize_, MessageType Type_)
    {
        Type = Type_;

        auto Time = std::time(NULL);
        auto TimeStamp = *std::localtime(&Time);
        std::ostringstream oss;
        oss << std::put_time(&TimeStamp, "%Hh %Mm %Ss - ");
        std::string Out = oss.str();

        Out += Message_;
        Message = Out.c_str();
        MessageSize = Out.size() + 1;
    }

    ~logger()
    {
        char Buffer[256] = {};
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

