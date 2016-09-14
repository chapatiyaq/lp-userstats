<?php
	function microtime_float()
	{
		list($usec, $sec) = explode(" ", microtime());
		return ((float)$usec + (float)$sec);
	}

	$action = isset($_GET['action']) ? $_GET['action'] : NULL;
	$debug = isset($_GET['debug']) ? ($_GET['debug'] == 'true') : false;

	if ($debug) {
		$time_start = microtime_float();
	}

	if ($action == 'statsperwiki') {
		include_once('api_statsperwiki.inc');
		if ($debug) {
			$result['time'] = microtime_float() - $time_start;
		}
		echo json_encode($result);
	} else {
		echo 'API action missing or not recognized!';
	}
?>