<?php
	define ('DBNAME', 'tolueno_liquipedia');
	define ('DBUSER', 'tolueno_admin');
	define ('DBPASSWORD', '7(C4?`cZ;DSFG+=.h`@&KHm$');

	class Connection {

		protected static $db;

		private function __construct() {
			try {
				self::$db = new PDO( 'mysql:host=localhost;dbname=' . DBNAME, DBUSER, DBPASSWORD);
				self::$db->setAttribute( PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION );
			}
			catch (PDOException $e) {
				echo "Connection Error: " . $e->getMessage();
			}

		}

		public static function getConnection() {
			if (!self::$db) {
				new Connection();
			}
			return self::$db;
		}
	}
?>