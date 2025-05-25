#include "ConfigServer.hpp"



ConfigServer::~ConfigServer() {}


const Location& ConfigServer::getLocation(const std::string& path) const {
    const Location* bestMatch = NULL;
    size_t maxLength = 0;

    for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const Location& loc = *it;

        if (path.find(loc.uri) == 0 && loc.uri.length() > maxLength) {
            maxLength = loc.uri.length();
            bestMatch = &loc;
        }
    }

    if (!bestMatch) {
        std::cout << "[getLocation] No exact match. Falling back to last location: "
                  << locations[locations.size() - 1].uri << std::endl;
        return locations[locations.size() - 1];
    }

    return *bestMatch;
}



// const Location &ConfigServer::getLocation(const std::string &path) const {
// const Location *bestMatch = NULL;
// size_t maxLength = 0;

// return locations[locations.size() - 1];
// for (std::vector<Location>::const_iterator it = locations.begin(); it != locations.end(); ++it) {
// const Location &loc = *it;
// // return loc;
// // i have some mistiks with this uri  don't forget about it
// if (path.find(loc.uri) == 0 && loc.uri.length() > maxLength) {
//     maxLength = loc.uri.length();
//     bestMatch = &loc;
// } }
// if (!bestMatch) {
// std::cout << "location uri form getLocation function :: "
//             << locations[locations.size() - 1].uri << std::endl;
// return locations[locations.size() - 1];
// }
// return *bestMatch;
// }