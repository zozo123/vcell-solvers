/* ----------------------------------------------------------------------------
 * This file was automatically generated by SWIG (http://www.swig.org).
 * Version 2.0.0
 *
 * Do not make changes to this file unless you know what you are doing--modify
 * the SWIG interface file instead.
 * ----------------------------------------------------------------------------- */

namespace libsbml {

using System;
using System.Runtime.InteropServices;

public class Parameter : SBase {
	private HandleRef swigCPtr;
	
	internal Parameter(IntPtr cPtr, bool cMemoryOwn) : base(libsbmlPINVOKE.Parameter_SWIGUpcast(cPtr), cMemoryOwn)
	{
		//super(libsbmlPINVOKE.ParameterUpcast(cPtr), cMemoryOwn);
		swigCPtr = new HandleRef(this, cPtr);
	}
	
	internal static HandleRef getCPtr(Parameter obj)
	{
		return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
	}
	
	internal static HandleRef getCPtrAndDisown (Parameter obj)
	{
		HandleRef ptr = new HandleRef(null, IntPtr.Zero);
		
		if (obj != null)
		{
			ptr             = obj.swigCPtr;
			obj.swigCMemOwn = false;
		}
		
		return ptr;
	}

  ~Parameter() {
    Dispose();
  }

  public override void Dispose() {
    lock(this) {
      if (swigCPtr.Handle != IntPtr.Zero) {
        if (swigCMemOwn) {
          swigCMemOwn = false;
          libsbmlPINVOKE.delete_Parameter(swigCPtr);
        }
        swigCPtr = new HandleRef(null, IntPtr.Zero);
      }
      GC.SuppressFinalize(this);
      base.Dispose();
    }
  }

  public Parameter(long level, long version) : this(libsbmlPINVOKE.new_Parameter__SWIG_0(level, version), true) {
    if (libsbmlPINVOKE.SWIGPendingException.Pending) throw libsbmlPINVOKE.SWIGPendingException.Retrieve();
  }

  public Parameter(SBMLNamespaces sbmlns) : this(libsbmlPINVOKE.new_Parameter__SWIG_1(SBMLNamespaces.getCPtr(sbmlns)), true) {
    if (libsbmlPINVOKE.SWIGPendingException.Pending) throw libsbmlPINVOKE.SWIGPendingException.Retrieve();
  }

  public Parameter(Parameter orig) : this(libsbmlPINVOKE.new_Parameter__SWIG_2(Parameter.getCPtr(orig)), true) {
    if (libsbmlPINVOKE.SWIGPendingException.Pending) throw libsbmlPINVOKE.SWIGPendingException.Retrieve();
  }

  public new Parameter clone() {
    IntPtr cPtr = libsbmlPINVOKE.Parameter_clone(swigCPtr);
    Parameter ret = (cPtr == IntPtr.Zero) ? null : new Parameter(cPtr, true);
    return ret;
  }

  public void initDefaults() {
    libsbmlPINVOKE.Parameter_initDefaults(swigCPtr);
  }

  public new string getId() {
    string ret = libsbmlPINVOKE.Parameter_getId(swigCPtr);
    return ret;
  }

  public new string getName() {
    string ret = libsbmlPINVOKE.Parameter_getName(swigCPtr);
    return ret;
  }

  public double getValue() {
    double ret = libsbmlPINVOKE.Parameter_getValue(swigCPtr);
    return ret;
  }

  public string getUnits() {
    string ret = libsbmlPINVOKE.Parameter_getUnits(swigCPtr);
    return ret;
  }

  public bool getConstant() {
    bool ret = libsbmlPINVOKE.Parameter_getConstant(swigCPtr);
    return ret;
  }

  public new bool isSetId() {
    bool ret = libsbmlPINVOKE.Parameter_isSetId(swigCPtr);
    return ret;
  }

  public new bool isSetName() {
    bool ret = libsbmlPINVOKE.Parameter_isSetName(swigCPtr);
    return ret;
  }

  public bool isSetValue() {
    bool ret = libsbmlPINVOKE.Parameter_isSetValue(swigCPtr);
    return ret;
  }

  public bool isSetUnits() {
    bool ret = libsbmlPINVOKE.Parameter_isSetUnits(swigCPtr);
    return ret;
  }

  public bool isSetConstant() {
    bool ret = libsbmlPINVOKE.Parameter_isSetConstant(swigCPtr);
    return ret;
  }

  public new int setId(string sid) {
    int ret = libsbmlPINVOKE.Parameter_setId(swigCPtr, sid);
    if (libsbmlPINVOKE.SWIGPendingException.Pending) throw libsbmlPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public new int setName(string name) {
    int ret = libsbmlPINVOKE.Parameter_setName(swigCPtr, name);
    if (libsbmlPINVOKE.SWIGPendingException.Pending) throw libsbmlPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int setValue(double value) {
    int ret = libsbmlPINVOKE.Parameter_setValue(swigCPtr, value);
    return ret;
  }

  public int setUnits(string units) {
    int ret = libsbmlPINVOKE.Parameter_setUnits(swigCPtr, units);
    if (libsbmlPINVOKE.SWIGPendingException.Pending) throw libsbmlPINVOKE.SWIGPendingException.Retrieve();
    return ret;
  }

  public int setConstant(bool flag) {
    int ret = libsbmlPINVOKE.Parameter_setConstant(swigCPtr, flag);
    return ret;
  }

  public int unsetName() {
    int ret = libsbmlPINVOKE.Parameter_unsetName(swigCPtr);
    return ret;
  }

  public int unsetValue() {
    int ret = libsbmlPINVOKE.Parameter_unsetValue(swigCPtr);
    return ret;
  }

  public int unsetUnits() {
    int ret = libsbmlPINVOKE.Parameter_unsetUnits(swigCPtr);
    return ret;
  }

  public UnitDefinition getDerivedUnitDefinition() {
    IntPtr cPtr = libsbmlPINVOKE.Parameter_getDerivedUnitDefinition__SWIG_0(swigCPtr);
    UnitDefinition ret = (cPtr == IntPtr.Zero) ? null : new UnitDefinition(cPtr, false);
    return ret;
  }

  public override int getTypeCode() {
    int ret = libsbmlPINVOKE.Parameter_getTypeCode(swigCPtr);
    return ret;
  }

  public override string getElementName() {
    string ret = libsbmlPINVOKE.Parameter_getElementName(swigCPtr);
    return ret;
  }

  public override bool hasRequiredAttributes() {
    bool ret = libsbmlPINVOKE.Parameter_hasRequiredAttributes(swigCPtr);
    return ret;
  }

}

}