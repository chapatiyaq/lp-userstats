<?php

class UserStats {
	public $totalSizediff;
	public $contribsCount;
	public $data;

	public function __construct() {
		$this->totalSizediff = 0;
		$this->contribsCount = 0;
		$this->data = array();
	}

	function getContributions($curl, $wiki, $name, $continue) {
		$url = 'http://wiki.teamliquid.net/' . $wiki . '/api.php?action=query&list=usercontribs';
		$url .= '&ucuser=' . str_replace(' ', '%20', htmlspecialchars($name));
		$url .= ($continue ? '&ucstart=' . $continue : '');
		$url .= '&ucdir=newer&ucprop=title|timestamp|flags|sizediff|size&uclimit=500&format=json';

		curl_setopt($curl, CURLOPT_URL, $url);
		$json = curl_exec($curl);
		$jsonData = json_decode($json);

		$contribs = $jsonData->query->usercontribs;
		$i = 0;
		foreach ($contribs as $contrib) {
			$title = $contrib->title;
			$sizediff = isset($contrib->sizediff) ? $contrib->sizediff : (isset($contrib->size) ? $contrib->size : 0);
			$this->data[] = array(
				'timestamp' => $contrib->timestamp,
				'title' => $title,
				'sizediff' => $sizediff,
				'minor' => isset($contrib->minor),
				'new' => isset($contrib->new)
			);
			$this->contribsCount++;
			$this->totalSizediff += abs($sizediff);

			//echo "title: " . htmlspecialchars($title) . ", sizediff: $sizediff<br/>";
			++$i;
		}

		return isset($jsonData->{'query-continue'}) ? $jsonData->{'query-continue'}->usercontribs->ucstart : 0;
	}

	public function getNamespaces($wiki) {
		$namespaces = array(
			'Talk',
			'User',
			'User talk',
			'Liquipedia',
			'Liquipedia talk',
			'File',
			'File talk',
			'MediaWiki',
			'MediaWiki talk',
			'Template',
			'Template talk',
			'Help',
			'Help talk',
			'Category',
			'Category talk'
		);
		if ($wiki == 'starcraft2') {
			$namespaces[] = 'Starbow';
			$namespaces[] = 'Starbow talk';
		}
		$namespaces[] = 'Portal';
		return $namespaces;
	}
}