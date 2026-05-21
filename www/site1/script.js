function sendDelete(filename) {
    if (!filename) return;

    fetch("/upload/" + encodeURIComponent(filename), {
        method: "DELETE"
    })
    .then(response => {

        if (response.status === 404)
        {
            document.getElementById("result").innerHTML =
                "<p style='color:red;'>File not found ❌</p>";
            return null;
        }
        else if (response.status === 204)
        {
            document.getElementById("result").innerHTML =
                "<p style='color:green;'>File delete successfully ✅</p>";
            return null;
        }
        return response.text();
    })
    .then(html => {
        if (html)
        {
            document.body.innerHTML = html;
        }
    });
}

function sendUpload(file)
{
    const formData = new FormData();
    formData.append("file", file);

    fetch("/upload", {
        method: "POST",
        body: formData
    })
    .then(response => {
        if (response.status === 200)
        {
            document.getElementById("result").innerHTML =
                "<p style='color:green;'>Upload done successfully ✅</p>";
            return null;
        }
        return response.text();
    })
}

function uploadFromInput() {
    const file = document.getElementById("fileInput").files[0];
    if (!file) return;
    sendUpload(file);  // ← passe file
}

function deleteFromInput() {
    const filename = document.getElementById("filename").value.trim();
    if (!file) return;
    sendDelete(filename);
}
