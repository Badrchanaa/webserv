#ifndef _ConfigStructs__
#define _ConfigStructs__
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

enum HttpMethod {
  METHOD_NONE = 0,
  GET = 1 << 0,
  POST = 1 << 1,
  DELETE = 1 << 2
};

struct MethodPair {
  const char *name;
  HttpMethod method;
};

struct Location {
  std::string uri;
  std::string root;
  unsigned int allowed_methods;
  unsigned int allowed_cgi_methods;
  bool autoindex;
  std::string upload;
  std::map<std::string, std::string> cgi;
  Location() : allowed_methods(METHOD_NONE), allowed_cgi_methods(METHOD_NONE) {}
};

struct ConfigServer {
  std::string host;
  std::vector<int> ports;
  std::vector<std::string> server_names;
  std::string body_size;
  std::map<std::string, std::string> errors;
  Location location;
  ~ConfigServer(){}
};

#endif // !DEBUG


                      
  
  

  
  
  

      

 
  

  
  
  
    
  

  
  
    

    
    
      
      

      
      
      
      
      

      
      
          
      
        
        
      

      
      
        
            

        
          
          
        

        
        
        
                       
          
          
        

        
        
          
            
            
          

          
          
          
          
          
                    
          
        
      
      
    
  

  
    
  

    

  

  
