<?php
	
$servername = “SERVERIP”;
$username = “USERNAME”;
$password = “PASSWORD”;
$dbname = “DBNAME”;

// Create connection
$connection = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($connection->connect_error) {
    die("Connection failed: " . $connection->connect_error);
} 

	
	$data = array();
	$queenData = array();
	
	
	$QueenSQL ="SELECT * FROM  `nmr_data` WHERE  `tag_id` =  '034*330*827' ORDER BY  `nmr_data`.`time` DESC LIMIT 1;";
	
	$MaleSQL ="SELECT * FROM  `nmr_data` WHERE  `tag_id` =  '045*307*104' ORDER BY  `nmr_data`.`time` DESC LIMIT 1;";
	
	$FemaleSQL ="SELECT * FROM  `nmr_data` WHERE  `tag_id` =  '045*036*091' ORDER BY  `nmr_data`.`time` DESC LIMIT 1;";

	$queenResult = $connection->query($QueenSQL);
	$i=0;
	if($queenResult->num_rows > 0) {
		while($row = $queenResult->fetch_assoc()) {
			$lastQueenPosition = $row["r_id"];
			$queenData[$i]["position"] = $row["r_id"];
			$queenData[$i]["time"] = $row["time"];
        $i++;
    	}

	}

	$mResult = $connection->query($MaleSQL);
	$i=0;
	if($mResult->num_rows > 0) {
		while($row = $mResult->fetch_assoc()) {
			$lastMalePosition = $row["r_id"];
        $mData[$i]["position"] = $row["r_id"];
        $mData[$i]["time"] = $row["time"];
        $i++;
    	}

	}
	
	$fResult = $connection->query($FemaleSQL);
	$i=0;
	if($fResult->num_rows > 0) {
		while($row = $fResult->fetch_assoc()) {
			$lastFemalePosition = $row["r_id"];
        $fData[$i]["position"] = $row["r_id"];
        $fData[$i]["time"] = $row["time"];
        $i++;
    	}

	}


	$data["queen"] = $queenData;
	$data["male"] = $mData;
	$data["female"] = $fData;

	$jsonOutput = json_encode($data);
	//header('Cache-Control: no-cache, must-revalidate');
	//header('Expires: Mon, 26 Jul 1997 05:00:00 GMT');
	//header('Content-type: application/json');
	echo ''.$lastQueenPosition.' ';
	echo ''.$lastMalePosition.' ';
	echo ''.$lastFemalePosition.'';
	
	/*
	echo "{\n";
	echo '"queen":"'.$lastQueenPosition.'",';
	echo '"male":"'.$lastMalePosition.'",';;
	echo '"female":"'.$lastFealePosition.'"';
	echo "\n}";
	*/
	
	//print_r($jsonOutput);

//$ratfile = fopen("data.json", "w") or die("Unable to open file!");
//fwrite($ratfile, $jsonOutput);
//fclose($myfile);
	
