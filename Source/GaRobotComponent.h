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
			BcU32 State,
			std::string Condition,
			BcU32 ConditionVar,
			std::string Operation,
			BcU32 OperationVar ) :
		State_( State ),
		Condition_( Condition ),
		ConditionVar_( ConditionVar ),
		Operation_( Operation ),
		OperationVar_( OperationVar )
	{}

	BcU32 State_;
	std::string Condition_;
	BcU32 ConditionVar_;
	std::string Operation_;
	BcU32 OperationVar_;
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

	void fireWeaponA();
	void fireWeaponB();
	void takeDamage( BcF32 Damage );

	std::vector< GaRobotComponent* > getRobots( BcU32 Team );

public:
	BcU32 Team_;

	MaVec3d StartPosition_;

	BcF32 TargetDistance_;
	MaVec3d TargetPosition_;

	BcF32 MaxVelocity_;
	MaVec3d Velocity_;

	// Health + energy.
	BcF32 Health_;
	BcF32 Energy_;
	BcF32 EnergyChargeRate_;

	// Action cool down + costs.
	BcF32 WeaponACoolDown_;
	BcF32 WeaponACost_;
	BcF32 WeaponATimer_;
	BcF32 WeaponBCoolDown_;
	BcF32 WeaponBCost_;
	BcF32 WeaponBTimer_;

	typedef BcU32(*ProgramFunction)( GaRobotComponent*, BcU32 );
	static std::map< std::string, ProgramFunction > ProgramFunctionMap_;

	std::vector< GaRobotOperation > Program_;
	BcU32 CurrentState_;
};

#endif

