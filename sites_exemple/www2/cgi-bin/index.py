#!/usr/bin/env python3
import sys
import os
import re

# Juste après l'import os
script_dir = os.path.dirname(os.path.abspath(__file__))

# On change le répertoire de travail du processus
os.chdir(script_dir)

UPLOAD_DIR = "../../uploads"

def safe_filename(filename):
    if not filename: return None
    filename = os.path.basename(filename)
    return re.sub(r'[^a-zA-Z0-9._-]', '', filename)

def send_chunk(data):
    """Envoie proprement un morceau de données au format binaire."""
    if not data:
        return
    # 1. Convertir en bytes impérativement pour calculer la vraie taille
    if isinstance(data, str):
        data = data.encode('utf-8')
    
    # 2. Calculer la taille en octets (hexadécimal)
    size_hex = hex(len(data))[2:].upper().encode('ascii')
    
    # 3. Tout écrire sur le buffer binaire (STDOUT binaire)
    sys.stdout.buffer.write(size_hex + b"\r\n")
    sys.stdout.buffer.write(data + b"\r\n")
    sys.stdout.buffer.flush()

def handle_upload():
    """Logique de traitement de l'upload (inchangée)"""
    content_type = os.environ.get('CONTENT_TYPE', '')
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        body = sys.stdin.buffer.read(content_length)
    except:
        return "Erreur de lecture."

    if not body: return "Corps vide."

    match = re.search(r'boundary=(.+)', content_type)
    if not match: return "Pas de boundary."
    
    boundary = b"--" + match.group(1).encode().strip()
    parts = body.split(boundary)
    saved = []
    for part in parts:
        if b'filename="' in part:
            headers, content = part.split(b'\r\n\r\n', 1)
            fname = safe_filename(re.search(r'filename="([^"]+)"', headers.decode(errors='ignore')).group(1))
            if content.endswith(b'\r\n'): content = content[:-2]
            with open(os.path.join(UPLOAD_DIR, fname), 'wb') as f: f.write(content)
            saved.append(fname)
    return f"Succès : {', '.join(saved)}" if saved else "Aucun fichier."

if __name__ == "__main__":
    method = os.environ.get('REQUEST_METHOD', 'GET')
    
    # --- HEADERS (CGI Style) ---
    # Si ton serveur C++ ne rajoute PAS de header lui-même, garde "HTTP/1.1 200 OK"
    # Sinon, utilise "Status: 200 OK"
    # sys.stdout.buffer.write(b"HTTP/1.1 200 OK\r\n")
    sys.stdout.buffer.write(b"Content-Type: text/html; charset=utf-8\r\n")
    sys.stdout.buffer.write(b"Transfer-Encoding: chunked\r\n")
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.flush()

    status_msg = ""
    if method == 'POST':
        status_msg = handle_upload()

    # --- CHUNKS ---
    html_start = f"""<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>C++ Webserv Chunked</title>
    <style>
        body {{ font-family: sans-serif; background: #2c3e50; color: white; text-align: center; padding-top: 50px; }}
        .card {{ background: #ecf0f1; color: #2c3e50; display: inline-block; padding: 30px; border-radius: 10px; }}
    </style>
</head>
<body>
    <div class="card">
        <h1>📁 Serveur C++ : Mode Chunked</h1>"""
    
    send_chunk(html_start)

    form_html = f"""
        <form action="/index.py" method="POST" enctype="multipart/form-data">
            <input type="file" name="file" required><br><br>
            <button type="submit" style="background:#e67e22; color:white; border:none; padding:10px; border-radius:5px;">🚀 Envoyer</button>
        </form>
        <p style="color: green;">{status_msg}</p>
    </div>
</body>
</html>"""
    
    send_chunk(form_html)

    # --- FIN DU TRANSFERT ---
    # Le chunk final 0\r\n\r\n doit être envoyé seul
    sys.stdout.buffer.write(b"0\r\n\r\n")
    sys.stdout.buffer.flush()