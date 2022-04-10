#ifndef ADDRESS_EXTRACTOR_H
#define ADDRESS_EXTRACTOR_H

#include <string>
#include "nlohmann/json.hpp"
#include "Address.h"

class AddressExtractor {
public:
   Address addressFrom(const std::string &str) const;

private:
   nlohmann::json jsonAddressFrom(const std::string &str) const;
   void populate(Address &address, nlohmann::json &jsonAddress) const;
};

#endif