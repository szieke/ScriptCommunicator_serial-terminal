/*************************************************************************
QR Code plotter example.

Pier Andrea Serra, University of Sassari, Italy. 2015
***************************************************************************/
/*
 * Converts a number to a bit array.
 *
 * @param num
 *   The number.
 * @return
 * The created bit array.
 */
function numToBinaryArray(num) 
{
  var bin = num.toString(2);
  if (bin.length < 8) 
  {
    bin = '0000000'.split('').slice( 0, 8 - bin.length ).join('') + bin;
  }
  return bin.split('').map(Number);
}

/*
 * Converts a CSV string to an byte array.
 *
 * @param strData
 *   The CSV string.
 * @param strDelimiter
 *   Extra string delimiter (comma is the standard delimiter) .
 * @return
 * The created bit array.
 */
function CSVToArray( strData, strDelimiter ){
        strDelimiter = (strDelimiter || ",");
        var objPattern = new RegExp(
            (
                "(\\" + strDelimiter + "|\\r?\\n|\\r|^)" +
                "(?:\"([^\"]*(?:\"\"[^\"]*)*)\"|" +
                "([^\"\\" + strDelimiter + "\\r\\n]*))"
            ),
            "gi"
            );
        var arrData = [[]];
        var arrMatches = null;
        while (arrMatches = objPattern.exec( strData )){
            var strMatchedDelimiter = arrMatches[ 1 ];
            if (
                strMatchedDelimiter.length &&
                strMatchedDelimiter !== strDelimiter
                ){
                arrData.push( [] );
            }
            var strMatchedValue;
            if (arrMatches[ 2 ]){
                strMatchedValue = arrMatches[ 2 ].replace(
                    new RegExp( "\"\"", "g" ),
                    "\""
                    );
            } else {
                strMatchedValue = arrMatches[ 3 ];
            }
            arrData[ arrData.length - 1 ].push( strMatchedValue );
        }
        return( arrData );
    }
