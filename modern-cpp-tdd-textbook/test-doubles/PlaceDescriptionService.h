#ifndef PLACE_DESCRIPTION_SERVICE_H
#define PLACE_DESCRIPTION_SERVICE_H

#include <string>

#include "Address.h"

class Http;

class PlaceDescriptionService {
public:
   PlaceDescriptionService(Http* http);
   std::string summaryDescription(const std::string &latitude, const std::string &longitude) const;

private:
   std::string createGetRequestUrl(const std::string &latitude, const std::string &longitude) const;
   std::string keyValue(const std::string &key, const std::string &value) const;
   std::string summaryDescription(const Address &address) const;

   Http* mHttp;
};

#endif