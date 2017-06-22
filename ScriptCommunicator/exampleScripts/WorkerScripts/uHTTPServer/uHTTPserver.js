/************************************************************
This script demonstrates how to create a TCP server which delivers html pages
to TCP clients (browser).
************************************************************/

//Reads a parameter value from the recieved http string.
function getParameterByName(name, line) 
{
    var results = new RegExp('[\\?&]' + name + '=([^&#]*)').exec(line);
    if (!results) 
	{ 
        return undefined;
    }
	var retVal = results[1];
	retVal = retVal.replace("HTTP/1.0","");
	retVal = retVal.replace("HTTP/1.1","");
    return retVal || undefined;
}

//Is called if the user closes the dialog.
function dialogFinished(e)
{
	scriptThread.stopScript()
}

//Is called if a client is connected to the server.
function connectionEstablished()
{
	tcpServerClient = tcpServer.nextPendingConnection();
	tcpServerClient.readyReadSignal.connect(tcpServerClient, readyReadSlot);
}

//Is called if a client sends data.
function readyReadSlot()
{
	var autorefresh;	

	if(this.canReadLine())
	{ //process the received http string
		
		var line = this.readLine();
		var par = getParameterByName("slider1",line) ;
		if(par != undefined) UI_horizontalSlider.setValue(par);
			
		par = getParameterByName("slider2",line) ;
		if(par != undefined) UI_horizontalSlider_2.setValue(par);
			
		par = getParameterByName("slider3",line) ;
		if(par != undefined) UI_horizontalSlider_3.setValue(par);
			
		par = getParameterByName("slider4",line) ;
		if(par != undefined) UI_horizontalSlider_4.setValue(par);
			
		par = getParameterByName("refresh",line) ;
		if(par != undefined) autorefresh = parseInt(par);  // -1 = read standard refresh rate
			
		//send the answer to the client
		sendAnswerToClient(this,autorefresh);
	}
	//close the connection
	this.close();
}

//Sends the html page to the client
function sendAnswerToClient(serverClient,customRefreshRate)
{
	var date = new Date().toUTCString();
	var lastModification = date; // may be different
	var architecture = scriptThread.currentCpuArchitecture();
	var server = "ScriptCommunicator/" + scriptThread.getCurrentVersion() + " (" + scriptThread.currentCpuArchitecture() + ")"
	var autorefresh;
		
	var cr = "\r\n";
	var hrd = "";
	var pl = "";
		
	// prepare  payload	
	pl += "<html>" + cr;
	pl += "<head>" + cr;
	pl += "<title>uHTTPServer</title>" + cr;
	pl += "<link rel='icon' type='image/gif' href='data:image/gif;base64,R0lGODlhEAAQAIAAAAAAAAAAACH5BAkAAAEALAAAAAAQABAAAAIgjI+py+0PEQiT1lkNpppnz4HfdoEH2W1nCJRfBMfyfBQAOw==' />" + cr;

	if(customRefreshRate > -1 ) autorefresh = customRefreshRate; else autorefresh = HTMLPageRefreshRate;
	if (autorefresh > 0) pl += "<meta http-equiv='refresh' content='" + autorefresh + "'>" + cr;

	pl += "</head>" + cr;	
	pl += "<body>" + cr;	
	pl += "<h1>" + cr;	

	pl += "<font color='blue'> Slider1 value: " + UI_horizontalSlider.value() + "<br> </font>" + cr;	
	pl += "<svg width='400' height='25'>" + cr;	
	pl += "<rect width='"+ UI_horizontalSlider.value()*4+"' height='25' style='fill:rgb(0,0,255)'>" + cr;	
	pl += "</svg><br>" + cr;
	
	pl += "<font color='red'> Slider2 value: " + UI_horizontalSlider_2.value()+ "<br> </font>" + cr;	
	pl += "<svg width='400' height='25'>" + cr;	
	pl += "<rect width='"+ UI_horizontalSlider_2.value()*4+"' height='25' style='fill:rgb(255,0,0)'>" + cr;	
	pl += "</svg><br>" + cr;	
		
	pl += "<font color='magenta'> Slider3 value: " + UI_horizontalSlider_3.value()+ "<br> </font>" + cr;	
	pl += "<svg width='400' height='25'>" + cr;	
	pl += "<rect width='"+ UI_horizontalSlider_3.value()*4+"' height='25' style='fill:rgb(255,0,255)'>" + cr;	
	pl += "</svg><br>" + cr;	

	pl += "<font color='lime'> Slider4 value: " + UI_horizontalSlider_4.value()+ "<br> </font>" + cr;	
	pl += "<svg width='400' height='25'>" + cr;	
	pl += "<rect width='"+ UI_horizontalSlider_4.value()*4+"' height='25' style='fill:rgb(0,255,0)'>" + cr;	
	pl += "</svg><br>" + cr;	

	pl += "</h1>" + cr;			
	pl += "</body>" + cr;		
	pl += "</html>" + cr;	

	//Header
	hrd += "HTTP/1.1 200 OK" + cr;
	hrd += "Date: " + date + cr;
	hrd += "Server: " + server + cr;
	hrd += "Last-Modified: " + lastModification + cr;
	hrd += 'ETag: "3f80f-1b6-3e1cb03b"' + cr;

	hrd += "Content-Type: text/html; charset=UTF-8" + cr;
	hrd += "Content-Length: " + pl.length + cr;
	hrd += "Accept-Ranges: bytes" + cr;
	hrd += "Connection: close" + cr;
	hrd += "" + cr; // empty line

	serverClient.writeString(hrd+pl);	
}
//The value of the refresh rate has been changed.
function spinBoxValueChanged(value)
{
	HTMLPageRefreshRate = UI_spinBox.value();
}

//The value of the server port has been changed.
function spinBox2ValueChanged(value)
{	
	var port = UI_spinBox_2.value();
    tcpServer.close();
    tcpServer.listen(port);
    UI_label_7.setText('<html><head/><body><p><a href="http://localhost:'+port+'"><span style=" text-decoration: underline; color:#0000ff;">http://localhost:'+port+'</span></a></p></body></html>');
}

var maxPendingConnections = 10;
var HTMLPageRefreshRate = 1;

UI_Dialog.setWindowFlags(0x00040000 | 0x00000100 | 0x00004000); // ontop, not resizable, with minimize button
UI_Dialog.show();
UI_Dialog.setWindowTitle("uHTTPServer");
UI_Dialog.finishedSignal.connect(dialogFinished);

UI_spinBox_2.setRange(8080,8100);
UI_spinBox_2.setValue(8080);
UI_spinBox_2.valueChangedSignal.connect(spinBox2ValueChanged);

UI_spinBox.setRange(0,5);
UI_spinBox.setValue(HTMLPageRefreshRate);
UI_spinBox.valueChangedSignal.connect(spinBoxValueChanged);

var tcpServer = scriptInf.createTcpServer();
tcpServer.setMaxPendingConnections(maxPendingConnections);
var tcpServerClient = undefined;
tcpServer.newConnectionSignal.connect(connectionEstablished);
tcpServer.listen(UI_spinBox_2.value());


