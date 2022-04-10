#include <string>

#include "AddressExtractor.h"

using namespace std;
using json = nlohmann::json;

Address AddressExtractor::addressFrom(const string &str) const {
    Address address;
    json jsonAddress = jsonAddressFrom(str);
    if (!jsonAddress.is_null()) {
        populate(address, jsonAddress);
    }
    return address; 
}

json AddressExtractor::jsonAddressFrom(const string &str) const {
    try {
        json location = json::parse(str);
        return location.value("address", json(nullptr));
    } catch (json::parse_error &e) {
        return json(nullptr);
    }
}

void AddressExtractor::populate(Address &address, json &jsonAddress) const {
    address.road = jsonAddress.value("road", "");
    address.city = jsonAddress.value("city", "");
    address.state = jsonAddress.value("state", "");
    address.country = jsonAddress.value("country", "");
}
