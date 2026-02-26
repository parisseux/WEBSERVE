#include "delete.hpp"

int Delete::isFileExisting(const Request& req)
{
    access(req.getRequestTarget().c_str(), F_OK);
}
