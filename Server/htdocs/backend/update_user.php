<?php
header("Content-Type: application/json");

$host = "localhost";
$user = "root";
$password = "";
$database = "gestione_impronte";

$conn = new mysqli($host, $user, $password, $database);

if ($conn->connect_error) {
    die(json_encode(["error" => "Connessione fallita: " . $conn->connect_error]));
}

// Leggi i dati inviati da Qt
$data = json_decode(file_get_contents("php://input"), true);

if (!isset($data["ID_Template"], $data["Campo"], $data["Valore"])) {
    die(json_encode(["error" => "Dati mancanti"]));
}

$id = $conn->real_escape_string($data["ID_Template"]);
$campo = $conn->real_escape_string($data["Campo"]);
$valore = $conn->real_escape_string($data["Valore"]);

// Assicuriamoci che il campo modificabile sia tra quelli accettati
$campi_validi = ["Nome", "Cognome", "Stanza", "DataUltimoAccesso"];
if (!in_array($campo, $campi_validi)) {
    die(json_encode(["error" => "Campo non valido"]));
}

// Esegui la query di aggiornamento
$sql = "UPDATE studenti SET $campo = '$valore' WHERE ID_Template = '$id'";

if ($conn->query($sql) === TRUE) {
    echo json_encode(["success" => "Dati aggiornati con successo", "query" => $sql]);
} else {
    echo json_encode(["error" => "Errore nell'aggiornamento: " . $conn->error, "query" => $sql]);
}

$conn->close();
?>
