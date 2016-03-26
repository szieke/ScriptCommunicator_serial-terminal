/********************************************************************************************************
* The script demonstrates the usage of the ScriptCommunicator XML API.
**********************************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
}

scriptThread.appendTextToConsole('script has started');

var xmlWriter = scriptThread.createXmlWriter();
xmlWriter.setCodec("UTF-8");
xmlWriter.setAutoFormatting(true);
xmlWriter.setAutoFormattingIndent(2);

xmlWriter.writeStartDocument();
xmlWriter.writeStartElement("RootElement");

/*********Create SubElement1*****************/
xmlWriter.writeStartElement("SubElement1");
xmlWriter.writeAttribute("attr1", "attrValue1");
xmlWriter.writeCDATA("CDATA1");
xmlWriter.writeComment("comment1");
xmlWriter.writeCharacters("\n    characters1\n  ");
xmlWriter.writeEndElement();//"SubElement1"


/*********Create SubElement2*****************/
xmlWriter.writeStartElement("SubElement2");
xmlWriter.writeAttribute("attr2", "attrValue2");
xmlWriter.writeCDATA("CDATA2");
xmlWriter.writeComment("comment2");
xmlWriter.writeCharacters("\n    characters2\n  ");
xmlWriter.writeEndElement();//"SubElement2"

//Create text element
xmlWriter.writeTextElement("TextElement", "text");


xmlWriter.writeEndElement();//"RootElement"
xmlWriter.writeEndDocument();
if(!xmlWriter.writeBufferToFile("testWrite.xml"))
{
	scriptThread.messageBox("Critical", "Error", "writteBufferToFile failed");
}


scriptThread.stopScript();
