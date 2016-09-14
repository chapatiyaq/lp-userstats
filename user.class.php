<?php

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

	public function getUserInfo($curl, $wiki, $name) {
		$url = 'http://wiki.teamliquid.net/' . $wiki . '/api.php?action=query&list=users&ususers='
			. str_replace(' ', '%20', htmlspecialchars($name)) . '&usprop=groups|registration|editcount|gender&format=json';

		curl_setopt($curl, CURLOPT_URL, $url);
		$json = curl_exec($curl);
		$jsonData = json_decode($json);

		if (isset($jsonData->query)) {
			$user = $jsonData->query->users[0];
			if (!isset($user->missing)) {
				$this->name = $user->name;
				$this->userId = $user->userid;
				$this->registration = strtotime($user->registration);
				$this->editcount = $user->editcount;
				$this->gender = ucfirst($user->gender);
				$this->groups = $user->groups;
				$this->found = true;
				return 1;
			}
			return 0;
		}

		return -1;
	}
}