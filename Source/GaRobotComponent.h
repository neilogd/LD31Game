/**************************************************************************
*
* File:		GaRobotComponent.h
* Author:	Neil Richardson 
* Ver/Date:		
* Description:
*		
*		
*
*
* 
**************************************************************************/

#ifndef __GaRobotComponent_H__
#define __GaRobotComponent_H__

#include "Psybrus.h"
#include "System/Scene/ScnComponent.h"

//////////////////////////////////////////////////////////////////////////
// GaExampleComponentRef
typedef ReObjectRef< class GaRobotComponent > GaRobotComponentRef;

//////////////////////////////////////////////////////////////////////////
// GaRobotOperation
struct GaRobotOperation
{
public:
	REFLECTION_DECLARE_BASIC( GaRobotOperation );
public:
	GaRobotOperation(){};
	GaRobotOperation( 
			std::string Condition,
			BcF32 ConditionVar,
			std::string Operation,
			BcF32 OperationVar ) :
		Condition_( Condition ),
		ConditionVar_( ConditionVar ),
		Operation_( Operation ),
		OperationVar_( OperationVar )
	{}

	std::string Condition_;
	BcF32 ConditionVar_;
	std::string Operation_;
	BcF32 OperationVar_;
};

//////////////////////////////////////////////////////////////////////////
// GaRobotComponent
class GaRobotComponent:
	public ScnComponent
{
public:
	DECLARE_RESOURCE( GaRobotComponent, ScnComponent );

	void initialise( const Json::Value& Object );

	virtual void update( BcF32 Tick );
	
	virtual void onAttach( ScnEntityWeakRef Parent );
	virtual void onDetach( ScnEntityWeakRef Parent );

	std::vector< GaRobotComponentRef > getRobots( BcU32 Team );

private:
	BcU32 Team_;

	BcF32 TargetDistance_;
	MaVec3d TargetPosition_;

	BcF32 MaxVelocity_;
	MaVec3d Velocity_;

	typedef BcBool(*ProgramFunction)( GaRobotComponent*, BcF32 );
	static std::map< std::string, ProgramFunction > ProgramFunctionMap_;

	std::vector< GaRobotOperation > Program_;

};

#endif

