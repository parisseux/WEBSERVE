#include "autoindex.hpp"

//en gros cest pouvoir pas que le nom d'un fichier casse le html
//on change &, <, > et ""
static std::string EscapeHtml(const std::string& name)
{
    std::string new_name;
    new_name.reserve(name.size());
    for (size_t i = 0; i < name.size(); ++i)
    {
        char c = name[i];
        if (c == '&')
            new_name += "&amp;";
        else if (c == '<')
            new_name += "&lt;";
        else if (c == '>') 
            new_name += "&gt;";
        else if (c == '"')
            new_name += "&quot;";
        else
            new_name += c;
    }
    return new_name;
}

//on créer le chemin complet vers l'entré qui nous interresse
//exemple diskpath = "./www/listing/images/", file = "cat.png"
//donc "./www/listing/images/cat.png"
static std::string JoinDisk(const std::string& diskPath, const std::string& file)
{
    if (diskPath.empty())
        return file;
    if (diskPath[diskPath.size() - 1] == '/')
        return diskPath + file;
    return diskPath + "/" + file;
}

bool CompareEntries(const AutoIndexEntry& a, const AutoIndexEntry& b)
{
    if (a.isDir != b.isDir)
        return a.isDir > b.isDir;
    return a.name < b.name;
}

// Cette fonction génère dynamiquement une page HTML en fonction du contenu actuel du dossier sur le disque.
// On ne peut pas utiliser un fichier .html statique, car contenu du dossier peut changer à tout moment.
// Donc à chaque requête GET sur un dossier --> on lit le contenu du dossier en temps réel
// et on construit une page HTML
std::string GenerateAutoIndexHtml(const std::string& diskPath, const std::string& requestPath)
{
    std::string baseUrl = requestPath;
    if (baseUrl.empty() || baseUrl[baseUrl.size() - 1] != '/')
        baseUrl += '/';

    DIR* dir = opendir(diskPath.c_str());
    if (!dir)
        return "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
               "<title>403 Forbidden</title></head>"
               "<body><h1>403 Forbidden</h1><p>Cannot open directory.</p></body></html>";
    std::vector<AutoIndexEntry> entries;
    struct dirent* ent;
    while ((ent = readdir(dir)) != NULL)
    {
        //ignore cache
        std::string name = ent->d_name;
        if (name == "." || name == "..")
            continue;
        std::string full = JoinDisk(diskPath, name);
        struct stat st;
        bool isDir = (stat(full.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
        entries.push_back(AutoIndexEntry(name, isDir));
    }
    closedir(dir);

    //on va trier dans lordre dabbord dossier et apres fichier ordre alpha 
    std::sort(entries.begin(), entries.end(), CompareEntries);

    //on va avoir un <li> pas dossier/fichier contenue dans notre dossier
    std::ostringstream list;
    if (requestPath != "/")
        list << "<li><a class=\"parent\" href=\"../\">Parent directory</a></li>\n";
    for (size_t i = 0; i < entries.size(); ++i)
    {
        const AutoIndexEntry& e = entries[i];
        std::string display = EscapeHtml(e.name);
        std::string href = baseUrl + e.name;
        //je rajotue un /apres dossier plus pro
        if (e.isDir)
        {
            display += "/";
            href += "/";
        }
        list << "<li><a class=\"" << (e.isDir ? "dir" : "file") << "\" href=\""
             << EscapeHtml(href) << "\">"
             << display
             << "</a></li>\n";
    }

    std::string safePath = EscapeHtml(requestPath);
    std::ostringstream html;
    
    html <<
"<!DOCTYPE html>\n"
"<html lang=\"en\">\n"
"<head>\n"
"  <meta charset=\"UTF-8\">\n"
"  <title>Index of " << safePath << "</title>\n"
"  <style>\n"
"    body{margin:0;padding:40px;background:#fdeef3;font-family:Segoe UI,Tahoma,sans-serif;color:#5e4a54;}\n"
"    .container{max-width:820px;margin:auto;background:#fff7fa;padding:35px 45px;border-radius:22px;"
"box-shadow:0 10px 35px rgba(214,164,181,.25);}\n"
"    h1{margin-top:0;font-weight:500;font-size:26px;color:#c26d8d;}\n"
"    .subtitle{font-size:14px;margin-bottom:30px;color:#b98aa0;}\n"
"    ul{list-style:none;padding:0;margin:0;}\n"
"    li{margin:10px 0;}\n"
"    a{text-decoration:none;color:#a05574;padding:8px 12px;border-radius:12px;display:inline-block;"
"transition:all .25s ease;font-size:15px;}\n"
"    a:hover{background:#ffe3ec;transform:translateX(6px);}\n"
"    .dir::before{content:\"🌸 \";}\n"
"    .file::before{content:\"💗 \";}\n"
"    .parent::before{content:\"↩ \";}\n"
"    footer{margin-top:35px;font-size:12px;color:#c9a5b5;text-align:center;}\n"
"  </style>\n"
"</head>\n"
"<body>\n"
"<div class=\"container\">\n"
"  <h1>🦥 Index of " << safePath << "</h1>\n"
"  <div class=\"subtitle\">Browsing softly… take your time.</div>\n"
"  <ul>\n" << list.str() << "  </ul>\n"
"  <footer>Served slowly by your webserv 💕</footer>\n"
"</div>\n"
"</body>\n"
"</html>\n";

    return html.str();
}