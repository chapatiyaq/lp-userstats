<?php

$wikis = array(
	'Brood War' => 'starcraft',
	'StarCraft II' => 'starcraft2',
	'Dota 2' => 'dota2',
	'Hearthstone' => 'hearthstone',
	'Heroes' => 'heroes',
	'Smash Bros' => 'smash',
	'Counter-Strike' => 'counterstrike',
	'Overwatch' => 'overwatch',
	'Commons' => 'commons',
	'Warcraft' => 'warcraft',
	'Fighting Games' => 'fighters',
	'Rocket League' => 'rocketleague'
);

class User {
	public $userId;
	public $name;
	public $groups;
	public $registration;
	public $editcount;
	public $gender;
	public $found;

	public function __construct() {
		$this->userId = 0;
		$this->name = '';
		$this->groups = array();
		$this->registration = 0;
		$this->editcount = '';
		$this->gender = '';
		$this->found = false;
	}
}

function time_elapsed($secs) {
	$bit = array(
		'y' => $secs / 31556926 % 12,
		'w' => $secs / 604800 % 52,
		'd' => $secs / 86400 % 7,
		'h' => $secs / 3600 % 24,
		'm' => $secs / 60 % 60,
		's' => $secs % 60
	);
		
	foreach($bit as $k => $v)
		if($v > 0)$ret[] = $v . $k;
		
	return join(' ', $ret);
}

function dataCmp($a, $b) {
	if ($a['timestamp'] == $b['timestamp']) {
        return 0;
    }
    return ($a['timestamp'] < $b['timestamp']) ? -1 : 1;
}

function wikilink($wiki, $title) {
	return '<a href="http://wiki.teamliquid.net/' . $wiki . '/' . str_replace(' ', '_', $title) . '">' . htmlspecialchars($title) . '</a>';
}

function getUserInfo($curl, $url, &$userInfo) {
	curl_setopt($curl, CURLOPT_URL, $url);
	$phpResult = curl_exec($curl);
	$phpData = unserialize($phpResult);

	if (isset($phpData['query'])) {
		$user = $phpData['query']['users'][0];
		if (!isset($user['missing'])) {
			$userInfo->name = $user['name'];
			$userInfo->userId = $user['userid'];
			$userInfo->registration = strtotime($user['registration']);
			$userInfo->editcount = $user['editcount'];
			$userInfo->gender = ucfirst($user['gender']);
			$userInfo->groups = $user['groups'];
			$userInfo->found = true;
			return 1;
		}
		return 0;
	}

	return -1;
}

function getContributions($curl, $url, $wiki, &$stats) {
	global $namespaces;
	curl_setopt($curl, CURLOPT_URL, $url);
	$phpResult = curl_exec($curl);
	$phpData = unserialize($phpResult);

	$contribs = $phpData['query']['usercontribs'];
	foreach ($contribs as $contrib) {
		if ($contrib['userid'] != 0) {
			$title = $contrib['title'];
			$sizediff = isset($contrib['sizediff']) ? $contrib['sizediff'] : (isset($contrib['size']) ? $contrib['size'] : 0);
			$stats->data[] = array(
				'timestamp' => $contrib['timestamp'],
				'title' => $title,
				'sizediff' => $sizediff,
				'minor' => isset($contrib['minor']),
				'new' => isset($contrib['new']),
				'wiki' => $wiki
			);
			$stats->contribsCount++;
			$stats->totalSizediff += abs($sizediff);
			//echo "title: " . htmlspecialchars($title) . ", sizediff: $sizediff<br/>";
		}
	}

	return isset($phpData['continue']) ? $phpData['continue'] : 0;
}

function getUserInfoOnAllWikis($curl, $user, &$userInfos) {
	global $wikis;

	$userFound = false;
	foreach ($wikis as $wiki) {
		$url = 'http://wiki.teamliquid.net/' . $wiki . '/api.php?action=query&list=users&ususers='
			. str_replace(' ', '%20', htmlspecialchars($user)) . '&usprop=groups|registration|editcount|gender&format=php';
		$userInfos[$wiki] = new User();
		$userFound |= (getUserInfo($curl, $url, $userInfos[$wiki]) == 1);
	}

	return $userFound ? 1 : 0;
}

function getContribsByWiki($curl, $wiki, $url, &$stats) {

	curl_setopt($curl, CURLOPT_URL, $url);
	$phpResult = curl_exec($curl);
	$phpData = unserialize($phpResult);

	if (isset($phpData['query'])) {
		$contribs = $phpData['query']['usercontribs'];
		$contribsCount = 0;
		foreach ($contribs as $contrib) {
			if ($contrib['userid'] != 0) {
				$contribsCount++;
			}
		}
		$stats->contribsCounts[$wiki] += $contribsCount;
	}

	return isset($phpData['continue']) ? $phpData['continue'] : 0;
}

function getContribs($curl, $request, &$stats) {
	global $wikis;

	foreach ($wikis as $wiki) {
		$stats->contribsCounts[$wiki] = 0;
		$stats->firstEdits[$wiki] = "3000";
		$stats->lastEdits[$wiki] = "1900";

		$url = 'http://wiki.teamliquid.net/' . $wiki . '/api.php'
		       . '?action=query&list=usercontribs&ucuser='
		       . str_replace(' ', '%20', htmlspecialchars($request->user))
		       . '&ucprop=timestamp|sizediff'
		       . '&ucdir=newer'
		       . '&continue='
		       . '&uclimit=500'
		       . '&format=php';

		$continue = getContribsByWiki($curl, $wiki, $url, $stats);

		while( $continue > 0 ) {
			$url = 'http://wiki.teamliquid.net/' . $wiki . '/api.php'
			       . '?action=query&list=usercontribs&ucuser='
			       . str_replace(' ', '%20', htmlspecialchars($request->user))
			       . '&ucdir=newer'
			       . '&ucprop=timestamp|sizediff'
			       . '&uccontinue=' . $continue['uccontinue']
			       . '&continue=' . $continue['continue']
			       . '&uclimit=500'
			       . '&format=php';
			$continue = getContribsByWiki($curl, $wiki, $url, $stats);
		}

		$stats->contribsCount += $stats->contribsCounts[$wiki];

		/*$url = 'http://wiki.teamliquid.net/' . $wiki . '/api.php?action=query&list=usercontribs&ucuser='
			. str_replace(' ', '%20', htmlspecialchars($request->user)) . '&ucdir=newer&ucprop=timestamp&uclimit=1&format=php';
		curl_setopt($curl, CURLOPT_URL, $url);
		$phpResult = curl_exec($curl);
		$phpData = unserialize($phpResult);
		if (isset($phpData['query']) && count($phpData['query']['usercontribs']) > 0) {
			$stats->firstEdits[$wiki] = $phpData['query']['usercontribs'][0]['timestamp'];
		}
		if ($stats->firstEdits[$wiki] < $stats->firstEdit) {
			$stats->firstEdit = $stats->firstEdits[$wiki];
		}

		$url = 'http://wiki.teamliquid.net/' . $wiki . '/api.php?action=query&list=usercontribs&ucuser='
			. str_replace(' ', '%20', htmlspecialchars($request->user)) . '&ucdir=older&ucprop=timestamp&uclimit=1&format=php';
		curl_setopt($curl, CURLOPT_URL, $url);
		$phpResult = curl_exec($curl);
		$phpData = unserialize($phpResult);
		if (isset($phpData['query']) && count($phpData['query']['usercontribs']) > 0) {
			$stats->lastEdits[$wiki] = $phpData['query']['usercontribs'][0]['timestamp'];
		}
		if ($stats->lastEdits[$wiki] > $stats->lastEdit) {
			$stats->lastEdit = $stats->lastEdits[$wiki];
		}*/
	}
}

$user = isset($_GET['user']) ? ucfirst($_GET['user']) : 'ChapatiyaqPTSM';
$curl = curl_init();
curl_setopt_array($curl, array(
	CURLOPT_USERAGENT => 'Mozilla/5.0 (compatible; userstats/1.0; chapatiyaq@gmail.com)',
	CURLOPT_RETURNTRANSFER => 1,
	CURLOPT_ENCODING => '',
	CURLOPT_TIMEOUT => 60
));

if (isset($_GET['wiki'])) {
	if ($_GET['wiki'] == 'all') {
		include_once('all_wiki.inc');
	} else {
		include_once('per_wiki.inc');
	}
} else {
	include_once('overview.inc');
}