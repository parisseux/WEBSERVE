#include "Upload.hpp"


int Upload::CheckBodySize(const LocationConfig &loc, const Request &req)
{
    if (!loc.getHasMaxBodySize())
        return 0;
    if (req.getBody().size() > loc.getMaxBodySize())
        return 413;
    return (0);
}

Response Upload::Handle(const LocationConfig &loc, const Request &req)
{
    
}