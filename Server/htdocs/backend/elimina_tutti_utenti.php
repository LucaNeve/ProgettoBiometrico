<?php
header("Content-Type: application/json");

// Configurazione del database
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "gestione_impronte";

// Connessione al database
$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Connection failed"]));
}

// Query per eliminare tutti gli utenti dal database
$sql = "DELETE FROM studenti";

if ($conn->query($sql) === TRUE) {
    echo json_encode(["status" => "success", "message" => "Tutti gli utenti sono stati eliminati correttamente"]);
} else {
    echo json_encode(["status" => "error", "message" => "Errore nell'eliminazione: " . $conn->error]);
}

$conn->close();
?>
