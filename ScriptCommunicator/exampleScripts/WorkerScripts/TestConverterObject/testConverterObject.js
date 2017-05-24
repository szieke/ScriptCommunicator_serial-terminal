/*This script demonstrates the use of the script converter object.*/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}



scriptThread.appendTextToConsole('script has started');

var array = Array(0);

array = conv.stringToArray("Test string 1");
array = conv.addStringToArray(array, "  string 2");
var string = conv.byteArrayToString(array);
scriptThread.appendTextToConsole("string: " + string);

array = Array(1,2,3,4,5,6);
var hexString = conv.byteArrayToHexString(array);
scriptThread.appendTextToConsole("hexString: " + hexString);

array = conv.addUint16ToArray(array, 61234, true);
var uint16 = conv.byteArrayToUint16(array, true);
scriptThread.appendTextToConsole("uint16: " + uint16);

array = conv.addUint32ToArray(array, 1234567891, true);
var uint32 = conv.byteArrayToUint32(array, true);
scriptThread.appendTextToConsole("uint32: " + uint32);

array = conv.addUint64ToArray(array, 12345678912345678, true);
var uint64 = conv.byteArrayToUint64(array, true);
scriptThread.appendTextToConsole("uint64: " + uint64);

var int8 = conv.unsignedCharToSignedChar(250);
scriptThread.appendTextToConsole("int8: " + int8);

array = conv.addInt16ToArray(array, -32123, true);
var int16 = conv.byteArrayToInt16(array, true);
scriptThread.appendTextToConsole("int16: " + int16);

array = conv.addInt32ToArray(array, -1234567891, true);
var int32 = conv.byteArrayToInt32(array, true);
scriptThread.appendTextToConsole("int32: " + int32);

array = conv.addInt64ToArray(array, -12345678912345678, true);
var int64 = conv.byteArrayToInt64(array, true);
scriptThread.appendTextToConsole("int64: " + int64);

array = conv.addFloat32ToArray(array, 5.4321, true);
var float32 = conv.byteArrayToFloat32(array, true);
scriptThread.appendTextToConsole("float32: " + float32);

array = conv.addFloat64ToArray(array, 5.123456789, true);
var float64 = conv.byteArrayToFloat64(array, true);
scriptThread.appendTextToConsole("float64: " + float64);


scriptThread.stopScript();


