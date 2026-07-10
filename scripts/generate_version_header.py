import os
import subprocess
from pathlib import Path
Import("env")
project_dir = Path(env.subst(""))
def get_git_short_hash():
    try:
        result = subprocess.run(["git", "rev-parse", "--short", "HEAD"], capture_output=True, text=True, cwd=project_dir)
        return result.stdout.strip() if result.returncode == 0 else "unknown"
    except Exception:
        return "unknown"
def generate_version_header():
    git_hash = get_git_short_hash()
    version_path = project_dir / "version.tmp"
    if version_path.exists():
        content = version_path.read_text()
        content = content.replace("newVersion", git_hash)
        for dest in ["include/version.h", "src/version.h"]:
            dest_path = project_dir / dest
            dest_path.parent.mkdir(parents=True, exist_ok=True)
            dest_path.write_text(content)
        print(f"Version header generated: {git_hash}")
    framework_dir = env.PioPlatform().get_package_dir("framework-arduinoespressif8266")
    if framework_dir:
        framework_libs = os.path.join(framework_dir, "libraries")
        env.Append(CPPPATH=[
            os.path.join(framework_libs, "ESP8266WiFi", "src"),
            os.path.join(framework_libs, "ESP8266mDNS", "src"),
            os.path.join(framework_libs, "ESP8266HTTPClient", "src"),
            os.path.join(framework_libs, "ESP8266httpUpdate", "src"),
        ])
def copy_publish_bin(target, source, env):
    bin_path = Path(env.subst("/" + "publish/" + env["PIOENV"] + ".bin"))
    publish_dir = project_dir / ".pio" / "build" / "publish"
    publish_dir.mkdir(parents=True, exist_ok=True)
    import shutil
    shutil.copy2(str(bin_path), str(publish_dir / f"{env['PIOENV']}.bin"))
generate_version_header()
env.AddPostAction("/" + "publish/" + env["PIOENV"] + ".bin", copy_publish_bin)