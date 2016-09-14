<?php
	//$wiki = $_GET['wiki'];
	$namespaces = array(
		'Talk',
		'User', 'User talk',
		'Liquipedia', 'Liquipedia talk',
		'File', 'File talk',
		'MediaWiki', 'MediaWiki talk',
		'Template', 'Template talk',
		'Help', 'Help talk',
		'Category', 'Category talk',
		'Property', 'Property talk',
		'Type', 'Type talk',
		'Concept', 'Concept talk',
		'Starbow', 'Starbow talk',
		'Portal'
	);

	$userFound = false;

	$stats = new StdClass();
	$stats->totalSizediff = 0;
	$stats->contribsCount = 0;
	$stats->data = array();

	$request = new StdClass();
	$request->user = $user;

	$userInfos = array();
	$userFound = getUserInfoOnAllWikis($curl, $request->user, $userInfos);
	$foundUser = null;
	$foundWikis = array();
	foreach ($wikis as $wikiName => $wiki) {
		if ($userInfos[$wiki]->found) {
			$foundWikis[] = $wiki;
		}
	}
	$editcount = 0;

	if ($userFound) {
		$statsPerWiki = array();
		foreach ($wikis as $wikiName => $wiki) {
			$statsPerWiki[$wiki] = new StdClass();
			$statsPerWiki[$wiki]->totalSizediff = 0;
			$statsPerWiki[$wiki]->contribsCount = 0;
			$statsPerWiki[$wiki]->data = array();
			if ($userInfos[$wiki]->found
				&& intval($userInfos[$wiki]->editcount) > 0) {
				$url = 'http://wiki.teamliquid.net/' . $wiki . '/api.php'
				       . '?action=query&list=usercontribs&ucuser='
				       . str_replace(' ', '%20', htmlspecialchars($user))
				       . '&ucprop=title|timestamp|flags|sizediff|size'
				       . '&ucdir=newer'
				       . '&continue='
				       . '&uclimit=500'
				       . '&format=php';

				$continue = getContributions($curl, $url, $wiki,
					$statsPerWiki[$wiki]);

				while ( $continue > 0 ) {
					$url = 'http://wiki.teamliquid.net/' . $wiki . '/api.php'
					       . '?action=query&list=usercontribs&ucuser='
					       . str_replace(' ', '%20', htmlspecialchars($user))
					       . '&ucdir=newer'
					       . '&ucprop=title|timestamp|flags|sizediff|size'
					       . '&uccontinue=' . $continue['uccontinue']
					       . '&continue=' . $continue['continue']
					       . '&uclimit=500'
					       . '&format=php';
					$continue = getContributions($curl, $url, $wiki,
						$statsPerWiki[$wiki]);
				}

				$foundUser = $userInfos[$wiki]->name;
				$editcount += intval($userInfos[$wiki]->editcount);
			}

			$stats->totalSizediff += $statsPerWiki[$wiki]->totalSizediff;
			$stats->contribsCount += $statsPerWiki[$wiki]->contribsCount;
			$stats->data = array_merge($stats->data, $statsPerWiki[$wiki]->data);
		}

		usort($stats->data, 'dataCmp');

		curl_close($curl);

		$mean = round($stats->totalSizediff / $stats->contribsCount, 3);
		$earliest = strtotime($stats->data[0]['timestamp']);
		$latest = strtotime($stats->data[count($stats->data) - 1]['timestamp']);
		$days = ($latest - $earliest) / (60 * 60 * 24);
		$timeElapsed = time_elapsed($latest - $earliest);

		$namespace = array();
		$namespace['Main'] = 0;
		foreach($namespaces as $ns) {
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
		$previousEditDay = '';
		$activeDays = 0;
		foreach ($stats->data as $index => $data) {
			$title = $data['title'];
			$timestamp = $data['timestamp'];
			$sizediff = $data['sizediff'];
			$minor = $data['minor'];
			$new = $data['new'];
			$wiki = $data['wiki'];

			$found = 0;
			foreach($namespaces as $ns) {
				if (strpos($title, $ns . ':') === 0) {
					$namespace[$ns]++;
					$found = 1;
				}
			}
			if (!$found) $namespace['Main']++;
			if ($title === $ownUserSpace || strpos($title, $ownUserSpace . '/') === 0) {
				$ownUserSpaceEdits++;
			}
			if (!isset($titles[$wiki . '[' . $title])) {
				$titles[$wiki . '[' . $title] = 1;
			} else {
				$titles[$wiki . '[' . $title]++;
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

			$timeStampForDump = str_replace(array('T', 'Z'), array(' ', ''), $timestamp);
			$sizediffCumul += abs($sizediff);
			if ($sizediff >= 0) {
				if ($sizediff > $biggestSizeDiffPlus['sizediff']) {
					$biggestSizeDiffPlus['sizediff'] = $sizediff;
					$biggestSizeDiffPlus['title'] = $title;
					$biggestSizeDiffPlus['wiki'] = $wiki;
				}
			} else if ($sizediff < 0) {
				if ($sizediff < $biggestSizeDiffMinus['sizediff']) {
					$biggestSizeDiffMinus['sizediff'] = $sizediff;
					$biggestSizeDiffMinus['title'] = $title;
					$biggestSizeDiffMinus['wiki'] = $wiki;
				}
			}

			if(strtotime($timestamp) - $previousEditTime > $longestBreak['duration']) {
				$longestBreak['duration'] = strtotime($timestamp) - $previousEditTime;
				$longestBreak['start'] = $previousEditTime;
				$longestBreak['end'] = strtotime($timestamp);
			}
			$previousEditTime = strtotime($timestamp);

			if (date('Y-m-d', strtotime($timestamp)) != $previousEditDay)
			{
				$previousEditDay = date('Y-m-d', strtotime($timestamp));
				$activeDays++;
			}

			$dumpTexts[] = ($index + 1) . "	$title	$timeStampForDump	$sizediff	$sizediffCumul	" . ($minor ? 1 : 0) . "	" . ($new ? 1 : 0);
		}
		arsort($titles);
		$titles = array_slice($titles, 0, 20);
	}
	$user = $foundUser | $user;
	?>
	<!DOCTYPE html>
	<html>
	<head>
		<title>Statistics - User:<?php echo $user; ?></title>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
		<link rel="stylesheet" href="style.css" type="text/css">
		<link href="http://fonts.googleapis.com/css?family=Lato" rel="stylesheet" type="text/css">
	</head>
	<body>
	<?php if ($userFound === -1) { ?>	
		No reply from API! Make sure the wiki exists.
	<?php } else { ?>
		<a href="../editcount/">Liquipedia Editcount rankings</a> &bull;
		<a href="?user=<?php echo $user; ?>">Overview statistics page for User:<?php echo $user; ?></a><br/>
	<?php if ($userFound === 0) { ?>
		User missing!
	<?php } else { ?>
		<h2><a href="http://wiki.teamliquid.net/<?php echo $wiki; ?>/User:<?php echo $user; ?>">User:<?php echo $user; ?></a></h2>
		<p>Wiki: <a href="http://wiki.teamliquid.net/<?php echo $wiki; ?>"><?php echo $wiki; ?></a></p>
		<h3>User information</h3>
		<ul>
			<li><a href="http://www.teamliquid.net/forum/profile.php?user=<?php echo $user;?>">TL.net profile</a></li>
			<li>Edit count (including deleted edits on deleted pages): <?php echo number_format($editcount); ?></li>
		</ul>
		<h3>Contributions statistics</h3>
		<ul>
			<li><b><a href="http://wiki.teamliquid.net/<?php echo $wiki; ?>/Special:Contributions/<?php echo $user; ?>"><?php echo number_format($stats->contribsCount); ?></a></b> contributions (includes edits, moves, file uploads, does not include deleted contributions)</li>
			<li><b>Time between first and last contribution:</b> <?php echo number_format($days, 2); ?> - <?php echo $timeElapsed; ?></li>
			<ul>
				<li><b>First contribution:</b> <?php echo date("Y-m-d H:i", $earliest)?></li>
				<li><b>Latest contribution:</b> <?php echo date("Y-m-d H:i", $latest)?></li>
				<li><b>Longest time between two edits:</b> <?php echo time_elapsed($longestBreak['duration'])?>, between <?php echo date("Y-m-d H:i", $longestBreak['start'])?> and <?php echo date("Y-m-d H:i", $longestBreak['end'])?></li>
			</ul>
			<li><b>Contributions per day:</b> <?php echo number_format(($stats->contribsCount) / (float)($days), 2); ?></li>
			<li><b>Active days:</b> <?php echo number_format($activeDays); ?> (A day is counted as active if a contribution was made on that day, with days taken in the KST timezone.)</li>
			<ul>
				<li><b>Active days / (time between first and last contribution):</b> <?php echo number_format(($activeDays) / (float)($days) * 100, 2); ?>%</li>
				<li><b>Contributions per active day:</b> <?php echo number_format(($stats->contribsCount) / (float)($activeDays), 2); ?></li>
			</ul>
			<li><b>Contributions marked as minor:</b> <?php echo number_format($minorEdits); ?> (<?php printf("%.2f", (100 * $minorEdits) / (float)($stats->contribsCount));?>% of total)</li>
			<li><b>Contributions in own userspace:</b> <?php echo number_format($ownUserSpaceEdits); ?> (<?php printf("%.2f", (100 * $ownUserSpaceEdits) / (float)($stats->contribsCount));?>% of total)</li>
			<li><b>Total sizediff:</b> <?php echo number_format($stats->totalSizediff); ?> (mean: <?php echo $mean; ?> per contribution)</li>
			<ul>
				<li><b>Biggest positive sizediff:</b> <?php echo number_format($biggestSizeDiffPlus['sizediff']); ?> on <?php echo isset($biggestSizeDiffPlus['title']) ? wikilink($biggestSizeDiffMinus['wiki'], $biggestSizeDiffPlus['title']) : ''; ?></li>
				<li><b>Biggest negative sizediff:</b> <?php echo number_format($biggestSizeDiffMinus['sizediff']); ?> on <?php echo isset($biggestSizeDiffMinus['title']) ? wikilink(
					$biggestSizeDiffPlus['wiki'], $biggestSizeDiffMinus['title']) : ''; ?></li>
			</ul>
			<li><b>Files uploaded:</b> <?php echo number_format($filesUploaded);?>
			<ul>
				<li><b>Without text:</b> <?php echo number_format($filesUploadedWithoutText);?></li>
				<?php if($filesUploadedWithoutText) { ?> 
				<li><b>Latest without text:</b> <?php echo date("Y-m-d H:i", $latestUploadedWithoutText); ?></li>
				<?php } ?>
			</ul>
		</ul>
		<h3>Contributions by namespace</h3>
		<small>Note: "Portal" is not a real namespace.</small>
		<table>
		<?php
			while (list($ns, $nsEdits) = each($namespace)) {
				echo '<tr><td style="background: #f8f8f8;"><b>' . $ns . '</b></td>';
				echo '<td>' . number_format($nsEdits) . '</td>';
				echo '<td><font color="silver"><small>';
				printf("%.2f", (100 * $nsEdits) / (float)($stats->contribsCount));
				echo '%</small></font></td>';
				if (list($ns, $nsEdits) = each($namespace)) {
					echo '<td style="background: #f8f8f8;"><b>' . $ns . '</b></td>';
					echo '<td>' . number_format($nsEdits) . '</td>';
					echo '<td><font color="silver"><small>';
					printf("%.2f", (100 * $nsEdits) / (float)($stats->contribsCount));
					echo '%</small></font></td>';
				} else {
					echo '<td colspan="3"> </td>';
				}
				echo '</tr>';
			} ?>
		</table>
		<h3>20 most edited pages</h3>
		<ol>
		<?php
			foreach ($titles as $wikiTitle => $titleEdits) {
				list($wiki, $title) = explode('[', $wikiTitle);
				echo '<li>' . wikilink($wiki, $title) . ': ' . number_format($titleEdits) . ' edits</li>';
			}?>
		</ol>
		<h3><?php echo number_format(count($createdTemplates) - 2 + count($createdTemplates['flagTemplates']) + count($createdTemplates['teamTemplates'])); ?> created templates
			(including <?php echo number_format(count($createdTemplates['flagTemplates'])); ?> flag templates, <?php echo number_format(count($createdTemplates['teamTemplates'])); ?> team templates)</h3>
		<ul>
		<?php
			$createdNonFlagTeamTemplates = array_slice($createdTemplates, 2);
			foreach ($createdNonFlagTeamTemplates as $title) {
				echo '<li>' . wikilink($wiki, $title) . '</li>';
			}
			if (count($createdTemplates['flagTemplates']) > 0) {
				echo '<li>plus ' . count($createdTemplates['flagTemplates']) . ' flag templates</li>';
				echo '<ul>';
				foreach ($createdTemplates['flagTemplates'] as $title) {
					echo '<li>' . wikilink($wiki, $title) . '</li>';
				}
				echo '</ul>';
			}
			if (count($createdTemplates['teamTemplates']) > 0) {
				echo '<li>plus ' . count($createdTemplates['teamTemplates']) . ' team templates</li>';
				echo '<ul>';
				foreach ($createdTemplates['teamTemplates'] as $title) {
					echo '<li>' . wikilink($wiki, $title) . '</li>';
				}
				echo '</ul>';
			}
			?>
		</ul>
		<h3>Raw list of contributions:</h3>
		<pre>|#	|Title	|Date & time	|Sizediff	|Combined sizediff	|Minor	|New</pre>
		<textarea rows="20" style="width: 100%;"><?php echo implode("\n", $dumpTexts);?></textarea>
	<?php }?>
	<?php }?>
	</body>
	</html>