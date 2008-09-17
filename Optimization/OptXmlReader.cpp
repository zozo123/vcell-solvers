#include <iostream>
#include <sstream>
using namespace std;

#include "OptXmlReader.h"
#include "OptXml.h"
#include "OdeOptJob.h"
#include "OdeResultSet.h"
#include "Expression.h"
#include "tinyxml.h"

OptXmlReader::OptXmlReader(){
}

OdeOptJob* OptXmlReader::parseOdeOptJob(const char* xmlText){
	TiXmlDocument doc;
	doc.Parse(xmlText);
	TiXmlElement* root = doc.FirstChildElement(OptProblemDescription_Tag);
	return parseOdeOptJob(root);
}

OdeOptJob* OptXmlReader::readOdeOptJob(const char* xmlFile){
	TiXmlDocument doc(xmlFile);
	doc.LoadFile();
	TiXmlElement* root = doc.FirstChildElement(OptProblemDescription_Tag);
	return parseOdeOptJob(root);
}

OdeOptJob* OptXmlReader::parseOdeOptJob(TiXmlElement* rootNode){
	//
	// parse parameters
	//
	TiXmlElement* paramDescNode = rootNode->FirstChildElement(ParameterDescription_Tag);
	vector<string> nameVector;
	vector<double> lowVector;
	vector<double> highVector;
	vector<double> initVector;
	vector<double> scaleVector;
	TiXmlElement* parameter = paramDescNode->FirstChildElement(Parameter_Tag);
	while (parameter!=0){
		nameVector.push_back(string(parameter->Attribute(ParameterName_Attr)));

		const char* lowAttr = parameter->Attribute(ParameterLow_Attr);
		double low = atof(lowAttr);
		if (!strcmp(lowAttr,"-Infinity")){
			low = -DBL_MAX;
		}
		lowVector.push_back(low);

		const char* highAttr = parameter->Attribute(ParameterHigh_Attr);
		double high = atof(highAttr);
		if (!strcmp(highAttr,"Infinity")){
			high = DBL_MAX;
		}
		highVector.push_back(high);

		initVector.push_back(atof(parameter->Attribute(ParameterInit_Attr)));
		scaleVector.push_back(atof(parameter->Attribute(ParameterScale_Attr)));
		parameter = parameter->NextSiblingElement(Parameter_Tag);
	}
	int numParameters = (int)nameVector.size();
	char** paramNames = new char*[numParameters];
	double* lows = new double[numParameters];
	double* highs = new double[numParameters];
	double* inits = new double[numParameters];
	double* scales = new double[numParameters];
	for (int i=0;i<numParameters;i++){
		paramNames[i] = new char[nameVector[i].size()+1];
		strcpy(paramNames[i],nameVector[i].c_str());
		lows[i] = lowVector[i];
		highs[i] = highVector[i];
		inits[i] = initVector[i];
		scales[i] = scaleVector[i];
	}
	//SimpleParameterDescription* simpleParamDescription = new SimpleParameterDescription(numParameters,nameVector,lows,highs,inits,scales);

	//
	// parse constraints
	//
	//SymbolTable* symbolTable = paramDesc->getSymbolTable();
	TiXmlElement* constDescNode = rootNode->FirstChildElement(ConstraintDescription_Tag);
	vector<char*> constraintExpsLE;
	vector<char*> constraintExpsLI;
	vector<char*> constraintExpsNE;
	vector<char*> constraintExpsNI;
	TiXmlElement* constraintNode = constDescNode->FirstChildElement(Constraint_Tag);
	while (constraintNode!=0){
		const char* constraintType = constraintNode->Attribute(ConstraintType_Attr);
		TiXmlText* textNode = constraintNode->FirstChild()->ToText();
		char* expString = (char*)textNode->Value();
		if (strcmp(constraintType,ConstraintType_Attr_LinearEquality)!=0){
			constraintExpsLE.push_back(expString);
		}else if (strcmp(constraintType,ConstraintType_Attr_NonlinearEquality)!=0){
			constraintExpsNE.push_back(expString);
		}else if (strcmp(constraintType,ConstraintType_Attr_LinearInequality)!=0){
			constraintExpsLI.push_back(expString);
		}else if (strcmp(constraintType,ConstraintType_Attr_NonlinearInequality)!=0){
			constraintExpsNI.push_back(expString);
		}
		constraintNode = constDescNode->NextSiblingElement(Constraint_Tag);
	}
	int numNonlinInequ = (int)constraintExpsNI.size();
	int numLinInequ = (int)constraintExpsLI.size();
	int numNonlinEqu = (int)constraintExpsNE.size();
	int numLinEqu = (int)constraintExpsLE.size();
	int numConstraints = numNonlinEqu+numLinEqu+numNonlinInequ+numLinInequ;
	char** constraintExps = new char*[numConstraints];
	for (int i=0;i<(int)constraintExpsNI.size();i++){
		constraintExps[i] = constraintExpsNI[i];
	}
	for (int i=0;i<(int)constraintExpsLI.size();i++){
		constraintExps[i] = constraintExpsLI[i];
	}
	for (int i=0;i<(int)constraintExpsNE.size();i++){
		constraintExps[i] = constraintExpsNE[i];
	}
	for (int i=0;i<(int)constraintExpsLE.size();i++){
		constraintExps[i] = constraintExpsLE[i];
	}
	//
	// parse objectiveFunction
	//
	TiXmlElement* objFuncNode = rootNode->FirstChildElement(ObjectiveFunction_Tag);
	const char* objFuncTypeStr = objFuncNode->Attribute(ObjectiveFunctionType_Attr);
	if (strcmp(objFuncTypeStr, ObjectiveFunctionType_Attr_Explicit)==0){
		//TiXmlPrinter printer;
		//TiXmlText* textNode = objFuncNode->FirstChild()->ToText();
		//printer.Visit(*textNode);
		//const char* expString = printer.CStr();
		//Expression* exp = new Expression(string(expString));
		//void (*checkStopRequested)(double, long) = 0;
		//ExplicitObjectiveFunction* objFunc = new ExplicitObjectiveFunction(exp,paramDesc,paramDesc->getSymbolTable(),checkStopRequested);
		throw "unknown objective function";
	}else if (strcmp(objFuncTypeStr, ObjectiveFunctionType_Attr_PDE)==0){
		throw "unknown objective function";
	}else if (strcmp(objFuncTypeStr,ObjectiveFunctionType_Attr_ODE)==0){
		//
		// get model (idaInput or cvodeInput text)
		//
		TiXmlElement* modelNode = objFuncNode->FirstChildElement(Model_Tag);
		string modelType(modelNode->Attribute(ModelType_Attr));
		if (modelType!=ModelType_Attr_IDA && modelType!=ModelType_Attr_CVODE){
			throw "unexpected model type, expected ida or cvode";
		}
		const char* solverInput = modelNode->GetText();
		//
		// get data set as OdeResultSet
		//
		TiXmlElement* dataNode = objFuncNode->FirstChildElement(Data_Tag);
		OdeResultSet* refData = parseOdeResultSet(dataNode);
		int numColumns = refData->getNumColumns();
		double* weights = new double[numColumns];
		memset(weights,0,numColumns*sizeof(double));

		void (*checkStopRequested)(double, long) = 0;
		//
		// parse modelMappings into expression strings
		//
		vector<const char *> modelMappingExpressions;
		TiXmlElement* modelMappingNode = objFuncNode->FirstChildElement(ModelMapping_Tag);
		while(modelMappingNode!=0){
			modelMappingExpressions.push_back(modelMappingNode->GetText());
			const char* dataColumnName = modelMappingNode->Attribute(ModelMappingDataColumn_Attr);
			int refColumnIndex = refData->findColumn(string(dataColumnName));
			weights[refColumnIndex] = atof(modelMappingNode->Attribute(ModelMappingWeight_Attr));
			modelMappingNode = modelMappingNode->NextSiblingElement(ModelMapping_Tag);
		}
		const char** columnExpressions = new const char*[modelMappingExpressions.size()];
		for (int i=0;i<modelMappingExpressions.size();i++){
			columnExpressions[i] = modelMappingExpressions[i];
		}
		refData->setColumnWeights(weights);
		OdeOptJob* odeOptJob = new OdeOptJob(
			numParameters, paramNames, lows, highs, inits, scales, 
			numNonlinInequ, numLinInequ, numNonlinEqu, numLinEqu, 
			constraintExps, refData, (char**)columnExpressions, 
			(char*)solverInput, checkStopRequested);
		return odeOptJob;
	}else{
		throw "unknown objective function";
		// Unknown
	}
}


OdeResultSet* OptXmlReader::parseOdeResultSet(TiXmlElement* dataNode){
	OdeResultSet* refData = new OdeResultSet();
	vector<string> varNames;
	vector<string> varTypes;
	vector<int> varDims;
	TiXmlElement* variableNode = dataNode->FirstChildElement(Variable_Tag);
	while (variableNode!=0){
		varNames.push_back(variableNode->Attribute(VariableName_Attr));
		varTypes.push_back(variableNode->Attribute(VariableType_Attr));
		varDims.push_back(atoi(variableNode->Attribute(VariableDimension_Attr)));
		variableNode = variableNode->NextSiblingElement(Variable_Tag);
	}
	// make sure 1st is independent, rest are dependent, and all are dim=1
	for (int i=0;i<varNames.size();i++){
		if (varDims[i]!=1){
			throw "unexpected data variable dimension != 1";
		}
		if (i==0){
			if (varTypes[i] != VariableType_Attr_Independent){
				throw "expected first data variable to be independent";
			}
		}else{
			if (varTypes[i] != VariableType_Attr_Dependent){
				throw "expected remaining data variables to be dependent";
			}
		}
		refData->addColumn(varNames[i]);
	}
	TiXmlElement* rowNode = dataNode->FirstChildElement(Row_Tag);
	while (rowNode!=0){
		const char* dataText = rowNode->GetText();
		stringstream ss(dataText);
		double* rowData = new double[varNames.size()];
		for (int i=0;i<varNames.size();i++){
			ss >> rowData[i];
		}
		refData->addRow(rowData);
		rowNode = rowNode->NextSiblingElement(Row_Tag);
	}
	return refData;
}