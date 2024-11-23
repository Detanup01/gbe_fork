#controller vdf script by mr_goldberg
#generates controller config from a vdf
import vdf
import sys
import os
import traceback

keymap_digital = {
    "button_a": "A",
    "button_b": "B",
    "button_x": "X",
    "button_y": "Y",
    "dpad_north": "DUP",
    "dpad_south": "DDOWN",
    "dpad_east": "DRIGHT",
    "dpad_west": "DLEFT",
    "button_escape": "START",
    "button_menu": "BACK",
    "left_bumper": "LBUMPER",
    "right_bumper": "RBUMPER",
    "button_back_left": "A",
    "button_back_right": "X",
    "button_back_left_upper": "B",
    "button_back_right_upper": "Y",
    "": "",
    "": "",
    "": "",
    "": "",
}

def add_input_bindings(group, bindings, force_binding=None, keymap=keymap_digital):
    if "inputs" not in group:
        return bindings
    for i, i_val in group["inputs"].iteritems():
        for act in i_val.itervalues():
            for fp in act.itervalues():
                for bd in fp.itervalues():
                    for bbd, ss in bd.iteritems():
                        if bbd.lower() == 'binding':
                            st = ss.split()
                            supported_binding = False
                            if st[0].lower() == 'game_action':
                                supported_binding = True
                                if st[2][-1] == ",":
                                    action_name = st[2][:-1]
                                else:
                                    action_name = st[2][:]
                            elif st[0].lower() == 'xinput_button':
                                supported_binding = True
                                if st[1][-1] == ",":
                                    action_name = st[1][:-1]
                                else:
                                    action_name = st[1][:]
                            
                            if supported_binding:    
                                if force_binding is None:
                                    binding = keymap.get(i.lower(), None)
                                else:
                                    binding = force_binding
                                
                                if binding:
                                    if action_name in bindings:
                                        if binding not in bindings[action_name]:
                                            bindings[action_name].append(binding)
                                    else:
                                        bindings[action_name] = [binding]
                                else:
                                    print(f"[X] missing keymap for {i}")
    return bindings


def generate_controller_config(controller_vdf, config_dir):
    d = vdf.loads(controller_vdf, mapper=vdf.VDFDict, merge_duplicate_keys=False)

    controller_mappings = d["controller_mappings"]

    groups = controller_mappings.get_all_for("group")
    groups_byid = {}
    for g in groups:
        groups_byid[g["id"]] = g

    actions = controller_mappings.get_all_for("actions")
    action_list = []
    for a in actions:
        for k in a:
            action_list.append(k)

    presets = controller_mappings.get_all_for("preset")
    all_bindings = {}
    for p in presets:
        name = p["name"]
        if (name not in action_list) and name.lower() != 'default':
            continue
        group_bindings = p["group_source_bindings"]
        bindings = {}
        for number in group_bindings:
            s = group_bindings[number].split()
            if s[1].lower() != "active":
                continue

            #print(s)
            if s[0].lower() in ["switch", "button_diamond", "dpad"]:
                group = groups_byid[number]
                #print(group)
                bindings = add_input_bindings(group, bindings)

            if s[0].lower() in ["left_trigger", "right_trigger"]:
                group = groups_byid[number]
                if group["mode"].lower() == "trigger":
                    for g in group:
                        if g.lower() == "gameactions":
                            #print(group)
                            action_name = group["gameactions"][name]
                            if s[0].lower() == "left_trigger":
                                binding = "LTRIGGER"
                            else:
                                binding = "RTRIGGER"
                            if action_name in bindings:
                                if binding not in bindings[action_name] and (binding + "=trigger") not in bindings[action_name]:
                                    bindings[action_name].insert(0, binding)
                            else:
                                bindings[action_name] = [binding + "=trigger"]
                        if g.lower() == "inputs":
                            if s[0].lower() == "left_trigger":
                                binding = "DLTRIGGER"
                            else:
                                binding = "DRTRIGGER"
                            bindings = add_input_bindings(group, bindings, binding)

                else:
                    print("unhandled trigger mode", group["mode"])
                
            if s[0].lower() in ["joystick", "right_joystick", "dpad"]:
                group = groups_byid[number]
                if group["mode"].lower() == "joystick_move":
                    for g in group:
                        if g.lower() == "gameactions":
                            #print(group)
                            action_name = group["gameactions"][name]
                            if s[0].lower() == "joystick":
                                binding = "LJOY"
                            elif s[0].lower() == "right_joystick":
                                binding = "RJOY"
                            elif s[0].lower() == "dpad":
                                binding = "DPAD"
                            else:
                                print("could not handle", s[0])
                            if action_name in bindings:
                                if binding not in bindings[action_name] and (binding + "=joystick_move") not in bindings[action_name]:
                                    bindings[action_name].insert(0, binding)
                            else:
                                bindings[action_name] = [binding + "=joystick_move"]
                        if g.lower() == "inputs":
                            if s[0].lower() == "joystick":
                                binding = "LSTICK"
                            else:
                                binding = "RSTICK"
                            bindings = add_input_bindings(group, bindings, binding)

                elif group["mode"].lower() == "dpad":
                    if s[0].lower() == "joystick":
                        binding_map = {"dpad_north":"DLJOYUP", "dpad_south": "DLJOYDOWN", "dpad_west": "DLJOYLEFT", "dpad_east": "DLJOYRIGHT", "click": "LSTICK"}
                        bindings = add_input_bindings(group, bindings, keymap=binding_map)
                    elif s[0].lower() == "right_joystick":
                        binding_map = {"dpad_north":"DRJOYUP", "dpad_south": "DRJOYDOWN", "dpad_west": "DRJOYLEFT", "dpad_east": "DRJOYRIGHT", "click": "RSTICK"}
                        bindings = add_input_bindings(group, bindings, keymap=binding_map)
                    else:
                        if s[0].lower() != "dpad":
                            print("no pad", s[0])
                else:
                    print("unhandled joy mode", group["mode"])

        all_bindings[name] = bindings

    #print(controller_mappings["preset"][(0, "group_source_bindings")])

    #print(all_bindings)

    if all_bindings:
        if not os.path.exists(config_dir):
            os.makedirs(config_dir)

        for k in all_bindings:
            with open(os.path.join(config_dir, f'{k}.txt'), 'w', encoding='utf-8') as f:
                for b in all_bindings[k]:
                    f.write(f"{b}=" + ','.join(all_bindings[k][b]) + "\n")


def help():
    exe_name = os.path.basename(sys.argv[0])
    print(f"\nUsage: {exe_name} xbox_controller_config.vdf [xbox360_controller_config.vdf] ... ")
    print(f" Example: {exe_name} xbox_controller_config.vdf")
    print(f" Example: {exe_name} xboxone_controller.vdf xbox360_controller.vdf")
    print("\nAt least 1 .vdf file must be provided\n")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        help()
        sys.exit(1)

    for vdf_file in sys.argv[1:]:
        try:
            print(f"parsing controller file '{vdf_file}'")
            t = ''
            with open(vdf_file, 'rb') as f:
                t = f.read().decode('utf-8')
            if t:
                filename = os.path.basename(vdf_file)
                outdir = os.path.join(f"{filename}_config", "steam_settings", "controller")
                print(f"output dir: '{outdir}'")
                generate_controller_config(t, outdir)
            else:
                print("[X] couldn't load file", file=sys.stderr)
            
            print('**********************************\n')
        except Exception as e:
            print("Unexpected error:")
            print(e)
            print("-----------------------")
            for line in traceback.format_exception(e):
                print(line)
            print('xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n')
        
    sys.exit(0)
