#include <maya/MPlugArray.h>
#include <maya/MTypeId.h>
#include <maya/MFloatVectorArray.h>

#include <maya/MItDag.h>
#include <maya/MItSelectionList.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <math.h>
#include <maya/MArgDatabase.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDagPath.h>

#include "errorMacros.h"

#include "delightCacheAlembic.h"
#include "alembicArchiveNode.h"

#include <ri.h>

//	static
void* delightCacheAlembic::creator()
{
	return new delightCacheAlembic();
}

MSyntax delightCacheAlembic::newSyntax()
{
	MSyntax syn;

	syn.addFlag(kAddFlag,kAddFlagL);
	syn.addFlag(kEmitFlag,kEmitFlagL);
	syn.addFlag(kFlushFlag,kFlushFlagL);
	syn.addFlag(kListFlag,kListFlagL);
	syn.addFlag(kRemoveFlag	,kRemoveFlagL);
	syn.addFlag(kSampleTimeFlag,kSampleTimeFlagL,MSyntax::kDouble );
	syn.addFlag(kInfoFlag	,kInfoFlagL);
	syn.addFlag(kRelativeTimeFlag,kRelativeTimeFlagL,MSyntax::kDouble );

	syn.setObjectType(MSyntax::kSelectionList, 0, 1);
	syn.useSelectionAsDefault(false);
	
	return syn;
}

MStatus delightCacheAlembic::doIt( const MArgList& args )
{
	MStatus st;
	MString method("delightCacheAlembic::doIt");
	
	// get the node if one is specified
	MSelectionList list;
	MArgDatabase  argData(syntax(), args);
	argData.getObjects(list);
	MItSelectionList iter( list, MFn::kPluginDependNode);
	MObject  archiveObject;
	MDagPath archiveDagPath;
	MFnDependencyNode archiveFn;
	alembicArchiveNode * alembicArchive = 0;
	MString fullPathName;

	for (;! iter.isDone() ; iter.next()) {
		iter.getDependNode( archiveObject );
		archiveFn.setObject( archiveObject );
		if (archiveFn.typeId() == alembicArchiveNode::id) {
			iter.getDagPath( archiveDagPath );
			fullPathName = archiveDagPath.fullPathName();
			alembicArchive =  (alembicArchiveNode*)archiveFn.userNode();
			break;
		}
	}
	
	if (argData.isFlagSet(kAddFlag)) {

	    //cout << "Add Flag found" << endl;
		if (!(alembicArchive)) {
			displayError("Object not in cache");
			return MS::kUnknownParameter;
		}
		if (!(argData.isFlagSet(kSampleTimeFlag))){
			displayError("Need to specify a sample num with -st");
			return MS::kUnknownParameter;
		}


		double dsampleTime;
		float sampleTime;
		st= argData.getFlagArgument (kSampleTimeFlag, 0, dsampleTime);
		sampleTime = float(dsampleTime);
/*		if (alembicArchive->hasCache(sampleTime)){
			displayError("Object and sample already in cache: "+fullPathName +" " + sampleTime);
			return MS::kUnknownParameter;
		}	else {
			st = alembicArchive->addSlice(sampleTime);
		} */

		// Add Object to Cache

		return st;
	}

	if (argData.isFlagSet(kEmitFlag)) {
	    //cout << "Emit Flag found" << endl;
		if (!(alembicArchive)) {
			displayError("Object not in cache");
			return MS::kUnknownParameter;
		}

		
		if (argData.isFlagSet(kSampleTimeFlag)){
			double sampleTime;
			st= argData.getFlagArgument (kSampleTimeFlag, 0, sampleTime);
			st = alembicArchive->emitCache(float(sampleTime));

		} else {
			double relativeTime;
			if (argData.isFlagSet(kRelativeTimeFlag)){
				st= argData.getFlagArgument (kRelativeTimeFlag, 0, relativeTime);
			} else {
				st = MGlobal::executeCommand("delightRenderState -qf",relativeTime);
			}
			st = alembicArchive->emitCache(float(relativeTime));
		}

		return st;
	}


	if (argData.isFlagSet(kRemoveFlag)) {
	    //cout << "Remove Flag found" << endl;
		if (!(alembicArchive)) {
			displayError("Need to specify a alembicArchiveNode");
			return MS::kUnknownParameter;
		}
		//st = alembicArchive->removeCache();
		return st;
	}

	
	if (argData.isFlagSet(kFlushFlag)) {
	    //cout << "Flush Flag found" << endl;
		MItDag dagIter( MItDag::kDepthFirst, MFn::kPluginDependNode, &st);
		for ( ;!dagIter.isDone();dagIter.next()){
			archiveObject = dagIter.currentItem();
			archiveFn.setObject( archiveObject );
			if (archiveFn.typeId() == alembicArchiveNode::id) {
				alembicArchive =  (alembicArchiveNode*)archiveFn.userNode();
				//st = alembicArchive->removeCache();
			}
		}
		return MS::kSuccess;
	}


	if (argData.isFlagSet(kListFlag)) {

	    //cout << "List Flag found" << endl;
		MItDag dagIter( MItDag::kDepthFirst, MFn::kPluginDependNode, &st);
		for ( ;!dagIter.isDone();dagIter.next()){
			archiveObject = dagIter.currentItem();
			archiveFn.setObject( archiveObject );
			if (archiveFn.typeId() == alembicArchiveNode::id) {
				alembicArchive =  (alembicArchiveNode*)archiveFn.userNode();
				//if (alembicArchive->hasCache()) {
					appendToResult(dagIter.fullPathName());
				//}

			}
		}
		return MS::kSuccess;
	}
	
	
	if (argData.isFlagSet(kInfoFlag)) {
	    //cout << "Info Flag found" << endl;
		MItDag dagIter( MItDag::kDepthFirst, MFn::kPluginDependNode, &st);
		for ( ;!dagIter.isDone();dagIter.next()){
			archiveObject = dagIter.currentItem();
			archiveFn.setObject( archiveObject );
			if (archiveFn.typeId() == alembicArchiveNode::id) {
				alembicArchive =  (alembicArchiveNode*)archiveFn.userNode();
				cerr << dagIter.fullPathName() << endl;;
				/*if (alembicArchive->hasCache()) {
					cerr << "Cache  Exists" << endl;
					alembicArchive->cacheInfo();
				} else {
					cerr << "Cache Does Not Exist" << endl;
				}*/
			}
		}
		return MS::kSuccess;
	}
	return MS::kSuccess;
}

