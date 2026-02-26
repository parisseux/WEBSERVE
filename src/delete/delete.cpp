#include "delete.hpp"
#include <cstdio>

int Delete::isFileExisting(const Request& req)
{
    int status;
    if (access(req.getRequestTarget().c_str(), F_OK) == 0)
    {
        status = remove(req.getRequestTarget().c_str());
        if (status == 0)
            std::cout << "file delete successfull" << std::endl;
        else if (status == -1)
            std::cout << "Error trying delete file" << std::endl;
    }
    return (status);
}
