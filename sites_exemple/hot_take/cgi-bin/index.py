#!/usr/bin/env python3
import warnings
warnings.filterwarnings("ignore", category=DeprecationWarning)
import json
import cgi
import cgitb
import os
import sys  # Toujours utile si on veut manipuler sys.stdout
from datetime import datetime


# Juste après l'import os
script_dir = os.path.dirname(os.path.abspath(__file__))

# On change le répertoire de travail du processus
os.chdir(script_dir)

with open("debug.log", "a") as f:
    f.write(f"Content-Length reçu : {os.environ.get('CONTENT_LENGTH')}\n")
    f.write(f"request reçu : {os.environ.get('REQUEST_METHOD')}\n")

# Configuration du debug
cgitb.enable()
DB_FILE = 'data.json'

# --- Logique de traitement des données (inchangée) ---
form = cgi.FieldStorage()
new_title = form.getvalue('title')
new_content = form.getvalue('content')

# counter = 0
# while True:
#   counter +=1

if new_title and new_content:
    if os.path.exists(DB_FILE):
        with open(DB_FILE, 'r', encoding='utf-8') as f:
            takes = json.load(f)
    else:
        takes = []
    takes.insert(0, {
        "title": new_title,
        "content": new_content,
        "date": datetime.now().strftime("%d %B %Y")
    })
    with open(DB_FILE, 'w', encoding='utf-8') as f:
        json.dump(takes, f, indent=4, ensure_ascii=False)

if os.path.exists(DB_FILE):
    with open(DB_FILE, 'r', encoding='utf-8') as f:
        takes = json.load(f)
else:
    takes = []

# --- Début de la réponse HTTP ---

# Headers standards
print("Content-Type: text/html; charset=UTF-8\r\n")  # Ligne vide obligatoire pour séparer headers et body
sys.stdout.flush()

# Le Head et le Header
print(f"""<!DOCTYPE html>
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

# Affichage des articles
for take in takes:
    print(f"""
    <article class="hot-take">
      <h2>{take['title']}</h2>
      <p class="date">Publié le {take['date']}</p>
      <p>{take['content']}</p>
    </article>
    """)

# Fermeture du HTML
print("""
  </main>
</body>
</html>
""")