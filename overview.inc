<?php
	$userFound = false;

	$stats = new StdClass();
	$stats->totalSizediff = 0;
	$stats->contribsCount = 0;
	$stats->contribsCounts = array();
	$stats->firstEdit = "3000";
	$stats->lastEdit = "1900";
	$stats->firstEdits = array();
	$stats->lastEdits = array();

	$request = new StdClass();
	$request->user = $user;

	$userInfos = array();
	$userFound = getUserInfoOnAllWikis($curl, $request->user, $userInfos);

	getContribs($curl, $request, $stats);
	curl_close($curl);

	/*if (strlen($stats->firstEdit) > 4) {
		$stats->firstEdit = strtotime($stats->firstEdit);
		$stats->lastEdit = strtotime($stats->lastEdit);
	} else {
		$stats->firstEdit = 0;
		$stats->lastEdit = 0;
	}*/

	$lines = array(
		'<th>Wiki</th>',
		'<th>User ID</th>',
		'<th>Edit count</th>',
		'<th>Contributions</th>',
		'<th>Groups</th>',
		'<th>Registration time</th>'
		/*'<th>First contribution</th>',
		'<th>Latest contribution</th>'*/
	);
	$editCountTotal = 0;
	foreach ($wikis as $wikiName => $wiki) {
		if ($userInfos[$wiki]->found && intval($userInfos[$wiki]->editcount) > 0) {
			$editCountTotal += intval($userInfos[$wiki]->editcount);
		}
		/*if (strlen($stats->firstEdits[$wiki]) > 4) {
			$stats->firstEdits[$wiki] = strtotime($stats->firstEdits[$wiki]);
			$stats->lastEdits[$wiki] = strtotime($stats->lastEdits[$wiki]);
		} else {
			$stats->firstEdits[$wiki] = 0;
			$stats->lastEdits[$wiki] = 0;
		}*/
		
		$lines[0] .= '<td><a href="?user=' . $request->user . '&wiki=' . $wiki . '">' . $wikiName . '</a></td>';
		$lines[1] .= '<td>#' . $userInfos[$wiki]->userId . '</td>';
		$lines[2] .= '<td>' . number_format(intval($userInfos[$wiki]->editcount)) . '</td>';
		$lines[3] .= '<td>' . '<a href="http://wiki.teamliquid.net/' . $wiki . '/Special:Contributions/' . str_replace(array('[', ']'), array('(', ')'), $request->user) . '">' . number_format($stats->contribsCounts[$wiki]) . '</a>' . '</td>';
		$lines[4] .= '<td>' . implode('<br/>', $userInfos[$wiki]->groups) . '</td>';
		$lines[5] .= '<td>' . ($userInfos[$wiki]->registration ? date("Y-m-d H:i", $userInfos[$wiki]->registration) : "None") . '</td>';
		/*$lines[6] .= '<td>' . ($stats->firstEdits[$wiki] ? date("Y-m-d H:i", $stats->firstEdits[$wiki]) : "None") . '</td>';
		$lines[7] .= '<td>' . ($stats->lastEdits[$wiki] ? date("Y-m-d H:i", $stats->lastEdits[$wiki]) : "None") . '</td>';*/
	}

	?>
	<!DOCTYPE html>
	<html>
	<head>
		<title>Statistics - User:<?php echo $request->user; ?></title>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
		<link rel="stylesheet" href="style.css" type="text/css">
	</head>
	<body>
		<a href="../editcount/">Liquipedia Editcount rankings</a>
		<h2>User:<?php echo $request->user; ?></h2>
	<?php if ($userFound) { ?>
		<h3>Total</h3>
		<ul>
			<li><b>Editcount:</b> <?php echo number_format($editCountTotal); ?> (including deleted edits on deleted pages)</li>
			<li><b>Contributions count:</b> <?php echo number_format($stats->contribsCount); ?> (includes edits, moves, file uploads, does not include deleted contributions)</li>
			<!--<li><b>First contribution:</b> <?php //echo date("Y-m-d H:i", $stats->firstEdit)?></li>
			<li><b>Latest contribution:</b> <?php //echo date("Y-m-d H:i", $stats->lastEdit)?></li>-->
		</ul>
		<h3>By wiki</h3>
		<table class="overview-table">
		<?php foreach($lines as $line) { ?>
			<tr><?php echo $line; ?></tr>
		<?php } ?>
		</table>
	<?php } else { ?>
		No information found for this user.
	<?php } ?>
	</body>
	</html>