var map3 = {
"HST" : {
        baudrate : HST_BAUDRATE,
        send : scriptThread.stringToArray("HST\n"),
		send2 : [ 60, 35, 83],
    },
	
}

function Class(arg0)
{
	var testString= scriptThread.getAllObjectPropertiesAndFunctions();
	var testTimer = scriptThread.createTimer();
	var testDate1 = Date();
	var testDate2 = new Date();
	
	
	var reset = function(arg1)
    {
	  var test = "";
	}	
	
	this.start = function(arg2)
    {
		var test = "";
        reset();
	}
}

Class.prototype.getInfo = function(arg) 
{
    return this.waitForResponse;
}

var array1 = 
{
	name: "ERROR", 
	ignoreReceivedData: true, 
	progress: 0, 
	testFunc: function (arg3) 
	{ return ""}
};
var map1 = 
{
	ERROR : {name: {subName :""}, ignoreReceivedData: true, progress: 0, testFunc: function (arg4) 
	{ return ""}},
}

var classSingelton1 = new function()
{
    var waitForResponse = 100;
	var testMap = {ignoreReceivedData: true, name: {subName :"", testFunc: function (arg4) 
	{ return ""}}}; 
	var reset = function(arg5)
    {
	  var test = "";
	  waitForResponse = false;
	}	
	
	this.start = function(arg6)
    {
        reset();
	}
}

classSingelton1.prototype.getInfo = function(arg) 
{
    return this.waitForResponse;
}