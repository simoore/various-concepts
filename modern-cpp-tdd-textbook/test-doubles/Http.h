#ifndef HTTP_H
#define HTTP_H

#include <string>

class Http {
public:
    virtual ~Http() {}
    virtual void initialize() = 0;
    virtual std::string get(const std::string &url) const = 0;
};

#endif // HTTP_H