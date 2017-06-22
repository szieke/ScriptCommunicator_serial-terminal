/***************************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
ScriptCanvas2D class.
****************************************************************************************/

function clock(){
  var now = new Date();
  //var ctxClock = document.getElementById('tutorial').getContext('2d');
  ctxClock.save();
  ctxClock.clearRect(0,0,150,150);
  ctxClock.translate(75,75);
  ctxClock.scale(0.4,0.4);
  ctxClock.rotate(-Math.PI/2);
  ctxClock.strokeStyle = "black";
  ctxClock.fillStyle = "white";
  ctxClock.lineWidth = 8;
  ctxClock.lineCap = "round";
  
  ctxClock.save();
  ctxClock.beginPath();
  ctxClock.lineWidth = 14;
  ctxClock.strokeStyle = '#325FA2';
  ctxClock.arc(0,0,142,0,Math.PI*2,true);
  ctxClock.fill();
  ctxClock.stroke();
  ctxClock.restore();

  
  // Hour marks
  ctxClock.save();
  ctxClock.beginPath();
  for (i=0;i<12;i++){
    ctxClock.rotate(Math.PI/6);
    ctxClock.moveTo(100,0);
    ctxClock.lineTo(120,0);
  }
  ctxClock.stroke();
  ctxClock.restore();

  // Minute marks
  ctxClock.save();
  ctxClock.lineWidth = 5;
  ctxClock.beginPath();
  for (i=0;i<60;i++){
    if (i%5!=0) {
      ctxClock.moveTo(117,0);
      ctxClock.lineTo(120,0);
    }
    ctxClock.rotate(Math.PI/30);
  }
  ctxClock.stroke();
  ctxClock.restore();

  var sec = now.getSeconds();
  var min = now.getMinutes();
  var hr  = now.getHours();
  hr = hr>=12 ? hr-12 : hr;

  ctxClock.fillStyle = "black";

  // write Hours
  ctxClock.save();
  ctxClock.rotate( hr*(Math.PI/6) + (Math.PI/360)*min + (Math.PI/21600)*sec )
  ctxClock.lineWidth = 14;
  ctxClock.beginPath();
  ctxClock.moveTo(-20,0);
  ctxClock.lineTo(80,0);
  ctxClock.stroke();
  ctxClock.restore();

  // write Minutes
  ctxClock.save();
  ctxClock.rotate( (Math.PI/30)*min + (Math.PI/1800)*sec )
  ctxClock.lineWidth = 10;
  ctxClock.beginPath();
  ctxClock.moveTo(-28,0);
  ctxClock.lineTo(112,0);
  ctxClock.stroke();
  ctxClock.restore();
  
  // Write seconds
  ctxClock.save();
  ctxClock.rotate(sec * Math.PI/30);
  ctxClock.strokeStyle = "#D40000";
  ctxClock.fillStyle = "#D40000";
  ctxClock.lineWidth = 6;
  ctxClock.beginPath();
  ctxClock.moveTo(-30,0);
  ctxClock.lineTo(83,0);
  ctxClock.stroke();
  ctxClock.beginPath();
  ctxClock.arc(0,0,10,0,Math.PI*2,true);
  ctxClock.fill();
  ctxClock.beginPath();
  ctxClock.arc(95,0,10,0,Math.PI*2,true);
  ctxClock.stroke();
  ctxClock.fillStyle = "#555";
  ctxClock.arc(0,0,3,0,Math.PI*2,true);
  ctxClock.fill();
  ctxClock.restore();
  
  ctxClock.restore();
  
}
function sizeChangedClock(width, height)
{
	ctxClock.reset();
	ctxClock.translate((width -160) / 2,(height -160) / 2);
	clock();
}
function mousePressedClock(x, y, button)
{
	UI_TextEdit.append("mousePressedClock: " + x + "  " + y + "  " + button)
}
function mouseReleasedClock(x, y, button)
{
	UI_TextEdit.append("mouseReleasedClock: " + x + "  " + y + "  " + button)
}
function mouseMovedClock(x, y)
{
	UI_TextEdit.append("mouseMovedClock: " + x + "  " + y)
}
function keyPressedClock(keyText, keyCode, modifiers)
{
	UI_TextEdit.append("keyPressedClock: " + keyText + "  " + keyCode + "  " + modifiers)
}
function keyReleasedClock(keyText, keyCode, modifiers)
{
	UI_TextEdit.append("keyReleasedClock: " + keyText + "  " + keyCode + "  " + modifiers)
}

var ctxClock = UI_GroupBoxClock.addCanvas2DWidget();
clock();
ctxClock.sizeChangeSignal.connect(sizeChangedClock);
ctxClock.mousePressSignal.connect(mousePressedClock);
ctxClock.mouseReleasSignal.connect(mouseReleasedClock);
ctxClock.mouseMoveSignal.connect(mouseMovedClock);
ctxClock.keyPressSignal.connect(keyPressedClock);
ctxClock.keyReleaseSignal.connect(keyReleasedClock);

var timerClock = scriptThread.createTimer();
timerClock.timeoutSignal.connect(clock);
timerClock.start(1000);