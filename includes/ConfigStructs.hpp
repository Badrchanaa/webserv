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
#include "http.hpp"


struct MethodPair {
  const char *name;
  httpMethod method;
};

struct Location {
  Location() : allowed_methods(METHOD_NONE), allowed_cgi_methods(METHOD_NONE) {}

  std::string uri;
  std::string root;
  unsigned int allowed_methods;
  unsigned int allowed_cgi_methods;
  bool autoindex;
  std::string upload;
  std::map<std::string, std::string> cgi;
  
  // MUST BE ADDED
};

struct ConfigServer {
  std::string host;
  std::vector<int> ports;
  std::vector<std::string> server_names;
  std::string body_size;
  std::map<std::string, std::string> errors;
  Location location;

  // MUST BE ADDED

  ~ConfigServer(){}
};

#endif // !DEBUG


                      
  
  

  
  
  

      

 
  

  
  
  
    
  

  
  
    

    
    
      
      

      
      
      
      
      

      
      
          
      
        
        
      

      
      
        
            

        
          
          
        

        
        
        
                       
          
          
        

        
        
          
            
            
          

          
          
          
          
          
                    
          
        
      
      
    
  

  
    
  

    

  

  
