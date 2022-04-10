#include <string>

#include "AddressExtractor.h"
#include "Http.h"
#include "PlaceDescriptionService.h"

using namespace std;

PlaceDescriptionService::PlaceDescriptionService(Http* http) : mHttp(http) {}

string PlaceDescriptionService::summaryDescription(const string &latitude, const string &longitude) const {
    auto jsonResponse = mHttp->get(createGetRequestUrl(latitude, longitude));
    AddressExtractor extractor;
    auto address = extractor.addressFrom(jsonResponse);
    return address.summaryDescription();
}

string PlaceDescriptionService::createGetRequestUrl(const string &latitude, const string &longitude) const {
    string server{"http://open.mapquestapi.com/"};
    string document{"nominatim/v1/reverse"};
    return server + document + "?" + keyValue("format", "json") + "&" + keyValue("lat", latitude) + "&" +
        keyValue("lon", longitude);
}

string PlaceDescriptionService::keyValue(const string& key, const string& value) const {
    return key + "=" + value;
}

