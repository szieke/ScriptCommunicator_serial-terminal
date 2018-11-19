/********************************************************************************************************
* The script demonstrates the usage of the ScriptCommunicator SQL API.
**********************************************************************************************/

//Is called if this script shall be exited.
function stopScript() 
{
    scriptThread.appendTextToConsole("script has been stopped");
	db.close();
}

function testSqlIndex()
{
	scriptThread.appendTextToConsole("\n\ntestSqlIndex");

	var sqlIndex = db.primaryIndex("test1");
	sqlIndex.setCursorName("test curser");
	scriptThread.appendTextToConsole("cursorName: " + sqlIndex.cursorName());
	
	var sqlField = scriptSql.createField("testFiled", 2);
	sqlIndex.append(sqlField)
	
}
function testRecordAndField()
{
	scriptThread.appendTextToConsole("\n\ntestRecord");

	var record = db.record("test1");
	var fieldcount = record.count();
	scriptThread.appendTextToConsole('fieldcount: ' + fieldcount );
	for (var i = 0; i < fieldcount; i++) 
	{
	    scriptThread.appendTextToConsole("field: " + i + ", named: " + record.fieldName(i));
	}
	
	var field = record.field("id")
	scriptThread.appendTextToConsole("fieldname: " + field.name());
	
	record= scriptSql.createRecord();
	record.append(scriptSql.createField("id", 2));
	scriptThread.appendTextToConsole('fieldcount: ' + record.count() );
	scriptThread.appendTextToConsole("field: " + (i + 1) + ", named: " + record.fieldName(0));
}
function testBinding()
{
	scriptThread.appendTextToConsole("\n\ntestBinding");
	var sqlQuery = scriptSql.createQuery(db);
	sqlQuery.prepare("INSERT INTO test(testcol) VALUES(:val1, :val2, :val3, :val4)");
	sqlQuery.bindValue(":val1", "11");
	sqlQuery.bindValue(":val2", 2);
	sqlQuery.bindValue(":val3", 2.5);
	sqlQuery.bindValue(":val4", Array(9,9,9,9));
	
	//Print all bounded values.
	var myMap = sqlQuery.boundValues();
	for( key in myMap )
	{
		scriptThread.appendTextToConsole('bound: ' + key + "," + myMap[key]);
	}
}
//Returns the largest id from a table.
function getLastIdFromTable(name)
{
	var resultNumber = 0;
	
	/******************************************Get all ids from the table.**************************/
	var sqlQuery = scriptSql.createQuery(db, "select id from " + name);
	if (!sqlQuery.isActive()) 
	{
		scriptThread.appendTextToConsole('Insert failed: ' + sqlQuery.lastError().text() + " " + sqlQuery.lastQuery());
		return -1;
    }
	db.commit();
	/**********************************************************************************************/
	
	//Get the largest id.
	while (sqlQuery.next()) 
	{
        var id = sqlQuery.value("id");
		if(id > resultNumber)
		{
			resultNumber = id;
		}
    }
	
	return resultNumber;
}
function createTableAndInsertData(name)
{
	scriptThread.appendTextToConsole("\n\createTableAndInsertData: " + name);
	
	/******************************************Create a table.**************************/
	var sqlString = "create table if not exists " +  name + " (id int primary key, name varchar(20))"	 
	var sqlQuery = scriptSql.createQuery(db, sqlString);
	if (!sqlQuery.isActive()) 
	{
		scriptThread.appendTextToConsole('Create Table Fail: ' + sqlQuery.lastError().text() + " " + sqlQuery.lastQuery());
		return false;
    }
	db.commit();
	
	/**********************************************************************************************/
	
	
	/************************Insert data into the table.**************************************/
	var id = getLastIdFromTable(name);
	if(id != -1)
	{
		id += 1;
		scriptThread.appendTextToConsole('new id: ' + id);
		sqlString
		sqlQuery.exec("INSERT INTO " + name + " (id, name) VALUES (" + id + ", 'John" + id +"')" )
		if (!sqlQuery.isActive()) 
		{
			scriptThread.appendTextToConsole('Insert failed: ' + sqlQuery.lastError().text() + " " + sqlQuery.lastQuery());
			return false;
		}
		db.commit();
		
	}
	/**********************************************************************************************/
		
    return true;
}

function createTableAndInsertByteArray(name)
{

	scriptThread.appendTextToConsole("\n\createTableAndInsertByteArray: " + name);
	
	/******************************************Create a table.**************************/
	var sqlString = "create table if not exists " +  name + "_byte (id int UNIQUE NOT NULL PRIMARY KEY, value BLOB NULL)"			 
	var sqlQuery = scriptSql.createQuery(db, sqlString);
	if (!sqlQuery.isActive()) 
	{
		scriptThread.appendTextToConsole('Create Table Fail: ' + sqlQuery.lastError().text() + " " + sqlQuery.lastQuery());
		return false;
    }
	db.commit();
	/**********************************************************************************************/
	
	/************************Insert a byte array and an id into the table.**************************************/
	var id = getLastIdFromTable(name);
	if(id != -1)
	{
		id += 1;
		scriptThread.appendTextToConsole('new id: ' + id);
	
		sqlQuery.prepare("INSERT INTO " + name + "_byte (id, value) VALUES (:id, :value);");
		sqlQuery.bindValue(":id", id);
		sqlQuery.bindValue(":value", Array(id + 1, id + 2, id + 3, id + 4, id + 5));
		
		if (!sqlQuery.exec()) 
		{
			scriptThread.appendTextToConsole('Insert failed: ' + sqlQuery.lastError().text() + " " + sqlQuery.lastQuery());
			return false;
		}
		db.commit();
	}
	/**********************************************************************************************/
	
	
	/************************Get and print all byte arrays from the table.**************************************/
	sqlQuery = scriptSql.createQuery(db, "select value from " + name + "_byte");
	if (!sqlQuery.isActive()) 
	{
		scriptThread.appendTextToConsole('Insert failed: ' + sqlQuery.lastError().text() + " " + sqlQuery.lastQuery());
		return false;
    }
	db.commit();
	
	while (sqlQuery.next()) 
	{
        var array = sqlQuery.value("value");
		scriptThread.appendTextToConsole("array:" + array);
    }
	/**********************************************************************************************/
		
    return true;
}



scriptThread.appendTextToConsole('script has started');
var db = scriptSql.addDatabase("QSQLITE")
db.setDatabaseName(scriptFile.createAbsolutePath("test.db3"));
db.open();
if(!db.isOpen())
{
	scriptThread.appendTextToConsole("could not open db");
}
else
{
	createTableAndInsertData("test1");
	createTableAndInsertData("test2");
	createTableAndInsertByteArray("test1")
	createTableAndInsertByteArray("test2")
	testBinding();
	testRecordAndField();
	testSqlIndex()
}

scriptThread.stopScript();
