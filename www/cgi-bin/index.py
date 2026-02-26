#!/usr/bin/env python3
import sys
import os
import re

script_dir = os.path.dirname(os.path.abspath(__file__))
os.chdir(script_dir)

UPLOAD_DIR = "../../uploads"


def parse_query_string(qs):
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
    if s is None:
        return ""
    s = s.replace("+", " ")
    def repl(m):
        try:
            return bytes([int(m.group(1), 16)]).decode("utf-8", errors="ignore")
        except:
            return ""
    return re.sub(r"%([0-9A-Fa-f]{2})", repl, s)


def safe_filename(filename):
    if not filename:
        return None
    filename = os.path.basename(filename)
    return re.sub(r'[^a-zA-Z0-9._-]', '', filename)


def handle_delete():
    qs = os.environ.get("QUERY_STRING", "")
    params = parse_query_string(qs)
    raw = url_decode(params.get("file", ""))

    fname = safe_filename(raw)
    if not fname:
        return ("error", "Nom de fichier invalide 🦥")

    target = os.path.join(UPLOAD_DIR, fname)
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


def handle_upload():
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

    try:
        os.makedirs(UPLOAD_DIR, exist_ok=True)
    except:
        return ("error", "Impossible de créer le dossier d'upload.")

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

            if content.endswith(b'\r\n'):
                content = content[:-2]

            try:
                with open(os.path.join(UPLOAD_DIR, fname), 'wb') as f:
                    f.write(content)
                saved.append(fname)
            except:
                return ("error", f"Erreur lors de l'écriture du fichier : {fname}")

    if saved:
        return ("ok", "Upload réussi 🦥💗 : " + ", ".join(saved))
    return ("warn", "Aucun fichier détecté dans le multipart 🦥")


def list_uploaded_files():
    try:
        if not os.path.isdir(UPLOAD_DIR):
            return []
        files = [name for name in os.listdir(UPLOAD_DIR) if os.path.isfile(os.path.join(UPLOAD_DIR, name))]
        files.sort(key=str.lower)
        return files
    except:
        return []


def escape_html(s):
    return (s.replace("&", "&amp;")
             .replace("<", "&lt;")
             .replace(">", "&gt;")
             .replace('"', "&quot;")
             .replace("'", "&#39;"))


if __name__ == "__main__":
    method = os.environ.get('REQUEST_METHOD', 'GET')

    # Headers CGI
    print("Content-Type: text/html; charset=utf-8\r")
    print("\r")

    status_tone = "info"
    status_msg = "Choisis un fichier et envoie-le au serveur 🦥"

    if method == "POST":
        status_tone, status_msg = handle_upload()
    elif method == "DELETE":
        status_tone, status_msg = handle_delete()

    # HTML head
    print(f"""<!DOCTYPE html>
<html lang="fr">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Sloth CGI Upload</title>
  <link rel="stylesheet" href="/cgi-bin/style.css">
</head>
<body>
  <div class="card">
    <div class="emoji">🦥💗</div>
    <h1>Sloth CGI Uploader</h1>
    <p class="subtitle">Interface CGI simple, rose et sleepy.</p>
    <div class="badge badge-{status_tone}">{status_msg}</div>""")

    # Form
    print("""    <form action="/cgi-bin/index.py" method="POST" enctype="multipart/form-data">
      <input type="file" name="file" required>
      <button class="btn" type="submit">Upload 🦥</button>
    </form>
    <div class="hint">Tip: si tu testes au <code>curl</code>, pense à envoyer du multipart/form-data.</div>""")

    # Files list
    files = list_uploaded_files()
    print('    <div class="uploads-box">')
    print('      <div class="uploads-title">Uploaded files</div>')

    if files:
        print('      <div class="uploads-grid">')
        for f in files:
            print(f'        <div class="file-pill" data-name="{escape_html(f)}">{escape_html(f)} <small>✕</small></div>')
        print('      </div>')
        print('      <div class="uploads-hint">Tip: you can delete via DELETE /uploads/&lt;filename&gt;</div>')
    else:
        print('      <div class="uploads-empty">No files yet 🦥</div>')

    print('    </div>')

    # Script
    print("""    <script>
    (function () {
      async function deleteFile(name) {
        if (!confirm("Delete " + name + " ?")) return;
        try {
          const res = await fetch("/cgi-bin/index.py?file=" + encodeURIComponent(name), { method: "DELETE" });
          if (res.ok) location.reload();
          else alert("Delete failed: HTTP " + res.status);
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
  </div>
</body>
</html>""")