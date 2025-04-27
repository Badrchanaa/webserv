// #include "Config.hpp"
// // int main(int argc, char **argv) {
// int main() {
//   // if (argc != 2)
//   // else
//   //   std::ifstream infile(argv[1]);
//   Config parse;
//   parse.ParseConfigFile(DEFAULT_PATH);

//   //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//   // ++++++++++++++++++++ 🎯 Print results +++++++++++++//
//   // +++++++++++++++++++++++++++++++++++++++++++++++++++++++

//   std::cout << "servers : " << parse.ServersNumber() << std::endl;
//   for (int i = 0; i < parse.ServersNumber(); ++i) {
//     const ServerConfig &s = parse.getServer(i);
//     std::cout << "==== Server #" << i + 1 << " ====\n";
//     std::cout << "Host: " << s.host << "\n";
//     std::cout << "server_names: " << std::endl;
//     for (std::vector<std::string>::const_iterator it = s.server_names.begin();
//          it != s.server_names.end(); it++) {
//       std::cout << " - " << *it << std::endl;
//     }

//     std::cout << "Ports: " << std::endl;
//     for (std::vector<int>::const_iterator it = s.ports.begin();
//          it != s.ports.end(); it++) {
//       std::cout << " - " << *it << std::endl;
//     }
//     // std::cout << "Server Name: " << s.server_name << "\n";
//     std::cout << "Body Size: " << s.body_size << "\n";
//     std::cout << "Errors:\n";
//     for (std::map<std::string, std::string>::const_iterator it =
//              s.errors.begin();
//          it != s.errors.end(); ++it)
//       std::cout << "  " << it->first << ": " << it->second << "\n";

//     std::cout << "+++++++++++++++++++++++++++++++++++++++" << std::endl;

//     std::cout << "Location:\n";
//     std::cout << "  URI: " << s.location.uri << "\n";
//     std::cout << "  Root: " << s.location.root << "\n";
//     std::cout << "  Autoindex: " << s.location.autoindex << "\n";
//     std::cout << "  Upload: " << s.location.upload << "\n";
//     // std::cout << "  Methods: ";
//     std::cout << "Methods: " << parse.method_bit_to_string(s.location.allowed_methods)
//               << "\n";
//     std::cout << std::endl;
//     std::cout << "+++++++++++++++++++++++++++++++++++++++" << std::endl;
//     std::cout << "\n  Errors:\n";
//     for (std::map<std::string, std::string>::const_iterator it =
//              s.errors.begin();
//          it != s.errors.end(); ++it) {
//       std::cout << "    " << it->first << ": " << it->second << "\n";
//     }
//     std::cout << "+++++++++++++++++++++++++++++++++++++++" << std::endl;
//     std::cout << "CGI Methods: "
//               << parse.method_bit_to_string(s.location.allowed_cgi_methods) << "\n";
//     std::cout << std::endl
//               << "+++++++++++++++++++++++++++++++++++++++" << std::endl;
//     std::cout << "\n  CGI:\n";
//     for (std::map<std::string, std::string>::const_iterator it =
//              s.location.cgi.begin();
//          it != s.location.cgi.end(); ++it)
//       std::cout << "    " << it->first << ": " << it->second << "\n";
//     std::cout << std::endl;
//   }

//   return 0;
// }
