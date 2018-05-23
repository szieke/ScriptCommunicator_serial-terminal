
Terminal.applyAddon(fullscreen);

var shellprompt = '$ ';
var counter = 0;
var term = new Terminal({cursorBlink: false, scrollback : 10000});
term.open(document.getElementById('terminal-container'));

var body = document.body,
    html = document.documentElement;

body.addEventListener("resize", fit);				   

term.setOption('fontFamily', "courier new, courier, monospace");
term.setOption('fontSize', 12);

term.setOption('cursorBlink', true);
//echo an/aus
//term.setOption('fontWeight', 550)
term.setOption('scrollback', 10000);

var theme = {};
//theme.foreground = '#000000';
//theme.background = '#f0f0f0';
theme.foreground = '#ffffff';
theme.background = '#000000';
theme.cursor = theme.foreground;
theme.cursorAccent = theme.background;
theme.selection = 'rgba(255, 255, 255, 0.3)';
theme.ansi = ['#2e3436', '#cc0000', '#4e9a06', '#c4a000', '#3465a4', '#75507b', '#06989a', '#d3d7cf',
'#555753', '#ef2929', '#8ae234', '#fce94f', '#729fcf', '#ad7fa8', '#34e2e2', '#eeeeec']		
term.setOption('theme', theme);



runFakeTerminal();	
term.toggleFullScreen(true);
fit()




function proposeGeometry() {

    var parentElementStyle = window.getComputedStyle(term.element.parentElement);
    var parentElementHeight = html.clientHeight;
    var parentElementWidth = html.clientWidth;	
					   
    var elementStyle = window.getComputedStyle(term.element);
    var elementPadding = {
        top: parseInt(elementStyle.getPropertyValue('padding-top')),
        bottom: parseInt(elementStyle.getPropertyValue('padding-bottom')),
        right: parseInt(elementStyle.getPropertyValue('padding-right')),
        left: parseInt(elementStyle.getPropertyValue('padding-left'))
    };
    var elementPaddingVer = elementPadding.top + elementPadding.bottom;
    var elementPaddingHor = elementPadding.right + elementPadding.left;
    var availableHeight = parentElementHeight - elementPaddingVer;
    var availableWidth = parentElementWidth - elementPaddingHor - term.viewport.scrollBarWidth;
    var geometry = {
        cols: Math.floor(availableWidth / term.renderer.dimensions.actualCellWidth),
        rows: Math.floor(availableHeight / term.renderer.dimensions.actualCellHeight)
    };
	
		
    return geometry;
}

function fit() {


    var geometry = proposeGeometry();
    if (geometry) {
        if (term.rows !== geometry.rows || term.cols !== geometry.cols) {
            term.renderer.clear();
            term.resize(geometry.cols, geometry.rows);
        }
    }
	

}

function runFakeTerminal() 
{

  term.on('key', function (key, ev) {
    var printable = (
      !ev.altKey && !ev.altGraphKey && !ev.ctrlKey && !ev.metaKey
    );

	if (ev.keyCode == 13)
	{
		var data = webView.callWorkerScript(Array('sendArray', "\r\n"))
	}
	else
	{
		var data = webView.callWorkerScript(Array('sendArray', key))
	}
  });

  term.on('paste', function (data, ev) {
    //xtermDataObject.sendData(data);
	term.write(data)
  });
  
  

}

function testAdd(){term.write("1234\r\n");}

function addData(data)
{
	term.write(data);
}


var timer = setInterval(readData, 100);

function unload()
{
clearTimeout(timer);
}

function readData()
{

	var data = webView.callWorkerScriptWithResult(Array('readData'))
	
	if(data.len != 0)
	{
		term.write(data)
	}
}


