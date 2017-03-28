
function treeWidgetItemClicked(item, column)
{
	UI_testTextEdit.append("UI_treeWidgetItemClicked: " + item.text(column));
}

function treeWidgetItemDoubleClicked(item, column)
{
	UI_testTextEdit.append("UI_treeWidgetItemDoubleClicked: " + item.text(column));
}

function currentItemChanged(current, previous)
{
	if(previous != null)
		UI_testTextEdit.append("UI_currentItemChanged: " + current.text(0) + "  " + previous.text(0));
}

UI_treeWidget.setColumnCount(UI_treeWidget.columnCount() + 1)
var headerList = Array("header 1", "header 2");
UI_treeWidget.setHeaderLabels(headerList);

var rootItem = UI_treeWidget.invisibleRootItem();


UI_treeWidget.setColumnWidth(0, 100);
var treeItem1 = UI_treeWidget.createScriptTreeWidgetItem();
treeItem1.setText(0, "value1");
treeItem1.setData(0, 0, "value 1");
treeItem1.setText(0, treeItem1.data(0, 0));
treeItem1.setItemIcon(0, scriptFile.createAbsolutePath("icons/openfolder.gif"));
treeItem1.setText(1, "value2");
treeItem1.setItemIcon(1, scriptFile.createAbsolutePath("icons/folder.gif"));
//UI_treeWidget.addTopLevelItem(treeItem1);
rootItem.addChild(treeItem1);

var treeItem2 = UI_treeWidget.createScriptTreeWidgetItem();

treeItem2.setText(0, "value3");
treeItem2.setItemIcon(0, scriptFile.createAbsolutePath("icons/browser.ico"));
treeItem2.setText(1, "value4");
treeItem2.setItemIcon(1, scriptFile.createAbsolutePath("icons/browser.ico"));
UI_treeWidget.insertTopLevelItem(UI_treeWidget.topLevelItemCount(), treeItem2)
treeItem2.setBackgroundColor(0, "red");
treeItem2.setForegroundColor(1, "blue");

var treeItem3 = UI_treeWidget.createScriptTreeWidgetItem();
treeItem3.setText(0, "value5");
treeItem3.setItemIcon(0, scriptFile.createAbsolutePath("icons/folder.gif"));
treeItem3.setText(1, "value6");
treeItem3.setItemIcon(1, scriptFile.createAbsolutePath("icons/browser.ico"));
treeItem1.addChild(treeItem3);
UI_treeWidget.expandItem(treeItem1);
treeItem1.setExpanded(treeItem1.isExpanded());

var treeItem4 = UI_treeWidget.createScriptTreeWidgetItem();
treeItem4.setText(0, "value7");
treeItem4.setItemIcon(0, scriptFile.createAbsolutePath("icons/folder.gif"));
treeItem4.setText(1, "value8");
treeItem4.setItemIcon(1, scriptFile.createAbsolutePath("icons/browser.ico"));
//treeItem2.addChild(treeItem4);
treeItem2.insertChild(treeItem2.childCount(), treeItem4);
treeItem4.setDisabled(true);
treeItem4.isDisabled();
var tmpItem = treeItem2.takeChild(0);
treeItem2.insertChild(0, tmpItem);
treeItem2.sortChildren(0, true);
var value = treeItem2.indexOfChild(tmpItem);
value = treeItem2.columnCount();
tmpItem = tmpItem.parent();

var treeItem5 = UI_treeWidget.createScriptTreeWidgetItem();
treeItem5.deleteItem();

UI_treeWidget.setCurrentItem(treeItem3);
var curentItem = UI_treeWidget.currentItem();
curentItem.setText(0, "currentItem");


tmpItem = UI_treeWidget.itemAbove(rootItem);
if(tmpItem != null)
{
	tmpItem.setText(0, "new text");
}

var tmpItem = UI_treeWidget.itemBelow(treeItem1);
if(tmpItem != null)
{
	tmpItem.setText(0, "new text");
}

tmpItem = UI_treeWidget.takeTopLevelItem(1);
UI_treeWidget.insertTopLevelItem(1, tmpItem)
tmpItem = UI_treeWidget.topLevelItem(1);
tmpItem.setText(1, "new text 2");


UI_treeWidget.setColumnWidth(0, UI_treeWidget.getColumnWidth(0) + 1);
UI_treeWidget.resizeColumnToContents(0);

UI_treeWidget.expandAll();

UI_treeWidget.sortItems(0);

UI_treeWidget.itemClickedSignal.connect(treeWidgetItemClicked);
UI_treeWidget.itemDoubleClickedSignal.connect(treeWidgetItemDoubleClicked);
UI_treeWidget.currentItemChangedSignal.connect(currentItemChanged);


