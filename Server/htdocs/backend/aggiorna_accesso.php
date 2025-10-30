<?php
// Configurazione database
$servername = "localhost";
$username = "root"; // Cambia con il tuo username del database
$password = ""; // Cambia con la tua password del database
$dbname = "gestione_impronte"; // Nome del database

// Connetti al database
$conn = new mysqli($servername, $username, $password, $dbname);

// Controlla la connessione
if ($conn->connect_error) {
    http_response_code(500); // Errore interno del server
    die("Connessione fallita: " . $conn->connect_error);
}

// Ricevi i dati dal payload JSON
$data = json_decode(file_get_contents("php://input"), true);

// Verifica se i dati sono stati inviati correttamente
if (!isset($data['id_template'])) {
    http_response_code(400); // Bad Request
    die("ID_Template non fornito.");
}

$id_template = $data['id_template'];
$data_accesso = date('Y-m-d H:i:s'); // Data e ora attuali

// Aggiorna la DataUltimoAccesso per il record corrispondente a ID_Template
$sql = "UPDATE studenti SET DataUltimoAccesso = '$data_accesso' WHERE ID_Template = $id_template";

if ($conn->query($sql) === TRUE) {
    http_response_code(200); // OK
    echo "Accesso aggiornato con successo.";
} else {
    http_response_code(500); // Errore interno del server
    echo "Errore durante l'aggiornamento: " . $conn->error;
}

$conn->close();
?>



