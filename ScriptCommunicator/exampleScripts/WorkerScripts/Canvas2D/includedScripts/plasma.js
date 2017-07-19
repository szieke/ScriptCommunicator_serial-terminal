/***************************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
ScriptCanvas2D class.
****************************************************************************************/

function dist(a, b, c, d)
{
    return Math.sqrt((a - c) * (a - c) + (b - d) * (b - d));
}

function renderPlasma()
{
    ctxPlasma.save();
	
	ctxPlasma.setFont("Arial",  15);
	ctxPlasma.textAlign = 0x0004 //Qt::AlignHCenter
	ctxPlasma.fillStyle = "rgb(255,255,255)";
	ctxPlasma.fillRect(0,0,128, 100);
	ctxPlasma.fillText(0, 0, 128, 50, "Plasma");
	
    var time = counter * 5;

    for( y = 20; y < 128; y+=PIXEL_SIZE) 
	{
		for( x = 0 ; x < 128; x+=PIXEL_SIZE) 
		{
		    
		    
		    var temp_val = Math.floor(Math.sin(dist(x + time, y, 128.0, 128.0) / 8.0)
					      + Math.sin(dist(x, y, 64.0, 64.0) / 8.0)
					      + Math.sin(dist(x, y + time / 7, 192.0, 64) / 7.0)
					      + Math.sin(dist(x, y, 192.0, 100.0) / 8.0));

		    var temp_col = Math.floor((2 + temp_val) * 50);
		    var rand_red = temp_col * 3;
		    var rand_green = temp_col  ;
		    var rand_blue = 128 - temp_col;

		    ctxPlasma.fillStyle = "rgb("+rand_red+","+rand_green+","+rand_blue+")";
		    ctxPlasma.fillRect(x,y,PIXEL_SIZE,PIXEL_SIZE);
		}
    }

    ctxPlasma.restore();
    counter++;

}
function sizeChangedPlasma(width, height)
{
	ctxPlasma.reset();
	ctxPlasma.translate((width -128) / 2,(height -128) / 2);
	renderPlasma();
}
function mousePressedPlasma(x, y, button)
{
	UI_TextEdit.append("mousePressedPlasma: " + x + "  " + y + "  " + button)
}
function mouseReleasedPlasma(x, y, button)
{
	UI_TextEdit.append("mouseReleasedPlasma: " + x + "  " + y + "  " + button)
}
function mouseMovedPlasma(x, y)
{
	UI_TextEdit.append("mouseMovedPlasma: " + x + "  " + y)
}
function keyPressedPlasma(keyText, keyCode, modifiers)
{
	UI_TextEdit.append("keyPressedPlasma: " + keyText + "  " + keyCode + "  " + modifiers)
}
function keyReleasedPlasma(keyText, keyCode, modifiers)
{
	UI_TextEdit.append("keyReleasedPlasma: " + keyText + "  " + keyCode + "  " + modifiers)
}

var counter = 0;
var PIXEL_SIZE = 4;
var temp_1 = 0;

var ctxPlasma = UI_GroupBoxPlasma.addCanvas2DWidget();
ctxPlasma.translate(45,20);
renderPlasma();
ctxPlasma.sizeChangeSignal.connect(sizeChangedPlasma);
ctxPlasma.mousePressSignal.connect(mousePressedPlasma);
ctxPlasma.mouseReleasSignal.connect(mouseReleasedPlasma);
ctxPlasma.mouseMoveSignal.connect(mouseMovedPlasma);
ctxPlasma.keyPressSignal.connect(keyPressedPlasma);
ctxPlasma.keyReleaseSignal.connect(keyReleasedPlasma);

var timerPlasma = scriptThread.createTimer();
timerPlasma.timeoutSignal.connect(renderPlasma);
timerPlasma.start(50);
