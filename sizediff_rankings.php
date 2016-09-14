<?php
include('connection.php');

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

$connection = Connection::getConnection();
$stmt = $connection->prepare('SELECT user_name, wiki, contribs_count, sizediff FROM userstats');
$stmt->execute();
$result = $stmt->fetchAll();

$sizediffs = array();
$contribsCount = array();

foreach ($result as $key => $data) {
	if (!isset($sizediffs[$data['user_name']])) {
		$sizediffs[$data['user_name']] = array('total_sizediff' => 0);
		$contribsCount[$data['user_name']] = array('total_contribsCount' => 0);
	}
	$sizediffs[$data['user_name']][$data['wiki']] = $data['sizediff'];
	$sizediffs[$data['user_name']]['total_sizediff'] += $data['sizediff'];
	$contribsCount[$data['user_name']][$data['wiki']] = $data['contribs_count'];
	$contribsCount[$data['user_name']]['total_contribsCount'] += $data['contribs_count'];
}
uasort($sizediffs, 'cmp');
function cmp($a, $b) {
    return $b['total_sizediff'] - $a['total_sizediff'];
}
?>
<!doctype html>
<html>
<head>
	<link rel="stylesheet" type="text/css" href="style_sizediff_rankings.css">
	<title>Liquipedia sizediff rankings</title>
</head>
<body>
	<h1>Liquipedia sizediff rankings</h1>
	<table>
		<tr class="header-row">
			<th class="pos"></th>
			<th class="name">Name</th>
			<?php foreach($wikis as $name => $url_part) {
				echo '<th class="wiki ' . $url_part . '">' . $name . '</th>';
			} ?>
			<th class="total">Total</th>
		</tr>
	<?php
	$total = array();
	foreach($wikis as $wiki) {
		$total[$wiki] = 0;
	}
	foreach($sizediffs as $user_name => $sizediff) {
		foreach($wikis as $name => $url_part) {
			$total[$url_part] += $sizediff[$url_part];
		}
	}
	$total['all'] = 0;
	foreach($wikis as $wiki) {
		$total['all'] += $total[$wiki];
	}
	echo '<tr class="total-row">';
	echo '<td class="pos"></td>';
	echo '<td class="name">Total</td>';
	foreach($wikis as $wiki) {
		echo '<td class="wiki ' . $wiki . '">' . number_format($total[$wiki]) . '</td>';
	}
	echo '<td class="total">' . number_format($total['all']) . '</td>';
	echo '</tr>';

	$i = 1;
	foreach($sizediffs as $user_name => $sizediff) {
		echo '<tr class="user-row">';
		echo '<td class="pos">' . $i . '.</td>';
		echo '<td class="name">' . $user_name . '</td>';
		foreach ($wikis as $name => $url_part) {
			echo '<td class="wiki ' . $url_part . '">';
			echo '<span';
			if ($contribsCount[$user_name][$url_part] != 0) {
				echo ' class="info" title="';
				echo number_format($sizediff[$url_part] / $contribsCount[$user_name][$url_part]);
				echo ' bytes per contribution"';
			}
			echo '>';
			echo number_format($sizediff[$url_part]);
			echo '</span>';
			echo '</td>';
		}
		echo '<td class="total">';
		echo '<span';
		if ($contribsCount[$user_name]['total_contribsCount'] != 0) {
			echo ' class="info" title="';
			echo number_format($sizediff['total_sizediff'] / $contribsCount[$user_name]['total_contribsCount']);
			echo ' bytes per contribution"';
		}
		echo '>';
		echo number_format($sizediff['total_sizediff']);
		echo '</span>';
		echo '</td>';
		echo '</tr>';
		++$i;
	}
	?>
	</table>
</tbody>