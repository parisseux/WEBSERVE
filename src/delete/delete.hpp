#ifndef DELETE_HPP
#define DELETE_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <iostream>
# include <fstream>
# include <vector>
# include <algorithm>
# include "../response/Response.hpp"
# include "../config/LocationConfig.hpp"

class Delete
{
    private:
    public:
        int isFileExisting(const Request& req);
};

#endif
