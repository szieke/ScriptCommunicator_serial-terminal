var map3 = {
"HST" : {
        baudrate : HST_BAUDRATE,
		send : scriptThread.getAllObjectPropertiesAndFunctions("HST\n"),
        unknown : obj.testFunc("dd"),
		send2 : [ 60, 35, 83],
    },
	
}



function testFunction()
{
	var testVar = 0;
	testVar.toExponential(); 
	
}

function Class(arg0)
{
	var testString= scriptThread.getAllObjectPropertiesAndFunctions();
	var testTimer = scriptThread.createTimer();
	var testDate1 = Date();
	var testDate2 = new Date();
	var testClassArray = [33, 55];
	
	
	var reset = function(arg1)
    {
	  var test = this.testClassArray;
	}	
	
	this.start = function(arg2)
    {
		var test = "";
	}

}

Class.prototype.getInfo = function(arg) 
{
	var test = this.testClassArray; 
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
	ERROR : {name: {subName :""}, ignoreReceivedData: true, progress: 0, 
		testFunc: function (arg4) 
		{ 
			var zzz = ""
			return ""
		}
	},
}


var classSingelton1 = new function()
{
    var waitForResponse = 100;
	var testMap = {ignoreReceivedData: true, name: {subName :"", 
		testFunc: function (arg4) 
		{ 
			var zzz = 0; 
			var testSub = this.subName; 
			var test2= map1; 
			return ""
		}
	}}; 
	
	var testMap2 = testMap;
	var testArray = array1
	var reset = function(arg5)
    {
	  var test = Date();
		var test2 = this.testMap ;
	}	
	
	this.start = function(arg6)
    {
        reset();
	}
}


classSingelton1.prototype.getInfo = function(arg) 
{
	var test = "";
	
	 function testFunc(arg4) 
		{ 
			var zzz = 0; 
			return ""
		}
    return this.waitForResponse.toExponential();
}
