function sendDelete(filename) {
    if (!filename) return;

    fetch("/uploads/" + encodeURIComponent(filename), {
        method: "DELETE"
    })
    .then(response => {

        if (response.status === 404) {
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
        if (html) {
            document.body.innerHTML = html;
        }
    });
}

function deleteFromInput() {
    const filename = document.getElementById("filename").value.trim();
    sendDelete(filename);
}