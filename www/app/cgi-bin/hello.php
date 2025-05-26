<?php
http_response_code(404);
phpinfo();
 echo '<pre>'.print_r($_SERVER, TRUE).'</pre>';
 echo "method: ";
 echo $_SERVER['REQUEST_METHOD'];
 echo "protocol: ";
 echo $_SERVER['SERVER_PROTOCOL'];
 echo "data:";
 echo $_GET['data'];
 echo $_SERVER['REQUEST_METHOD'];
echo "----------------------<br />";
$data = file_get_contents('php://input');
 if ($_SERVER['REQUEST_METHOD'] == "POST")
 {
	 echo "addr". $_SERVER['REMOTE_ADDR'];
	 echo "port:". $_SERVER['REMOTE_PORT'];
 	echo "name: ";
 	echo $_POST["name"];
 	echo "foo: ";
 	echo $_POST["foo"];
 	echo "\n";
 }
echo "----------------------<br />";
?>
