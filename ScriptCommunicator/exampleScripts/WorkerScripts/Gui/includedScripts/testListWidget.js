
function listWidgetCurrentRowChanged(currentRow)
{
	UI_testTextEdit.append("UI_listWidgetCurrentRowChanged: " + currentRow);
}
function listWidgetItemClicked(currentRow)
{
	UI_testTextEdit.append("UI_listWidgetItemClicked: " + currentRow);
}
function listWidgetItemDoubleClicked(currentRow)
{
	UI_testTextEdit.append("UI_listWidgetItemDoubleClicked: " + currentRow);
}

UI_sendListWidget.insertNewItem(UI_sendListWidget.rowCount(), "item0", scriptFile.createAbsolutePath("icons/folder.gif"));
UI_sendListWidget.insertNewItem(UI_sendListWidget.rowCount(), "item", "");
UI_sendListWidget.setItemText(UI_sendListWidget.rowCount() - 1, UI_sendListWidget.getItemText(UI_sendListWidget.rowCount() - 1) + "1");
UI_sendListWidget.setItemIcon(UI_sendListWidget.rowCount() - 1, scriptFile.createAbsolutePath("icons/openfolder.gif"));
UI_sendListWidget.seItemBackgroundColor(UI_sendListWidget.rowCount() - 1, "red")
UI_sendListWidget.insertNewItem(UI_sendListWidget.rowCount(), "item2", scriptFile.createAbsolutePath("icons/browser.ico"));

//Test remove item.
UI_sendListWidget.insertNewItem(UI_sendListWidget.rowCount(), "item2", scriptFile.createAbsolutePath("icons/browser.ico"));
UI_sendListWidget.removeItem(UI_sendListWidget.rowCount() - 1);

UI_sendListWidget.setCurrentRow(0);
UI_testTextEdit.append("current select list widget row: " + UI_sendListWidget.currentSelectedRow());
UI_sendListWidget.setFocus();
UI_sendListWidget.setItemForegroundColor(UI_sendListWidget.rowCount() - 1, "blue")

UI_sendListWidget.sortItems();

UI_sendListWidget.currentRowChangedSignal.connect(listWidgetCurrentRowChanged);
UI_sendListWidget.itemClickedSignal.connect(listWidgetItemClicked);
UI_sendListWidget.itemDoubleClickedSignal.connect(listWidgetItemDoubleClicked);

