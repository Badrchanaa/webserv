#include "ConfigServer.hpp"



ConfigServer::~ConfigServer() {}


const Location* ConfigServer::getLocation(const std::string& path) const {
    const Location* bestMatch = NULL;
    const Location* defaultLocation = NULL;
    size_t maxLength = 0;

    for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const Location& loc = *it;

        if (loc.uri == "/") {
            defaultLocation = &loc;
        }

        if (path.find(loc.uri) == 0 && loc.uri.length() > maxLength) {
            maxLength = loc.uri.length();
            bestMatch = &loc;
        }
    }

    if (!bestMatch) {
        if (defaultLocation) {
            std::cout << "[getLocation] No match found. Falling back to default location: /" << std::endl;
            return defaultLocation;
        } else {
            std::cout << "[getLocation] No matching location and no default location found." << std::endl;
            return NULL;
        }
    }

    return bestMatch;
}
