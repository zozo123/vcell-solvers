/*
 * @file    TestSyntaxChecker.java
 * @brief   SyntaxChecker unit tests
 *
 * @author  Akiya Jouraku (Java conversion)
 * @author  Sarah Keating 
 *
 * $Id: TestSyntaxChecker.java 13309 2011-03-24 23:13:08Z fbergmann $
 * $HeadURL: http://sbml.svn.sourceforge.net/svnroot/sbml/trunk/libsbml/src/bindings/java/test/org/sbml/libsbml/test/sbml/TestSyntaxChecker.java $
 *
 * ====== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING ======
 *
 * DO NOT EDIT THIS FILE.
 *
 * This file was generated automatically by converting the file located at
 * src/sbml/test/TestSyntaxChecker.c
 * using the conversion program dev/utilities/translateTests/translateTests.pl.
 * Any changes made here will be lost the next time the file is regenerated.
 *
 * -----------------------------------------------------------------------------
 * This file is part of libSBML.  Please visit http://sbml.org for more
 * information about SBML, and the latest version of libSBML.
 *
 * Copyright 2005-2010 California Institute of Technology.
 * Copyright 2002-2005 California Institute of Technology and
 *                     Japan Science and Technology Corporation.
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.  A copy of the license agreement is provided
 * in the file named "LICENSE.txt" included with this software distribution
 * and also available online as http://sbml.org/software/libsbml/license.html
 * -----------------------------------------------------------------------------
 */

package org.sbml.libsbml.test.sbml;

import org.sbml.libsbml.*;

import java.io.File;
import java.lang.AssertionError;

public class TestSyntaxChecker {

  static void assertTrue(boolean condition) throws AssertionError
  {
    if (condition == true)
    {
      return;
    }
    throw new AssertionError();
  }

  static void assertEquals(Object a, Object b) throws AssertionError
  {
    if ( (a == null) && (b == null) )
    {
      return;
    }
    else if ( (a == null) || (b == null) )
    {
      throw new AssertionError();
    }
    else if (a.equals(b))
    {
      return;
    }

    throw new AssertionError();
  }

  static void assertNotEquals(Object a, Object b) throws AssertionError
  {
    if ( (a == null) && (b == null) )
    {
      throw new AssertionError();
    }
    else if ( (a == null) || (b == null) )
    {
      return;
    }
    else if (a.equals(b))
    {
      throw new AssertionError();
    }
  }

  static void assertEquals(boolean a, boolean b) throws AssertionError
  {
    if ( a == b )
    {
      return;
    }
    throw new AssertionError();
  }

  static void assertNotEquals(boolean a, boolean b) throws AssertionError
  {
    if ( a != b )
    {
      return;
    }
    throw new AssertionError();
  }

  static void assertEquals(int a, int b) throws AssertionError
  {
    if ( a == b )
    {
      return;
    }
    throw new AssertionError();
  }

  static void assertNotEquals(int a, int b) throws AssertionError
  {
    if ( a != b )
    {
      return;
    }
    throw new AssertionError();
  }

  public void test_SyntaxChecker_validID()
  {
    assertTrue( SyntaxChecker.isValidXMLID("cell") == true );
    assertTrue( SyntaxChecker.isValidXMLID("1cell") == false );
    assertTrue( SyntaxChecker.isValidXMLID("_cell") == true );
  }

  public void test_SyntaxChecker_validId()
  {
    assertTrue( SyntaxChecker.isValidSBMLSId("cell") == true );
    assertTrue( SyntaxChecker.isValidSBMLSId("1cell") == false );
    assertTrue( SyntaxChecker.isValidSBMLSId("") == false );
  }

  public void test_SyntaxChecker_validUnitId()
  {
    assertTrue( SyntaxChecker.isValidUnitSId("cell") == true );
    assertTrue( SyntaxChecker.isValidUnitSId("1cell") == false );
  }

  public void test_SyntaxChecker_validXHTML()
  {
    SBMLNamespaces NS24 = new  SBMLNamespaces(2,4);
    SBMLNamespaces NS31 = new  SBMLNamespaces(3,1);
    XMLToken toptoken;
    XMLNode topnode;
    XMLTriple toptriple = new  XMLTriple("notes", "", "");
    XMLToken token;
    XMLNode node;
    XMLTriple triple = new  XMLTriple("p", "", "");
    XMLAttributes att = new  XMLAttributes();
    XMLNamespaces ns = new  XMLNamespaces();
    ns.add( "http://www.w3.org/1999/xhtml", "");
    XMLToken tt = new  XMLToken("This is my text");
    XMLNode n1 = new XMLNode(tt);
    toptoken = new  XMLToken(toptriple,att);
    topnode = new XMLNode(toptoken);
    token = new  XMLToken(triple,att,ns);
    node = new XMLNode(token);
    node.addChild(n1);
    topnode.addChild(node);
    assertTrue( SyntaxChecker.hasExpectedXHTMLSyntax(topnode,null) == true );
    assertTrue( SyntaxChecker.hasExpectedXHTMLSyntax(topnode,NS24) == true );
    assertTrue( SyntaxChecker.hasExpectedXHTMLSyntax(topnode,NS31) == true );
    triple = new  XMLTriple("html", "", "");
    token = new  XMLToken(triple,att,ns);
    node = new XMLNode(token);
    node.addChild(n1);
    topnode.removeChild(0);
    topnode.addChild(node);
    assertTrue( SyntaxChecker.hasExpectedXHTMLSyntax(topnode,null) == true );
    assertTrue( SyntaxChecker.hasExpectedXHTMLSyntax(topnode,NS24) == false );
    assertTrue( SyntaxChecker.hasExpectedXHTMLSyntax(topnode,NS31) == true );
    triple = new  XMLTriple("html", "", "");
    ns.clear();
    token = new  XMLToken(triple,att,ns);
    node = new XMLNode(token);
    node.addChild(n1);
    topnode.removeChild(0);
    topnode.addChild(node);
    assertTrue( SyntaxChecker.hasExpectedXHTMLSyntax(topnode,null) == false );
    assertTrue( SyntaxChecker.hasExpectedXHTMLSyntax(topnode,NS24) == false );
    assertTrue( SyntaxChecker.hasExpectedXHTMLSyntax(topnode,NS31) == false );
  }

  /**
   * Loads the SWIG-generated libSBML Java module when this class is
   * loaded, or reports a sensible diagnostic message about why it failed.
   */
  static
  {
    String varname;
    String shlibname;

    if (System.getProperty("mrj.version") != null)
    {
      varname = "DYLD_LIBRARY_PATH";    // We're on a Mac.
      shlibname = "libsbmlj.jnilib and/or libsbml.dylib";
    }
    else
    {
      varname = "LD_LIBRARY_PATH";      // We're not on a Mac.
      shlibname = "libsbmlj.so and/or libsbml.so";
    }

    try
    {
      System.loadLibrary("sbmlj");
      // For extra safety, check that the jar file is in the classpath.
      Class.forName("org.sbml.libsbml.libsbml");
    }
    catch (SecurityException e)
    {
      e.printStackTrace();
      System.err.println("Could not load the libSBML library files due to a"+
                         " security exception.\n");
      System.exit(1);
    }
    catch (UnsatisfiedLinkError e)
    {
      e.printStackTrace();
      System.err.println("Error: could not link with the libSBML library files."+
                         " It is likely\nyour " + varname +
                         " environment variable does not include the directories\n"+
                         "containing the " + shlibname + " library files.\n");
      System.exit(1);
    }
    catch (ClassNotFoundException e)
    {
      e.printStackTrace();
      System.err.println("Error: unable to load the file libsbmlj.jar."+
                         " It is likely\nyour -classpath option and CLASSPATH" +
                         " environment variable\n"+
                         "do not include the path to libsbmlj.jar.\n");
      System.exit(1);
    }
  }
}