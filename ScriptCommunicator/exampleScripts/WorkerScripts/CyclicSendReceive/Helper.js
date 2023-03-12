
//Is called if the script has been stopped.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}

//Is called if the user closes the user interface.
function dialogFinished(e)
{
	scriptThread.stopScript()
}

/*
 * This function sends a get managed ring buffer list command to the device.
 */
/*
 * This function appends a string to an byte array.
 * @param array
 *		The byte array.
 * @param str
 *      	The string.
 */
function appendStringToByteArray(array, str)
{
	for (var i = 0; i < str.length; ++i)
	{
	    array.push(str.charCodeAt(i));
	}
	return array;
	
}

/*
 * This function appends a byte array to an byte array.
 * @param dest
 *	The destination byte array.
 * @param source
 * 	The source byte array.
  * @param maxBytes
 * 	The max. number bytes which shall be appended.
 */
function appendByteArrayAtByteArray(dest, source, maxBytes)
{
	for(var i = 0; i < source.length; i++)
	{
		if(maxBytes <= i )
		{//To many bytes in source.
			break;
		}
		dest.push(source[i]);
	}
	return dest;
}
/*
 * This function appends a number to an byte array.
 * @param array
 *	The byte array.
 * @param number
 * 	The number.
  * @param numberOfBytes
 * 	The number bytes which shall be appended.
 */
function appendNumberAtByteArray(array, number, numberOfBytes)
{
	for(var i = 0; i < numberOfBytes; i++)
	{
		var tmpValue = number >> ((numberOfBytes - (i + 1)) * 8);
		tmpValue = tmpValue & 0xff;
		array.push(tmpValue);
	}
	return array;
}


scriptThread.appendTextToConsole('script has started');
UI_Dialog.finishedSignal.connect(dialogFinished);
