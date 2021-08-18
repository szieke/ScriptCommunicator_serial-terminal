
INCLUDEPATH += QScintilla include src lexlib
DEFINES += SCINTILLA_QT SCI_LEXER
CONFIG += qt warn_off thread exceptions

# Comment this in if you want the internal Scintilla classes to be placed in a
# Scintilla namespace rather than pollute the global namespace.
#DEFINES += SCI_NAMESPACE

QT += widgets
QT += xml
QT += printsupport
QT += gui


RC_FILE = images/ScriptEditor.rc
macx:QT += macextras
CONFIG += c++11

unix{
QMAKE_RPATHDIR += lib
}


HEADERS      = \
    QScintilla/Qsci/qsciabstractapis.h \
    QScintilla/Qsci/qsciapis.h \
    QScintilla/Qsci/qscicommand.h \
    QScintilla/Qsci/qscicommandset.h \
    QScintilla/Qsci/qscidocument.h \
    QScintilla/Qsci/qsciglobal.h \
    QScintilla/Qsci/qscilexer.h \
    QScintilla/Qsci/qscilexercpp.h \
    QScintilla/Qsci/qscilexercustom.h \
    QScintilla/Qsci/qscilexerjavascript.h \
    QScintilla/Qsci/qscimacro.h \
    QScintilla/Qsci/qsciprinter.h \
    QScintilla/Qsci/qsciscintilla.h \
    QScintilla/Qsci/qsciscintillabase.h \
    QScintilla/Qsci/qscistyle.h \
    QScintilla/Qsci/qscistyledtext.h \
    QScintilla/ListBoxQt.h \
    QScintilla/SciClasses.h \
    QScintilla/SciNamespace.h \
    QScintilla/ScintillaQt.h \
    include/ILexer.h \
    include/Platform.h \
    include/SciLexer.h \
    include/Scintilla.h \
    include/ScintillaWidget.h \
    src/AutoComplete.h \
    src/CallTip.h \
    src/CaseConvert.h \
    src/CaseFolder.h \
    src/Catalogue.h \
    src/CellBuffer.h \
    src/CharClassify.h \
    src/ContractionState.h \
    src/Decoration.h \
    src/Document.h \
    src/EditModel.h \
    src/Editor.h \
    src/EditView.h \
    src/ExternalLexer.h \
    src/FontQuality.h \
    src/Indicator.h \
    src/KeyMap.h \
    src/LineMarker.h \
    src/MarginView.h \
    src/Partitioning.h \
    src/PerLine.h \
    src/PositionCache.h \
    src/RESearch.h \
    src/RunStyles.h \
    src/ScintillaBase.h \
    src/Selection.h \
    src/SplitVector.h \
    src/Style.h \
    src/UnicodeFromUTF8.h \
    src/UniConversion.h \
    src/ViewStyle.h \
    src/XPM.h \
    lexlib/Accessor.h \
    lexlib/CharacterCategory.h \
    lexlib/CharacterSet.h \
    lexlib/LexAccessor.h \
    lexlib/LexerBase.h \
    lexlib/LexerModule.h \
    lexlib/LexerNoExceptions.h \
    lexlib/LexerSimple.h \
    lexlib/OptionSet.h \
    lexlib/PropSetSimple.h \
    lexlib/SparseState.h \
    lexlib/StringCopy.h \
    lexlib/StyleContext.h \
    lexlib/SubStyles.h \
    lexlib/WordList.h \
    findDialog.h \
    mainwindow.h \
    singledocument.h \
    parseThread.h \
    version.h \
    esprima/esprima.h \
    esprima/esprimaparsefunctions.h
SOURCES      = main.cpp \
    QScintilla/InputMethod.cpp \
    QScintilla/ListBoxQt.cpp \
    QScintilla/MacPasteboardMime.cpp \
    QScintilla/PlatQt.cpp \
    QScintilla/qsciabstractapis.cpp \
    QScintilla/qsciapis.cpp \
    QScintilla/qscicommand.cpp \
    QScintilla/qscicommandset.cpp \
    QScintilla/qscidocument.cpp \
    QScintilla/qscilexer.cpp \
    QScintilla/qscilexercpp.cpp \
    QScintilla/qscilexercustom.cpp \
    QScintilla/qscilexerjavascript.cpp \
    QScintilla/qscimacro.cpp \
    QScintilla/qsciprinter.cpp \
    QScintilla/qsciscintilla.cpp \
    QScintilla/qsciscintillabase.cpp \
    QScintilla/qscistyle.cpp \
    QScintilla/qscistyledtext.cpp \
    QScintilla/SciClasses.cpp \
    QScintilla/ScintillaQt.cpp \
    src/AutoComplete.cpp \
    src/CallTip.cpp \
    src/CaseConvert.cpp \
    src/CaseFolder.cpp \
    src/Catalogue.cpp \
    src/CellBuffer.cpp \
    src/CharClassify.cpp \
    src/ContractionState.cpp \
    src/Decoration.cpp \
    src/Document.cpp \
    src/EditModel.cpp \
    src/Editor.cpp \
    src/EditView.cpp \
    src/ExternalLexer.cpp \
    src/Indicator.cpp \
    src/KeyMap.cpp \
    src/LineMarker.cpp \
    src/MarginView.cpp \
    src/PerLine.cpp \
    src/PositionCache.cpp \
    src/RESearch.cpp \
    src/RunStyles.cpp \
    src/ScintillaBase.cpp \
    src/Selection.cpp \
    src/Style.cpp \
    src/UniConversion.cpp \
    src/ViewStyle.cpp \
    src/XPM.cpp \
    lexlib/Accessor.cpp \
    lexlib/CharacterCategory.cpp \
    lexlib/CharacterSet.cpp \
    lexlib/LexerBase.cpp \
    lexlib/LexerModule.cpp \
    lexlib/LexerNoExceptions.cpp \
    lexlib/LexerSimple.cpp \
    lexlib/PropSetSimple.cpp \
    lexlib/StyleContext.cpp \
    lexlib/WordList.cpp \
    lexers/LexCPP.cpp \
    findDialog.cpp \
    mainwindow.cpp \
    singledocument.cpp \
    parseThread.cpp \
    esprima/esprima.cpp


RESOURCES    = ScriptEditor.qrc

FORMS += \
    findDialog.ui \
    mainwindow.ui

DISTFILES += \
    images/font.png \
    images/close.png
