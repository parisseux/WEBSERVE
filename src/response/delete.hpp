#ifndef DELETE_HPP
#define DELETE_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <iostream>
# include <fstream>
# include <vector>
# include <algorithm>
# include <cstdio>
# include "../response/Response.hpp"
# include "../config/LocationConfig.hpp"

class Delete
{
    private:
        int _status;
        std::string _uploadDir;
    public:
        int isFileExisting(const Request& req, const LocationConfig &loc);
};

#endif
