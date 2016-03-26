/***************************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
ScriptCanvas2D class.
****************************************************************************************/

function roundedRect(ctx,x,y,width,height,radius)
{
  ctx.beginPath();
  ctx.moveTo(x,y+radius);
  ctx.lineTo(x,y+height-radius);
  ctx.quadraticCurveTo(x,y+height,x+radius,y+height);
  ctx.lineTo(x+width-radius,y+height);
  ctx.quadraticCurveTo(x+width,y+height,x+width,y+height-radius);
  ctx.lineTo(x+width,y+radius);
  ctx.quadraticCurveTo(x+width,y,x+width-radius,y);
  ctx.lineTo(x+radius,y);
  ctx.quadraticCurveTo(x,y,x,y+radius);
  ctx.stroke();
}

function drawPacman()
{
    // Draw shapes
    roundedRect(ctxPacman,12,12,150,150,15);
    roundedRect(ctxPacman,19,19,150,150,9);
    roundedRect(ctxPacman,53,53,49,33,10);
    roundedRect(ctxPacman,53,119,49,16,6);
    roundedRect(ctxPacman,135,53,49,33,10);
    roundedRect(ctxPacman,135,119,25,49,10);

    // Character 1
    ctxPacman.beginPath();
    ctxPacman.arc(37,37,13,Math.PI/7,-Math.PI/7,false);
    ctxPacman.lineTo(34,37);
    ctxPacman.fill();

    // blocks
    for(i=0;i<8;i++){
      ctxPacman.fillRect(51+i*16,35,4,4);
    }
    for(i=0;i<6;i++){
      ctxPacman.fillRect(115,51+i*16,4,4);
    }
    for(i=0;i<8;i++){
      ctxPacman.fillRect(51+i*16,99,4,4);
    }

    // character 2
    ctxPacman.beginPath();
    ctxPacman.moveTo(83,116);
    ctxPacman.lineTo(83,102);

    ctxPacman.bezierCurveTo(83,94,89,88,97,88);
    ctxPacman.bezierCurveTo(105,88,111,94,111,102);
    ctxPacman.lineTo(111,116);
    ctxPacman.lineTo(106.333,111.333);
    ctxPacman.lineTo(101.666,116);
    ctxPacman.lineTo(97,111.333);
    ctxPacman.lineTo(92.333,116);
    ctxPacman.lineTo(87.666,111.333);
    ctxPacman.lineTo(83,116);
    ctxPacman.fill();
    ctxPacman.fillStyle = "white";
    ctxPacman.beginPath();
    ctxPacman.moveTo(91,96);
    ctxPacman.bezierCurveTo(88,96,87,99,87,101);
    ctxPacman.bezierCurveTo(87,103,88,106,91,106);
    ctxPacman.bezierCurveTo(94,106,95,103,95,101);
    ctxPacman.bezierCurveTo(95,99,94,96,91,96);
    ctxPacman.moveTo(103,96);
    ctxPacman.bezierCurveTo(100,96,99,99,99,101);
    ctxPacman.bezierCurveTo(99,103,100,106,103,106);
    ctxPacman.bezierCurveTo(106,106,107,103,107,101);
    ctxPacman.bezierCurveTo(107,99,106,96,103,96);
    ctxPacman.fill();
    ctxPacman.fillStyle = "black";
    ctxPacman.beginPath();
    ctxPacman.arc(101,102,2,0,Math.PI*2,true);
    ctxPacman.fill();
    ctxPacman.beginPath();
    ctxPacman.arc(89,102,2,0,Math.PI*2,true);
    ctxPacman.fill();
}
function sizeChangedPacman(width, height)
{
	ctxPacman.reset();
	ctxPacman.translate((width -190) / 2,(height -200) / 2);
	drawPacman();
}
function mousePressedPacman(x, y, button)
{
	UI_TextEdit.append("mousePressedPacman: " + x + "  " + y + "  " + button)
}
function mouseReleasedPacman(x, y, button)
{
	UI_TextEdit.append("mouseReleasedPacman: " + x + "  " + y + "  " + button)
}
function mouseMovedPacman(x, y)
{
	UI_TextEdit.append("mouseMovedPacman: " + x + "  " + y)
}
function keyPressedPacman(keyText, keyCode, modifiers)
{
	UI_TextEdit.append("keyPressedPacman: " + keyText + "  " + keyCode + "  " + modifiers)
}
function keyReleasedPacman(keyText, keyCode, modifiers)
{
	UI_TextEdit.append("keyReleasedPacman: " + keyText + "  " + keyCode + "  " + modifiers)
}

var ctxPacman = UI_GroupBoxPacman.addCanvas2DWidget();
ctxPacman.sizeChangeSignal.connect(sizeChangedPacman);
ctxPacman.mousePressSignal.connect(mousePressedPacman);
ctxPacman.mouseReleasSignal.connect(mouseReleasedPacman);
ctxPacman.mouseMoveSignal.connect(mouseMovedPacman);
ctxPacman.keyPressSignal.connect(keyPressedPacman);
ctxPacman.keyReleaseSignal.connect(keyReleasedPacman);
drawPacman();


