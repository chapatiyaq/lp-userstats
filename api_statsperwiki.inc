<?php
require_once('user.class.php');
require_once('userstats.class.php');

$user = isset($_GET['user']) ? ucfirst($_GET['user']) : NULL;
$wiki = isset($_GET['wiki']) ? $_GET['wiki'] : NULL;
$no_details = isset($_GET['no_details']) ? ($_GET['no_details'] == 'true') : false;

class ApiStatsPerWiki {
	public $result;

	public function __construct() {
		$this->result = array();
	}

	public function getStats($user, $wiki, $no_details) {
		if (!$user || !$wiki) {
			$this->result['error'] = 'User or wiki missing';
		} else {
			$this->result['input'] = array( 'wiki' => $wiki, 'user' => $user );

			$userInfo = new User();
			$stats = new UserStats();

			$curl = curl_init();
			curl_setopt_array($curl, array(
				CURLOPT_USERAGENT => 'Mozilla/5.0 (compatible; userstats/1.0; chapatiyaq@gmail.com)',
				CURLOPT_RETURNTRANSFER => 1,
				CURLOPT_ENCODING => '',
				CURLOPT_TIMEOUT => 60
			));
			
			$userFound = $userInfo->getUserInfo($curl, $wiki, $user);

			if ($userFound > 0) {
				$continue = $stats->getContributions($curl, $wiki, $user, false);
				while ($continue > 0) {
					$continue = $stats->getContributions($curl, $wiki, $user, $continue);
					usleep(500000);
				}

				curl_close($curl);

				if (!$no_details) {
					$mean = round($stats->totalSizediff / $stats->contribsCount, 3);
					$earliest = strtotime($stats->data[0]['timestamp']);
					$latest = strtotime($stats->data[count($stats->data) - 1]['timestamp']);
					$days = ($latest - $earliest) / (60 * 60 * 24);

					$namespace = array();
					$namespace['Main'] = 0;
					foreach(UserStats::getNamespaces($wiki) as $ns) {
						$namespace[$ns] = 0;
					}
					$titles = array();
					$createdTemplates = array('flagTemplates' => array(), 'teamTemplates' => array());
					$ownUserSpace = 'User:' . $userInfo->name;
					$ownUserSpaceEdits = 0;
					$minorEdits = 0;
					$filesUploaded = 0;
					$filesUploadedWithoutText = 0;
					$latestUploadedWithoutText = '';
					$teamTemplates = array(
						'Template:Team/',
						'Template:Team2/',
						'Template:TeamShort/',
						'Template:Team2Short/',
						'Template:TeamIcon/',
						'Template:TeamPart/',
						'Template:TeamBracket/',
						'Template:TeamPage/'
					);
					$dumpTexts = array();
					$sizediffCumul = 0;
					$biggestSizeDiffPlus = array('sizediff' => -1);
					$biggestSizeDiffMinus = array('sizediff' => 1);
					$longestBreak = array('duration' => 0, 'start' => $earliest, 'end' => $earliest);
					$previousEditTime = $latest;
					foreach ($stats->data as $index => $data) {
						$title = $data['title'];
						$timestamp = $data['timestamp'];
						$sizediff = $data['sizediff'];
						$minor = $data['minor'];
						$new = $data['new'];

						$found = 0;
						foreach(UserStats::getNamespaces($wiki) as $ns) {
							if (strpos($title, $ns . ':') === 0) {
								$namespace[$ns]++;
								$found = 1;
							}
						}
						if (!$found) $namespace['Main']++;
						if ($title === $ownUserSpace || strpos($title, $ownUserSpace . '/') === 0) {
							$ownUserSpaceEdits++;
						}
						if (!isset($titles[$title])) {
							$titles[$title] = 1;
						} else {
							$titles[$title]++;
						}
						$found = 0;
						if (strpos($title, 'Template:') === 0 && $new) {
							if (strpos($title, 'Template:Flag/') === 0) {
								$createdTemplates['flagTemplates'][] = $title;
								$found = 1;
							} else {
								foreach ($teamTemplates as $tt) {
									if (strpos($title, $tt) === 0) {
										$createdTemplates['teamTemplates'][] = $title;
										$found = 1;
									}
								}
							}
							if (!$found) {
								$createdTemplates[] = $title;
							}
						}
						if (strpos($title, 'File:') === 0 && $new) {
							$filesUploaded++;
							if ($sizediff == 0) {
								$filesUploadedWithoutText++;
								$latestUploadedWithoutText = strtotime($timestamp);
							}
						}
						if ($minor) {
							$minorEdits++;
						}

						$timeStampExcel = str_replace(array('T', 'Z'), array(' ', ''), $timestamp);
						$sizediffCumul += abs($sizediff);
						if ($sizediff >= 0) {
							if ($sizediff > $biggestSizeDiffPlus['sizediff']) {
								$biggestSizeDiffPlus['sizediff'] = $sizediff;
								$biggestSizeDiffPlus['title'] = $title;
							}
						} else if ($sizediff < 0) {
							if ($sizediff < $biggestSizeDiffMinus['sizediff']) {
								$biggestSizeDiffMinus['sizediff'] = $sizediff;
								$biggestSizeDiffMinus['title'] = $title;
							}
						}

						if(strtotime($timestamp) - $previousEditTime > $longestBreak['duration']) {
							$longestBreak['duration'] = strtotime($timestamp) - $previousEditTime;
							$longestBreak['start'] = $previousEditTime;
							$longestBreak['end'] = strtotime($timestamp);
						}
						$previousEditTime = strtotime($timestamp);

						$dumpTexts[] = ($index + 1) . "	$title	$timeStampExcel	$sizediff	$sizediffCumul	" . ($minor ? 1 : 0) . "	" . ($new ? 1 : 0);
					}
					arsort($titles);
					$titles = array_slice($titles, 0, 20);
				}
			} else {
				curl_close($curl);
			}

			$this->result['stats'] = array (
				'totalSizediff' => $stats->totalSizediff,
				'contribsCount' => $stats->contribsCount,
				'user' => $userInfo->name | $user
			);
		}
	}
}

$s = new ApiStatsPerWiki();
$s->getStats($user, $wiki, $no_details);
$result = $s->result;