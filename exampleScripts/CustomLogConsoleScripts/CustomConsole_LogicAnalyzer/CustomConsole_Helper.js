
/*
 * Converts a number to a bit arry.
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