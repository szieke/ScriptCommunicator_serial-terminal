function Class(arg0)
{
    var waitForResponse = "100";
	var reset = function(arg1)
    {
	  var test = "";
	  waitForResponse = false;
	}	
	
	this.start = function(arg2)
    {
        reset();
	}
}

Class.prototype.getInfo = function(arg) 
{
    return this.waitForResponse;
}

var classVar = new Class("www");

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

classSingelton1.prototype.getInfo = function(arg)  //auswerten?
{
    return this.waitForResponse;
}

var test= "";

