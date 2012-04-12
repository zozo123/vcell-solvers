/* Steven Andrews, started 10/22/2001.
 This is an application programming interface for the Smoldyn program.  See
 documentation called Smoldyn_doc1.pdf and Smoldyn_doc2.pdf.
 Copyright 2003-2011 by Steven Andrews.  This work is distributed under the terms
 of the Gnu General Public License (GPL). */

#ifndef __libsmoldyn_h__
#define __libsmoldyn_h__

/* The following Swig directives are only read by the swig program */
#ifdef SWIG
%module libsmoldyn
%{
#define SWIG_FILE_WITH_INIT
#include "libsmoldyn.h"
%}
#endif


#include "smoldyn.h"

enum ErrorCode {ECok=0,ECnotify=-1,ECwarning=-2,ECerror=-3,ECbug=-4,ECmissing=-5,ECbounds=-6,ECmemory=-7,ECsyntax=-8,ECnonexist=-9,ECsame=-10,ECall=-11};

/******************************* Miscellaneous ********************************/

double         smolGetVersion(void);

/*********************************** Errors ***********************************/

void           smolSetLogging(FILE *logfile,void (*logFunction)(simptr,int,const char*, ...));
void           smolSetThrowing(int corethreshold,int libthreshold);
void           smolSetError(const char *errorfunction,enum ErrorCode errorcode,const char *errorstring);
enum ErrorCode smolGetError(char *errorfunction,char *errorstring,int clearerror);
void           smolClearError(void);
void           smolSetDebugMode(int debugmode);
char*          smolErrorCodeToString(enum ErrorCode erc,char *string);

/******************************** Sim structure *******************************/

simptr         smolNewSim(int dim,double *lowbounds,double *highbounds);
enum ErrorCode smolUpdateSim(simptr sim);
enum ErrorCode smolRunTimeStep(simptr sim);
enum ErrorCode smolRunSim(simptr sim);
enum ErrorCode smolRunSimUntil(simptr sim,double breaktime);
enum ErrorCode smolFreeSim(simptr sim);
enum ErrorCode smolDisplaySim(simptr sim);

/************************** Read configuration file ***************************/

simptr         smolPrepareSimFromFile(const char *filepath,const char *filename,const char *flags);
enum ErrorCode smolLoadSimFromFile(const char *filepath,const char *filename,simptr *simpointer,const char *flags);
enum ErrorCode smolReadConfigString(simptr sim,const char *statement,char *parameters);

/***************************** Simulation settings ****************************/

enum ErrorCode smolSetSimTimes(simptr sim,double timestart,double timestop,double timestep);
enum ErrorCode smolSetTimeStart(simptr sim,double timestart);
enum ErrorCode smolSetTimeStop(simptr sim,double timestop);
enum ErrorCode smolSetTimeNow(simptr sim,double timenow);
enum ErrorCode smolSetTimeStep(simptr sim,double timestep);
enum ErrorCode smolSetRandomSeed(simptr sim,long int seed);
enum ErrorCode smolSetPartitions(simptr sim,char *method,double value);

/********************************** Graphics **********************************/

enum ErrorCode smolSetGraphicsParams(simptr sim,char *method,int timesteps,double delay);
enum ErrorCode smolSetTiffParams(simptr sim,int timesteps,const char *tiffname,int lowcount,int highcount);
enum ErrorCode smolSetLightParams(simptr sim,int lightindex,double *ambient,double *diffuse,double *specular,double *position);
enum ErrorCode smolSetBackgroundStyle(simptr sim,double *color);
enum ErrorCode smolSetFrameStyle(simptr sim,double thickness,double *color);
enum ErrorCode smolSetGridStyle(simptr sim,double thickness,double *color);
enum ErrorCode smolSetTextStyle(simptr sim,double *color);
enum ErrorCode smolAddTextDisplay(simptr sim,char *item);

/***************************** Runtime commands *******************************/

enum ErrorCode smolSetOutputPath(simptr sim,const char *path);
enum ErrorCode smolAddOutputFile(simptr sim,char *filename,int suffix,int append);
enum ErrorCode smolAddCommand(simptr sim,char type,double on,double off,double step,double multiplier,const char *commandstring);
enum ErrorCode smolAddCommandFromString(simptr sim,char *string);

/********************************* Molecules **********************************/

enum ErrorCode smolAddSpecies(simptr sim,const char *species,const char *mollist);
int            smolGetSpeciesIndex(simptr sim,const char *species);
char*          smolGetSpeciesName(simptr sim,int speciesindex,char *species);
enum ErrorCode smolSetSpeciesMobility(simptr sim,const char *species,enum MolecState state,double difc,double *drift,double *difmatrix);
enum ErrorCode smolAddMolList(simptr sim,const char *mollist);
int            smolGetMolListIndex(simptr sim,const char *mollist);
char*          smolGetMolListName(simptr sim,int mollistindex,char *mollist);
enum ErrorCode smolSetMolList(simptr sim,const char *species,enum MolecState state,const char *mollist);
enum ErrorCode smolSetMaxMolecules(simptr sim,int maxmolecules);
enum ErrorCode smolAddSolutionMolecules(simptr sim,const char *species,int number,double *lowposition,double *highposition);
enum ErrorCode smolAddCompartmentMolecules(simptr sim,const char *species,int number,const char *compartment);
enum ErrorCode smolAddSurfaceMolecules(simptr sim,const char *species,enum MolecState state,int number,const char *surface,enum PanelShape panelshape,const char *panel,double *position);
int            smolGetMoleculeCount(simptr sim,const char *species,enum MolecState state);
enum ErrorCode smolSetMoleculeStyle(simptr sim,const char *species,enum MolecState state,double size,double *color);

/********************************** Surfaces **********************************/

enum ErrorCode smolSetBoundaryType(simptr sim,int dimension,int highside,char type);
enum ErrorCode smolAddSurface(simptr sim,const char *surface);
int            smolGetSurfaceIndex(simptr sim,const char *surface);
char*          smolGetSurfaceName(simptr sim,int surfaceindex,char *surface);
enum ErrorCode smolSetSurfaceAction(simptr sim,const char *surface,enum PanelFace face,const char *species,enum MolecState state,enum SrfAction action);
enum ErrorCode smolSetSurfaceRate(simptr sim,const char *surface,const char *species,enum MolecState state,enum MolecState state1,enum MolecState state2,double rate,const char *newspecies,int isinternal);
enum ErrorCode smolAddPanel(simptr sim,const char *surface,enum PanelShape panelshape,const char *panel,const char *axisstring,double *params);
int            smolGetPanelIndex(simptr sim,const char *surface,enum PanelShape *panelshapeptr,const char *panel);
char*          smolGetPanelName(simptr sim,const char *surface,enum PanelShape panelshape,int panelindex,char *panel);
enum ErrorCode smolSetPanelJump(simptr sim,const char *surface,const char *panel1,enum PanelFace face1,const char *panel2,enum PanelFace face2,int isbidirectional);
enum ErrorCode smolAddSurfaceUnboundedEmitter(simptr sim,const char *surface,enum PanelFace face,const char *species,double emitamount,double *emitposition);
enum ErrorCode smolSetSurfaceSimParams(simptr sim,const char *parameter,double value);
enum ErrorCode smolAddPanelNeighbor(simptr sim,const char *surface1,const char *panel1,const char *surface2,const char *panel2,int reciprocal);
enum ErrorCode smolSetSurfaceStyle(simptr sim,const char *surface,enum PanelFace face,enum DrawMode mode,double thickness,double *color,int stipplefactor,int stipplepattern,double shininess);

/********************************* Compartments *******************************/

enum ErrorCode smolAddCompartment(simptr sim,const char *compartment);
int            smolGetCompartmentIndex(simptr sim,const char *compartment);
char*          smolGetCompartmentName(simptr sim,int compartmentindex,char *compartment);
enum ErrorCode smolAddCompartmentSurface(simptr sim,const char *compartment,const char *surface);
enum ErrorCode smolAddCompartmentPoint(simptr sim,const char *compartment,double *point);
enum ErrorCode smolAddCompartmentLogic(simptr sim,const char *compartment,enum CmptLogic logic,const char *compartment2);

/********************************* Reactions **********************************/

enum ErrorCode smolAddReaction(simptr sim,const char *reaction,const char *reactant1,enum MolecState rstate1,const char *reactant2,enum MolecState rstate2,int nproduct,const char **productspecies,enum MolecState *productstates,double rate);
int            smolGetReactionIndex(simptr sim,int *orderptr,const char *reaction);
char*          smolGetReactionName(simptr sim,int order,int reactionindex,char *reaction);
enum ErrorCode smolSetReactionRate(simptr sim,const char *reaction,double rate,int type);
enum ErrorCode smolSetReactionRegion(simptr sim,const char *reaction,const char *compartment,const char *surface);
enum ErrorCode smolSetReactionProducts(simptr sim,const char *reaction,enum RevParam method,double parameter,const char *product,double *position);

/************************************ Ports ***********************************/

enum ErrorCode smolAddPort(simptr sim,const char *port,const char *surface,enum PanelFace face);
int            smolGetPortIndex(simptr sim,const char *port);
char*          smolGetPortName(simptr sim,int portindex,char *port);
enum ErrorCode smolAddPortMolecules(simptr sim,const char *port,int nmolec,const char *species,double **positions);
int            smolGetPortMolecules(simptr sim,const char *port,const char *species,enum MolecState state,int remove);


#endif
