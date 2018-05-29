#ifndef SCRIPTWEBWIDGET_H
#define SCRIPTWEBWIDGET_H

#include <QObject>
#include "qwebviewplugin.h"
#include <QWebView>
#include <QWebFrame>
#include <QPrinter>
#include <QPrintDialog>
#include <QApplication>
#include <QThread>

/********************Exported functions needed by ScriptCommunicator.**********************/
/**
* Creates the wrapper classe
* @param scriptThread
*      Pointer to the script thread.
* @param customWidget
*      The custom widget.
* @param scriptRunsInDebugger
*       True if the script thread runs in a script debugger.
*/
extern "C" Q_DECL_EXPORT QObject *CreateScriptCommunicatorWidget(QObject *scriptThread, QWidget* customWidget, bool scriptRunsInDebugger);

///Returns the class name of the custom widget.
extern "C" Q_DECL_EXPORT const char* GetScriptCommunicatorWidgetName(void);


class ScriptWebWidgetSlots : public QObject
{

    Q_OBJECT
public:

    ScriptWebWidgetSlots(QWebView* webView) : QObject(webView), m_webView(webView), m_hasReturned(false)
    {

    }


public slots:
    ///Loads the specified url and displays it.
    void load(QString url){attachToWebView();m_webView->load(url);}

    ///Finds the specified string, subString, in the page, using the given options.
    void findText(QString subString, quint32 options, bool* result){*result = m_webView->findText(subString, (QWebPage::FindFlags)options);}

    ///Loads the specified url and displays it.
    void setHtml(QString html, QString baseUrl){attachToWebView();m_webView->setHtml(html, baseUrl);}

    ///Sets the given render hints on the painter if on is true; otherwise clears the render hints.
    void setRenderHint(quint32 hints){m_webView->setRenderHints((QPainter::RenderHints)hints);}

    ///Sets the value of the multiplier used to scale the text in a Web page to the factor.
    void setTextSizeMultiplier(qreal factor){m_webView->setTextSizeMultiplier(factor);}

    ///Sets the zoom factor for the view.
    void setZoomFactor(qreal factor){m_webView->setZoomFactor(factor);}

    ///Executes a javascript inside the web view.
    void evaluateJavaScript(QString script, QVariant* result, bool* hasReturned)
    {
        *hasReturned  = false;
        *result = m_webView->page()->mainFrame()->evaluateJavaScript(script);
        *hasReturned = true;
    }

    ///Opens a print dialog.
    void print(QString printDialogTitle)
    {
        QPrinter printer(QPrinter::HighResolution);
        printer.setFullPage(true);

        QPrintDialog dialog(&printer, m_webView);
        dialog.setWindowTitle(printDialogTitle);

        if (dialog.exec() == QDialog::Accepted)
        {
            m_webView->print(&printer);
        }

    }

    QVariant callWorkerScriptWithResult(QVariant params, quint32 timeOut=5000)
    {
        QDateTime start = QDateTime::currentDateTime();

        m_result  = QVariant();
        m_hasReturned = false;

        emit callWorkerScriptWithResultSignal(params, &m_result, &m_hasReturned);


        while(!m_hasReturned && (start.msecsTo(QDateTime::currentDateTime()) < timeOut) &&
              (QObject::receivers(SIGNAL(callWorkerScriptWithResultSignal(QVariant, QVariant*, bool*))) > 0))
        {//callWorkerScriptWithResult has not returned, timeout has not elapsed and
         //the signal is connected.

            QThread::yieldCurrentThread();
            QCoreApplication::processEvents();
        }


        return m_result;
    }

    void callWorkerScript(QVariant params){emit callWorkerScriptSignal(params);}

    void javaScriptWindowObjectCleared()
    {
        m_webView->page()->mainFrame()->addToJavaScriptWindowObject( QString("webView"), this );
    }



Q_SIGNALS:

    ///This signal is emitted when the webView calls the webView.callWorkerScriptWithResult(QVariant) routine.
    ///This signal is private and must not be used inside a script.
    void callWorkerScriptWithResultSignal(QVariant params, QVariant* result, bool* hasReturned);

    ///This signal is emitted when the webView calls the webView.callWorkerScriptWithResult(QVariant) routine.
    ///This signal is private and must not be used inside a script.
    void callWorkerScriptSignal(QVariant params);

private:


    void attachToWebView()
    {
        QWebFrame* frame = m_webView->page()->mainFrame();
        javaScriptWindowObjectCleared();
        connect(frame, SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));

    }

    QWebView* m_webView;

    //The variable below are used in callWorkerScriptWithResult
    //(it can happen that a timeout occurs and this function returns
    //while the connected stub is running and trys to write to the variables from callWorkerScriptWithResultSignal (the stack is not valid).
    QVariant m_result;
    bool m_hasReturned ;
};

class ResultClass : public QObject
{
    Q_OBJECT
public slots:

    void setResult(QVariant value){result = value;}

public:
    QVariant result;

};

extern QMap<QWebView*, ScriptWebWidgetSlots*> m_pointerMap;
class ScriptWebWidget : public QObject
{
    Q_OBJECT

    ///Returns a semicolon separated list with all public functions, signals and properties.
    ///Note: ScriptCommunicator uses this property for showing more information
    ///during an exception.
    Q_PROPERTY(QString publicScriptElements READ getPublicScriptElements)

public:
    ScriptWebWidget(QObject* scriptThread, QWidget* webView, bool scriptRunsInDebugger) :
        QObject(scriptThread), m_webView(static_cast<QWebView*> (webView))
    {
        //Note: If scriptRunsInDebugger is true Qt::DirectConnection instead of Qt::BlockingQueuedConnection
        //must be used (the wrapper ScriptWebWidgetSlots and the WebView widget are running in the main thread
        //if scriptRunsInDebugger true).

        qRegisterMetaType<ResultClass*>("ResultClass*");

        //Create the wrapper object and move it to the main thread.
        ScriptWebWidgetSlots* slotObject = m_pointerMap[m_webView];

        Qt::ConnectionType directConnectionType = scriptRunsInDebugger ? Qt::DirectConnection : Qt::BlockingQueuedConnection;

        connect(this, SIGNAL(loadSignal(QString)), slotObject, SLOT(load(QString)), directConnectionType);
        connect(this, SIGNAL(findTextSignal(QString,quint32,bool*)), slotObject, SLOT(findText(QString,quint32,bool*)), directConnectionType);
        connect(this, SIGNAL(setHtmlSignal( QString,QString)), slotObject, SLOT(setHtml(QString,QString)), directConnectionType);
        connect(this, SIGNAL(setRenderHintSignal(quint32)), slotObject, SLOT(setRenderHint(quint32)), directConnectionType);
        connect(this, SIGNAL(setTextSizeMultiplierSignal(qreal)), slotObject, SLOT(setTextSizeMultiplier(qreal)), directConnectionType);
        connect(this, SIGNAL(setZoomFactorSignal(qreal)), slotObject, SLOT(setZoomFactor(qreal)), directConnectionType);
        connect(this, SIGNAL(evaluateJavaScriptSignal(QString,QVariant*,bool*)), slotObject, SLOT(evaluateJavaScript(QString,QVariant*,bool*)), Qt::QueuedConnection);
        connect(this, SIGNAL(printSignal(QString)), slotObject, SLOT(print(QString)), directConnectionType);

        connect(this, SIGNAL(backSignal()), m_webView, SLOT(back()), directConnectionType);
        connect(this, SIGNAL(forwardSignal()), m_webView, SLOT(forward()), directConnectionType);
        connect(this, SIGNAL(reloadSignal()), m_webView, SLOT(reload()), directConnectionType);
        connect(this, SIGNAL(stopSignal()), m_webView, SLOT(stop()), directConnectionType);

        connect(m_webView, SIGNAL(loadFinished(bool)), this, SLOT(loadFinished_stub(bool)), Qt::QueuedConnection);
        connect(m_webView, SIGNAL(loadProgress(int)), this, SLOT(loadProgress_stub(int)), Qt::QueuedConnection);
        connect(m_webView, SIGNAL(loadStarted()), this, SLOT(loadStarted_stub()), Qt::QueuedConnection);
        connect(m_webView, SIGNAL(selectionChanged()), this, SLOT(selectionChanged_stub()), Qt::QueuedConnection);
        connect(m_webView, SIGNAL(statusBarMessage(QString)), this, SLOT(statusBarMessage_stub(QString)), Qt::QueuedConnection);
        connect(m_webView, SIGNAL(titleChanged(QString)), this, SLOT(titleChanged_stub(QString)), Qt::QueuedConnection);
        connect(m_webView, SIGNAL(urlChanged(QUrl)), this, SLOT(urlChanged_stub(QUrl)), Qt::QueuedConnection);
        connect(slotObject, SIGNAL(callWorkerScriptWithResultSignal(QVariant,QVariant*,bool*)), this, SLOT(callWorkerScriptWithResultSignal_stub(QVariant,QVariant*,bool*)), Qt::QueuedConnection);
        connect(slotObject, SIGNAL(callWorkerScriptSignal(QVariant)), this, SLOT(callWorkerScriptSignal_stub(QVariant)), Qt::QueuedConnection);

    }

    ///Returns a semicolon separated list with all public functions, signals and properties.
    ///Note: ScriptCommunicator uses this property/function for showing more information
    ///during an exception.
    virtual QString getPublicScriptElements(void)
    {
        return "void load(QString url);QString url(void);bool findText(QString subString, quint32 options = 0);"
               "bool hasSelection(void);bool isModified(void);quint32 renderHints(void);"
               "void setRenderHint(quint32 hints);QString selectedHtml(void);QString selectedText(void);"
               "void setHtml(QString html, QString baseUrl="");void setTextSizeMultiplier(qreal factor);"
               "qreal textSizeMultiplier(void);QString title(void);void setZoomFactor(qreal factor);"
               "qreal zoomFactor(void);void back(void);void forward(void);void reload(void);"
               "void stop(void);QVariant evaluateJavaScript(QString script);void print(QString printDialogTitle = 'Print');"
               "loadFinishedSignal(bool ok);loadProgressSignal(int progress);loadStartedSignal(void);"
               "selectionChangedSignal(void);statusBarMessageSignal(QString text);titleChangedSignal(QString text);"
               "urlChangedSignal(QString text);callWorkerScriptWithResultSignal(QVariant params, ResultClass* resultObject);"
               "callWorkerScriptSignal(QVariant params)"
;
    }

    ///Loads the specified url and displays it.
    Q_INVOKABLE void load(QString url){emit loadSignal(url);}

    ///Returns the url of the web page currently viewed.
    Q_INVOKABLE QString url(void){return m_webView->url().toString();}

    ///Finds the specified string, subString, in the page, using the given options.
    ///If the HighlightAllOccurrences flag is passed, the function will highlight all occurrences
    ///that exist in the page. All subsequent calls will extend the highlight, rather than replace it,
    ///with occurrences of the new string.
    ///If the HighlightAllOccurrences flag is not passed, the function will select an occurrence and
    ///all subsequent calls will replace the current occurrence with the next one.
    ///To clear the selection, just pass an empty string.
    ///Returns true if subString was found; otherwise returns false.
    ///Note: options has the type QWebPage::FindFlags.
    Q_INVOKABLE bool findText(QString subString, quint32 options = 0)
    {
        bool result = false;
        emit findTextSignal(subString, options, &result);
        return result;
    }

    ///Returns whether this page contains selected content or not.
    Q_INVOKABLE bool hasSelection(void){return m_webView->hasSelection();}

    ///Returns whether the document was modified by the user.
    Q_INVOKABLE bool isModified(void){return m_webView->isModified();}

    ///Returns the default render hints (QPainter::RenderHints) for the view.
    ///These hints are used to initialize QPainter before painting the Web page.
    Q_INVOKABLE quint32 renderHints(void){return (quint32)m_webView->renderHints();}

    ///Sets the given render hints on the painter if on is true; otherwise clears the render hints.
    Q_INVOKABLE void setRenderHint(quint32 hints){emit setRenderHintSignal(hints);}

    ///Returns the HTML currently selected.
    ///By default, this property contains an empty string.
    Q_INVOKABLE QString selectedHtml(void){return m_webView->selectedHtml();}

    ///Returns the text currently selected.
    ///By default, this property contains an empty string.
    Q_INVOKABLE QString selectedText(void){return m_webView->selectedText();}

    ///Sets the content of the web view to the specified html.
    ///External objects such as stylesheets or images referenced in the HTML document are located relative
    ///to baseUrl.
    ///The html is loaded immediately; external objects are loaded asynchronously.
    ///When using this method, WebKit assumes that external resources such as JavaScript programs or style
    ///sheets are encoded in UTF-8 unless otherwise specified. For example, the encoding of an external script
    ///can be specified through the charset attribute of the HTML script tag. Alternatively, the encoding can also
    ///be specified by the web server.
    Q_INVOKABLE void setHtml(QString html, QString baseUrl=""){emit setHtmlSignal(html, baseUrl);}

    ///Sets the value of the multiplier used to scale the text in a Web page to the factor.
    Q_INVOKABLE void setTextSizeMultiplier(qreal factor){emit setTextSizeMultiplierSignal(factor);}

    ///Returns the zoom factor for the view.
    Q_INVOKABLE qreal textSizeMultiplier(void){return m_webView->textSizeMultiplier();}

    ///Returns the title of the web page currently viewed.
    Q_INVOKABLE QString title(void){return m_webView->title();}

    ///Sets the zoom factor for the view.
    Q_INVOKABLE void setZoomFactor(qreal factor){emit setZoomFactorSignal(factor);}

    ///Returns the zoom factor for the view.
    Q_INVOKABLE qreal zoomFactor(void){return m_webView->zoomFactor();}

    ///Loads the previous document in the list of documents built by navigating links. Does nothing if there is no previous
    ///document.
    Q_INVOKABLE void back(void){emit backSignal();}

    ///Loads the next document in the list of documents built by navigating links. Does nothing if there is no next document.
    Q_INVOKABLE void forward(void){emit forwardSignal();}

    ///Reloads the current document.
    Q_INVOKABLE void reload(void){emit reloadSignal();}

    ///Stops loading the document.
    Q_INVOKABLE void stop(void){emit stopSignal();}

    ///Executes a javascript inside the web view.
    Q_INVOKABLE QVariant evaluateJavaScript(QString script)
    {
        QVariant result;
        bool hasReturned = false;

        emit evaluateJavaScriptSignal(script, &result, &hasReturned);

        while(!hasReturned)
        {
            QThread::yieldCurrentThread();
            QCoreApplication::processEvents();
        }
        return result;
    }

    ///Opens a print dialog.
    Q_INVOKABLE void print(QString printDialogTitle = "Print"){emit printSignal(printDialogTitle);}

Q_SIGNALS:

    ///This signal is emitted when a load of the page is finished. ok will
    ///indicate whether the load was successful or any error occurred.
    ///Scripts can connect to this signal.
    void loadFinishedSignal(bool ok);

    ///This signal is emitted every time an element in the web page completes loading and the overall loading progress advances.
    ///Scripts can connect to this signal.
    void loadProgressSignal(int progress);

    ///This signal is emitted when a new load of the page is started.
    ///Scripts can connect to this signal.
    void loadStartedSignal(void);

    ///This signal is emitted whenever the selection changes.
    ///Scripts can connect to this signal.
    void selectionChangedSignal(void);

    ///This signal is emitted when the status bar text is changed by the page.
    ///Scripts can connect to this signal.
    void statusBarMessageSignal(QString text);

    ///This signal is emitted whenever the title of the main frame changes.
    ///Scripts can connect to this signal.
    void titleChangedSignal(QString text);

    ///This signal is emitted when the url of the view changes.
    ///Scripts can connect to this signal.
    void urlChangedSignal(QString text);

    ///This signal is emitted when the webView calls the webView.callWorkerScriptWithResult(QVariant) routine.
    ///Scripts can connect to this signal.
    ///Note: The connected slot function blocks the WebView. Therefore no time consuming or blocking operations should be performed.
    void callWorkerScriptWithResultSignal(QVariant params, ResultClass* resultObject);

    ///This signal is emitted when the webView calls the webView.callWorkerScript(QVariant) routine.
    ///Scripts can connect to this signal.
    ///Note: The connected slot function does not block the WebView. Therefore time consuming or blocking operations can be performed.
    void callWorkerScriptSignal(QVariant params);

    ///Is emitted by the load function.
    ///This signal is private and must not be used inside a script.
    void loadSignal(QString);

    ///Is emitted by the findText function.
    ///This signal is private and must not be used inside a script.
    void findTextSignal(QString, quint32, bool*);

    ///Is emitted by the setHtml function.
    ///This signal is private and must not be used inside a script.
    void setHtmlSignal(QString, QString);

    ///Is emitted by the setRenderHint function.
    ///This signal is private and must not be used inside a script.
    void setRenderHintSignal(quint32);

    ///Is emitted by the setTextSizeMultiplier function.
    ///This signal is private and must not be used inside a script.
    void setTextSizeMultiplierSignal(qreal);

    ///Is emitted by the setZoomFactor function.
    ///This signal is private and must not be used inside a script.
    void setZoomFactorSignal(qreal);

    ///Is emitted by the back function.
    ///This signal is private and must not be used inside a script.
    void backSignal(void);

    ///Is emitted by the forward function.
    ///This signal is private and must not be used inside a script.
    void forwardSignal(void);

    ///Is emitted by the reload function.
    ///This signal is private and must not be used inside a script.
    void reloadSignal(void);

    ///Is emitted by the stop function.
    ///This signal is private and must not be used inside a script.
    void stopSignal(void);

    ///Is emitted by the evaluateJavaScript function.
    ///This signal is private and must not be used inside a script.
    void evaluateJavaScriptSignal(QString,QVariant*, bool*);

    ///Is emitted by the print function.
    ///This signal is private and must not be used inside a script.
    void printSignal(QString printDialogTitle);

public slots:

    void loadFinished_stub(bool ok){emit loadFinishedSignal(ok);}
    void loadProgress_stub(int progress){emit loadProgressSignal(progress);}
    void loadStarted_stub(void){emit loadStartedSignal();}
    void selectionChanged_stub(void){emit selectionChangedSignal();}
    void statusBarMessage_stub(QString text){emit statusBarMessageSignal(text);}
    void titleChanged_stub(QString text){emit titleChangedSignal(text);}
    void urlChanged_stub(QUrl text){emit urlChangedSignal(text.toString());}
    void callWorkerScriptWithResultSignal_stub(QVariant params, QVariant* result, bool* hasReturned)
    {
        *hasReturned = false;
        ResultClass reslutObject;
        emit callWorkerScriptWithResultSignal(params, &reslutObject);
        *result = reslutObject.result;
        *hasReturned = true;
    }
    void callWorkerScriptSignal_stub(QVariant params){emit callWorkerScriptSignal(params);}

private:
    QWebView* m_webView;

};










#endif // SCRIPTWEBWIDGET_H
