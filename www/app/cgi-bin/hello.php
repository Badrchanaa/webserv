<?php
//http_response_code(404);
//phpinfo();
echo '<pre>'.print_r($_SERVER, TRUE).'</pre>';
echo "method: ";
echo $_SERVER['REQUEST_METHOD'];
echo "protocol: ";
echo $_SERVER['SERVER_PROTOCOL'];
echo "data:";
echo $_GET['data'];
if ($_SERVER['REQUEST_METHOD'] == "POST")
{
	echo "REQUEST JSON:";
	echo file_get_contents('php://input');
}
?>
