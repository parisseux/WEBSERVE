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

//en gros faut que tu utilise upload path je pense pour que tu puisse gerer les deletes dans dautres fichiers quoi 
// int Delete::isFileExisting(const Request& req, const LocationConfig &loc)
// {
//     if (!loc.getHasUploadPath())
//     {
//         std::cout << "pas de upload path" << std::endl;
//         return (400);
//     }
//     _uploadDir =  loc.getUploadPath();
//     std::string path = _uploadDir + req.getRequestTarget();
//     std::cout << "REQUEST TARGET"  << req.getPath() << std::endl;
//     std::cout << "PAAAAAATH:" << path << std::endl;
//     // Vérifie si le fichier existe
//     if (access(_uploadDir.c_str(), F_OK) != 0)
//     {
//         std::cout << "File does not exist: " << path << std::endl;
//         return -1;
//     }

//     // Essaye de supprimer le fichier
//     if (remove(path.c_str()) == 0)
//     {
//         std::cout << "File deleted successfully: " << path << std::endl;
//         return 0;
//     }
//     else
//     {
//         std::perror("Error deleting file");
//         return -1;
//     }
// }
