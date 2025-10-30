<?php
// Abilita CORS per qualsiasi origine (*)
header("Access-Control-Allow-Origin: *");

// Permetti solo i metodi POST e OPTIONS
header("Access-Control-Allow-Methods: POST, OPTIONS");

// Permetti determinati header
header("Access-Control-Allow-Headers: Content-Type, Authorization");

// Se la richiesta è di tipo OPTIONS (pre-flight), termina subito
if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(200);
    exit;
}

// Imposta il tipo di contenuto come JSON
header("Content-Type: application/json");

// Configurazione del database
$servername = "localhost";
$username = "root";
$password = "";
$dbname = "gestione_impronte";

// Connessione al database
$conn = new mysqli($servername, $username, $password, $dbname);

if ($conn->connect_error) {
    die(json_encode(["status" => "error", "message" => "Connection failed: " . $conn->connect_error]));
}

// Recupera i dati inviati
$nome = $_POST['Nome'] ?? null;
$cognome = $_POST['Cognome'] ?? null;
$stanza = $_POST['Stanza'] ?? null;
$id_template = $_POST['ID_Template'] ?? null;
$data_ultimo_accesso = date("Y-m-d H:i:s");

// Controlla se i dati sono validi
if (!$nome || !$cognome || !$stanza || !$id_template) {
    echo json_encode(["status" => "error", "message" => "Dati mancanti"]);
    exit;
}

// Protezione contro SQL Injection
$nome = $conn->real_escape_string($nome);
$cognome = $conn->real_escape_string($cognome);
$stanza = $conn->real_escape_string($stanza);
$id_template = $conn->real_escape_string($id_template);

// Inserimento dei dati nel database
$sql = "INSERT INTO studenti (Nome, Cognome, Stanza, ID_Template, DataUltimoAccesso)
        VALUES ('$nome', '$cognome', '$stanza', '$id_template', '$data_ultimo_accesso')";

if ($conn->query($sql) === TRUE) {
    echo json_encode(["status" => "success", "message" => "Utente aggiunto correttamente"]);
} else {
    echo json_encode(["status" => "error", "message" => "Errore nell'inserimento: " . $conn->error]);
}

$conn->close();
?>

