// Begin CVS Header
//   $Source: /fs/turing/cvs/copasi_dev/copasi/MIRIAM/WebServicesIssues/soapStub.h,v $
//   $Revision: 1.4 $
//   $Name: Build-33 $
//   $Author: shoops $
//   $Date: 2009/04/21 16:17:18 $
// End CVS Header

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

/* soapStub.h
   Generated by gSOAP 2.7.12 from MIRIAM.h
   Copyright(C) 2000-2008, Robert van Engelen, Genivia Inc. All Rights Reserved.
   This part of the software is released under one of the following licenses:
   GPL, the gSOAP public license, or Genivia's license for commercial use.
 */

#ifndef soapStub_H
#define soapStub_H

#ifdef WIN32
#define _GLIBCXX_VECTOR 1
# pragma warning (disable: 4100)
#endif

#ifndef _GLIBCXX_VECTOR
#define _GLIBCXX_VECTOR 1
#pragma GCC system_header
#include <bits/functexcept.h>
#include <bits/stl_algobase.h>
#include <bits/allocator.h>
#include <bits/stl_construct.h>
#include <bits/stl_uninitialized.h>
#include <bits/stl_vector.h>
#include <bits/stl_bvector.h>
#ifndef _GLIBCXX_EXPORT_TEMPLATE
#endif
#ifdef _GLIBCXX_DEBUG
#endif
#endif /* _GLIBCXX_VECTOR */
#include "stdsoap2.h"

/******************************************************************************\
 *                                                                            *
 * Enumerations                                                               *
 *                                                                            *
\******************************************************************************/

/******************************************************************************\
 *                                                                            *
 * Classes and Structs                                                        *
 *                                                                            *
\******************************************************************************/

#if 0 /* volatile type: do not redeclare here */

#endif

#if 0 /* volatile type: do not redeclare here */

#endif

#ifndef SOAP_TYPE_ArrayOf_USCORExsd_USCOREstring
#define SOAP_TYPE_ArrayOf_USCORExsd_USCOREstring (7)
/* SOAP encoded array of xsd:string schema type: */
class SOAP_CMAC ArrayOf_USCORExsd_USCOREstring
{
public:
  std::string *__ptr;
  int __size;
  struct soap *soap; /* transient */
public:
  virtual int soap_type() const {return 7;} /* = unique id SOAP_TYPE_ArrayOf_USCORExsd_USCOREstring */
  virtual void soap_default(struct soap*);
  virtual void soap_serialize(struct soap*) const;
  virtual int soap_put(struct soap*, const char*, const char*) const;
  virtual int soap_out(struct soap*, const char*, int, const char*) const;
  virtual void *soap_get(struct soap*, const char*, const char*);
  virtual void *soap_in(struct soap*, const char*, const char*);
  ArrayOf_USCORExsd_USCOREstring() : __ptr(NULL), __size(0), soap(NULL) {}
  virtual ~ArrayOf_USCORExsd_USCOREstring() {}};
#endif

#ifndef SOAP_TYPE_ns2__getNameResponse
#define SOAP_TYPE_ns2__getNameResponse (13)
/* ns2:getNameResponse */
struct ns2__getNameResponse
{
public:
  std::string _getNameReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getName
#define SOAP_TYPE_ns2__getName (14)
/* ns2:getName */
struct ns2__getName
{
public:
  std::string _uri; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getLocationResponse
#define SOAP_TYPE_ns2__getLocationResponse (16)
/* ns2:getLocationResponse */
struct ns2__getLocationResponse
{
public:
  std::string _getLocationReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getLocation
#define SOAP_TYPE_ns2__getLocation (17)
/* ns2:getLocation */
struct ns2__getLocation
{
public:
  std::string _uri; /* required element of type xsd:string */
  std::string _resource; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getURIResponse
#define SOAP_TYPE_ns2__getURIResponse (19)
/* ns2:getURIResponse */
struct ns2__getURIResponse
{
public:
  std::string _getURIReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getURI
#define SOAP_TYPE_ns2__getURI (20)
/* ns2:getURI */
struct ns2__getURI
{
public:
  std::string _name; /* required element of type xsd:string */
  std::string _id; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataResourcesResponse
#define SOAP_TYPE_ns2__getDataResourcesResponse (21)
/* ns2:getDataResourcesResponse */
struct ns2__getDataResourcesResponse
{
public:
  ArrayOf_USCORExsd_USCOREstring *_getDataResourcesReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* optional element of type ArrayOfstring */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataResources
#define SOAP_TYPE_ns2__getDataResources (25)
/* ns2:getDataResources */
struct ns2__getDataResources
{
public:
  std::string _nickname; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getServicesInfoResponse
#define SOAP_TYPE_ns2__getServicesInfoResponse (27)
/* ns2:getServicesInfoResponse */
struct ns2__getServicesInfoResponse
{
public:
  std::string getServicesInfoReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getServicesInfo
#define SOAP_TYPE_ns2__getServicesInfo (28)
/* ns2:getServicesInfo */
struct ns2__getServicesInfo
{
#ifdef WITH_NOEMPTYSTRUCT
private:
  char dummy; /* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_ns2__getServicesVersionResponse
#define SOAP_TYPE_ns2__getServicesVersionResponse (30)
/* ns2:getServicesVersionResponse */
struct ns2__getServicesVersionResponse
{
public:
  std::string getServicesVersionReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getServicesVersion
#define SOAP_TYPE_ns2__getServicesVersion (31)
/* ns2:getServicesVersion */
struct ns2__getServicesVersion
{
#ifdef WITH_NOEMPTYSTRUCT
private:
  char dummy; /* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_ns2__getJavaLibraryVersionResponse
#define SOAP_TYPE_ns2__getJavaLibraryVersionResponse (33)
/* ns2:getJavaLibraryVersionResponse */
struct ns2__getJavaLibraryVersionResponse
{
public:
  std::string getJavaLibraryVersionReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getJavaLibraryVersion
#define SOAP_TYPE_ns2__getJavaLibraryVersion (34)
/* ns2:getJavaLibraryVersion */
struct ns2__getJavaLibraryVersion
{
#ifdef WITH_NOEMPTYSTRUCT
private:
  char dummy; /* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypeURIResponse
#define SOAP_TYPE_ns2__getDataTypeURIResponse (36)
/* ns2:getDataTypeURIResponse */
struct ns2__getDataTypeURIResponse
{
public:
  std::string _getDataTypeURIReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypeURI
#define SOAP_TYPE_ns2__getDataTypeURI (37)
/* ns2:getDataTypeURI */
struct ns2__getDataTypeURI
{
public:
  std::string _name; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypeURIsResponse
#define SOAP_TYPE_ns2__getDataTypeURIsResponse (38)
/* ns2:getDataTypeURIsResponse */
struct ns2__getDataTypeURIsResponse
{
public:
  ArrayOf_USCORExsd_USCOREstring *_getDataTypeURIsReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* optional element of type ArrayOfstring */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypeURIs
#define SOAP_TYPE_ns2__getDataTypeURIs (41)
/* ns2:getDataTypeURIs */
struct ns2__getDataTypeURIs
{
public:
  std::string _name; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypeDefResponse
#define SOAP_TYPE_ns2__getDataTypeDefResponse (43)
/* ns2:getDataTypeDefResponse */
struct ns2__getDataTypeDefResponse
{
public:
  std::string _getDataTypeDefReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypeDef
#define SOAP_TYPE_ns2__getDataTypeDef (44)
/* ns2:getDataTypeDef */
struct ns2__getDataTypeDef
{
public:
  std::string _nickname; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getLocationsResponse
#define SOAP_TYPE_ns2__getLocationsResponse (45)
/* ns2:getLocationsResponse */
struct ns2__getLocationsResponse
{
public:
  ArrayOf_USCORExsd_USCOREstring *_getLocationsReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* optional element of type ArrayOfstring */
};
#endif

#ifndef SOAP_TYPE_ns2__getLocations
#define SOAP_TYPE_ns2__getLocations (48)
/* ns2:getLocations */
struct ns2__getLocations
{
public:
  std::string _nickname; /* required element of type xsd:string */
  std::string _id; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getLocationsResponse_
#define SOAP_TYPE_ns2__getLocationsResponse_ (49)
/* ns2:getLocationsResponse */
struct ns2__getLocationsResponse_
{
public:
  ArrayOf_USCORExsd_USCOREstring *_getLocationsReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* optional element of type ArrayOfstring */
};
#endif

#ifndef SOAP_TYPE_ns2__getLocations_
#define SOAP_TYPE_ns2__getLocations_ (52)
/* ns2:getLocations */
struct ns2__getLocations_
{
public:
  std::string _nickname; /* required element of type xsd:string */
  std::string _id; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__isDeprecatedResponse
#define SOAP_TYPE_ns2__isDeprecatedResponse (54)
/* ns2:isDeprecatedResponse */
struct ns2__isDeprecatedResponse
{
public:
  std::string _isDeprecatedReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__isDeprecated
#define SOAP_TYPE_ns2__isDeprecated (55)
/* ns2:isDeprecated */
struct ns2__isDeprecated
{
public:
  std::string _uri; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getOfficialDataTypeURIResponse
#define SOAP_TYPE_ns2__getOfficialDataTypeURIResponse (57)
/* ns2:getOfficialDataTypeURIResponse */
struct ns2__getOfficialDataTypeURIResponse
{
public:
  std::string _getOfficialDataTypeURIReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getOfficialDataTypeURI
#define SOAP_TYPE_ns2__getOfficialDataTypeURI (58)
/* ns2:getOfficialDataTypeURI */
struct ns2__getOfficialDataTypeURI
{
public:
  std::string _uri; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getMiriamURIResponse
#define SOAP_TYPE_ns2__getMiriamURIResponse (60)
/* ns2:getMiriamURIResponse */
struct ns2__getMiriamURIResponse
{
public:
  std::string _getMiriamURIReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getMiriamURI
#define SOAP_TYPE_ns2__getMiriamURI (61)
/* ns2:getMiriamURI */
struct ns2__getMiriamURI
{
public:
  std::string _uri; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypePatternResponse
#define SOAP_TYPE_ns2__getDataTypePatternResponse (63)
/* ns2:getDataTypePatternResponse */
struct ns2__getDataTypePatternResponse
{
public:
  std::string _getDataTypePatternReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypePattern
#define SOAP_TYPE_ns2__getDataTypePattern (64)
/* ns2:getDataTypePattern */
struct ns2__getDataTypePattern
{
public:
  std::string _nickname; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getResourceInfoResponse
#define SOAP_TYPE_ns2__getResourceInfoResponse (66)
/* ns2:getResourceInfoResponse */
struct ns2__getResourceInfoResponse
{
public:
  std::string _getResourceInfoReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getResourceInfo
#define SOAP_TYPE_ns2__getResourceInfo (67)
/* ns2:getResourceInfo */
struct ns2__getResourceInfo
{
public:
  std::string _id; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getResourceInstitutionResponse
#define SOAP_TYPE_ns2__getResourceInstitutionResponse (69)
/* ns2:getResourceInstitutionResponse */
struct ns2__getResourceInstitutionResponse
{
public:
  std::string _getResourceInstitutionReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getResourceInstitution
#define SOAP_TYPE_ns2__getResourceInstitution (70)
/* ns2:getResourceInstitution */
struct ns2__getResourceInstitution
{
public:
  std::string _id; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getResourceLocationResponse
#define SOAP_TYPE_ns2__getResourceLocationResponse (72)
/* ns2:getResourceLocationResponse */
struct ns2__getResourceLocationResponse
{
public:
  std::string _getResourceLocationReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getResourceLocation
#define SOAP_TYPE_ns2__getResourceLocation (73)
/* ns2:getResourceLocation */
struct ns2__getResourceLocation
{
public:
  std::string _id; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypeSynonymsResponse
#define SOAP_TYPE_ns2__getDataTypeSynonymsResponse (74)
/* ns2:getDataTypeSynonymsResponse */
struct ns2__getDataTypeSynonymsResponse
{
public:
  ArrayOf_USCORExsd_USCOREstring *_getDataTypeSynonymsReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* optional element of type ArrayOfstring */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypeSynonyms
#define SOAP_TYPE_ns2__getDataTypeSynonyms (77)
/* ns2:getDataTypeSynonyms */
struct ns2__getDataTypeSynonyms
{
public:
  std::string _name; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getNamesResponse
#define SOAP_TYPE_ns2__getNamesResponse (78)
/* ns2:getNamesResponse */
struct ns2__getNamesResponse
{
public:
  ArrayOf_USCORExsd_USCOREstring *_getNamesReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* optional element of type ArrayOfstring */
};
#endif

#ifndef SOAP_TYPE_ns2__getNames
#define SOAP_TYPE_ns2__getNames (81)
/* ns2:getNames */
struct ns2__getNames
{
public:
  std::string _uri; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypesNameResponse
#define SOAP_TYPE_ns2__getDataTypesNameResponse (82)
/* ns2:getDataTypesNameResponse */
struct ns2__getDataTypesNameResponse
{
public:
  ArrayOf_USCORExsd_USCOREstring *getDataTypesNameReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* optional element of type ArrayOfstring */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypesName
#define SOAP_TYPE_ns2__getDataTypesName (85)
/* ns2:getDataTypesName */
struct ns2__getDataTypesName
{
#ifdef WITH_NOEMPTYSTRUCT
private:
  char dummy; /* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypesIdResponse
#define SOAP_TYPE_ns2__getDataTypesIdResponse (86)
/* ns2:getDataTypesIdResponse */
struct ns2__getDataTypesIdResponse
{
public:
  ArrayOf_USCORExsd_USCOREstring *getDataTypesIdReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* optional element of type ArrayOfstring */
};
#endif

#ifndef SOAP_TYPE_ns2__getDataTypesId
#define SOAP_TYPE_ns2__getDataTypesId (89)
/* ns2:getDataTypesId */
struct ns2__getDataTypesId
{
#ifdef WITH_NOEMPTYSTRUCT
private:
  char dummy; /* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_ns2__checkRegExpResponse
#define SOAP_TYPE_ns2__checkRegExpResponse (91)
/* ns2:checkRegExpResponse */
struct ns2__checkRegExpResponse
{
public:
  std::string _checkRegExpReturn; /* SOAP 1.2 RPC return element (when namespace qualified) */ /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns2__checkRegExp
#define SOAP_TYPE_ns2__checkRegExp (92)
/* ns2:checkRegExp */
struct ns2__checkRegExp
{
public:
  std::string _identifier; /* required element of type xsd:string */
  std::string _datatype; /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Header
#define SOAP_TYPE_SOAP_ENV__Header (93)
/* SOAP Header: */
struct SOAP_ENV__Header
{
#ifdef WITH_NOEMPTYSTRUCT
private:
  char dummy; /* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Code
#define SOAP_TYPE_SOAP_ENV__Code (94)
/* SOAP Fault Code: */
struct SOAP_ENV__Code
{
public:
  char *SOAP_ENV__Value; /* optional element of type xsd:QName */
  struct SOAP_ENV__Code *SOAP_ENV__Subcode; /* optional element of type SOAP-ENV:Code */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Detail
#define SOAP_TYPE_SOAP_ENV__Detail (96)
/* SOAP-ENV:Detail */
struct SOAP_ENV__Detail
{
public:
  int __type; /* any type of element <fault> (defined below) */
  void *fault; /* transient */
  char *__any;
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Reason
#define SOAP_TYPE_SOAP_ENV__Reason (99)
/* SOAP-ENV:Reason */
struct SOAP_ENV__Reason
{
public:
  char *SOAP_ENV__Text; /* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Fault
#define SOAP_TYPE_SOAP_ENV__Fault (100)
/* SOAP Fault: */
struct SOAP_ENV__Fault
{
public:
  char *faultcode; /* optional element of type xsd:QName */
  char *faultstring; /* optional element of type xsd:string */
  char *faultactor; /* optional element of type xsd:string */
  struct SOAP_ENV__Detail *detail; /* optional element of type SOAP-ENV:Detail */
  struct SOAP_ENV__Code *SOAP_ENV__Code; /* optional element of type SOAP-ENV:Code */
  struct SOAP_ENV__Reason *SOAP_ENV__Reason; /* optional element of type SOAP-ENV:Reason */
  char *SOAP_ENV__Node; /* optional element of type xsd:string */
  char *SOAP_ENV__Role; /* optional element of type xsd:string */
  struct SOAP_ENV__Detail *SOAP_ENV__Detail; /* optional element of type SOAP-ENV:Detail */
};
#endif

/******************************************************************************\
 *                                                                            *
 * Types with Custom Serializers                                              *
 *                                                                            *
\******************************************************************************/

/******************************************************************************\
 *                                                                            *
 * Typedefs                                                                   *
 *                                                                            *
\******************************************************************************/

#ifndef SOAP_TYPE__QName
#define SOAP_TYPE__QName (5)
typedef char *_QName;
#endif

#ifndef SOAP_TYPE__XML
#define SOAP_TYPE__XML (6)
typedef char *_XML;
#endif

/******************************************************************************\
 *                                                                            *
 * Typedef Synonyms                                                           *
 *                                                                            *
\******************************************************************************/

/******************************************************************************\
 *                                                                            *
 * Externals                                                                  *
 *                                                                            *
\******************************************************************************/

#endif

/* End of soapStub.h */
