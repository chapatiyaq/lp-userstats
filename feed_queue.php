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

// 100 users
$users = array('Itsjustatank', 'Koorb', 'Yhamm', 'Inflicted', 'Blahz0r', 'Shz', 'Imallinson', 'NovemberstOrm', 'ChapatiyaqPTSM', 'DiMano', 'Miwi', 'DaRSaiNT', 'Salle', 'Seeker', 'PhiLtheFisH', 'Capiston', 'DARKING', 'Karrow17', 'FO-nTTaX', 'Prech', 'Dangthatsright', 'Noam', 'PPingu', 'Chapabot', 'Hainrich', 'Clubfan', 'Tofucake', 'Itsjustabot', 'Farone', 'KristofferAG', 'Mikkmagro', 'Epoxide', 'ZodaSoda', 'Aeroblaster', 'Tephus', 'Whisper', 'Aesop', 'ImperatorBot', 'Hzhenyu007', 'FO-BoT', 'Boucot', 'Elyvilon', 'Dialogue', 'Dr. Waffles', 'Darkaros', 'Mewka', 'Caesarion', 'Wren', 'Flicky', 'Aeromi', 'JBright', 'Antylamon', 'Androxide', 'Ibo422', 'Xpaperclip', 'DivinO', 'Presidenten', 'Pholon', 'Dexington', 'Seanzk', 'Chris CityHunter', 'FlamingPie', 'Endy', 'Muriloricci', 'AndaGalant', 'SNSeigifried', 'JohnnyBB85', 'PhiLtheBoT', 'Wodger', 'FreedomSC2', 'Imperator', 'Corvuuss', 'Jacky9185', 'Kiekaboe', 'IMAniaC', 'Blueblister', 'Gecko(Xp)', 'PolskaGora', 'XYc', 'Dalnore', 'Freeamount', 'GeckoXp', 'Riw', 'TheFallofTroy', 'Team .SCA', 'Nimdil', 'Lumpur', 'Astroorion', 'Heyoka', 'Serinox', 'Xyl', 'Poboxy', 'Namakaye', 'MediKing', 'WarhOk', 'Quirinus', 'Wardi', 'Shaetan', 'Oukka', 'Lorning');

function addJob($conn, $user, $wiki) {
	$conn->beginTransaction();
	$stmt = $conn->prepare("INSERT INTO `userstats_queue` (`user_name`,`wiki`) VALUES (:user_name,:wiki)");
	$stmt->bindValue(':user_name', $user);
	$stmt->bindValue(':wiki', $wiki);
	$stmt->execute();

	// Commit changes, if unsucessful, rollback
	if (!$conn->commit()) {
		$conn->rollBack();
		return $status;
	}
	$status['success'] = true;
	return $status;
}

$connection = Connection::getConnection();

foreach ($users as $user) {
	foreach ($wikis as $wiki) {
		addJob($connection, $user, $wiki);
	}
}