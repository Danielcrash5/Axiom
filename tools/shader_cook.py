import os
import sys
import subprocess
import urllib.request
import zipfile
import platform

def get_tools():
    """Stellt sicher, dass glslangValidator und tint im 'tools'-Ordner existieren."""
    tools_dir = os.path.join(os.path.dirname(__file__), "tools")
    os.makedirs(tools_dir, exist_ok=True)
    
    is_win = platform.system() == "Windows"
    glslang_exe = "glslangValidator.exe" if is_win else "glslangValidator"
    tint_exe = "tint.exe" if is_win else "tint"
    
    glslang_path = os.path.join(tools_dir, glslang_exe)
    tint_path = os.path.join(tools_dir, tint_exe)
    
    # 1. glslangValidator herunterladen falls nicht da
    if not os.path.exists(glslang_path):
        print("Lade glslangValidator herunter...")
        url = "https://github.com/KhronosGroup/glslang/releases/download/master-tot/glslang-master-windows-x64-Release.zip" if is_win else "https://github.com/KhronosGroup/glslang/releases/download/master-tot/glslang-master-linux-Release.zip"
        zip_path = os.path.join(tools_dir, "glslang.zip")
        urllib.request.urlretrieve(url, zip_path)
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            # Suchen der Binary im Zip
            for file in zip_ref.namelist():
                if file.endswith(glslang_exe):
                    basename = os.path.basename(file)
                    with open(os.path.join(tools_dir, basename), "wb") as f:
                        f.write(zip_ref.read(file))
        os.remove(zip_path)
        if not is_win: os.chmod(glslang_path, 0o755)

    # 2. Tint herunterladen falls nicht da
    if not os.path.exists(tint_path):
        print("Lade Google Tint herunter...")
        # Google Tint Standalone-Binary URL (Beispielhaft für Windows/Linux über Dawn-Builds)
        # Falls dieser Link veraltet, kann eine lokale tint-Binary im tools-Ordner abgelegt werden.
        # Alternativ nutzen wir hier den Fallback auf ein lokales Tool.
        if not os.path.exists(tint_path):
            print("[HINWEIS] Bitte lege die 'tint' Binary manuell in den 'tools/' Ordner.")
            
    return glslang_path, tint_path

def main():
    if len(sys.argv) < 3:
        print("Benutzung: python shader_cook.py <quell_ordner> <ziel_ordner>")
        sys.exit(1)
        
    src_dir = os.path.abspath(sys.argv[1])
    dst_dir = os.path.abspath(sys.argv[2])
    
    if not os.path.exists(src_dir):
        print(f"Fehler: Quellordner '{src_dir}' existiert nicht!")
        sys.exit(1)
        
    os.makedirs(dst_dir, exist_ok=True)
    
    # Tools besorgen
    glslang, tint = get_tools()
    
    valid_extensions = {".vert", ".frag", ".comp", ".geom"}
    
    for filename in os.listdir(src_dir):
        ext = os.path.splitext(filename)[1]
        if ext not in valid_extensions:
            continue
            
        glsl_path = os.path.join(src_dir, filename)
        
        # 1. Generiere temporäres/finales SPIR-V für Vulkan
        spv_path = os.path.join(dst_dir, f"{filename}.spv")
        print(f"Kompiliere {filename} -> SPIR-V...")
        
        # glslangValidator benötigt das Target-Environment Flag für Vulkan 1.3
        result = subprocess.run([glslang, "-V", "--target-env", "vulkan1.3", glsl_path, "-o", spv_path], capture_output=True, text=True)
        if result.returncode != 0:
            print(f"[FEHLER GLSL] In Datei {filename}:\n{result.stdout}\n{result.stderr}")
            continue
            
        # 2. Generiere WGSL aus dem SPIR-V für WebGPU (falls tint vorhanden ist)
        if os.path.exists(tint):
            wgsl_path = os.path.join(dst_dir, f"{filename}.wgsl")
            print(f"Übersetze {filename}.spv -> WGSL...")
            
            result = subprocess.run([tint, spv_path, "--format", "wgsl", "-o", wgsl_path], capture_output=True, text=True)
            if result.returncode != 0:
                print(f"[FEHLER TINT] Konnte {filename} nicht in WGSL übersetzen:\n{result.stderr}")
        else:
            print(f"[WARNUNG] 'tint' nicht gefunden. WebGPU-Shader für {filename} übersprungen.")

    print("\nFertig! Alle Binaries und Shaders wurden exportiert.")

if __name__ == "__main__":
    main()