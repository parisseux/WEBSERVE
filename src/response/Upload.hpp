#ifndef UPLOAD_HPP
# define UPLOAD_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <iostream>
# include <fstream>
# include "../response/Response.hpp"
# include "../config/LocationConfig.hpp"

class LocationConfig;
class Request;
class Response;

class Upload {
public:
    int CheckBodySize(const LocationConfig &loc, const Request &req);
    Response Handle(const LocationConfig &loc, const Request &req);


};


#endif 