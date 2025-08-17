<?php
session_start();

// Logout
if (isset($_GET['logout'])) {
    session_destroy();
    if (!empty($_COOKIE)) {
      foreach ($_COOKIE as $name => $value) {
	setcookie($name, '', time() - 3600, '/'); // expire in the past
	unset($_COOKIE[$name]);
      }
      header("Location: test.php");
    }
    exit;
}

// Login
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['login'])) {
    if ($_POST['user'] === 'test' && $_POST['pass'] === 'test') {
        $_SESSION['logged_in'] = true;
        $_SESSION['username'] = $_POST['user'];
    } else {
        $error = "Invalid login";
    }
}

// Set cookies
if ($_SERVER['REQUEST_METHOD'] === 'POST' && isset($_POST['set_cookie'])) {
    if (!empty($_POST['cookie_name'])) {
        setcookie($_POST['cookie_name'], $_POST['cookie_value'], time() + 3600, '/'); // expires in 1 hour
       	$_COOKIE[$_POST['cookie_name']] = $_POST['cookie_value']; // update for immediate display
    }
}
?>
<!DOCTYPE html>
<html>
<head>
    <title>Login & Cookies Test</title>
    <style>
        body { font-family: Arial, sans-serif; background: #f0f0f0; display: flex; justify-content: center; align-items: center; height: 100vh; }
        .container { background: #fff; padding: 20px 30px; border-radius: 8px; box-shadow: 0 0 10px rgba(0,0,0,0.1); text-align: center; width: 300px; }
        input { padding: 8px; margin: 5px 0; width: 100%; box-sizing: border-box; }
        input[type="submit"] { width: auto; background: #007BFF; color: #fff; border: none; cursor: pointer; }
        input[type="submit"]:hover { background: #0056b3; }
        p { margin: 10px 0; }
        .error { color: red; }
        a { color: #007BFF; text-decoration: none; }
        a:hover { text-decoration: underline; }
        table { width: 100%; border-collapse: collapse; margin-top: 10px; }
        td, th { border: 1px solid #ccc; padding: 5px; text-align: left; }
    </style>
</head>
<body>
<div class="container">
<?php if (!empty($error)) echo "<p class='error'>$error</p>"; ?>

<?php if (!empty($_SESSION['logged_in'])): ?>
    <p>Welcome, <strong><?php echo htmlspecialchars($_SESSION['username']); ?></strong>!</p>
    <p><a href="?logout=1">Logout</a></p>

    <!-- Cookie form -->
    <form method="post">
        <input type="text" name="cookie_name" placeholder="Cookie name" required><br>
        <input type="text" name="cookie_value" placeholder="Cookie value" required><br>
        <input type="submit" name="set_cookie" value="Set Cookie">
    </form>

    <!-- Display cookies -->
    <?php if (!empty($_COOKIE)): ?>
        <h4>Current Cookies:</h4>
        <table>
            <tr><th>Name</th><th>Value</th></tr>
            <?php foreach ($_COOKIE as $name => $value): ?>
                <tr><td><?php echo htmlspecialchars($name); ?></td><td><?php echo htmlspecialchars($value); ?></td></tr>
            <?php endforeach; ?>
        </table>
    <?php endif; ?>

<?php else: ?>
    <form method="post">
        <input name="user" placeholder="Username"><br>
        <input type="password" name="pass" placeholder="Password"><br>
        <input type="submit" name="login" value="Login">
    </form>
<?php endif; ?>
</div>
</body>
</html>
