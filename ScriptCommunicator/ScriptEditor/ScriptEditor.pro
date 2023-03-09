
INCLUDEPATH += QScintilla_src-2.13.4/scintilla/include
INCLUDEPATH += QScintilla_src-2.13.4/scintilla/lexlib
INCLUDEPATH += QScintilla_src-2.13.4/scintilla/src
INCLUDEPATH += QScintilla_src-2.13.4/src

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
    QScintilla_src-2.13.4/scintilla/include/ILexer.h \
    QScintilla_src-2.13.4/scintilla/include/ILoader.h \
    QScintilla_src-2.13.4/scintilla/include/Platform.h \
    QScintilla_src-2.13.4/scintilla/include/SciLexer.h \
    QScintilla_src-2.13.4/scintilla/include/Sci_Position.h \
    QScintilla_src-2.13.4/scintilla/include/Scintilla.h \
    QScintilla_src-2.13.4/scintilla/include/ScintillaWidget.h \
    QScintilla_src-2.13.4/scintilla/lexlib/Accessor.h \
    QScintilla_src-2.13.4/scintilla/lexlib/CharacterCategory.h \
    QScintilla_src-2.13.4/scintilla/lexlib/CharacterSet.h \
    QScintilla_src-2.13.4/scintilla/lexlib/DefaultLexer.h \
    QScintilla_src-2.13.4/scintilla/lexlib/LexAccessor.h \
    QScintilla_src-2.13.4/scintilla/lexlib/LexerBase.h \
    QScintilla_src-2.13.4/scintilla/lexlib/LexerModule.h \
    QScintilla_src-2.13.4/scintilla/lexlib/LexerNoExceptions.h \
    QScintilla_src-2.13.4/scintilla/lexlib/LexerSimple.h \
    QScintilla_src-2.13.4/scintilla/lexlib/OptionSet.h \
    QScintilla_src-2.13.4/scintilla/lexlib/PropSetSimple.h \
    QScintilla_src-2.13.4/scintilla/lexlib/SparseState.h \
    QScintilla_src-2.13.4/scintilla/lexlib/StringCopy.h \
    QScintilla_src-2.13.4/scintilla/lexlib/StyleContext.h \
    QScintilla_src-2.13.4/scintilla/lexlib/SubStyles.h \
    QScintilla_src-2.13.4/scintilla/lexlib/WordList.h \
    QScintilla_src-2.13.4/scintilla/src/AutoComplete.h \
    QScintilla_src-2.13.4/scintilla/src/CallTip.h \
    QScintilla_src-2.13.4/scintilla/src/CaseConvert.h \
    QScintilla_src-2.13.4/scintilla/src/CaseFolder.h \
    QScintilla_src-2.13.4/scintilla/src/Catalogue.h \
    QScintilla_src-2.13.4/scintilla/src/CellBuffer.h \
    QScintilla_src-2.13.4/scintilla/src/CharClassify.h \
    QScintilla_src-2.13.4/scintilla/src/ContractionState.h \
    QScintilla_src-2.13.4/scintilla/src/DBCS.h \
    QScintilla_src-2.13.4/scintilla/src/Decoration.h \
    QScintilla_src-2.13.4/scintilla/src/Document.h \
    QScintilla_src-2.13.4/scintilla/src/EditModel.h \
    QScintilla_src-2.13.4/scintilla/src/EditView.h \
    QScintilla_src-2.13.4/scintilla/src/Editor.h \
    QScintilla_src-2.13.4/scintilla/src/ElapsedPeriod.h \
    QScintilla_src-2.13.4/scintilla/src/ExternalLexer.h \
    QScintilla_src-2.13.4/scintilla/src/FontQuality.h \
    QScintilla_src-2.13.4/scintilla/src/Indicator.h \
    QScintilla_src-2.13.4/scintilla/src/IntegerRectangle.h \
    QScintilla_src-2.13.4/scintilla/src/KeyMap.h \
    QScintilla_src-2.13.4/scintilla/src/LineMarker.h \
    QScintilla_src-2.13.4/scintilla/src/MarginView.h \
    QScintilla_src-2.13.4/scintilla/src/Partitioning.h \
    QScintilla_src-2.13.4/scintilla/src/PerLine.h \
    QScintilla_src-2.13.4/scintilla/src/Position.h \
    QScintilla_src-2.13.4/scintilla/src/PositionCache.h \
    QScintilla_src-2.13.4/scintilla/src/RESearch.h \
    QScintilla_src-2.13.4/scintilla/src/RunStyles.h \
    QScintilla_src-2.13.4/scintilla/src/ScintillaBase.h \
    QScintilla_src-2.13.4/scintilla/src/Selection.h \
    QScintilla_src-2.13.4/scintilla/src/SparseVector.h \
    QScintilla_src-2.13.4/scintilla/src/SplitVector.h \
    QScintilla_src-2.13.4/scintilla/src/Style.h \
    QScintilla_src-2.13.4/scintilla/src/UniConversion.h \
    QScintilla_src-2.13.4/scintilla/src/UniqueString.h \
    QScintilla_src-2.13.4/scintilla/src/ViewStyle.h \
    QScintilla_src-2.13.4/scintilla/src/XPM.h \
    QScintilla_src-2.13.4/src/ListBoxQt.h \
    QScintilla_src-2.13.4/src/Qsci/qsciabstractapis.h \
    QScintilla_src-2.13.4/src/Qsci/qsciapis.h \
    QScintilla_src-2.13.4/src/Qsci/qscicommand.h \
    QScintilla_src-2.13.4/src/Qsci/qscicommandset.h \
    QScintilla_src-2.13.4/src/Qsci/qscidocument.h \
    QScintilla_src-2.13.4/src/Qsci/qsciglobal.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexer.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexeravs.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerbash.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerbatch.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexercmake.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexercoffeescript.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexercpp.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexercsharp.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexercss.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexercustom.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerd.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerdiff.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexeredifact.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerfortran.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerfortran77.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerhtml.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexeridl.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerjava.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerjavascript.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerjson.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerlua.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexermakefile.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexermarkdown.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexermatlab.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexeroctave.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerpascal.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerperl.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerpo.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerpostscript.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerpov.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerproperties.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerpython.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerruby.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerspice.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexersql.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexertcl.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexertex.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerverilog.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexervhdl.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexerxml.h \
    QScintilla_src-2.13.4/src/Qsci/qscilexeryaml.h \
    QScintilla_src-2.13.4/src/Qsci/qscimacro.h \
    QScintilla_src-2.13.4/src/Qsci/qsciprinter.h \
    QScintilla_src-2.13.4/src/Qsci/qsciscintilla.h \
    QScintilla_src-2.13.4/src/Qsci/qsciscintillabase.h \
    QScintilla_src-2.13.4/src/Qsci/qscistyle.h \
    QScintilla_src-2.13.4/src/Qsci/qscistyledtext.h \
    QScintilla_src-2.13.4/src/SciAccessibility.h \
    QScintilla_src-2.13.4/src/SciClasses.h \
    QScintilla_src-2.13.4/src/ScintillaQt.h \
    findDialog.h \
    mainwindow.h \
    singledocument.h \
    parseThread.h \
    version.h \
    esprima/esprima.h \
    esprima/esprimaparsefunctions.h
SOURCES      = main.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexA68k.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexAPDL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexASY.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexAU3.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexAVE.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexAVS.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexAbaqus.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexAda.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexAsm.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexAsn1.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexBaan.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexBash.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexBasic.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexBatch.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexBibTeX.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexBullant.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexCLW.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexCOBOL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexCPP.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexCSS.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexCaml.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexCmake.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexCoffeeScript.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexConf.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexCrontab.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexCsound.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexD.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexDMAP.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexDMIS.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexDiff.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexECL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexEDIFACT.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexEScript.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexEiffel.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexErlang.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexErrorList.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexFlagship.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexForth.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexFortran.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexGAP.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexGui4Cli.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexHTML.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexHaskell.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexHex.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexIndent.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexInno.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexJSON.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexKVIrc.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexKix.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexLPeg.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexLaTeX.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexLisp.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexLout.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexLua.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMMIXAL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMPT.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMSSQL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMagik.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMake.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMarkdown.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMatlab.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMaxima.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMetapost.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexModula.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexMySQL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexNimrod.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexNsis.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexNull.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexOScript.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexOpal.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPB.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPLM.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPO.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPOV.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPS.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPascal.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPerl.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPowerPro.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPowerShell.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexProgress.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexProps.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexPython.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexR.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexRebol.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexRegistry.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexRuby.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexRust.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexSAS.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexSML.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexSQL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexSTTXT.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexScriptol.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexSmalltalk.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexSorcus.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexSpecman.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexSpice.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexStata.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexTACL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexTADS3.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexTAL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexTCL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexTCMD.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexTeX.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexTxt2tags.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexVB.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexVHDL.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexVerilog.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexVisualProlog.cpp \
    QScintilla_src-2.13.4/scintilla/lexers/LexYAML.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/Accessor.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/CharacterCategory.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/CharacterSet.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/DefaultLexer.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/LexerBase.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/LexerModule.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/LexerNoExceptions.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/LexerSimple.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/PropSetSimple.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/StyleContext.cpp \
    QScintilla_src-2.13.4/scintilla/lexlib/WordList.cpp \
    QScintilla_src-2.13.4/scintilla/src/AutoComplete.cpp \
    QScintilla_src-2.13.4/scintilla/src/CallTip.cpp \
    QScintilla_src-2.13.4/scintilla/src/CaseConvert.cpp \
    QScintilla_src-2.13.4/scintilla/src/CaseFolder.cpp \
    QScintilla_src-2.13.4/scintilla/src/Catalogue.cpp \
    QScintilla_src-2.13.4/scintilla/src/CellBuffer.cpp \
    QScintilla_src-2.13.4/scintilla/src/CharClassify.cpp \
    QScintilla_src-2.13.4/scintilla/src/ContractionState.cpp \
    QScintilla_src-2.13.4/scintilla/src/DBCS.cpp \
    QScintilla_src-2.13.4/scintilla/src/Decoration.cpp \
    QScintilla_src-2.13.4/scintilla/src/Document.cpp \
    QScintilla_src-2.13.4/scintilla/src/EditModel.cpp \
    QScintilla_src-2.13.4/scintilla/src/EditView.cpp \
    QScintilla_src-2.13.4/scintilla/src/Editor.cpp \
    QScintilla_src-2.13.4/scintilla/src/ExternalLexer.cpp \
    QScintilla_src-2.13.4/scintilla/src/Indicator.cpp \
    QScintilla_src-2.13.4/scintilla/src/KeyMap.cpp \
    QScintilla_src-2.13.4/scintilla/src/LineMarker.cpp \
    QScintilla_src-2.13.4/scintilla/src/MarginView.cpp \
    QScintilla_src-2.13.4/scintilla/src/PerLine.cpp \
    QScintilla_src-2.13.4/scintilla/src/PositionCache.cpp \
    QScintilla_src-2.13.4/scintilla/src/RESearch.cpp \
    QScintilla_src-2.13.4/scintilla/src/RunStyles.cpp \
    QScintilla_src-2.13.4/scintilla/src/ScintillaBase.cpp \
    QScintilla_src-2.13.4/scintilla/src/Selection.cpp \
    QScintilla_src-2.13.4/scintilla/src/Style.cpp \
    QScintilla_src-2.13.4/scintilla/src/UniConversion.cpp \
    QScintilla_src-2.13.4/scintilla/src/ViewStyle.cpp \
    QScintilla_src-2.13.4/scintilla/src/XPM.cpp \
    QScintilla_src-2.13.4/src/InputMethod.cpp \
    QScintilla_src-2.13.4/src/ListBoxQt.cpp \
    QScintilla_src-2.13.4/src/MacPasteboardMime.cpp \
    QScintilla_src-2.13.4/src/PlatQt.cpp \
    QScintilla_src-2.13.4/src/SciAccessibility.cpp \
    QScintilla_src-2.13.4/src/SciClasses.cpp \
    QScintilla_src-2.13.4/src/ScintillaQt.cpp \
    QScintilla_src-2.13.4/src/qsciabstractapis.cpp \
    QScintilla_src-2.13.4/src/qsciapis.cpp \
    QScintilla_src-2.13.4/src/qscicommand.cpp \
    QScintilla_src-2.13.4/src/qscicommandset.cpp \
    QScintilla_src-2.13.4/src/qscidocument.cpp \
    QScintilla_src-2.13.4/src/qscilexer.cpp \
    QScintilla_src-2.13.4/src/qscilexeravs.cpp \
    QScintilla_src-2.13.4/src/qscilexerbash.cpp \
    QScintilla_src-2.13.4/src/qscilexerbatch.cpp \
    QScintilla_src-2.13.4/src/qscilexercmake.cpp \
    QScintilla_src-2.13.4/src/qscilexercoffeescript.cpp \
    QScintilla_src-2.13.4/src/qscilexercpp.cpp \
    QScintilla_src-2.13.4/src/qscilexercsharp.cpp \
    QScintilla_src-2.13.4/src/qscilexercss.cpp \
    QScintilla_src-2.13.4/src/qscilexercustom.cpp \
    QScintilla_src-2.13.4/src/qscilexerd.cpp \
    QScintilla_src-2.13.4/src/qscilexerdiff.cpp \
    QScintilla_src-2.13.4/src/qscilexeredifact.cpp \
    QScintilla_src-2.13.4/src/qscilexerfortran.cpp \
    QScintilla_src-2.13.4/src/qscilexerfortran77.cpp \
    QScintilla_src-2.13.4/src/qscilexerhtml.cpp \
    QScintilla_src-2.13.4/src/qscilexeridl.cpp \
    QScintilla_src-2.13.4/src/qscilexerjava.cpp \
    QScintilla_src-2.13.4/src/qscilexerjavascript.cpp \
    QScintilla_src-2.13.4/src/qscilexerjson.cpp \
    QScintilla_src-2.13.4/src/qscilexerlua.cpp \
    QScintilla_src-2.13.4/src/qscilexermakefile.cpp \
    QScintilla_src-2.13.4/src/qscilexermarkdown.cpp \
    QScintilla_src-2.13.4/src/qscilexermatlab.cpp \
    QScintilla_src-2.13.4/src/qscilexeroctave.cpp \
    QScintilla_src-2.13.4/src/qscilexerpascal.cpp \
    QScintilla_src-2.13.4/src/qscilexerperl.cpp \
    QScintilla_src-2.13.4/src/qscilexerpo.cpp \
    QScintilla_src-2.13.4/src/qscilexerpostscript.cpp \
    QScintilla_src-2.13.4/src/qscilexerpov.cpp \
    QScintilla_src-2.13.4/src/qscilexerproperties.cpp \
    QScintilla_src-2.13.4/src/qscilexerpython.cpp \
    QScintilla_src-2.13.4/src/qscilexerruby.cpp \
    QScintilla_src-2.13.4/src/qscilexerspice.cpp \
    QScintilla_src-2.13.4/src/qscilexersql.cpp \
    QScintilla_src-2.13.4/src/qscilexertcl.cpp \
    QScintilla_src-2.13.4/src/qscilexertex.cpp \
    QScintilla_src-2.13.4/src/qscilexerverilog.cpp \
    QScintilla_src-2.13.4/src/qscilexervhdl.cpp \
    QScintilla_src-2.13.4/src/qscilexerxml.cpp \
    QScintilla_src-2.13.4/src/qscilexeryaml.cpp \
    QScintilla_src-2.13.4/src/qscimacro.cpp \
    QScintilla_src-2.13.4/src/qsciprinter.cpp \
    QScintilla_src-2.13.4/src/qsciscintilla.cpp \
    QScintilla_src-2.13.4/src/qsciscintillabase.cpp \
    QScintilla_src-2.13.4/src/qscistyle.cpp \
    QScintilla_src-2.13.4/src/qscistyledtext.cpp \
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
    QScintilla_src-2.13.4/scintilla/include/License.txt \
    QScintilla_src-2.13.4/scintilla/include/Scintilla.iface \
    QScintilla_src-2.13.4/scintilla/lexers/License.txt \
    QScintilla_src-2.13.4/scintilla/lexlib/License.txt \
    QScintilla_src-2.13.4/scintilla/src/License.txt \
    QScintilla_src-2.13.4/scintilla/src/SciTE.properties \
    QScintilla_src-2.13.4/src/features/qscintilla2.prf \
    QScintilla_src-2.13.4/src/features_staticlib/qscintilla2.prf \
    QScintilla_src-2.13.4/src/qscintilla_cs.qm \
    QScintilla_src-2.13.4/src/qscintilla_cs.ts \
    QScintilla_src-2.13.4/src/qscintilla_de.qm \
    QScintilla_src-2.13.4/src/qscintilla_de.ts \
    QScintilla_src-2.13.4/src/qscintilla_es.qm \
    QScintilla_src-2.13.4/src/qscintilla_es.ts \
    QScintilla_src-2.13.4/src/qscintilla_fr.qm \
    QScintilla_src-2.13.4/src/qscintilla_fr.ts \
    QScintilla_src-2.13.4/src/qscintilla_pt_br.qm \
    QScintilla_src-2.13.4/src/qscintilla_pt_br.ts \
    images/font.png \
    images/close.png

SUBDIRS += \
  QScintilla_src-2.13.4/src/qscintilla.pro
