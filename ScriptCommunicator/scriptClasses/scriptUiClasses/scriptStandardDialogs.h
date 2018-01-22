#ifndef SCRIPTSTANDARDDIALOGS_H
#define SCRIPTSTANDARDDIALOGS_H

#include <QObject>
#include <QMessageBox>

class ScriptWindow;

class ScriptStandardDialogs : public QObject
{
    Q_OBJECT
public:
    ScriptStandardDialogs(QObject *parent);

    ///Connects all signals.
    void intSignals(ScriptWindow* scriptWindow, bool runsInDebugger);


    ///Wrapper for QFileDialog::getSaveFileName and QFileDialog::getOpenFileName.
    QString showFileDialog(bool isSaveDialog, QString caption, QString dir, QString filter, QWidget* parent);

    ///Wrapper for QFileDialog::getOpenFileNames.
    QStringList showOpenFileNamesDialog(QString caption, QString dir, QString filter, QWidget* parent);

    ///Wrapper for QFileDialog::getExistingDirectory.
    QString showDirectoryDialog(QString caption, QString dir, QWidget* parent);

    ///Shows a message box.
    void messageBox(QString icon, QString title, QString text, QWidget* parent);

    ///Shows a yes/no dialog.
    bool showYesNoDialog(QString icon, QString title, QString text, QWidget* parent);

    ///Convenience function to get a string from the user.
    ///Shows a QInputDialog::getText dialog (line edit).
    QString showTextInputDialog(QString title, QString label, QString displayedText, QWidget* parent);

    ///Convenience function to get a multiline string from the user.
    ///Shows a QInputDialog::getMultiLineText dialog (plain text edit).
    QString showMultiLineTextInputDialog(QString title, QString label, QString displayedText, QWidget* parent);

    ///Convenience function to let the user select an item from a string list.
    ///Shows a QInputDialog::getItem dialog (combobox).
    QString showGetItemDialog(QString title, QString label, QStringList displayedItems,
                               int currentItemIndex, bool editable, QWidget* parent);

    ///Convenience function to get an integer input from the user.
    ///Shows a QInputDialog::getInt dialog (spinbox).
    QList<int> showGetIntDialog(QString title, QString label, int initialValue, int min, int max, int step, QWidget* parent);

    ///Convenience function to get a floating point number from the user.
    ///Shows a QInputDialog::getDouble dialog (spinbox).
    QList<double> showGetDoubleDialog(QString title, QString label, double initialValue, double min, double max, int decimals, QWidget* parent);

    ///Convenience function to get color settings from the user.
    ///Shows a color_widgets::ColorDialog dialog.
    QList<int> showColorDialog(quint8 initInitalRed, quint8 initInitalGreen, quint8 initInitalBlue, quint8 initInitalAlpha, bool alphaIsEnabled, QWidget* parent);

signals:

    ///Is connected with MainWindow::showFileDialogSlot (shows a file dialog).
    ///This function must not be used from script.
    void showFileDialogSignal(bool isSaveDialog, QString caption, QString dir, QString filter, QString *resultFileName, QWidget* parent);

    ///Is connected with MainWindow::showOpenFileNamesSlot (shows a file dialog).
    ///This function must not be used from script.
    void showOpenFileNamesDialogSignal(QString caption, QString dir, QString filter, QStringList *resultFileNames, QWidget* parent);

    ///Is connected with MainWindow::showDirectoryDialogSlot (shows a file dialog).
    ///This function must not be used from script.
    void showDirectoryDialogSignal(QString caption, QString dir, QString *directoryName, QWidget* parent);

    ///Is connected with MainWindow::showMessageBoxSlot (shows a message box).
    ///This function must not be used from script.
    void showMessageBoxSignal(QMessageBox::Icon icon, QString title, QString text, QMessageBox::StandardButtons buttons, QWidget *parent);

    ///Is connected with MainWindow::showYesNoDialogSlot (shows a yes/no dialog).
    ///This function must not be used from script.
    void showYesNoDialogSignal(QMessageBox::Icon icon, QString title, QString text,  QWidget *parent, bool* yesButtonPressed);

    ///This event is emitted in showTextInputDialog.
    ///This function must not be used from script.
    void showTextInputDialogSignal(QString title, QString label, QString displayedText, QString* result, QWidget* parent);

    ///This event is emitted in showMultiLineTextInputDialog.
    ///This function must not be used from script.
    void showMultiLineTextInputDialogSignal(QString title, QString label, QString displayedText, QString* result, QWidget* parent);

    ///This event is emitted in showGetItemDialog.
    ///This function must not be used from script.
    void showGetItemDialogSignal(QString title, QString label, QStringList displayedItems,
                               int currentItemIndex, bool editable, QString* result, QWidget* parent);

    ///This event is emitted in showGetIntDialog.
    ///This function must not be used from script.
    void showGetIntDialogSignal(QString title, QString label, int initialValue, int min, int max, int step,
                                            int* result, bool* okPressed, QWidget* parent);

    ///This event is emitted in showGetDoubleDialog.
    ///This function must not be used from script.
    void showGetDoubleDialogSignal(QString title, QString label, double initialValue, double min, double max,
                                   int decimals, double* result, bool* okPressed, QWidget* parent);

    ///This event is emitted in showColorDialog.
    ///This function must not be used from script.
    void showColorDialogSignal(quint8 initInitalRed, quint8 initInitalGreen, quint8 initInitalBlue, quint8 initInitalAlpha,
                                     bool alphaIsEnabled, QWidget* parent, QList<int>* result);

    ///Disables all mouse events for all windows.
    ///This function must not be used from script.
    void disableMouseEventsSignal(void);

    ///Enables all mouse events for all windows.
    ///This function must not be used from script.
    void enableMouseEventsSignal(void);

public slots:

};

#endif // SCRIPTSTANDARDDIALOGS_H
