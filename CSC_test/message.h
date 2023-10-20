#pragma once

#ifndef MESSAGE_H
#define MESSAGE_H

#endif // MESSAGE_H

#include <string>

struct Message
{
    std::string text;
    std::string sender;
};

Message ParseMessage(std::string message, std::string separator);
