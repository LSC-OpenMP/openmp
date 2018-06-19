package org.llvm.openmp

object Util {

  /**
   * Efficient XOR of two ByteArrays
   * @param x the first ByteArray
   * @param y the second ByteArray
   * @return the resulting ByteArray (x for memory efficiency)
   */
  def bitor(x: Array[Byte], y: Array[Byte]): Array[Byte] = {
    var i = 0
    while (i < x.length) {
      x(i) = (x(i) | y(i)).toByte
      i += 1
    }
    return x
  }

  /**
   * Build a ByteArray from an hexadecimal string
   * @param hexaStr	the hexadecimal string
   * @return the resulting ByteArray 
   */
  def hexString2byteArray(hexaStr: String): Array[Byte] = {
    hexaStr.sliding(2, 2).toArray.map(Integer.parseInt(_, 16).toByte)
  }

}