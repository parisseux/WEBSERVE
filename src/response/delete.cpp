#include "delete.hpp"

int Delete::isFileExisting(const Request& req, const LocationConfig& loc)
{
    std::string target = req.getRequestTarget();
    std::string filename = target.substr(req.getRequestTarget().find_last_of("/"));
    std::string path = loc.getUploadPath() + filename;

    // Vérifie si le fichier existe
    if (access(path.c_str(), F_OK) != 0)
    {
        std::cout << "File does not exist: " << path << std::endl;
        return -1;
    }
    // Essaye de supprimer le fichier
    if (remove(path.c_str()) == 0)
    {
        return 0;
    }
    else
    {
        std::perror("Error deleting file");
        return -1;
    }
}
