#include "EngineCVars.h"

namespace df3d {

ConfigVariableBool EngineCVars::bulletDebugDraw = { false, "bullet_dbg_draw", "Draw bullet physics debug" };
ConfigVariableInt EngineCVars::preferredFPS = { 30, "pref_fps", "Preferred FPS" };
ConfigVariableBool EngineCVars::objIndexize = { true, "obj_indexize", "Indexize obj meshes" };

}
