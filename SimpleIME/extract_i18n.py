import os
import re
import sys
import toml

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
    """将 'Settings.Sidebar.Appearance' 转换为嵌套字典"""
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
    new_data = build_nested_dict(found_keys)

    existing_data = {}
    translateFile = TRANSLATE_FILE_FORMAT.format(language)
    if os.path.exists(translateFile):
        with open(translateFile, 'r', encoding='utf-8') as f:
            existing_data = toml.load(f)

    def merge(src, dst):
        for key, value in src.items():
            if isinstance(value, dict):
                node = dst.setdefault(key, {})
                merge(value, node)
            else:
                if key not in dst or not dst[key]:
                    dst[key] = value

    merge(new_data, existing_data)

    with open(translateFile, 'w', encoding='utf-8') as f:
        toml.dump(dict(sorted(existing_data.items())), f)

    print(f"Already sync {len(found_keys)} term to translate file: {translateFile}!")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        sys.stderr.write("Please pass a language: \n")
        sys.stderr.write("Usage: python ./extract_i18n.py english \n")
    else:
        update_toml(sys.argv[1])