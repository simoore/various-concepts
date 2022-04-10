#include "gmock/gmock.h"

#include "Http.h"
#include "PlaceDescriptionService.h"

using namespace ::testing;

class APlaceDescriptionService: public Test {
public:
   static inline const std::string ValidLatitude{"38.005"};
   static inline const std::string ValidLongitude{"-104.44"};
};

class HttpStub: public Http {
public:
    MOCK_METHOD((void), initialize, (), (override));
    MOCK_METHOD((std::string), get, (const std::string &), (const, override));
};

TEST_F(APlaceDescriptionService, MakesHttpRequestToObtainAddress) {
    HttpStub httpStub;
    std::string urlStart{"http://open.mapquestapi.com/nominatim/v1/reverse?format=json&"};
    auto expectedUrl = urlStart + "lat=" + APlaceDescriptionService::ValidLatitude + "&" +
        "lon=" + APlaceDescriptionService::ValidLongitude;
    EXPECT_CALL(httpStub, get(expectedUrl));
    PlaceDescriptionService service{&httpStub};
    auto description = service.summaryDescription(ValidLatitude, ValidLongitude);
}

TEST_F(APlaceDescriptionService, FormatsRetrievedAddressIntoSummaryDescription) {
    HttpStub httpStub;
    EXPECT_CALL(httpStub, get(_))
        .WillOnce(Return(R"({ "address": {"road":"Drury Ln", "city":"Fountain", "state":"CO", "country":"US" }})"));
    PlaceDescriptionService service(&httpStub);
    auto description = service.summaryDescription(ValidLatitude, ValidLongitude);
    ASSERT_THAT(description, Eq("Drury Ln, Fountain, CO, US"));
}