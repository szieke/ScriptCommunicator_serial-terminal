/***************************************************************************************
This worker script (worker scripts can be added in the script window) demonstrates the usage of the 
ScriptCanvas2D class.
****************************************************************************************/

function renderRotate()
{
  for (var i=1;i<6;i++)
  { // Loop through rings (from inside to out)
    ctxRotate.save();
    ctxRotate.fillStyle = 'rgb('+(51*i)+','+(255-51*i)+',255)';

    for (j=0;j<i*6;j++){ // draw individual dots
      ctxRotate.rotate(Math.PI*2/(i*6));
      ctxRotate.beginPath();
      ctxRotate.arc(0,i*12.5,5,0,Math.PI*2,true);
      ctxRotate.fill();
    }

    ctxRotate.restore();
  }

}

function sizeChangedRotate(width, height)
{
	ctxRotate.reset();
	ctxRotate.translate(width / 2,height / 2);
	renderRotate();
}
function mousePressedRotate(x, y, button)
{
	UI_TextEdit.append("mousePressedRotate: " + x + "  " + y + "  " + button)
}
function mouseReleasedRotate(x, y, button)
{
	UI_TextEdit.append("mouseReleasedRotate: " + x + "  " + y + "  " + button)
}
function mouseMovedRotate(x, y)
{
	UI_TextEdit.append("mouseMovedRotate: " + x + "  " + y)
}
function keyPressedRotate(keyText, keyCode, modifiers)
{
	UI_TextEdit.append("keyPressedRotate: " + keyText + "  " + keyCode + "  " + modifiers)
}
function keyReleasedRotate(keyText, keyCode, modifiers)
{
	UI_TextEdit.append("keyReleasedRotate: " + keyText + "  " + keyCode + "  " + modifiers)
}

var ctxRotate = UI_GroupBoxRotate.addCanvas2DWidget();

ctxRotate.sizeChangeSignal.connect(sizeChangedRotate);
ctxRotate.mousePressSignal.connect(mousePressedRotate);
ctxRotate.mouseReleasSignal.connect(mouseReleasedRotate);
ctxRotate.mouseMoveSignal.connect(mouseMovedRotate);
ctxRotate.keyPressSignal.connect(keyPressedRotate);
ctxRotate.keyReleaseSignal.connect(keyReleasedRotate);
renderRotate();

