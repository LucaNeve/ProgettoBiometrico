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

// Recupera l'ID_Template inviato
$id_template = $_POST['ID_Template'];

if (empty($id_template)) {
    echo json_encode(["status" => "error", "message" => "ID_Template non fornito"]);
    $conn->close();
    exit();
}

// Query per eliminare l'utente dal database
$sql = "DELETE FROM studenti WHERE ID_Template = '$id_template'";

if ($conn->query($sql) === TRUE) {
    echo json_encode(["status" => "success", "message" => "Utente eliminato correttamente"]);
} else {
    echo json_encode(["status" => "error", "message" => "Errore nell'eliminazione: " . $conn->error]);
}

$conn->close();
?>
