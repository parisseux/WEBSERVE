#!/usr/bin/env python3
import json
import cgi
import cgitb
import os
from datetime import datetime

import os

# print ("PYTHON SCRIPT")

# for cle, valeur in os.environ.items():
#     print(f"<tr><td><b>{cle}</b></td><td>{valeur}</td></tr>")

# print("</table>")

# Juste après l'import os
script_dir = os.path.dirname(os.path.abspath(__file__))

# On change le répertoire de travail du processus
os.chdir(script_dir)


with open("debug.log", "a") as f:
    f.write(f"Content-Length reçu : {os.environ.get('CONTENT_LENGTH')}\n")
    f.write(f"request reçu : {os.environ.get('REQUEST_METHOD')}\n")



# Affiche les erreurs sur la page pour le debug
cgitb.enable()

DB_FILE = 'data.json'

# 1. Traitement du formulaire (si envoyé)
form = cgi.FieldStorage()
new_title = form.getvalue('title')
new_content = form.getvalue('content')

if new_title and new_content:
    # Charger les données existantes
    if os.path.exists(DB_FILE):
        with open(DB_FILE, 'r', encoding='utf-8') as f:
            takes = json.load(f)
    else:
        takes = []

    # Ajouter la nouvelle take
    takes.insert(0, {
        "title": new_title,
        "content": new_content,
        "date": datetime.now().strftime("%d %B %Y")
    })

    # Sauvegarder
    with open(DB_FILE, 'w', encoding='utf-8') as f:
        json.dump(takes, f, indent=4, ensure_ascii=False)

# 2. Lecture des données pour l'affichage
if os.path.exists(DB_FILE):
    with open(DB_FILE, 'r', encoding='utf-8') as f:
        takes = json.load(f)
else:
    takes = []

# 3. Génération de la réponse HTTP
print(f"""
<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8" />
  <title>🔥 Hot Takes</title>
  <link rel="stylesheet" href="style.css" />
</head>
<body>
  <header>
    <h1>🔥 Hot Takes</h1>
    <form action="index.py" method="post" class="add-take">
      <input type="text" name="title" placeholder="Titre brûlant" required>
      <textarea name="content" placeholder="Ton avis sans filtre..." required></textarea>
      <button type="submit">Balancer ma Take</button>
    </form>
  </header>

  <main>
""")

for take in takes:
    print(f"""
    <article class="hot-take">
      <h2>{take['title']}</h2>
      <p class="date">Publié le {take['date']}</p>
      <p>{take['content']}</p>
    </article>
    """)

print("""
  </main>
</body>
</html>
""")