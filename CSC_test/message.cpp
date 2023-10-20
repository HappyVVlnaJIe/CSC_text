#include "message.h"

Message ParseMessage(std::string message, std::string separator)
{
    const char* indx = message.c_str();
    while(*indx != '\0') indx++;
    std::string text = message.substr(0, indx - message.c_str());
    size_t pos = text.find_last_of(separator);
    if (pos == std::string::npos) return {"",""};
    return {text.substr(0, pos), text.substr(pos+1)};
}
