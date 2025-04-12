
/*   Six-Way Volumetric Lighting for Blazium v2.0   */
/*            Developed by Hamid.Memar              */

#include "register_types.h"

// Module Routines
void initialize_sixway_mtl_module(ModuleInitializationLevel p_level) 
{
	// Check for Initialization Level
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;

	// Initialize Module
	SixWayLightingMaterial::RegisterModule();
}
void uninitialize_sixway_mtl_module(ModuleInitializationLevel p_level)
{
	// Check for Initialization Level
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
}
