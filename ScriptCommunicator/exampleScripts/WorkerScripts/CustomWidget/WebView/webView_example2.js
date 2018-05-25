//Is called if the dialog is closed.
function dialogFinished(e)
{
	scriptThread.stopScript()
}

function sendPage()
{

	var cr = "\r\n";
	var hrd = "";
	var pl = "";
		
	// prepare  HTML	
	pl += "<html>" + cr;
	pl += "<head>" + cr;
	//pl += "<title>webKit evaluateJavaScript Example</title>" + cr;
	pl += '<script src="js/script.js" type="text/javascript"> </script>' + cr; // /js/script.js = ROOT; js/script.js = relative path
	pl += "</head>" + cr;	
	pl += "<body>" + cr;	

	pl += "<svg width='650' height='25'>" + cr;	
	pl += "<rect id='slider1' width='"+ UI_horizontalSlider.value()*7+"' height='25' style='fill:rgb(0,0,255)'>" + cr;	
	pl += "</svg><br>" + cr;
	
	pl += "<svg width='650' height='25'>" + cr;	
    pl += "<rect id='slider2' width='"+ UI_horizontalSlider_2.value()*7+"' height='25' style='fill:rgb(255,0,0)'>" + cr;	
	pl += "</svg><br>" + cr;	
	
	pl += "<svg width='650' height='25'>" + cr;	
	pl += "<rect id='slider3' width='"+ UI_horizontalSlider_3.value()*7+"' height='25' style='fill:rgb(255,0,255)'>" + cr;	
	pl += "</svg><br>" + cr;	

	pl += "<svg width='650' height='25'>" + cr;	
	pl += "<rect id='slider4' width='"+ UI_horizontalSlider_4.value()*7+"' height='25' style='fill:rgb(0,255,0)'>" + cr;	
	pl += "</svg><br>" + cr;	

	pl += '<script>function sendHello(){var result = webView.callWorkerScriptWithResult(Array("sendHello","hello world", 1));return result + " from WebView sendHello";} </script>' + cr;  // SC routine called from inside the webView component
	pl += '<script>function sendArray(){var a=[]; for(i=0;i<3;i++)a.push(i);webView.callWorkerScript(Array("sendArray",a, 2));return "result  from WebView sendArray";}</script>' + cr;  // SC routine called from inside the webView component	
	pl += "</body>" + cr;		
	pl += "</html>" + cr;	

    UI_webView.setHtml(pl,"file:///"+ scriptFile.getScriptFolder()+"/");
    UI_textEdit.setPlainText(UI_webView.evaluateJavaScript("document.body.innerHTML"));
	
	UI_statusTextEdit.append(UI_webView.evaluateJavaScript("sendHello()") );	
	UI_statusTextEdit.append(UI_webView.evaluateJavaScript("sendArray()")  + "\n");	
	
}

function horizontalSliderValueChanged(value)
{
	if (this == UI_horizontalSlider) UI_webView.evaluateJavaScript("document.getElementById('slider1').setAttribute('width',test("+value+"))");
	else if (this == UI_horizontalSlider_2) UI_webView.evaluateJavaScript("document.getElementById('slider2').setAttribute('width',test("+value+"))");
	else if (this == UI_horizontalSlider_3) UI_webView.evaluateJavaScript("document.getElementById('slider3').setAttribute('width',test("+value+"))");
	else if (this == UI_horizontalSlider_4) UI_webView.evaluateJavaScript("document.getElementById('slider4').setAttribute('width',test("+value+"))");
	UI_textEdit.setPlainText(UI_webView.evaluateJavaScript("document.body.innerHTML"));
	
	UI_statusTextEdit.append(UI_webView.evaluateJavaScript("sendHello()") );	
	UI_statusTextEdit.append(UI_webView.evaluateJavaScript("sendArray()") + "\n");	
}


//Note: This slot function blocks the WebView. Therefore no time consuming or blocking operations should be performed
//(for this reason all outputs are stored in outPutFrom_callWorkerScriptWithResult).
function callWorkerScriptWithResult(value, resultObject)
{
	resultObject.setResult("TestResult");
	
	if(value[0] == "sendHello")
	{
		outPutFrom_callWorkerScriptWithResult += "\ncallWorkerScriptWithResult caller: " + value[0];
		outPutFrom_callWorkerScriptWithResult += "\narguments: " + value[1] + "  " + value[2];
	}
	else
	{
		outPutFrom_callWorkerScriptWithResult += "\ncallWorkerScriptWithResult caller: unknown";
		outPutFrom_callWorkerScriptWithResult += "\n" + value;
	}
	
}

function callWorkerScript(value)
{
	UI_statusTextEdit.insertPlainText(outPutFrom_callWorkerScriptWithResult);
	outPutFrom_callWorkerScriptWithResult = "";
	
	 if(value[0] == "sendArray")
	{
		UI_statusTextEdit.append("callWorkerScript caller: " + value[0]);
		UI_statusTextEdit.append("arguments: " + value[1] + "  " + value[2]);
	}
	else
	{
		UI_statusTextEdit.append("callWorkerScript caller: unknown");
		UI_statusTextEdit.append(value);
	}
	
}

UI_Dialog.setWindowTitle("webKit evaluateJavaScript Example");
//UI_Dialog.setWindowFlags( 0x00000100 | 0x00004000); //not resizable, with minimize button
UI_Dialog.show();
UI_Dialog.finishedSignal.connect(dialogFinished);
var outPutFrom_callWorkerScriptWithResult = "";

try
{
	//Test if the script web view has been loaded.
	UI_webView.hasSelection();
	
	UI_horizontalSlider.valueChangedSignal.connect(UI_horizontalSlider,horizontalSliderValueChanged);
	UI_horizontalSlider_2.valueChangedSignal.connect(UI_horizontalSlider_2,horizontalSliderValueChanged);
	UI_horizontalSlider_3.valueChangedSignal.connect(UI_horizontalSlider_3,horizontalSliderValueChanged);
	UI_horizontalSlider_4.valueChangedSignal.connect(UI_horizontalSlider_4,horizontalSliderValueChanged);
    
	UI_webView.callWorkerScriptWithResultSignal.connect(callWorkerScriptWithResult);
	UI_webView.callWorkerScriptSignal.connect(callWorkerScript);
	
	sendPage();
	
}
catch(e)
{
	scriptThread.messageBox("Error", "Missing libraries", "Missing libraries. See " + scriptFile.createAbsolutePath("readme.txt") + " for more informations.");
	scriptThread.stopScript()
}



