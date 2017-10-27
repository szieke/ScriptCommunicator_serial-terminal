#include "searchconsole.h"
#include "ui_mainwindow.h"
#include <QScrollBar>

/**
 * Constructor.
 * @param mainWindow
 *      Pointer to the main window.
 */
SearchConsole::SearchConsole(MainWindow *mainWindow) : m_mainWindow(mainWindow), m_resultTimer(), m_lastSearchString()
{
    connect(&m_resultTimer, SIGNAL(timeout()),this, SLOT(timerSlot()));
    connect(m_mainWindow->m_userInterface->findPushButton, SIGNAL(clicked()),this, SLOT(findButtonClickedSlot()));

    connect(m_mainWindow->m_userInterface->findWhatComboBox, SIGNAL(currentTextChanged(QString)),this, SLOT(currentSearchTextChangedSlot(QString)));

    m_mainWindow->m_userInterface->findWhatComboBox->setAutoCompletion(false);
}

/**
 * Destructor.
 */
SearchConsole::~SearchConsole()
{
}

/**
 * Is called if the result timer has elapsed
 */
void SearchConsole::timerSlot(void)
{
    m_mainWindow->m_userInterface->resultLabel->setText(" ");
    m_resultTimer.stop();
}

/**
 * Returns the last search strings (separated with ;).
 */
QString SearchConsole::getLastSearchStrings(void)
{
    QString string = "";

    if(!m_lastSearchString.isEmpty())
    {
        for(auto el : m_lastSearchString)
        {
            string += el + ";";
        }

        //Remove the last ';'.
        string.remove(string.length() - 1, 1);
    }

    return string;
}

/**
 * Sets the last search strings.
 * @param string
 *  The last search strings (separated with ;).
 */
void SearchConsole::setLastSearchStrings(QString string)
{
    if(!string.isEmpty())
    {
        QStringList list = string.split(";");

        for(auto el : list)
        {
            if(!el.isEmpty())
            {
                m_mainWindow->m_userInterface->findWhatComboBox->insertItem(m_mainWindow->m_userInterface->findWhatComboBox->count(), el);
                m_lastSearchString.append(el);
            }
        }
    }
}

/**
 * Is call when the user changes the search text.
 * @param text
 *      The current search text.
 */
void SearchConsole::currentSearchTextChangedSlot(QString text)
{
    (void)text;
    activateDeactiveSearchButton();

}

/**
 * This function activtes or deactivates the search button
 * (depends on the current console tab and the current seaarch text)
 */
void SearchConsole::activateDeactiveSearchButton()
{
    QTextEdit* textEdit = 0;
    QWidget* widget = m_mainWindow->m_userInterface->tabWidget->currentWidget();
    if(widget)
    {
        textEdit = m_mainWindow->getConsoleFromCurrentTab(widget);
    }

    if(textEdit)
    {//The current tab has a console.

        if(m_mainWindow->m_userInterface->findWhatComboBox->currentText().isEmpty())
        {//The search text is empty.
            m_mainWindow->m_userInterface->findPushButton->setEnabled(false);
        }
        else
        {
            m_mainWindow->m_userInterface->findPushButton->setEnabled(true);
        }

    }
    else
    {
        m_mainWindow->m_userInterface->findPushButton->setEnabled(false);
    }

}

/**
 * Button find slot.
 */
void SearchConsole::findButtonClickedSlot(void)
{
    QTextEdit* textEdit = 0;
    QWidget* widget = m_mainWindow->m_userInterface->tabWidget->currentWidget();
    if(widget)
    {
        textEdit = m_mainWindow->getConsoleFromCurrentTab(widget);
    }

    if(textEdit && !m_mainWindow->m_userInterface->findWhatComboBox->currentText().isEmpty())
    {

        QString findText = m_mainWindow->m_userInterface->findWhatComboBox->currentText();
        int pos = -1;

        for(int index = 0; index < m_lastSearchString.length(); index++)
        {
            if(m_lastSearchString[index].indexOf(findText, 0, Qt::CaseSensitive) != -1)
            {//String found.
                pos = index;
                break;
            }
        }
        if(-1 != pos)
        {//The current search string is included in the list.
            m_lastSearchString.removeAt(pos);
            m_mainWindow->m_userInterface->findWhatComboBox->removeItem(pos);
        }

        m_mainWindow->m_userInterface->findWhatComboBox->insertItem(0, findText);
        m_lastSearchString.push_front(findText);

        if(m_lastSearchString.size() > MAX_LAST_SEARCH_STRING)
        {
            m_lastSearchString.removeLast();
            m_mainWindow->m_userInterface->findWhatComboBox->removeItem(m_mainWindow->m_userInterface->findWhatComboBox->count() - 1);
        }

        m_mainWindow->m_userInterface->findWhatComboBox->setCurrentIndex(0);

        QTextDocument::FindFlags options = 0;

        if(m_mainWindow->m_userInterface->directionUpRadioButton->isChecked())
        {
            options |= QTextDocument::FindBackward;
        }

        if(m_mainWindow->m_userInterface->matchCaseCheckBox->isChecked())
        {
            options |= QTextDocument::FindCaseSensitively;
        }

        if(m_mainWindow->m_userInterface->matchWholeWordCheckBox->isChecked())
        {
            options |= QTextDocument::FindWholeWords;
        }

        bool stringFound = textEdit->find(findText,options);

        QTextCursor savedCursor = textEdit->textCursor();
        //Store the scroll bar position.
        int savedScrollbarPosition = textEdit->verticalScrollBar()->value();
        if(!stringFound)
        {
            if(m_mainWindow->m_userInterface->directionUpRadioButton->isChecked())
            {
                textEdit->moveCursor(QTextCursor::End);
            }
            else
            {
                textEdit->moveCursor(QTextCursor::Start);
            }

            stringFound = textEdit->find(findText,options);
        }

        if(stringFound)
        {
            int cursorLine = 0;
            QTextCursor cursor = textEdit->textCursor();
            QTextBlock block = textEdit->document()->begin();
            QTextLine line ;

            //Get the number of lines from the blocks in front of the block in which the string was found.
            while (block.isValid() && (block.blockNumber() < cursor.blockNumber()))
            {
                line = block.layout()->lineForTextPosition(block.length() - 1);
                cursorLine += line.lineNumber() + 1;
                block = block.next();
            }

            //Get the line number of the found string in his block.
            line = block.layout()->lineForTextPosition(cursor.position() - block.position());
            cursorLine += line.lineNumber() + 1;

            m_mainWindow->m_userInterface->resultLabel->setText(QString("result: string found in line %1").arg(cursorLine));
            textEdit->setFocus();
        }
        else
        {
            m_mainWindow->m_userInterface->resultLabel->setText("result: string not found");
            textEdit->setTextCursor(savedCursor);
            textEdit->verticalScrollBar()->setValue(savedScrollbarPosition);
        }

        m_resultTimer.start(10000);
    }

}
