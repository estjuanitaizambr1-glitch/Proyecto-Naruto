#!/usr/bin/env python3
"""Comprueba archivos del proyecto antes de compilar o subirlo a GitHub."""
from pathlib import Path
import json

root = Path(__file__).resolve().parent.parent
required = ["glad/include/glad/glad.h", "glad/src/glad.c", "stb_image.h",
            "shaders/model.vert", "shaders/model.frag"]
missing = []

for filename in required:
    if not (root / filename).is_file():
        missing.append(filename)

for folder in ("konoha", "pajaro", "naruto"):
    gltf = root / folder / "scene.gltf"
    if not gltf.exists():
        missing.append(str(gltf.relative_to(root)))
        continue
    try:
        data = json.loads(gltf.read_text(encoding="utf-8"))
        for item in data.get("buffers", []) + data.get("images", []):
            uri = item.get("uri")
            if uri and not uri.startswith("data:") and not (gltf.parent / uri).is_file():
                missing.append(str(Path(folder) / uri))
    except (OSError, ValueError) as exc:
        print(f"Error leyendo {gltf.name} de {folder}: {exc}")
        missing.append(str(gltf.relative_to(root)))

if missing:
    print("FALTAN ESTOS RECURSOS (copialos del proyecto anterior):")
    for name in sorted(set(missing)):
        print("  -", name)
    raise SystemExit(1)
print("OK: shaders, GLAD, stb_image y dependencias de los 3 glTF presentes.")
