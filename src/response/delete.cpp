#include "delete.hpp"

int Delete::isFileExisting(const Request& req)
{
    std::string path = "." + req.getRequestTarget();

    // Vérifie si le fichier existe
    if (access(path.c_str(), F_OK) != 0)
    {
        std::cout << "File does not exist: " << path << std::endl;
        return -1;
    }

    // Essaye de supprimer le fichier
    if (remove(path.c_str()) == 0)
    {
        std::cout << "File deleted successfully: " << path << std::endl;
        return 0;
    }
    else
    {
        std::perror("Error deleting file");
        return -1;
    }
}
