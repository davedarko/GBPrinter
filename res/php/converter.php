<?php
	/**
		converts data from the gameboy java applet on davedarko.com/gameboy.php to pngs or arduino binaries
	*/


	// Create image instances
	if (empty($_GET['file'])) $file_name= "./gameboyapp/20111227023533-DAVE.dat";
	else $file_name = $_GET['file'];

	if (!empty($_GET['color'])) $color = $_GET['color'];
	else $color = "#00ee00";

	if (!empty($_GET['format'])) $format = $_GET['format'];
	else $format = "img";

	if ($format == "img") {
		$color = hexdec($color);
		$r = intval($color/256/256);
		$g = intval(($color-$r*256*256)/256);
		$b = intval($color - $r*256*256 - $g*256);

		$datei = fopen($file_name,"r");
		$cnt = 0;
	    while (!feof($datei)) {
	        $zeile = fgets($datei, 4096);
			$cnt++;
	    }
	    rewind($datei);

		$dest = imagecreatetruecolor(160, $cnt);

		imagecolorset ( $dest , 0 , 0 , 0 , 0 );//0 ,   0 , 0 );
		imagecolorset ( $dest , 1 , $r*1/3 , $g*1/3 , $b*1/3 );//0 ,  86 , 0 );
		imagecolorset ( $dest , 2 , $r*2/3 , $g*2/3 , $b*2/3 );//0 , 172 , 0 );
		imagecolorset ( $dest , 3 , $r , $g , $b );
		
		$col[3] = imagecolorallocate ( $dest , 0 ,   0 , 0);
		$col[2] = imagecolorallocate ( $dest , $r*1/3 , $g*1/3 , $b*1/3 );//0 ,  86 , 0 );
		$col[1] = imagecolorallocate ( $dest , $r*2/3 , $g*2/3 , $b*2/3 );//0 , 172 , 0 );0 , 172 , 0 );
		$col[0] = imagecolorallocate ( $dest , $r , $g , $b );

		$cnt=0;
	    while (!feof($datei)) {
	        $zeile = fgets($datei, 4096);
			for ($i=0; $i<strlen($zeile); $i++) imagesetpixel($dest, $i, $cnt, $col[$zeile[$i]]);
			$cnt++;
	    }
	    rewind($datei);
		fclose($datei);

		imagetruecolortopalette($dest,TRUE,4);

	// 	Output and free from memory
		header('Content-Type: image/png');
		header("Content-Disposition: inline; filename=\"".substr($file_name,13,14).".png\"");
		imagepng($dest);
		imagedestroy($dest);
	}
	else if ($format == "bin") {
		// 16 * 40 each row is a tile row
		// 2 bytes * 8 ergeben ein tyle
		// 20 tyles pro zeile
		// 2 zeilen pro row 2*8*20*2 = 640bytes


		$datei = fopen($file_name,"r");
		echo "<pre>";
		$bytes = 0;
		$arr = 1;

		$twister = array();
	    while (!feof($datei)) {
	        $zeile = fgets($datei, 4096);
	        $cnt = 0;
	        $low = 0;
	        $high = 0;

			for ($i=0; $i<strlen($zeile); $i++) {
				if ($zeile[$i]!="\n") {
					$nr = intval($zeile[$i]);
					//echo $nr; 
					if ($nr==1) $low = $low + 1;
					if ($nr==2) $high = $high + 1;
					if ($nr==3) {
						$low = $low + 1;
						$high = $high + 1;
					}
					$cnt++;

					if ($cnt==8) {
						/*
						echo " " . $bytes .  ": ";
						echo dhex($high);
						echo " ";
						echo dhex($low);
						echo "<br />";
						*/

						$ai = $bytes % 20;
						$a = intval($bytes / 20);

						$index = 160 * intval($a/8) + $ai * 8 + ($a % 8);
						//echo "$bytes: $ai $a $index \n";
						$twister[$index*2] = $high; 
						$twister[$index*2+1] = $low; 
						
						$low = 0;
				        $high = 0;	
				        $cnt=0;
				        $bytes++;
					}
					else {
						$high = $high * 2;
						$low = $low * 2;
					}
				}
			}
	    }
	    ksort($twister);
	    echo "uint8_t row0[640] = {\n";
	    for ($k=0;$k<count($twister); $k++) {
			if($k%640==0 && $k!=0) echo "\n};\nuint8_t row" . ($k / 640) . "[640] = {\n";
			echo dhex($twister[$k]) . ", ";
			
	    }
	    echo "\n};\n";
	    rewind($datei);
	    echo "</pre>";
	}

	function dhex ($h) {
		$s = strtoupper(dechex($h));
		if (strlen($s)==1) return "0x0" . $s;
		else return "0x" . $s;
	}
?>