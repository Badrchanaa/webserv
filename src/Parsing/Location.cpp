
#include <unistd.h>
#include "Location.hpp"

std::string Location::getScriptPath(const std::string &scriptName) const
{
    std::string::size_type dotPos = scriptName.rfind('.');
    if (dotPos == std::string::npos )
    return (std::string(""));
    std::string ext;
    // if (dotPos == 0)
    //   ext = scriptName;
    // else 
    ext = scriptName.substr(dotPos);
    std::map<std::string, std::string>::const_iterator it = cgi.find(ext);
    if (it == cgi.end())
    return (std::string(""));
    // return NULL;
    return it->second;
}

std::string Location::getScriptName(const std::string &path) const
{
    std::string::size_type slashPos = path.rfind('/');
    if (slashPos == std::string::npos || path.length() == 1)
        return (std::string(""));
    std::string temp = path.substr(slashPos + 1);
    return temp;
}

bool Location::isCgiPath(const std::string &path) const
{
    std::string filename = getScriptName(path);
    std::string scriptPath = getScriptPath(filename);
    return (!scriptPath.empty() &&  (this->uri + this->cgi_uri == path.substr(0, path.length() - filename.length())));
}

bool Location::isMethodAllowed(httpMethod method) const {
    return (allowed_methods & method) != 0;
}

bool Location::isValidFormat(const std::string& filename) {
    const std::string validExts[] = {".html", ".htm", ".php", ".html5", ".bad_extension"};
    for (int i = 0; i < 5; ++i) {
        if (filename.size() >= validExts[i].size() &&
            filename.compare(filename.size() - validExts[i].size(), validExts[i].size(), validExts[i]) == 0) {
            return true;
        }
    }
    return false;
}
void Location::parseValidIndexes(const std::string& indexLine) {
    std::vector<std::string> validIndexes;
    std::istringstream iss(indexLine);
    std::string token;

    while (iss >> token) {
        if (isValidFormat(token)) {
            this->indexes.push_back(token);
        }
        else
          throw std::runtime_error("Invalid Index FileName: " + token);
    }

}

void Location::parseRedirectionValue(const std::string &value){
    if (value.empty())
        throw std::runtime_error("Redirection Value is Empty\n");

    for (size_t i = 0; i < value.length(); ++i) {
        if (value[i] == ' ' || value[i] == '\t')
            throw std::runtime_error("Invalid Redirection Value: " + value + "\n");
    }

    if (value[0] == '/')
        this->redirection = value;
    else if (value.compare(0, 7, "http://") == 0 || value.compare(0, 8, "https://") == 0)
        this->redirection = value;
    else 
        throw std::runtime_error("Redirection Value " + value + " Should Start: / https://url http://url\n");
}

bool Location::hasRedirection() const {
    return (!this->redirection.empty());
}

std::string Location::getRedirection() const{
    return this->redirection;
}



// std::string getIndexPath(const std::string &path) const {
//   if (this->index.empty())
//     return path;
//   if (!path.empty() && path[path.size() - 1] == '/')
//     return path + this->index;
//   return path + "/" + this->index;
// }

bool Location::fileExists(const std::string& path) const {
    struct stat buffer;
    const char *path_c = path.c_str();
    return (access(path_c, R_OK) == 0 && stat(path_c, &buffer) == 0);
}

std::string Location::getIndexPath(const std::string &path) const {
if (this->indexes.empty())
    return path;

std::string basePath = path;
if (!path.empty() && path[path.size() - 1] != '/')
    basePath += "/";

for (size_t i = 0; i < this->indexes.size(); ++i) {
    std::string fullPath = basePath + this->indexes[i];
    if (this->fileExists(fullPath))
        return fullPath;
}
return path;
}

Location::Location() : autoindex(0), allowed_methods(0), allowed_cgi_methods(0) {}
