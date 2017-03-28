/*************************************************************************
CustomConsole_QRCoder.js helper functions.

Pier Andrea Serra, University of Sassari, Italy. 2015
***************************************************************************/

cust.loadScript("CustomConsole_QRCodeHelper.js");

var imageFolder = scriptFile.getScriptFolder() + "/Media/"
var QRCode = scriptFile.readFile("QRData.txt");

var QRCodeArray =  CSVToArray(QRCode);

function createString(data, timeStamp, type, isLog)
{
	
	if ((data[0]== 81) && (data[1]== 82)) // "QR"
	{
		var resultString = "<p style='font-size:1px'>";
		var HTMLContent = "";
	
		for(var u = 0; u < QRCodeArray.length-1; u++)
		{
			for(var i = 0; i < QRCodeArray[u].length-1; i++)
			{
				var bitArray = numToBinaryArray( parseInt( QRCodeArray[u][i] ) );
			  
				for(var y = 0; y < bitArray.length; y++)
				{
					if ( bitArray[y] == "0" ) 
					{//The currentbit is 0.
						HTMLContent = "<img src='"+imageFolder+"0.gif'>";
				 
					} 				
					else
					{//The current bit is 1.
						HTMLContent = "<img src='"+imageFolder+"1.gif'>";
					}
					resultString += HTMLContent;
				} 
		   
			}
			resultString += "<BR>";
		}
			
		return resultString;
	}
	return "";
}

cust.appendTextToConsole("CustomConsole_QRCoder.js started", true, false);