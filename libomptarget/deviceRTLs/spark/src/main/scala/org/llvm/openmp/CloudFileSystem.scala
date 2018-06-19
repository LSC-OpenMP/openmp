package org.llvm.openmp

import java.io.InputStream
import java.io.OutputStream
import java.util.HashMap

import scala.util.Try

import org.apache.commons.io.IOUtils
import org.apache.hadoop.conf.Configuration
import org.apache.hadoop.fs.FileSystem
import org.apache.hadoop.fs.Path
import org.apache.hadoop.io.compress.CompressionCodecFactory
import org.apache.hadoop.io.compress.CompressionCodec

class AddressTable(fs: CloudFileSystem) {
  
  val OMP_TGT_MAPTYPE_TO = 0x001
  val OMP_TGT_MAPTYPE_FROM = 0x002
  val OMP_TGT_MAPTYPE_LITERAL = 0x100

  val mapSize = new HashMap[String, Int]
  val mapType = new HashMap[String, Int]
  val mapId = new HashMap[String, Int]
  val mapName = new HashMap[Int, String]
  val mapValue = new HashMap[String, Array[Byte]]

  // Initialize address table
  val csv = IOUtils.toString(fs.read("addressTable"))
  
  for (line <- csv.lines) {
    val values = line.split(";").map(_.trim)
    if (values.size != 5)
      throw new RuntimeException("Problem when reading the address table")
    val tgtPtr = values(0)
    val size = values(1).toInt
    val typeOfmapping = values(2).toInt
    val scalaId = values(3).toInt
    // Literals are passed by value within the address table
    val value = Util.hexString2byteArray(values(4))
    mapSize.put(tgtPtr, size)
    mapType.put(tgtPtr, typeOfmapping)
    mapId.put(tgtPtr, scalaId)
    mapName.put(scalaId, tgtPtr)
    mapValue.put(tgtPtr, value)
    println(tgtPtr + ";" + size + ";" + typeOfmapping + ";" + scalaId + ";")
  }
  
  def init(scalaId: Int): Array[Byte] = {
    val tgtPtr = mapName.get(scalaId)
    val size = mapSize.get(tgtPtr)
    println("XXXX DEBUG XXXX SizeOf " + tgtPtr + " = " + size)
    
    if ((mapType.get(tgtPtr) & OMP_TGT_MAPTYPE_TO) != 0)
      fs.read(tgtPtr, size)
    else if ((mapType.get(tgtPtr) & OMP_TGT_MAPTYPE_LITERAL) != 0)
      mapValue.get(tgtPtr)
    else
      new Array[Byte](size)
  }
  
  def finalize(scalaId: Int, data: Array[Byte]) = {
    val tgtPtr = mapName.get(scalaId)
    val size = mapSize.get(tgtPtr)
    if ((mapType.get(tgtPtr) & OMP_TGT_MAPTYPE_FROM) != 0)
      fs.write(tgtPtr, size, data)
  }
  
  def getSize(scalaId: Int): Int = {
    val tgtPtr = mapName.get(scalaId)
    mapSize.get(tgtPtr)
  }

}

class CloudFileSystem(fs: FileSystem, path: String, compressOption: String) {

  val MIN_SIZE_COMPRESSION = 1000000

  val ccf = new CompressionCodecFactory(new Configuration)

  var compress = false
  var codec : CompressionCodec = null

  compressOption match {
    case "gzip" =>
      compress = true
      codec = ccf.getCodecByName(compressOption)
    case "false" =>
      compress = false
    case _ =>
      throw new RuntimeException("Unsupported compression codec ("+ compressOption + ")")
  }

  def write(name: String, size: Int, data: Array[Byte]): Unit = {
    val compressIt = compress && size >= MIN_SIZE_COMPRESSION
    val filepath = new Path(path + name)
    if (data.size != size)
      throw new RuntimeException("Wrong output size of " +
        filepath.toString() + " : " + data.size + " instead of " + size)
    var os: OutputStream = fs.create(filepath)
    if (compressIt)
      os = codec.createOutputStream(os)
    os.write(data)
    os.close
  }

  def read(name: String): InputStream = {
    val filepath = new Path(path + name)
    fs.open(filepath)
  }

  def read(name: String, size: Int): Array[Byte] = {
    val compressIt = compress && size >= MIN_SIZE_COMPRESSION
    val filepath = new Path(path + name)
    var is: InputStream = fs.open(filepath)
    if (compressIt)
      is = codec.createInputStream(is)
    val data = IOUtils.toByteArray(is)
    is.close
    if (data.size != size)
      throw new RuntimeException("Wrong input size of " +
        filepath.toString() + " : " + data.size + " instead of " + size)

    return data
  }

}
