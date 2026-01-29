import os
import re
import sys

import toml
from colorama import Fore, Style

SOURCE_DIR = "./src"
TRANSLATE_FILE_FORMAT = "contrib/Distribution/translate/translate_{}.toml"
REGEX_PATTERN = r'Translate\s*\(\s*"([^"]+)"\s*\)'

def extract_keys_from_source(directory):
    keys = set()
    for root, _, files in os.walk(directory):
        for file in files:
            if file.endswith(('.cpp', '.h', '.cppm')):
                path = os.path.join(root, file)
                with open(path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    found = re.findall(REGEX_PATTERN, content)
                    keys.update(found)
    return keys

def build_nested_dict(keys):
    result = {}
    for key in sorted(keys):
        parts = key.split('.')
        current = result
        for part in parts[:-1]:
            current = current.setdefault(part, {})
        if parts[-1] not in current:
            current[parts[-1]] = ""
    return result

def update_toml(language):
    found_keys = extract_keys_from_source(SOURCE_DIR)
    translate_dict_in_codes = build_nested_dict(found_keys)

    translate_dict_in_file = {}
    translateFile = TRANSLATE_FILE_FORMAT.format(language)
    if os.path.exists(translateFile):
        with open(translateFile, 'r', encoding='utf-8') as f:
            translate_dict_in_file = toml.load(f)

    def check_unused_keys(prefix, keys_in_file, keys_in_code):
        for key, value in keys_in_file.items():
            current_key = key
            if prefix != "":
                current_key = prefix + "." + key
            if isinstance(value, dict):
                if not isinstance(keys_in_code[key], dict):
                    print(Fore.YELLOW + f"The key: [{current_key}] is incompatible with same key in code.")
                else:
                    check_unused_keys(current_key, value, keys_in_code[key])
            elif key not in keys_in_code:
                print(Fore.YELLOW + f"The key: [{current_key}] seems unused.")

    check_unused_keys("", translate_dict_in_file, translate_dict_in_codes)

    print(Style.RESET_ALL)
    print("Reading merge keys to files...")

    missing_count = 0
    def merge(in_codes, in_files):
        for key, value in in_codes.items():
            if isinstance(value, dict):
                node = in_files.setdefault(key, {})
                merge(value, node)
            else:
                if key not in in_files or not in_files[key]:
                    missing_count = missing_count + 1
                    print(Fore.YELLOW + f"The key: [{key}] missing in file.")
                    in_files[key] = value

    merge(translate_dict_in_codes, translate_dict_in_file)
    print(Style.RESET_ALL)

    if missing_count > 0:
        with open(translateFile, 'w', encoding='utf-8') as f:
            toml.dump(dict(sorted(translate_dict_in_file.items())), f)
        print(f"Already sync {missing_count} missing term to translate file: {translateFile}!")
    else:
        print(Fore.GREEN + f"All translate keys is satisfied!")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.stderr.write("Please pass a language: \n")
        sys.stderr.write("Usage: python ./extract_i18n.py english \n")
    else:
        update_toml(sys.argv[1])
