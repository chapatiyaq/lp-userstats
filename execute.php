<?php
include('connection.php');
include('api_statsperwiki.inc');

$connection = Connection::getConnection();
$stmt = $connection->prepare('SELECT id, user_name, wiki FROM userstats_queue ORDER BY id LIMIT 6');
$stmt->execute();
$result = $stmt->fetchAll();

if (count($result)) {
	foreach ($result as $key => $data) {
		$s = new ApiStatsPerWiki();
		$s->getStats($data['user_name'], $data['wiki'], $no_details);
		$res = $s->result;

		if (isset($res) && !isset($res['error'])) {
			$stmt = $connection->prepare("SELECT user_name, wiki FROM userstats WHERE `user_name` = :user_name AND `wiki` = :wiki");
			$stmt->bindValue(':user_name', $data['user_name']);
			$stmt->bindValue(':wiki', $data['wiki']);
			$stmt->execute();

			if (!$stmt->fetch()) {
				$stmt = $connection->prepare("INSERT INTO `userstats` (`user_name`,`wiki`,`contribs_count`,`sizediff`) VALUES (:user_name,:wiki,:contribs_count,:sizediff)");
			} else {
				$stmt = $connection->prepare("UPDATE `userstats` SET `contribs_count` = :contribs_count, `sizediff` = :sizediff WHERE `user_name` = :user_name AND `wiki` = :wiki");
			}
			$stmt->bindValue(':user_name', $data['user_name']);
			$stmt->bindValue(':wiki', $data['wiki']);
			$stmt->bindValue(':contribs_count', $res['stats']['contribsCount']);
			$stmt->bindValue(':sizediff', $res['stats']['totalSizediff']);
			$stmt->execute();
			
			$stmt = $connection->prepare("DELETE FROM `userstats_queue` WHERE `id`= :id");
			$stmt->bindValue(':id', $data['id']);
			$stmt->execute();
		}
	}
}