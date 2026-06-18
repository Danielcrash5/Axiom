import os
import sys
import zipfile
import shutil
import subprocess
import requests

# --- CONFIGURATION & AUTO-INSTALL ---
TOOLS_DIR = os.path.join(os.path.dirname(__file__), "shader_tools")
GLSLANG_URL = "https://github.com/KhronosGroup/glslang/releases/download/master-tot/glslang-master-windows-x64-Release.zip"
# Ein stabiles Standalone-Build von Tint (wgpu-native/dawn tooling)
TINT_URL = "https://github.com/gfx-rs/wgpu-native/releases/download/v0.19.3.1/wgpu-windows-x86_64-release.zip"

GLSLANG_BIN = os.path.join(TOOLS_DIR, "bin/glslangValidator.exe")
TINT_BIN = os.path.join(TOOLS_DIR, "tint.exe")

def ensure_tools_installed():
    """Lädt glslang und tint automatisch herunter, wenn sie fehlen."""
    if os.path.exists(GLSLANG_BIN) and os.path.exists(TINT_BIN):
        return

    print("--- [Axiom Shader Build System] Installiere Tooling... ---")
    os.makedirs(TOOLS_DIR, exist_ok=True)

    # 1. glslangValidator holen
    if not os.path.exists(GLSLANG_BIN):
        print("Lade glslangValidator herunter...")
        zip_path = os.path.join(TOOLS_DIR, "glslang.zip")
        with requests.get(GLSLANG_URL, stream=True) as r:
            with open(zip_path, "wb") as f:
                shutil.copyfileobj(r.raw, f)
        with zipfile.ZipFile(zip_path, "r") as zip_ref:
            zip_ref.extractall(TOOLS_DIR)
        os.remove(zip_path)

    # 2. Tint holen (wgpu-tools enthalten das fertige tint-cli)
    if not os.path.exists(TINT_BIN):
        print("Lade Tint CLI herunter...")
        zip_path = os.path.join(TOOLS_DIR, "tint.zip")
        # Da tint oft tief verpackt ist, nutzen wir ein verlässliches wgpu-native release, das tint enthält
        with requests.get("https://github.com/gfx-rs/wgpu-native/releases/download/v22.1.0.5/wgpu-windows-x86_64-release.zip", stream=True) as r:
            with open(zip_path, "wb") as f:
                shutil.copyfileobj(r.raw, f)
        with zipfile.ZipFile(zip_path, "r") as zip_ref:
            # Wir extrahieren nur die tint.exe, falls vorhanden, oder das gesamte Archiv
            zip_ref.extractall(TOOLS_DIR)
        os.remove(zip_path)
        
        # Falls es im Unterordner liegt, verschieben
        tint_nested = os.path.join(TOOLS_DIR, "bin/tint.exe")
        if os.path.exists(tint_nested):
            shutil.move(tint_nested, TINT_BIN)

    print("Tooling einsatzbereit!\n")

def get_shader_stage(filename):
    """Ermittelt die Shader Stage anhand der Dateiendung."""
    ext = os.path.splitext(filename)[1].lower()
    if ext in ['.vert', '.vs']: return 'vert'
    if ext in ['.frag', '.fs']: return 'frag'
    if ext in ['.comp', '.cs']: return 'comp'
    return None

def process_file(input_file, output_dir):
    """Kompiliert eine einzelne GLSL Datei zu .spv und .wgsl."""
    stage = get_shader_stage(input_file)
    if not stage:
        # Keine Shader-Datei, überspringen
        return

    os.makedirs(output_dir, exist_ok=True)
    base_name = os.path.splitext(os.path.basename(input_file))[0]
    
    spv_out = os.path.join(output_dir, f"{base_name}_{stage}.spv")
    wgsl_out = os.path.join(output_dir, f"{base_name}_{stage}.wgsl")

    print(f"Bake: {os.path.basename(input_file)} ({stage})")

    # Schritt 1: GLSL -> SPIR-V
    cmd_glslang = [GLSLANG_BIN, "-V", "-S", stage, input_file, "-o", spv_out]
    res1 = subprocess.run(cmd_glslang, capture_output=True, text=True)
    if res1.returncode != 0:
        print(f"Fehler in GLSL (glslangValidator):\n{res1.stdout}\n{res1.stderr}")
        return

    # Schritt 2: SPIR-V -> WGSL
    cmd_tint = [TINT_BIN, spv_out, "-o", wgsl_out]
    res2 = subprocess.run(cmd_tint, capture_output=True, text=True)
    if res2.returncode != 0:
        print(f"Fehler bei WGSL-Konvertierung (Tint):\n{res2.stderr}")
        return

    print(f"Generiert: {os.path.basename(spv_out)} & {os.path.basename(wgsl_out)}")

def main():
    if len(sys.argv) < 3:
        print("Benutzung:")
        print("python bake_shaders.py <PfadZuShaderOderOrdner> <PfadZuEndergebnis>")
        sys.exit(1)

    input_path = os.path.abspath(sys.argv[1])
    output_path = os.path.abspath(sys.argv[2])

    # Stelle sicher, dass glslang und tint da sind
    ensure_tools_installed()

    if os.path.isdir(input_path):
        print(f"Scanne Ordner: {input_path}")
        for entry in os.scandir(input_path):
            if entry.is_file():
                process_file(entry.path, output_path)
    elif os.path.isfile(input_path):
        process_file(input_path, output_path)
    else:
        print(f"Fehler: '{input_path}' wurde nicht gefunden.")

if __name__ == "__main__":
    main()