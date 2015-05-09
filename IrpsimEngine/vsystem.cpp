// vsystem.cpp : implementation file
//
// Copyright © 1998-2015 Casey McSpadden   
//		mailto:casey@crossriver.com
//		http://www.crossriver.com/
//
// ==========================================================================  
// DESCRIPTION:	
// ==========================================================================
// CMVSystem is an IRPSIM "system" variable. Inherits from CMVariable.
// ==========================================================================
//
// ==========================================================================  
// HISTORY:	
// ==========================================================================
//			1.00	09 March 2015	- Initial re-write and release.
// ==========================================================================
#include "vsystem.h"
#include "node.h"
#include "simulat.h"

const wchar_t* CMVSystem::varnames[] = { L"_putpotential", L"_takepotential",
	L"_transferpotential", L"_trial"
};

CMVSystem::CMVSystem(int aType) :
CMVariable(CMVSystem::varnames[aType]),
system_type(aType)
{
   if (aType <= TransferPotential) {
		SetType(aType-10000);
		SetState(vsAlwaysEvaluate|vsAggregate|vsDontDestroy|vsSum,TRUE);
   }
   else
	  	SetState(vsDontMaintain|vsSystem|vsDontDestroy|vsNoUnits,TRUE);
}

double CMVSystem::evaluate(CMTimeMachine*,int,int)
{
	switch (system_type) {
		case PutPotential: return CMNode::PutPotential();
		case TakePotential: return CMNode::TakePotential();
		case TransferPotential: return CMNode::TransferPotential();
      case Trial:
      	CMSimulation* activesim = CMSimulation::ActiveSimulation();
         return activesim ? (activesim->Trial()+1) : 0;
	}
   return 0;
}


