#include <cstdlib>
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <memory>
#include <curl/curl.h>
#include "nlohmann/json.hpp"

// Example geolocation app: https://timothystotts.github.io/2020/12/17/a-cross-platform-geolocation-information-application-in-c.html
// Examples of using libcurl in c++: https://www.apriorit.com/dev-blog/344-libcurl-usage-downloading-protocols

/*************************************************************************/
/**************** CLASS: CurlManager *************************************/
/*************************************************************************/

/// This class wraps the initialization and cleanup functions of the libcurl.
class CurlManager {
public:

    /*************************************************************************/
    /**************** PUBLIC TYPES *******************************************/
    /*************************************************************************/

    /// Curl unique pointer typedef.
    using CurlHandle = std::unique_ptr<CURL, std::function<void(CURL *)>>;

    /*************************************************************************/
    /**************** PUBLIC FUNCTIONS ***************************************/
    /*************************************************************************/

    /// Initializes libcurl.
    CurlManager() { 
        curl_global_init(CURL_GLOBAL_ALL); 
    }

    /// Deinitializes libcurl.
    ~CurlManager() { 
        curl_global_cleanup(); 
    }

    /// Initialize a new curl handle for a HTTPS request.
    ///
    /// @return
    ///     The new curl handle.
    CurlHandle getNewCurlHandle() {
        auto curl = CurlHandle(curl_easy_init(), curl_easy_cleanup);
        if (!curl) {
            throw std::runtime_error("Failed creating CURL handle.");
        }
        return curl;
    }
};

/*************************************************************************/
/**************** CLASS: LocationFinder **********************************/
/*************************************************************************/

/// This class makes API request to find the location of the IP address of this machine.
class LocationFinder {
public:

    /*************************************************************************/
    /**************** PUBLIC CONSTANTS ***************************************/
    /*************************************************************************/

    static inline const std::string sIPAddressRequestURL{"https://api.ipify.org/?format=json"};
    static inline const std::string sUserAgent{"libcurl-agent/1.0"};
    static inline const std::string sLocationRequestURL{"http://api.ipstack.com/"};

    /*************************************************************************/
    /**************** PUBLIC FUNCTIONS ***************************************/
    /*************************************************************************/

    /// Assocaites the process libcurl manager with this object.
    LocationFinder(CurlManager &manager): mCurlManager(manager) {};

    /// Requests the public IP address of this machine.
    /// Uses the service: https://www.ipify.org/
    ///
    /// @return 
    ///     A json object with the ip address with key "ip".
    nlohmann::json requestIPAddress() {

        auto curl = mCurlManager.getNewCurlHandle();
        
        // Set curl options.
        // Example at: https://curl.se/libcurl/c/getinmemory.html
        curl_easy_setopt(curl.get(), CURLOPT_URL, sIPAddressRequestURL.c_str());

        // Some servers don't like requests that are made without a user-agent field, so we provide one.
        curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, sUserAgent.c_str());

        // Returned data is processed by this function.
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, sWriteMemoryCallback);

        // This is the user defined parameter in the CURLOPT_WRITEFUNCTION callback.
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, reinterpret_cast<void *>(this));

        // Perform the request, res will get the return code. This is a blocking call.
        CURLcode res = curl_easy_perform(curl.get());
        if (res != CURLE_OK) {
            std::cout << "ERROR: curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return nlohmann::json();
        }

        auto json = nlohmann::json::parse(mContents.back());
        mContents.pop_back();
        return json;
    }

    /// Request the data about an IP address which includes its latitude and longitude.
    /// IP address alone doesn't give an accurate position of the machine.
    /// Use the service: https://ipstack.com/
    /// 
    /// @param ipAddress
    ///     The json object returned from requestIPAddress().
    /// 
    /// @return 
    ///     json object containing various information about the IP address.
    nlohmann::json requestLatLong(const nlohmann::json &ipAddress) {
        
        auto curl = mCurlManager.getNewCurlHandle();

        // Build URL// 122.199.36.166?access_key=&format=1
        std::ostringstream oss;
        oss << sLocationRequestURL << ipAddress["ip"].get<std::string>() 
            << "?access_key=" << std::getenv("IPSTACK_KEY") << "&format=1";

        // Set curl options.
        curl_easy_setopt(curl.get(), CURLOPT_URL, oss.str().c_str());
        curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, sUserAgent.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, sWriteMemoryCallback);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, reinterpret_cast<void *>(this));

        std::cout << "Performing API request:" << std::endl;
        std::cout << oss.str() << std::endl;

        // Perform the request, res will get the return code. This is a blocking call.
        CURLcode res = curl_easy_perform(curl.get());
        if (res != CURLE_OK) {
            std::cout << "ERROR: curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
            return nlohmann::json();
        }

        // Parse returned data and return it.
        auto json = nlohmann::json::parse(mContents.back());
        mContents.pop_back();
        return json;
    }

    /*************************************************************************/
    /**************** PRIVATE FUNCTIONS **************************************/
    /*************************************************************************/
private:

    /// This provides the context to the Curl write function callback to capture response from curl request. The data
    /// is pushed to an internal container.
    ///
    /// @param contents
    ///     The pointer to the buffer with the return request.
    /// @param size
    ///     The size of a single element in the buffer.
    /// @param nmemb
    ///     The number of elements in the buffer.
    /// @param userp
    ///     Use defined data. In this application it is a reference to the LocationFinder that called the 
    ///     the GET request.
    size_t writeMemoryCallback(void *contents, size_t size, size_t nmemb, bool debug=false) {
        size_t realsize = size * nmemb;
        if (debug) {
            printf("%.*s\n", realsize, contents);
        }
        mContents.push_back(std::string(reinterpret_cast<const char *>(contents), realsize));
        return realsize;
    }

    /// Curl write function callback to capture response from curl request. It is a static function to support
    /// callbacks to the c-library libcurl.
    ///
    /// @param contents
    ///     The pointer to the buffer with the return request.
    /// @param size
    ///     The size of a single element in the buffer.
    /// @param nmemb
    ///     The number of elements in the buffer.
    /// @param userp
    ///     Use defined data. In this application it is a reference to the LocationFinder that called the 
    ///     the GET request.
    static size_t sWriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
        return reinterpret_cast<LocationFinder *>(userp)->writeMemoryCallback(contents, size, nmemb);
    }

    /*************************************************************************/
    /**************** PRIVATE FIELDS *****************************************/
    /*************************************************************************/

    /// Stores the results of HTTP requests.
    std::vector<std::string> mContents;

    /// A reference to the processes libcurl manager object.
    CurlManager &mCurlManager;
};


int main() {

    // Initializes and cleans up libcurl for this application.
    CurlManager curlManager;

    // Make request to fnd IP address and location.
    LocationFinder finder(curlManager);
    auto json = finder.requestIPAddress();
    std::cout << "Query for location of IP Address: " << json["ip"] << std::endl;
    auto jsonLocation = finder.requestLatLong(json);
    std::cout << jsonLocation.dump(4) << std::endl;
}
