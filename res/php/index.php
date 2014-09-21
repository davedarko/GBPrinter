<!DOCTYPE html>
<html>
<head>
</head>
<body>
<div id="canvas_area"></div>
<img src="123.jpg" id="img" width="160px" style="border:1px solid #f06;" class="img">
<canvas id="canvas" width="160" height="128" style="border:1px solid #f06;" class="img"></canvas>
<div id="response"></div>

<style>
.img {
    image-rendering: -moz-crisp-edges;
    image-rendering: -o-crisp-edges;
    image-rendering: -webkit-optimize-contrast;
    -ms-interpolation-mode: nearest-neighbor;
}
</style>
<img id="img2" class="img">
<br />
<input id="name_of" placeholder="enter your name">
<textarea id="ta_hex"></textarea>
<input type="button" id="sendbutton" value="submit">
<script src="//code.jquery.com/jquery-1.10.1.min.js"></script>

<script>
function supports_canvas() {
  return !!document.createElement('canvas').getContext;
}

if (supports_canvas()) {
	console.log("yes");
	//draw_b();
}

function draw() {
  var canvas = document.getElementById("canvas");
  var img = document.getElementById("img");
  var img2 = document.getElementById("img2");
  var ta_hex = document.getElementById("ta_hex");

  canvas.height = img.height;
  var context = canvas.getContext("2d");
  context.imageSmoothingEnabled = false;
  context.mozImageSmoothingEnabled = false;
  context.webkitImageSmoothingEnabled = false;
//  context.translate(0.5,0.5);

  context.drawImage(img, 0, 0, img.width, img.height);

  // erstmal schwarz weiss
  for (var y=0; y<img.height; y++) {
  	for (var x = 0; x<img.width; x++) {
  		var data = context.getImageData(x,y,1,1).data;
      var oldpixel = parseInt((data[0] + data[1] + data[2]) / 3);
      
      context.fillStyle = 'rgba(' + oldpixel + ',' + oldpixel + ',' + oldpixel + ',' + 1 + ')';
      context.fillRect(x, y, 1, 1);
  	}	
  }

  var txt = "";
  // dann floydd
  for (var y=0; y<img.height; y++) {
    for (var x = 0; x<img.width; x++) {
      var data = context.getImageData(x,y,1,1).data;
      var oldpixel = data[0];
      var newpixel = map(oldpixel);
      
      txt+= "" + map4(oldpixel);

      var quant_error = oldpixel-newpixel;

      if (x!=img.width-1)
        set_q (context, x+1, y  , quant_error, 7);

      if (x!=0 && y!=img.height-1) 
        set_q (context, x-1, y+1, quant_error, 3);

      if (y!=img.height-1) 
        set_q (context, x  , y+1, quant_error, 5);

      if (x!=img.width-1 && y!=img.height-1) 
        set_q (context, x+1, y+1, quant_error, 1);
    } 
    txt += "\n";
  }
    img2.src = canvas.toDataURL();
    ta_hex.value = txt;

}

function map (oldpixel) {
  var pallette = new Array(
    0,64,128,192,256
  );
  for (i=1; i<pallette.length; i++) {
    if (
        oldpixel >= pallette[i-1] &&
        oldpixel < pallette[i]
      ) return pallette[i-1];
  }  
}
function map4 (oldpixel) {
  var pallette = new Array(
    0,64,128,192,256
  );
  for (i=1; i<pallette.length; i++) {
    if (
        oldpixel >= pallette[i-1] &&
        oldpixel < pallette[i]
      ) return 4-i;
  }  
}

function set_q (context, x, y, quant_error, q) {
  var data = context.getImageData(x,y,1,1).data;
  var oldpixel = data[0];
  var newpixel = data[0] + parseInt(quant_error * q / 16);
  if (newpixel>255) newpixel = newpixel;
  context.fillStyle = 'rgba(' + newpixel + ',' + newpixel + ',' + newpixel + ',' + 1 + ')';
  context.fillRect(x, y, 1, 1);
}

$('#sendbutton').click(function (){
  $.ajax({
    type: "POST",
    async: false,
    url: "gba.php",
    data: [
      {
        name: 'name',
        value: $('#name_of').val() 
      },
      {
        name: 'eintrag',
        value: $('#ta_hex').val()
      }
    ],
    success: function(responsedata) {
      console.log(responsedata);
      var link = 'converter.php?file='+responsedata+'&color=ffddbb';
      var ahref = '<a href="'+link+'">'+responsedata+'</a>';
      
      $('#response').html(ahref);
      
    }
  });
});

$('#img').load(function(){
  draw();
});

</script>
</body>
</html>