organization := "org.llvm.openmp"

name := "omptarget-spark"

version := "1.0.0-SNAPSHOT"

scalaVersion := "2.11.8"

val sparkVersion = "2.2.0"

//libraryDependencies += "com.amazonaws" % "aws-java-sdk" %   "1.7.4"
//libraryDependencies += "org.apache.hadoop" % "hadoop-aws" % "2.7.3" excludeAll(
//    ExclusionRule("com.amazonaws", "aws-java-sdk"),
//    ExclusionRule("commons-beanutils")
//)

libraryDependencies += "org.apache.spark" %% "spark-core" % sparkVersion % "provided"
libraryDependencies += "org.apache.spark" %% "spark-sql" % sparkVersion % "provided"
