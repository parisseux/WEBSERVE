#!/usr/bin/env python3
import sys
import os
import re

# Se placer dans le dossier du script (pour chemins relatifs stables)
script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

UPLOAD_DIR = "../../uploads"

def parse_query_string(qs):
    # parse ultra simple: key=value&key2=value2 (sans dépendances)
    out = {}
    if not qs:
        return out
    for pair in qs.split("&"):
        if "=" in pair:
            k, v = pair.split("=", 1)
            out[k] = v
        else:
            out[pair] = ""
    return out

def url_decode(s):
    # decode minimal %XX + +
    if s is None:
        return ""
    s = s.replace("+", " ")
    def repl(m):
        try:
            return bytes([int(m.group(1), 16)]).decode("utf-8", errors="ignore")
        except:
            return ""
    return re.sub(r"%([0-9A-Fa-f]{2})", repl, s)

def handle_delete():
    qs = os.environ.get("QUERY_STRING", "")
    params = parse_query_string(qs)
    raw = url_decode(params.get("file", ""))

    fname = safe_filename(raw)
    if not fname:
        return ("error", "Nom de fichier invalide 🦥")

    target = os.path.join(UPLOAD_DIR, fname)

    # sécurité simple : refuser si ça sort du dossier upload
    # (safe_filename + join = déjà très safe, mais on double-check)
    base = os.path.abspath(UPLOAD_DIR)
    t_abs = os.path.abspath(target)
    if not t_abs.startswith(base + os.sep) and t_abs != base:
        return ("error", "Chemin interdit 🦥🚫")

    if not os.path.exists(target):
        return ("warn", f"Fichier introuvable : {fname} 🦥")

    try:
        os.remove(target)
        return ("ok", f"Supprimé : {fname} 🦥🗑")
    except:
        return ("error", f"Impossible de supprimer : {fname}")

def safe_filename(filename):
    if not filename:
        return None
    filename = os.path.basename(filename)
    return re.sub(r'[^a-zA-Z0-9._-]', '', filename)

def send_chunk(data):
    """Envoie un chunk (Transfer-Encoding: chunked)."""
    if not data:
        return
    if isinstance(data, str):
        data = data.encode('utf-8')

    size_hex = hex(len(data))[2:].upper().encode('ascii')
    sys.stdout.buffer.write(size_hex + b"\r\n")
    sys.stdout.buffer.write(data + b"\r\n")
    sys.stdout.buffer.flush()

def handle_upload():
    """Parse multipart/form-data et sauvegarde les fichiers dans UPLOAD_DIR."""
    content_type = os.environ.get('CONTENT_TYPE', '')
    try:
        content_length = int(os.environ.get('CONTENT_LENGTH', 0))
        body = sys.stdin.buffer.read(content_length)
    except:
        return ("error", "Impossible de lire le corps de la requête.")

    if not body:
        return ("error", "Le corps de la requête est vide 🦥")

    match = re.search(r'boundary=(.+)', content_type)
    if not match:
        return ("error", "Pas de boundary détectée (multipart/form-data requis).")

    boundary = b"--" + match.group(1).encode().strip()
    parts = body.split(boundary)

    # Assure que le dossier d’upload existe (utile en éval)
    try:
        os.makedirs(UPLOAD_DIR, exist_ok=True)
    except:
        return ("error", "Impossible de créer le dossier d’upload.")

    saved = []
    for part in parts:
        if b'filename="' in part:
            try:
                headers, content = part.split(b'\r\n\r\n', 1)
            except ValueError:
                continue

            m = re.search(r'filename="([^"]+)"', headers.decode(errors='ignore'))
            if not m:
                continue

            fname = safe_filename(m.group(1))
            if not fname:
                continue

            # Retire le CRLF final si présent
            if content.endswith(b'\r\n'):
                content = content[:-2]

            try:
                with open(os.path.join(UPLOAD_DIR, fname), 'wb') as f:
                    f.write(content)
                saved.append(fname)
            except:
                return ("error", f"Erreur lors de l’écriture du fichier : {fname}")

    if saved:
        return ("ok", "Upload réussi 🦥💗 : " + ", ".join(saved))
    return ("warn", "Aucun fichier détecté dans le multipart 🦥")

def list_uploaded_files():
    """Retourne une liste triée des fichiers dans UPLOAD_DIR (sans dossiers)."""
    try:
        if not os.path.isdir(UPLOAD_DIR):
            return []
        files = []
        for name in os.listdir(UPLOAD_DIR):
            p = os.path.join(UPLOAD_DIR, name)
            if os.path.isfile(p):
                files.append(name)
        files.sort(key=str.lower)
        return files
    except:
        return []

def escape_html(s):
    # petit escape pour éviter d'afficher des trucs bizarres
    return (s.replace("&", "&amp;")
             .replace("<", "&lt;")
             .replace(">", "&gt;")
             .replace('"', "&quot;")
             .replace("'", "&#39;"))

if __name__ == "__main__":
    method = os.environ.get('REQUEST_METHOD', 'GET')

    # --- HEADERS ---
    # Option A (souvent CGI standard) :
    # sys.stdout.buffer.write(b"Status: 200 OK\r\n")

    # Option B (si ton webserv attend une status line HTTP brute depuis le CGI) :
    sys.stdout.buffer.write(b"HTTP/1.1 200 OK\r\n")

    sys.stdout.buffer.write(b"Content-Type: text/html; charset=utf-8\r\n")
    sys.stdout.buffer.write(b"Transfer-Encoding: chunked\r\n")
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.flush()

    status_tone = "info"
    status_msg = "Choisis un fichier et envoie-le au serveur 🦥"
    if method == "POST":
        status_tone, status_msg = handle_upload()
    elif method == "DELETE":
        status_tone, status_msg = handle_delete()

    badge_bg = {
        "ok":   "#E7FFF2",
        "warn": "#FFF4E5",
        "error":"#FFE7F0",
        "info": "#FFF0F8"
    }.get(status_tone, "#FFF0F8")

    badge_border = {
        "ok":   "#7BE3B1",
        "warn": "#FFB86B",
        "error":"#FF7BB0",
        "info": "#FF9ECF"
    }.get(status_tone, "#FF9ECF")

    badge_text = {
        "ok":   "#0F6B3E",
        "warn": "#8A4A00",
        "error":"#8A003C",
        "info": "#6A0036"
    }.get(status_tone, "#6A0036")

    html_start = f"""<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Sloth CGI Upload</title>
  <style>
    body {{
      margin: 0;
      font-family: "Segoe UI", sans-serif;
      background: linear-gradient(135deg, #ff9ecf, #ffcce6);
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      color: #4a0033;
      padding: 24px;
      box-sizing: border-box;
    }}
    .card {{
      background: #fff;
      width: 100%;
      max-width: 520px;
      border-radius: 26px;
      box-shadow: 0 18px 50px rgba(0,0,0,0.15);
      padding: 34px 30px;
      text-align: center;
    }}
    .uploads-box {{
        margin-top: 18px;
        padding: 14px 14px;
        border-radius: 18px;
        background: #fff0f8;
        border: 1px solid #ff9ecf;
        text-align: left;
    }}
    .file-pill {{
        background: #ff66b3;
        color: #fff;
        padding: 6px 10px;
        border-radius: 999px;
        font-size: 13px;
        font-weight: 600;
        cursor: pointer;
        user-select: none;
        transition: transform 0.05s ease-in-out, background 0.2s ease;
    }}
    .file-pill:hover 
    {{ background: #e0559c; }}
        
    .file-pill:active 
    {{ transform: scale(0.98); }}

    .file-pill small {{
    opacity: 0.9;
    font-weight: 700;
    margin-left: 6px;
    }}
    .uploads-title {{
    font-weight: 700;
    margin-bottom: 10px;
    color: #6A0036;
    }}
    .uploads-grid {{
    display: flex;
    flex-wrap: wrap;
    gap: 8px;
    }}
    .file-pill {{
    background: #ff66b3;
    color: #fff;
    padding: 6px 10px;
    border-radius: 999px;
    font-size: 13px;
    font-weight: 600;
    max-width: 100%;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    }}
    .uploads-empty {{
    color: #6A0036;
    opacity: 0.85;
    font-weight: 600;
    }}
    .uploads-hint {{
    margin-top: 10px;
    font-size: 12px;
    opacity: 0.8;
    }}
    .emoji {{
      font-size: 64px;
      margin-bottom: 10px;
    }}
    h1 {{
      margin: 0 0 6px 0;
      font-size: 28px;
      letter-spacing: 0.2px;
    }}
    .subtitle {{
      margin: 0 0 18px 0;
      opacity: 0.9;
    }}
    .badge {{
      margin: 14px auto 22px auto;
      padding: 12px 14px;
      border-radius: 16px;
      background: {badge_bg};
      border: 1px solid {badge_border};
      color: {badge_text};
      font-weight: 600;
      line-height: 1.25;
    }}
    form {{
      margin-top: 10px;
    }}
    input[type="file"] {{
      width: 100%;
      box-sizing: border-box;
      padding: 10px;
      border-radius: 16px;
      border: 1px solid #ff99cc;
      background: #fff7fb;
    }}
    .btn {{
      margin-top: 16px;
      background: #ff66b3;
      border: none;
      padding: 12px 22px;
      border-radius: 20px;
      color: white;
      font-size: 16px;
      cursor: pointer;
      transition: transform 0.05s ease-in-out, background 0.2s ease;
    }}
    .btn:hover {{
      background: #e0559c;
    }}
    .btn:active {{
      transform: scale(0.98);
    }}
    .hint {{
      margin-top: 18px;
      font-size: 13px;
      opacity: 0.85;
    }}
    code {{
      background: #fff0f8;
      padding: 2px 6px;
      border-radius: 8px;
    }}
  </style>
</head>
<body>
  <div class="card">
    <div class="emoji">🦥💗</div>
    <h1>Sloth CGI Uploader</h1>
    <p class="subtitle">Interface CGI simple, rose et sleepy.</p>
    <div class="badge">{status_msg}</div>
"""
    send_chunk(html_start)

    form_html = """
        <form action="/cgi-bin/index.py" method="POST" enctype="multipart/form-data">
        <input type="file" name="file" required>
        <button class="btn" type="submit">Upload 🦥</button>
        </form>
        <div class="hint">
        Tip: si tu testes au <code>curl</code>, pense à envoyer du multipart/form-data.
        </div>
    """
    send_chunk(form_html)
    files = list_uploaded_files()

    if files:
        files_html = """
    <div class="uploads-box">
      <div class="uploads-title">Uploaded files</div>
      <div class="uploads-grid">
    """
        for f in files:
            files_html += f'        <div class="file-pill" data-name="{escape_html(f)}">{escape_html(f)} <small>✕</small></div>\n'
        files_html += """      </div>
      <div class="uploads-hint">Tip: you can delete via DELETE /uploads/&lt;filename&gt;</div>
    </div>
    """
    else:
        files_html = """
    <div class="uploads-box">
      <div class="uploads-title">Uploaded files</div>
      <div class="uploads-empty">No files yet 🦥</div>
    </div>
    """

    send_chunk(files_html)

    script_html = """
    <script>
    (function () {
    const result = document.getElementById("result") || null;

    async function deleteFile(name) {
        if (!confirm("Delete " + name + " ?")) return;

        try {
        const res = await fetch("/cgi-bin/index.py?file=" + encodeURIComponent(name), {
            method: "DELETE"
        });

        if (res.ok) {
            location.reload();
        } else {
            alert("Delete failed: HTTP " + res.status);
        }
        } catch (e) {
        alert("Server error 🦥");
        }
    }

    document.querySelectorAll(".file-pill").forEach(pill => {
        pill.addEventListener("click", () => {
        const name = pill.getAttribute("data-name");
        if (name) deleteFile(name);
        });
    });
    })();
    </script>
    """
    send_chunk(script_html)

    # fin chunked
    sys.stdout.buffer.write(b"0\r\n\r\n")
    sys.stdout.buffer.flush()

