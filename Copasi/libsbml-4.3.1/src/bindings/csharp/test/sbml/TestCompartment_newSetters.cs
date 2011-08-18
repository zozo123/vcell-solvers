///  @file    TestCompartment_newSetters.cs
///  @brief   Compartment unit tests for new set function API
///  @author  Frank Bergmann (Csharp conversion)
///  @author  Akiya Jouraku (Csharp conversion)
///  @author  Sarah Keating 
/// 
///  $Id: TestCompartment_newSetters.cs 11545 2010-07-23 02:19:10Z mhucka $
///  $HeadURL: http://sbml.svn.sourceforge.net/svnroot/sbml/trunk/libsbml/src/bindings/csharp/test/sbml/TestCompartment_newSetters.cs $
/// 
///  ====== WARNING ===== WARNING ===== WARNING ===== WARNING ===== WARNING ======
/// 
///  DO NOT EDIT THIS FILE.
/// 
///  This file was generated automatically by converting the file located at
///  src/sbml/test/TestCompartment_newSetters.c
///  using the conversion program dev/utilities/translateTests/translateTests.pl.
///  Any changes made here will be lost the next time the file is regenerated.
/// 
///  -----------------------------------------------------------------------------
///  This file is part of libSBML.  Please visit http://sbml.org for more
///  information about SBML, and the latest version of libSBML.
/// 
///  Copyright 2005-2010 California Institute of Technology.
///  Copyright 2002-2005 California Institute of Technology and
///                      Japan Science and Technology Corporation.
///  
///  This library is free software; you can redistribute it and/or modify it
///  under the terms of the GNU Lesser General Public License as published by
///  the Free Software Foundation.  A copy of the license agreement is provided
///  in the file named "LICENSE.txt" included with this software distribution
///  and also available online as http://sbml.org/software/libsbml/license.html
///  -----------------------------------------------------------------------------


namespace LibSBMLCSTest {

  using libsbml;

  using System;

  using System.IO;

  public class TestCompartment_newSetters {
    public class AssertionError : System.Exception 
    {
      public AssertionError() : base()
      {
        
      }
    }


    static void assertTrue(bool condition)
    {
      if (condition == true)
      {
        return;
      }
      throw new AssertionError();
    }

    static void assertEquals(object a, object b)
    {
      if ( (a == null) && (b == null) )
      {
        return;
      }
      else if ( (a == null) || (b == null) )
      {
        throw new AssertionError();
      }
      else if (a.Equals(b))
      {
        return;
      }
  
      throw new AssertionError();
    }

    static void assertNotEquals(object a, object b)
    {
      if ( (a == null) && (b == null) )
      {
        throw new AssertionError();
      }
      else if ( (a == null) || (b == null) )
      {
        return;
      }
      else if (a.Equals(b))
      {
        throw new AssertionError();
      }
    }

    static void assertEquals(bool a, bool b)
    {
      if ( a == b )
      {
        return;
      }
      throw new AssertionError();
    }

    static void assertNotEquals(bool a, bool b)
    {
      if ( a != b )
      {
        return;
      }
      throw new AssertionError();
    }

    static void assertEquals(int a, int b)
    {
      if ( a == b )
      {
        return;
      }
      throw new AssertionError();
    }

    static void assertNotEquals(int a, int b)
    {
      if ( a != b )
      {
        return;
      }
      throw new AssertionError();
    }

    private Compartment C;

    public void setUp()
    {
      C = new  Compartment(1,2);
      if (C == null);
      {
      }
    }

    public void tearDown()
    {
      C = null;
    }

    public void test_Compartment_setCompartmentType1()
    {
      int i = C.setCompartmentType( "cell");
      assertTrue( i == libsbml.LIBSBML_UNEXPECTED_ATTRIBUTE );
      assertEquals( false, C.isSetCompartmentType() );
      i = C.unsetCompartmentType();
      assertTrue( i == libsbml.LIBSBML_UNEXPECTED_ATTRIBUTE );
      assertEquals( false, C.isSetCompartmentType() );
    }

    public void test_Compartment_setCompartmentType2()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setCompartmentType( "1cell");
      assertTrue( i == libsbml.LIBSBML_INVALID_ATTRIBUTE_VALUE );
      assertEquals( false, c.isSetCompartmentType() );
      i = c.unsetCompartmentType();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, c.isSetCompartmentType() );
      c = null;
    }

    public void test_Compartment_setCompartmentType3()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setCompartmentType( "cell");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( true, c.isSetCompartmentType() );
      assertTrue((  "cell"  == c.getCompartmentType() ));
      i = c.unsetCompartmentType();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, c.isSetCompartmentType() );
      c = null;
    }

    public void test_Compartment_setCompartmentType4()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setCompartmentType("");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, c.isSetCompartmentType() );
      c = null;
    }

    public void test_Compartment_setConstant1()
    {
      int i = C.setConstant(false);
      assertTrue( i == libsbml.LIBSBML_UNEXPECTED_ATTRIBUTE );
      assertTrue( C.getConstant() == false );
    }

    public void test_Compartment_setConstant2()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setConstant(false);
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertTrue( c.getConstant() == false );
      c = null;
    }

    public void test_Compartment_setId2()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setId( "1cell");
      assertTrue( i == libsbml.LIBSBML_INVALID_ATTRIBUTE_VALUE );
      assertEquals( false, c.isSetId() );
      c = null;
    }

    public void test_Compartment_setId3()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setId( "cell");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( true, c.isSetId() );
      assertTrue((  "cell"  == c.getId() ));
      c = null;
    }

    public void test_Compartment_setId4()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setId("");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, c.isSetId() );
      c = null;
    }

    public void test_Compartment_setName1()
    {
      int i = C.setName( "cell");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( true, C.isSetName() );
      i = C.unsetName();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, C.isSetName() );
    }

    public void test_Compartment_setName2()
    {
      Compartment c = new  Compartment(1,2);
      int i = c.setName( "1cell");
      assertTrue( i == libsbml.LIBSBML_INVALID_ATTRIBUTE_VALUE );
      assertEquals( false, c.isSetName() );
      i = c.unsetName();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, c.isSetName() );
      c = null;
    }

    public void test_Compartment_setName3()
    {
      int i = C.setName("");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, C.isSetName() );
    }

    public void test_Compartment_setOutside1()
    {
      int i = C.setOutside( "1cell");
      assertTrue( i == libsbml.LIBSBML_INVALID_ATTRIBUTE_VALUE );
      assertEquals( false, C.isSetOutside() );
      i = C.unsetOutside();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, C.isSetOutside() );
    }

    public void test_Compartment_setOutside2()
    {
      int i = C.setOutside( "litre");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( true, C.isSetOutside() );
      i = C.unsetOutside();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, C.isSetOutside() );
    }

    public void test_Compartment_setOutside3()
    {
      int i = C.setOutside("");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, C.isSetOutside() );
    }

    public void test_Compartment_setSize1()
    {
      int i = C.setSize(2.0);
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertTrue( C.getSize() == 2.0 );
      i = C.unsetSize();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
    }

    public void test_Compartment_setSize2()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setSize(4);
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertTrue( c.getSize() == 4 );
      assertEquals( true, c.isSetSize() );
      i = c.unsetSize();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, c.isSetSize() );
      c = null;
    }

    public void test_Compartment_setSpatialDimensions1()
    {
      int i = C.setSpatialDimensions(2);
      assertTrue( i == libsbml.LIBSBML_UNEXPECTED_ATTRIBUTE );
      assertTrue( C.getSpatialDimensions() == 3 );
    }

    public void test_Compartment_setSpatialDimensions2()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setSpatialDimensions(4);
      assertTrue( i == libsbml.LIBSBML_INVALID_ATTRIBUTE_VALUE );
      assertTrue( c.getSpatialDimensions() == 3 );
      c = null;
    }

    public void test_Compartment_setSpatialDimensions3()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setSpatialDimensions(2);
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertTrue( c.getSpatialDimensions() == 2 );
      c = null;
    }

    public void test_Compartment_setSpatialDimensions4()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setSpatialDimensions(2.0);
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertTrue( c.getSpatialDimensions() == 2 );
      c = null;
    }

    public void test_Compartment_setSpatialDimensions5()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setSpatialDimensions(2.2);
      assertTrue( i == libsbml.LIBSBML_INVALID_ATTRIBUTE_VALUE );
      assertTrue( c.getSpatialDimensions() == 3 );
      c = null;
    }

    public void test_Compartment_setUnits1()
    {
      int i = C.setUnits( "1cell");
      assertTrue( i == libsbml.LIBSBML_INVALID_ATTRIBUTE_VALUE );
      assertEquals( false, C.isSetUnits() );
      i = C.unsetUnits();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, C.isSetUnits() );
    }

    public void test_Compartment_setUnits2()
    {
      int i = C.setUnits( "litre");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( true, C.isSetUnits() );
      i = C.unsetUnits();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, C.isSetUnits() );
    }

    public void test_Compartment_setUnits3()
    {
      int i = C.setUnits("");
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, C.isSetUnits() );
    }

    public void test_Compartment_setVolume1()
    {
      int i = C.setVolume(2.0);
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertTrue( C.getVolume() == 2.0 );
      assertEquals( true, C.isSetVolume() );
      i = C.unsetVolume();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertTrue( C.getVolume() == 1.0 );
      assertEquals( true, C.isSetVolume() );
    }

    public void test_Compartment_setVolume2()
    {
      Compartment c = new  Compartment(2,2);
      int i = c.setVolume(4);
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertTrue( c.getVolume() == 4.0 );
      assertEquals( true, c.isSetVolume() );
      i = c.unsetVolume();
      assertTrue( i == libsbml.LIBSBML_OPERATION_SUCCESS );
      assertEquals( false, c.isSetVolume() );
      c = null;
    }

  }
}