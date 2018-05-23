To execute the web view examples following must be done:
- add one example script file in the script window
- download the extra libraries for your platform (https://sourceforge.net/projects/scriptcommunicator/files/AdditionalLibraries/ScriptWebView)
	- Windows and Linux: copy the libraries into the ScriptCommunicator folder 
	- Mac OS X: copy the libraries into ScriptCommunicator.app/Contents/Frameworks

WorkerScript API:

///Loads the specified url and displays it.
void load(QString url)

///Returns the url of the web page currently viewed.
QString url(void)

///Finds the specified string, subString, in the page, using the given options.
///If the HighlightAllOccurrences flag is passed, the function will highlight all occurrences
///that exist in the page. All subsequent calls will extend the highlight, rather than replace it,
///with occurrences of the new string.
///If the HighlightAllOccurrences flag is not passed, the function will select an occurrence and
///all subsequent calls will replace the current occurrence with the next one.
///To clear the selection, just pass an empty string.
///Returns true if subString was found; otherwise returns false.
///Note: options has the type QWebPage::FindFlags.
bool findText(QString subString, quint32 options = 0)

///Returns whether this page contains selected content or not.
bool hasSelection(void)

///Returns whether the document was modified by the user.
bool isModified(void)

///Returns the default render hints (QPainter::RenderHints) for the view.
///These hints are used to initialize QPainter before painting the Web page.
quint32 renderHints(void)

///Sets the given render hints on the painter if on is true; otherwise clears the render hints.
void setRenderHint(quint32 hints)

///Returns the HTML currently selected.
///By default, this property contains an empty string.
QString selectedHtml(void)

///Returns the text currently selected.
///By default, this property contains an empty string.
QString selectedText(void)

///Sets the content of the web view to the specified html.
///External objects such as stylesheets or images referenced in the HTML document are located relative
///to baseUrl.
///The html is loaded immediately; external objects are loaded asynchronously.
///When using this method, WebKit assumes that external resources such as JavaScript programs or style
///sheets are encoded in UTF-8 unless otherwise specified. For example, the encoding of an external script
///can be specified through the charset attribute of the HTML script tag. Alternatively, the encoding can also
///be specified by the web server.
void setHtml(QString html, QString baseUrl="")

///Sets the value of the multiplier used to scale the text in a Web page to the factor.
void setTextSizeMultiplier(qreal factor){emit setTextSizeMultiplierSignal(m_webView, factor);}

///Returns the zoom factor for the view.
qreal textSizeMultiplier(void)

///Returns the title of the web page currently viewed.
QString title(void)

///Sets the zoom factor for the view.
void setZoomFactor(qreal factor)

///Returns the zoom factor for the view.
qreal zoomFactor(void)

///Loads the previous document in the list of documents built by navigating links. Does nothing if there is no previous
///document.
void back(void)

///Loads the next document in the list of documents built by navigating links. Does nothing if there is no next document.
void forward(void)

///Reloads the current document.
void reload(void)

///Stops loading the document.
void stop(void)

///Executes a javascript inside the web view.
QVariant evaluateJavaScript(QString script)

///Opens a print dialog.
void print(QString printDialogTitle = "Print")

///This signal is emitted when a load of the page is finished. ok will
///indicate whether the load was successful or any error occurred.
void loadFinishedSignal(bool ok);

///This signal is emitted every time an element in the web page completes loading and the overall loading progress advances.
void loadProgressSignal(int progress);

///This signal is emitted when a new load of the page is started.
void loadStartedSignal(void);

///This signal is emitted whenever the selection changes.
void selectionChangedSignal(void);

///This signal is emitted when the status bar text is changed by the page.
void statusBarMessageSignal(QString text);

///This signal is emitted whenever the title of the main frame changes.
void titleChangedSignal(QString text);

///This signal is emitted when the url of the view changes.
void urlChangedSignal(QString text);

///This signal is emitted when the script inside the web page calls the webView.callWorkerScriptWithResult(QVariant params, quint32 timeOut=5000) routine.
///To return a value ResultClass::setResult(QVariant value) must be used.
///Note: The connected slot function blocks the WebView. Therefore no time consuming or blocking operations should be performed.
void callWorkerScriptWithResultSignal(QVariant params, ResultClass* resultObject);

///This signal is emitted when the script inside the web page calls the webView.callWorkerScript(QVariant params) routine.
///Note: The connected slot function does not block the WebView. Therefore time consuming or blocking operations can be performed.
void callWorkerScriptSignal(QVariant params);

Note:
The script inside the web page can call WorkerScript functions. To do this the script can call: 
- QVariant webView.callWorkerScriptWithResult(QVariant params, quint32 timeOut=5000)
- void webView.callWorkerScript(QVariant params)
The function webView.callWorkerScriptWithResult emits callWorkerScriptWithResultSignal and webView.callWorkerScript emits callWorkerScriptSignal.
For an example see webView_example2.js.
Important: webView.callWorkerScriptWithResult blocks until the connected WorkerScript function returns or the the time out (quint32 timeOut=5000) has elapsed.
