<?php
// test_post.php
header("Content-Type: text/html");

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Read raw POST data
    $raw_input = file_get_contents('php://input');
    $post_data = $_POST;

    echo "<h1>POST Request Received</h1>";
    echo "<h2>Raw Input:</h2>";
    echo "<pre>" . htmlentities($raw_input) . "</pre>";
    
    echo "<h2>Parsed POST Data:</h2>";
    if (!empty($post_data)) {
        echo "<ul>";
        foreach ($post_data as $key => $value) {
            echo "<li><strong>" . htmlentities($key) . ":</strong> " . htmlentities($value) . "</li>";
        }
        echo "</ul>";
    } else {
        echo "<p>No POST data received.</p>";
    }

    echo "<h2>Server Environment:</h2>";
    echo "<pre>";
    echo "REQUEST_METHOD: " . $_SERVER['REQUEST_METHOD'] . "\n";
    echo "CONTENT_TYPE: " . ($_SERVER['CONTENT_TYPE'] ?? 'Not set') . "\n";
    echo "CONTENT_LENGTH: " . ($_SERVER['CONTENT_LENGTH'] ?? 'Not set') . "\n";
    echo "</pre>";
} else {
    echo "<h1>Error: This endpoint only accepts POST requests.</h1>";
}
?>
