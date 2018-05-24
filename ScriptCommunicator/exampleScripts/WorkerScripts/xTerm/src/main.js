
Terminal.applyAddon(fullscreen);

var term = new Terminal({cursorBlink: false, scrollback : 10000});
term.open(document.getElementById('terminal-container'));

var body = document.body;
var html = document.documentElement;

body.addEventListener("resize", fit);				   
term.setOption('fontFamily', "courier new, courier, monospace");
term.setOption('cursorStyle', "bar");

term.toggleFullScreen(true);
fit()

term.on('key', function (key, ev) {webView.callWorkerScript(Array('sendData', key))});
term.on('paste', function (data, ev) {webView.callWorkerScript(Array('sendData', data))});



function setOptions(fontSize, cursorBlink, maxLines, foregroundColor, backgroundColor)
{
	term.setOption('fontSize', fontSize);
	term.setOption('cursorBlink', cursorBlink);
	term.setOption('scrollback', maxLines);
	
	var theme = {};
	theme.foreground = foregroundColor;
	theme.background = backgroundColor;
	theme.cursor = theme.foreground;
	theme.cursorAccent = theme.background;
	theme.selection = 'rgba(255, 255, 255, 0.3)';
	theme.ansi = ['#2e3436', '#cc0000', '#4e9a06', '#c4a000', '#3465a4', '#75507b', '#06989a', '#d3d7cf',
	'#555753', '#ef2929', '#8ae234', '#fce94f', '#729fcf', '#ad7fa8', '#34e2e2', '#eeeeec']		
	term.setOption('theme', theme);
}


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

function getContent()
{
	var result = "";
	term.selectAll();
	result = term.getSelection();
	term.clearSelection();
	return result;
}

setTimeout(function(){ readData() }, 100);
function readData()
{

	var data = webView.callWorkerScriptWithResult(Array('readData'))
	
	if(data.len != 0)
	{
		term.write(data)
	}
	setTimeout(function(){ readData() }, 100);
}


