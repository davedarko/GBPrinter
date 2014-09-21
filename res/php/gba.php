<?php
	//include("head.php");
    $eintrag = stripslashes( 
                   str_replace(">","&gt;", 
                   str_replace("<","&lt;",
                   str_replace("ä","&auml;",
                   str_replace("ö","&ouml;",
                   str_replace("ü","&uuml;",
                   str_replace("Ä","&Auml;",
                   str_replace("Ö","&Ouml;",
                   str_replace("Ü","&Uuml;",
                             $_POST[eintrag]
                   ))))))))
               );
    $name = $_POST[name];
    //$title = "gameboy";

	$timestamp = time();
	$datum = date("YmdHis", $timestamp);
//	$daten= "<h2>Dave - $datum</h2> $eintrag <br>&nbsp;\n";
	$datenbank = "gameboyapp/" .$datum."-".$name. ".dat";
  
//	$datei = fopen($datenbank, "r");
//    $inhalt = fread($datei, filesize($datenbank));
//    fclose($datei);
    $datei = fopen($datenbank, "w+");
    $writestring = $eintrag;
    if (fwrite($datei, $writestring) == false) {
        echo "Cannot write to text file.";
    }
    fclose($datei);
    
	  echo $datenbank; 
	//include("fooder.php");
?>



