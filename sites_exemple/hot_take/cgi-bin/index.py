#!/usr/bin/env python3
import json
import cgi
import cgitb
import os
import sys  # Import nécessaire pour écrire dans stdout directement
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

def send_chunk(data):
    """Encapsule une chaîne de caractères au format Chunked Transfer Encoding."""
    if not data:
        return
    # On encode en utf-8 pour obtenir la taille réelle en octets
    chunk_body = data.encode('utf-8')
    size = hex(len(chunk_body))[2:].upper()
    sys.stdout.buffer.write(f"{size}\r\n".encode('utf-8'))
    sys.stdout.buffer.write(chunk_body + b"\r\n")
    sys.stdout.buffer.flush()

# --- Logique de traitement des données (inchangée) ---
form = cgi.FieldStorage()
new_title = form.getvalue('title')
new_content = form.getvalue('content')

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

# Headers : On utilise sys.stdout.buffer pour éviter les conflits d'encodage
sys.stdout.buffer.write(b"HTTP/1.0 200 OKOK\r\n")
sys.stdout.buffer.write(b"Content-Type: text/html; charset=UTF-8\r\n")
sys.stdout.buffer.write(b"Transfer-Encoding: chunked\r\n")
sys.stdout.buffer.write(b"\r\n")
sys.stdout.buffer.flush()

# 1er morceau : Le Head et le Header
send_chunk(f"""
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

# Morceaux suivants : Chaque article peut être un chunk séparé
for take in takes:
    article_html = f"""
    <article class="hot-take">
      <h2>{take['title']}</h2>
      <p class="date">Publié le {take['date']}</p>
      <p>{take['content']}</p>
    </article>
    """
    send_chunk(article_html)

# Dernier morceau : Fermeture du HTML
send_chunk("""
  </main>
</body>
</html>
""")

# Chunk de fin obligatoire
sys.stdout.buffer.write(b"0\r\n\r\n")
sys.stdout.buffer.flush()