import re
import sys

path = sys.argv[1]

pattern = re.compile(r'^#define\s+(KEY_\w+|BTN_\w+|REL_\w+|ABS_\w+)\s+(\S+)')

codes = {}

for line in open("/usr/include/linux/input-event-codes.h"):
    m = pattern.match(line)
    if not m:
        continue
    name, raw_value = m.group(1), m.group(2)

    # reject alias lines: value doesn't look like a number at all
    if not re.match(r'^(0x[0-9a-fA-F]+|\d+)$', raw_value):
        if re.match(r'^(KEY_\w+|BTN_\w+|REL_\w+|ABS_\w+)$', raw_value):
            raw_value = str(codes[raw_value])
        elif re.match(r'^\((KEY_\w+|BTN_\w+|REL_\w+|ABS_\w+)(\+\d+)\)$', raw_value):
            a = re.match(r'^\((KEY_\w+|BTN_\w+|REL_\w+|ABS_\w+)\+(\d+)\)$', raw_value)
            raw_value = str(codes[a.group(1)] + int(a.group(2)))
        else:
            continue

    value = int(raw_value, 0)
    codes[name] = value

lines = ["// GENERATED — do not edit by hand. See scripts/generate_key_map.py\n",
            '#include "motionplusplus/KeyCodeMap.hpp"\n',
            '\nconst std::unordered_map<std::string, uint16_t> motionplusplus::key_code_map = {\n']
for s1, s2 in codes.items():
    lines.append('    { "'+s1+'", '+str(s2)+' },\n')
lines.append("};")

with open(path, 'w') as f:
    f.writelines(lines)
