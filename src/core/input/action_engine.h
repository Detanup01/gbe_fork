#pragma once

#include "action_manifest.h"
#include "backend.h"
#include "steam_api.h"

#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

struct Controller_Settings;

namespace gbe::input {

using ActionSetHandle = ControllerActionSetHandle_t;
using DigitalActionHandle = ControllerDigitalActionHandle_t;
using AnalogActionHandle = ControllerAnalogActionHandle_t;

/**
 * Mapping for a single action set or layer.
 */
struct ActionMap {
  // Maps DigitalActionHandle to a set of hardware buttons
  std::map<DigitalActionHandle, std::set<int>> digital_mappings;

  // Maps AnalogActionHandle to hardware axis index and mode
  struct AnalogMapping {
    int axis_index;
    EInputSourceMode mode;
  };
  std::map<AnalogActionHandle, AnalogMapping> analog_mappings;
};

/**
 * The logic layer that translates raw inputs into Steam actions.
 */
class ActionEngine {
public:
  ActionEngine();

  // Manifest management
  void LoadManifest(const std::string &path);
  // Load form VDF (Action Manifest)
  bool LoadFromVDF(const std::string &path);

  // Load from Settings (Legacy Goldberg format)
  void LoadFromSettings(const Controller_Settings &settings);

  // Core Interaction
  void LoadFromVDF(const ActionManifest &manifest);
  ActionSetHandle GetActionSetHandle(const char *name);
  DigitalActionHandle GetDigitalActionHandle(const char *name);
  AnalogActionHandle GetAnalogActionHandle(const char *name);

  // Origin retrieval
  std::vector<int> GetDigitalActionOrigins(ActionSetHandle set,
                                           DigitalActionHandle action);
  std::vector<int> GetAnalogActionOrigins(ActionSetHandle set,
                                          AnalogActionHandle action);

  // Set/Layer management
  void ActivateActionSet(ControllerHandle controller, ActionSetHandle set);
  void ActivateActionSetLayer(ControllerHandle controller,
                              ActionSetHandle layer);
  void DeactivateActionSetLayer(ControllerHandle controller,
                                ActionSetHandle layer);
  void DeactivateAllActionSetLayers(ControllerHandle controller);

  ActionSetHandle GetCurrentActionSet(ControllerHandle controller);
  std::vector<ActionSetHandle>
  GetActiveActionSetLayers(ControllerHandle controller);

  // Data retrieval
  ControllerDigitalActionData_t
  GetDigitalActionData(ControllerHandle controller, DigitalActionHandle action);
  ControllerAnalogActionData_t GetAnalogActionData(ControllerHandle controller,
                                                   AnalogActionHandle action);

private:
  struct ControllerContext {
    ActionSetHandle active_set = 0;
    std::vector<ActionSetHandle> active_layers; // Ordered stack of layers
  };

  std::map<ControllerHandle, ControllerContext> m_contexts;
  std::map<ActionSetHandle, ActionMap> m_action_sets;

  // Handle name mappings
  std::map<std::string, ActionSetHandle, std::less<>> m_set_names;
  std::map<std::string, DigitalActionHandle, std::less<>> m_digital_names;
  std::map<std::string, AnalogActionHandle, std::less<>> m_analog_names;

  uint64_t m_next_handle = 1;

  mutable std::recursive_mutex m_mutex;

  // Helper to walk the stack
  ActionMap::AnalogMapping *ResolveAnalogMapping(ControllerHandle controller,
                                                 AnalogActionHandle action);
  std::set<int> *ResolveDigitalMapping(ControllerHandle controller,
                                       DigitalActionHandle action);
};

} // namespace gbe::input
