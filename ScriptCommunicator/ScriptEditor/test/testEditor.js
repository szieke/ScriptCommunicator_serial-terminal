function Class(arg0)
{
    var waitForResponse = "100";//ToDo: Den Typen parsen und ggf. mit in die Autocompletionliste eintragen, wenn dieser Wert einer anderen
	//Variablen zugewiesen wird, dann den Typen mit zuweisen (den Typen von Variablen mit im Outline anzeigen?).
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

var test= "";

classSingelton1.testMap.name.testFunc();
classSingelton1.getInfo();

var map2 = map1;
var array2 = array1;
var classVar = new Class("www");

