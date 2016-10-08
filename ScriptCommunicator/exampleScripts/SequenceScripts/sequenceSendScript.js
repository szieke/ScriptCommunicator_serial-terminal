/**********************************************************************
This is an example sequence send script which can be added to a sequence (send window), It demonstrates
the possibilities of sequence send scripts.

This script can not be executed by the "normal" script interface (script window).
***********************************************************************/
//This function appends a checksum and a CRC8 at data.
function sendData(data)
{
	var counter = 0;
	//Read the stored counter value
	var resultArray = seq.getGlobalUnsignedNumber("Counter");
	if(resultArray[0] == 1)
	{
		counter = resultArray[1];
	}

	//Append the counter value.
	data = conv.addUint32ToArray(data, counter, false);
	
	//Append a CRC8
	var crc8 = seq.calculateCrc8(data);
	data.push(crc8 & 0xff);	
	
	counter++;
	//Store the new counter value.
	seq.setGlobalUnsignedNumber("Counter", counter);
	seq.appendTextToConsole("sequenceSendScript.js send: " + data, true, false);

		
	return data;
}