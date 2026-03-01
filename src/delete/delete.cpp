#include "delete.hpp"

int Delete::isFileExisting(const Request& req)
{
    int status;
    std::string path = "." + req.getRequestTarget();
    if (access(path.c_str(), F_OK) == 0)
    {
        status = remove(path.c_str());
        if (status == 0)
            std::cout << "file delete successfull" << std::endl;
        else if (status == -1)
            std::cout << "Error trying delete file" << std::endl;
    }
    else
        std::cout << "the file you trying to delete doest not exist" << std::endl;
    return (status);
}
