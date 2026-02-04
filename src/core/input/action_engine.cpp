#include "action_engine.h"
#include "service.h"
#include "settings.h"
#include <algorithm>

namespace gbe::input {

struct ActionOriginMapping {
  const char *token;
  int hardware_id; // SDL_GamepadButton or SDL_GamepadAxis
  bool is_axis;
};

// Generic button/axis mappings (indices match common gamepad layouts)
static const ActionOriginMapping s_origin_mappings[] = {
    {"action_button_south", 0, false}, // A / Cross
    {"action_button_east", 1, false},  // B / Circle
    {"action_button_west", 2, false},  // X / Square
    {"action_button_north", 3, false}, // Y / Triangle
    {"action_left_bumper", 4, false},
    {"action_right_bumper", 5, false},
    {"action_select", 6, false},
    {"action_start", 7, false},
    {"action_left_stick_click", 8, false},
    {"action_right_stick_click", 9, false},
    {"action_dpad_up", 10, false},
    {"action_dpad_down", 11, false},
    {"action_dpad_left", 12, false},
    {"action_dpad_right", 13, false},
    {"action_joystick_left", 0, true},  // Left stick XY (axes 0,1)
    {"action_joystick_right", 2, true}, // Right stick XY (axes 2,3)
    {"action_trigger_left", 4, true},   // LT (axis 4)
    {"action_trigger_right", 5, true}   // RT (axis 5)
};

ActionEngine::ActionEngine() {}

void ActionEngine::LoadManifest(const std::string &manifest_path) {
  LoadFromVDF(manifest_path);
}

bool ActionEngine::LoadFromVDF(const std::string &path) {
  ActionManifest manifest;
  if (manifest.LoadFromFile(path)) {
    LoadFromVDF(manifest);
    return true;
  }
  return false;
}

void ActionEngine::LoadFromVDF(const ActionManifest &manifest) {
  const VDFNode *root = manifest.GetRoot();
  const VDFNode *actions_node = root->GetChild("In Game Actions");
  if (!actions_node)
    actions_node = root->GetChild("actions");
  if (!actions_node)
    return;

  const VDFNode *sets_node = actions_node->GetChild("Action Sets");
  if (!sets_node)
    return;

  for (size_t i = 0; i < sets_node->children.size(); ++i) {
    const auto &set_node = sets_node->children[i];
    ActionSetHandle set_handle = GetActionSetHandle(set_node->key.c_str());
    ActionMap &action_set = m_action_sets[set_handle];

    const VDFNode *digital_node = set_node->GetChild("Digital");
    if (digital_node) {
      for (size_t j = 0; j < digital_node->children.size(); ++j) {
        const auto &action_node = digital_node->children[j];
        DigitalActionHandle action_handle =
            GetDigitalActionHandle(action_node->key.c_str());
        for (const auto &mapping : s_origin_mappings) {
          if (action_node->value == mapping.token && !mapping.is_axis) {
            action_set.digital_mappings[action_handle].push_back(
                mapping.hardware_id);
            break;
          }
        }
      }
    }

    const VDFNode *analog_node = set_node->GetChild("Analog");
    if (analog_node) {
      for (size_t k = 0; k < analog_node->children.size(); ++k) {
        const auto &action_node = analog_node->children[k];
        AnalogActionHandle action_handle =
            GetAnalogActionHandle(action_node->key.c_str());
        for (const auto &mapping : s_origin_mappings) {
          if (action_node->value == mapping.token && mapping.is_axis) {
            action_set.analog_mappings[action_handle] = {
                mapping.hardware_id, k_EInputSourceMode_JoystickMove};
            break;
          }
        }
      }
    }
  }
}

ActionSetHandle ActionEngine::GetActionSetHandle(const char *name) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  auto it = m_set_names.find(name);
  if (it != m_set_names.end())
    return it->second;
  ActionSetHandle handle = m_next_handle++;
  m_set_names[name] = handle;
  return handle;
}

DigitalActionHandle ActionEngine::GetDigitalActionHandle(const char *name) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  auto it = m_digital_names.find(name);
  if (it != m_digital_names.end())
    return it->second;
  DigitalActionHandle handle = m_next_handle++;
  m_digital_names[name] = handle;
  return handle;
}

AnalogActionHandle ActionEngine::GetAnalogActionHandle(const char *name) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  auto it = m_analog_names.find(name);
  if (it != m_analog_names.end())
    return it->second;
  AnalogActionHandle handle = m_next_handle++;
  m_analog_names[name] = handle;
  return handle;
}

void ActionEngine::ActivateActionSet(ControllerHandle controller,
                                     ActionSetHandle actionSetHandle) {
  m_contexts[controller].active_set = actionSetHandle;
}

void ActionEngine::ActivateActionSetLayer(
    ControllerHandle controller, ActionSetHandle actionSetLayerHandle) {
  auto &layers = m_contexts[controller].active_layers;
  if (std::find(layers.begin(), layers.end(), actionSetLayerHandle) ==
      layers.end()) {
    layers.push_back(actionSetLayerHandle);
  }
}

void ActionEngine::DeactivateActionSetLayer(
    ControllerHandle controller, ActionSetHandle actionSetLayerHandle) {
  auto &layers = m_contexts[controller].active_layers;
  layers.erase(std::remove(layers.begin(), layers.end(), actionSetLayerHandle),
               layers.end());
}

void ActionEngine::DeactivateAllActionSetLayers(ControllerHandle controller) {
  m_contexts[controller].active_layers.clear();
}

std::vector<int>
ActionEngine::GetDigitalActionOrigins(ActionSetHandle set,
                                      DigitalActionHandle action) {
  std::vector<int> origins;
  if (set == 0)
    return origins;

  auto it_set = m_action_sets.find(set);
  if (it_set == m_action_sets.end())
    return origins;

  auto &action_set = it_set->second;
  auto it = action_set.digital_mappings.find(action);
  if (it != action_set.digital_mappings.end()) {
    for (int button : it->second) {
      origins.push_back(button);
    }
  }
  return origins;
}

std::vector<int>
ActionEngine::GetAnalogActionOrigins(ActionSetHandle set,
                                     AnalogActionHandle action) {
  std::vector<int> origins;
  if (set == 0)
    return origins;

  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  auto it_set = m_action_sets.find(set);
  if (it_set == m_action_sets.end())
    return origins;

  auto &action_set = it_set->second;
  auto it = action_set.analog_mappings.find(action);
  if (it != action_set.analog_mappings.end()) {
    origins.push_back(it->second.axis_index);
  }
  return origins;
}

ActionSetHandle ActionEngine::GetCurrentActionSet(ControllerHandle controller) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  return m_contexts[controller].active_set;
}

std::vector<ActionSetHandle>
ActionEngine::GetActiveActionSetLayers(ControllerHandle controller) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  return m_contexts[controller].active_layers;
}

std::vector<int> *ActionEngine::ResolveDigitalMapping(ControllerHandle controller,
                                                      DigitalActionHandle action) {
  const auto &ctx = m_contexts[controller];
  for (auto it = ctx.active_layers.rbegin(); it != ctx.active_layers.rend();
       ++it) {
    auto mit = m_action_sets[*it].digital_mappings.find(action);
    if (mit != m_action_sets[*it].digital_mappings.end())
      return &mit->second;
  }
  auto mit = m_action_sets[ctx.active_set].digital_mappings.find(action);
  if (mit != m_action_sets[ctx.active_set].digital_mappings.end())
    return &mit->second;
  return nullptr;
}

ActionMap::AnalogMapping *
ActionEngine::ResolveAnalogMapping(ControllerHandle controller,
                                   AnalogActionHandle action) {
  const auto &ctx = m_contexts[controller];
  for (auto it = ctx.active_layers.rbegin(); it != ctx.active_layers.rend();
       ++it) {
    auto mit = m_action_sets[*it].analog_mappings.find(action);
    if (mit != m_action_sets[*it].analog_mappings.end())
      return &mit->second;
  }
  auto mit = m_action_sets[ctx.active_set].analog_mappings.find(action);
  if (mit != m_action_sets[ctx.active_set].analog_mappings.end())
    return &mit->second;
  return nullptr;
}

ControllerDigitalActionData_t
ActionEngine::GetDigitalActionData(ControllerHandle controller,
                                   DigitalActionHandle action) {
  ControllerDigitalActionData_t data = {};
  std::vector<int> *mapping = ResolveDigitalMapping(controller, action);
  if (!mapping)
    return data;

  data.bActive = true;
  ControllerState hw =
      InputService::GetInstance().GetBackend()->GetControllerState(controller);
  for (int button_id : *mapping) {
    if (button_id >= 0 && button_id < 32 && (hw.buttons & (1 << button_id))) {
      data.bState = true;
      break;
    }
  }
  return data;
}

ControllerAnalogActionData_t
ActionEngine::GetAnalogActionData(ControllerHandle controller,
                                  AnalogActionHandle action) {
  ControllerAnalogActionData_t data = {};
  ActionMap::AnalogMapping *mapping = ResolveAnalogMapping(controller, action);
  if (!mapping)
    return data;

  data.bActive = true;
  ControllerState hw =
      InputService::GetInstance().GetBackend()->GetControllerState(controller);
  data.x = hw.axes[mapping->axis_index];
  if (mapping->axis_index < 4)
    data.y = hw.axes[mapping->axis_index + 1];
  return data;
}

// Mappings for legacy config format
static const std::map<std::string, int> s_legacy_button_strings = {
    {"DUP", 6}, // BUTTON_DPAD_UP (ControllerState bits: 6-9)
    {"DDOWN", 7},    {"DLEFT", 8}, {"DRIGHT", 9}, {"START", 4}, // BUTTON_START
    {"BACK", 5},                                                // BUTTON_BACK
    {"LSTICK", 10},  // BUTTON_LEFT_THUMB
    {"RSTICK", 11},  // BUTTON_RIGHT_THUMB
    {"LBUMPER", 12}, // BUTTON_LEFT_SHOULDER
    {"RBUMPER", 13}, // BUTTON_RIGHT_SHOULDER
    {"A", 0},        // BUTTON_A
    {"B", 1},        // BUTTON_B
    {"X", 2},        // BUTTON_X
    {"Y", 3},        // BUTTON_Y
};

static const std::map<std::string, int> s_legacy_analog_strings = {
    {"LTRIGGER", 0}, // TRIGGER_LEFT
    {"RTRIGGER", 1}, // TRIGGER_RIGHT
    {"LJOY", 0},     // STICK_LEFT (Axis index 0/1)
    {"RJOY", 2},     // STICK_RIGHT (Axis index 2/3)
    {"DPAD", 10},    // Legacy DPAD-as-axis mapping (Unsupported)
};

static const std::map<std::string, EInputSourceMode> s_legacy_analog_modes = {
    {"joystick_move", k_EInputSourceMode_JoystickMove},
    {"joystick_camera", k_EInputSourceMode_JoystickCamera},
    {"trigger", k_EInputSourceMode_Trigger},
};

void ActionEngine::LoadFromSettings(const Controller_Settings &settings) {
  std::lock_guard<std::recursive_mutex> lock(m_mutex);
  const auto &action_sets = settings.action_sets;

  for (const auto &set_pair : action_sets) {
    std::string set_name = set_pair.first;
    ActionSetHandle set_handle = GetActionSetHandle(set_name.c_str());
    ActionMap &action_set = m_action_sets[set_handle];

    for (const auto &action_pair : set_pair.second) {
      std::string action_name = action_pair.first;
      const auto &bindings = action_pair.second.first;
      const std::string &input_mode_str = action_pair.second.second;

      // Determine if digital or analog based on what it's bound to
      for (const auto &btn_str : bindings) {
        auto digital_it = s_legacy_button_strings.find(btn_str);
        if (digital_it != s_legacy_button_strings.end()) {
          DigitalActionHandle digital_handle =
              GetDigitalActionHandle(action_name.c_str());
          action_set.digital_mappings[digital_handle].push_back(
              digital_it->second);
        } else {
          auto analog_it = s_legacy_analog_strings.find(btn_str);
          if (analog_it != s_legacy_analog_strings.end()) {
            AnalogActionHandle analog_handle =
                GetAnalogActionHandle(action_name.c_str());

            EInputSourceMode mode = k_EInputSourceMode_JoystickMove; // Default
            if (analog_it->second <= 1)
              mode = k_EInputSourceMode_Trigger; // Triggers

            auto mode_it = s_legacy_analog_modes.find(input_mode_str);
            if (mode_it != s_legacy_analog_modes.end()) {
              mode = mode_it->second;
            }

            action_set.analog_mappings[analog_handle] = {analog_it->second,
                                                         mode};
          }
        }
      }
    }
  }
}

} // namespace gbe::input
