import os
import sys
import subprocess
from pathlib import Path
Import("env")

project_dir = Path(env.subst("PROJECT_DIR"))

def ensure_python_dependency(module_name, pip_name=None):
    try:
        __import__(module_name)
        return
    except ImportError:
        package = pip_name or module_name
        subprocess.run([sys.executable, "-m", "pip", "install", package], check=True)

def get_git_short_hash():
    try:
        result = subprocess.run(["git", "rev-parse", "--short", "HEAD"], capture_output=True, text=True, cwd=project_dir)
        return result.stdout.strip() if result.returncode == 0 else "unknown"
    except Exception:
        return "unknown"

def ensure_libdeps_dir():
    (project_dir / ".pio" / "libdeps" / env["PIOENV"]).mkdir(parents=True, exist_ok=True)

def ensure_platform_prerequisites():
    if env.get("PIOPLATFORM") == "espressif32":
        ensure_python_dependency("intelhex")

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

def add_framework_flags(env):
    for fw in ["framework-arduinoespressif8266", "framework-arduinoespressif32", "framework-arduino-avr"]:
        try:
            fw_dir = env.PioPlatform().get_package_dir(fw)
        except Exception:
            continue
        if fw_dir:
            fw_libs = os.path.join(fw_dir, "libraries")
            if os.path.isdir(fw_libs):
                env.Append(CPPPATH=[os.path.join(fw_libs, d) for d in os.listdir(fw_libs) if os.path.isdir(os.path.join(fw_libs, d))])
            break

def copy_publish_bin(target, source, env):
    bin_path = Path(env.subst("BUILD_DIR/{PROGNAME}.bin"))
    publish_dir = project_dir / ".pio" / "build" / "publish"
    publish_dir.mkdir(parents=True, exist_ok=True)
    import shutil
    shutil.copy2(str(bin_path), str(publish_dir / f"{env['PIOENV']}.bin"))

ensure_libdeps_dir()
ensure_platform_prerequisites()
generate_version_header()
add_framework_flags(env)
env.AddPostAction("BUILD_DIR/{PROGNAME}.bin", copy_publish_bin)