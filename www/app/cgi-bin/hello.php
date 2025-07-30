<?php
// http_response_code(404);
// phpinfo();
//  echo '<pre>'.print_r($_SERVER, TRUE).'</pre>';
//  echo "method: ";
//  echo $_SERVER['REQUEST_METHOD'];
//  echo "protocol: ";
//  echo $_SERVER['SERVER_PROTOCOL'];
//  echo "data:";
//  echo $_GET['data'];
//  echo $_SERVER['REQUEST_METHOD'];
session_start();

// $cookie_name = "user";
// $cookie_value = "John Doe";
// setcookie($cookie_name, $cookie_value, time() + (86400 * 30), "/app/cgi-bin/"); // 86400 = 1 day

// if(!isset($_COOKIE[$cookie_name])) {
//   echo "Cookie named '" . $cookie_name . "' is not set!";
// } else {
//   echo "Cookie '" . $cookie_name . "' is set!<br>";
//   echo "Value is: " . $_COOKIE[$cookie_name];
// }


wefowe;

if (isset($_GET["set-session"]))
{
	$_SESSION["favcolor"] = "green";
	$_SESSION["favanimal"] = "cat";
	echo "session is set<br/>";
}
else
{
	echo "SET-SESSION IS NOT THERE<br/>";
	if (isset($_SESSION["favcolor"]))
	{
		echo "Favorite color is " . $_SESSION["favcolor"] . ".<br>";
	}
	if (isset($_SESSION["favanimal"]))
	{
		echo "Favorite animal is " . $_SESSION["favanimal"] . ".";
	}
	echo "<br/>";
}

// echo "----------------------<br />";
// $data = file_get_contents('php://input');
// echo "data: <br/>";
// echo $data;
echo "=======<br/>";
 if ($_SERVER['REQUEST_METHOD'] == "POST")
 {
	if (isset($_SERVER['REMOTE_ADDR']))
	 	echo "addr". $_SERVER['REMOTE_ADDR'];
	if (isset($_SERVER['REMOTE_PORT']))
		 echo "port:". $_SERVER['REMOTE_PORT'];
	 echo "<br />";
 	echo '<pre>'.print_r($_POST, TRUE).'</pre>';
 }
echo "----------------------<br />";
?>
