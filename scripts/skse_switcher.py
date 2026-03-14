import argparse
import os
import shutil
from pathlib import Path

BASE_FILES = ["SkyrimSE.exe", "skse64_1_6_1170.dll", "skse64_loader.exe"]
DEBUG_FILES = ["skse64_1_6_1170.pdb"] + BASE_FILES
ENB_FILES = ["d3d11.dll", "d3dcompiler_46e.dll"]
ENB_PLUGINS = ["enblocal.ini", "enbseries.ini", "enbseries"]

PROFILE_MAP = {
    "base": BASE_FILES,
    "debug": DEBUG_FILES,
    "release": BASE_FILES,
    "enb": ENB_FILES,
    "enb_plugin": ENB_PLUGINS
}

STATE_FILE = "game-profile"

def get_current_profiles() -> list:
    if not os.path.exists(STATE_FILE):
        return []
    with open(STATE_FILE, "r") as f:
        return [line.strip() for line in f.readlines() if line.strip()]

def update_state_file(profiles: list):
    with open(STATE_FILE, "w") as f:
        f.write("\n".join(profiles))

class SkseSwitcher:
    def __init__(self, profile_dir: str, skyrim_path: str):
        self.profile_dir = Path(profile_dir)
        self.skyrim_path = Path(skyrim_path)

        if not self.profile_dir.exists():
            raise FileNotFoundError(f"Profile directory not found: {self.profile_dir}")
        if not self.skyrim_path.exists():
            raise FileNotFoundError(f"Skyrim path not found: {self.skyrim_path}")

    def remove_item(self, item_name: str):
        target = self.skyrim_path / item_name
        if not target.exists():
            return

        try:
            if target.is_dir():
                shutil.rmtree(target)
            else:
                target.unlink()
            print(f"[-] Removed: {item_name}")
        except Exception as e:
            print(f"[!] Error removing {item_name}: {e}")

    def copy_item(self, profile_name: str, item_name: str):
        source = self.profile_dir / profile_name / item_name
        target = self.skyrim_path / item_name

        if not source.exists():
            print(f"[!] Source missing: {source}")
            return

        try:
            if source.is_dir():
                if target.exists(): shutil.rmtree(target)
                shutil.copytree(source, target)
            else:
                shutil.copy2(source, target)  # copy2 keep metadata
            print(f"[+] Copied [{profile_name}]: {item_name}")
        except Exception as e:
            print(f"[!] Error copying {item_name}: {e}")

    def switch(self, target_profiles_str: str):
        target_profiles = [p.strip() for p in target_profiles_str.split(",")]
        current_profiles = get_current_profiles()

        print(f"Current profiles: {current_profiles}")
        print(f"Target profiles: {target_profiles}")
        print("=" * 30)

        # 1. Uninstall profile files that are no longer needed.
        for old_p in current_profiles:
            if old_p not in target_profiles and old_p in PROFILE_MAP:
                print(f"Deactivating profile: {old_p}")
                for item in PROFILE_MAP[old_p]:
                    self.remove_item(item)

        # 2. Install target Profile file
        for new_p in target_profiles:
            if new_p in PROFILE_MAP:
                print(f"Activating profile: {new_p}")
                for item in PROFILE_MAP[new_p]:
                    self.copy_item(new_p, item)
            else:
                print(f"[!] Unknown profile: {new_p}")

        # 3. Update profile state
        update_state_file(target_profiles)
        print("=" * 30)
        print(f"Successfully activated: {target_profiles}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Skyrim SE Profile Switcher")
    parser.add_argument("profile", help="Comma separated profiles, e.g., 'debug,enb'")
    parser.add_argument("--src", default=os.getenv("SkseProfileDir"), help="Source profile directory or use the env 'SkseProfileDir' if omitted.")
    parser.add_argument("--dest", default=os.getenv("Skyrim64Path"), help="Skyrim game directory or env 'Skyrim64Path' if omitted.")

    args = parser.parse_args()

    if not args.src or not args.dest:
        print("Error: SkseProfileDir or Skyrim64Path not set in Env or Args.")
    else:
        switcher = SkseSwitcher(args.src, args.dest)
        switcher.switch(args.arg_profile if hasattr(args, 'arg_profile') else args.profile)
