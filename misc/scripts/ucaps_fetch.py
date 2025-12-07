#!/usr/bin/env python3

# Script used to dump case mappings from
# the Unicode Character Database to the `ucaps.h` file.
# NOTE: This script is deliberately not integrated into the build system;
# you should run it manually whenever you want to update the data.

import os
import sys
from typing import Final, List, Tuple
from urllib.request import urlopen

if __name__ == "__main__":
    sys.path.insert(1, os.path.join(os.path.dirname(__file__), "../../"))

from methods import generate_copyright_header

URL: Final[str] = "https://www.unicode.org/Public/16.0.0/ucd/UnicodeData.txt"


lower_to_upper: List[Tuple[str, str]] = []
upper_to_lower: List[Tuple[str, str]] = []


def parse_unicode_data() -> None:
    lines: List[str] = [line.decode("utf-8") for line in urlopen(URL)]

    for line in lines:
        split_line: List[str] = line.split(";")

        code_value: str = split_line[0].strip()
        uppercase_mapping: str = split_line[12].strip()
        lowercase_mapping: str = split_line[13].strip()

        if uppercase_mapping:
            lower_to_upper.append((f"0x{code_value}", f"0x{uppercase_mapping}"))
        if lowercase_mapping:
            upper_to_lower.append((f"0x{code_value}", f"0x{lowercase_mapping}"))


def make_cap_table(table_name: str, table: List[Tuple[str, str]]) -> str:
    result: str = f"static const HashMap<int, int> {table_name} = {{\n"

    for first, second in table:
        result += f"\t{{ {first}, {second} }},\n"

    result += "};\n\n"

    return result


def generate_ucaps_fetch() -> None:
    parse_unicode_data()

    source: str = generate_copyright_header("ucaps.h")

    source += """
#pragma once

// This file was generated using the `misc/scripts/ucaps_fetch.py` script.

#include "core/templates/hash_map.h"

"""

    source += make_cap_table("caps_table", lower_to_upper)
    source += make_cap_table("reverse_caps_table", upper_to_lower)

    source += """static int _find_upper(const int ch) {
\tconst int *value = caps_table.getptr(ch);
\treturn value != nullptr ? *value : ch;
}

static int _find_lower(const int ch) {
\tconst int *value = reverse_caps_table.getptr(ch);
\treturn value != nullptr ? *value : ch;
}
"""

    ucaps_path: str = os.path.join(os.path.dirname(__file__), "../../core/string/ucaps.h")
    with open(ucaps_path, "w", newline="\n") as f:
        f.write(source)

    print("`ucaps.h` generated successfully.")


if __name__ == "__main__":
    generate_ucaps_fetch()
