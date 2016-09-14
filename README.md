# Sizediff database

__Note: it is not recommended to try to maintain a database as it uses too many resources for the value it provides.__

Use the code in the 'databases.sql' and execute it to create the two databases required to record sizediff data.

To generate the records, set up a cron task with the following command:
`php /path/to/userstats/execute.php`

Example of cron table entry to execute the script every day at 12:00 am:
`0 0 * * * php /path/to/userstats/execute.php`

## Login / password

__Note: Only if you want to use the sizediff rankings__

In 'connection.php', set the values of  DBNAME, DBUSER and DBPASSWORD, the database name, user name and password for access to the database (SELECT, INSERT INTO, and UPDATE operations)

# User agent

You can change the user agent in 'index.php' (search for "CURLOPT_USERAGENT"), especially to change the contact e-mail to your e-mail address.