<?php
header("Content-Type: application/json");

$host = "localhost";
$user = "root";
$password = "";
$database = "gestione_impronte";

// Connessione al database
$conn = new mysqli($host, $user, $password, $database);

if ($conn->connect_error) {
    die(json_encode(["error" => "Connessione fallita: " . $conn->connect_error]));
}

// Query per ottenere tutti gli utenti
$sql = "SELECT ID_Template, Nome, Cognome, Stanza, DataUltimoAccesso FROM studenti";
$result = $conn->query($sql);

$utenti = [];
while ($row = $result->fetch_assoc()) {
    $utenti[] = $row;
}

echo json_encode($utenti);

$conn->close();
?>
