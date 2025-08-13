<?php

session_start();



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
