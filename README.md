# Helix RND

Helix RND es una biblioteca de renderizado 2D/3D con API pública en C y núcleo C++20. El render headless dibuja triángulos sólidos y con textura 2D; también hay atlas RGBA8 con regiones en coordenadas explícitas, tween escalar y guardado PNG de imágenes/ventanas headless. PBR, dibujo a ventanas visibles, fuentes/texto, rig/clip, efectos y shaders no están soportados: sus APIs devuelven NULL, `HX_ERR_NOT_IMPLEMENTED` o no hacen nada según su firma. Windows y Linux/X11 son las plataformas objetivo; Wayland y WASM no están soportados. El backend software es la opción predeterminada; Vulkan se habilita con CMake.

## Requisitos

- CMake 3.20 o posterior.
- Compilador C11 y C++20.
- Ninja opcional (usado en los ejemplos).

## Compilar y probar

```sh
cmake -S . -B build -G Ninja -DHX_BUILD_TESTS=ON -DHX_BUILD_HEADLESS_DEMO=ON
cmake --build build --config Release --parallel
ctest --test-dir build --output-on-failure
```

El test `helix_headless_triangle` ejecuta el demo y genera `triangle.png`; `test_scene_render` compara una escena completa contra su hash de referencia.

## Uso

La API pública está en `include/helix.h`. El demo mínimo de render headless está en `demos/headless_triangle/main.cpp`; úsalo como ejemplo de inicialización con `HX_GPU_SOFT` y apagado del motor. Para integrar la API C# disponible, consulta `packaging/nuget/HelixRND/README.md`. No hay binding Python todavía. Linux usa X11; Wayland no está incluido, y Cocoa/WASM quedan limitados a ventanas headless hasta implementar sus adaptadores nativos.

El loop de ventana llama a `hx_tick`, actualiza/renderiza y luego llama a `hx_show`. `hx_get_win_dt` devuelve el tiempo transcurrido entre ticks. Se puede limitar cada ventana con `hx_set_win_fps_limit(win, 60)`; `0` significa sin límite. El límite se aplica dentro de `hx_tick`, también en headless; en ventanas nativas, VSync tiene prioridad.

Las matrices `HxMat4` usan almacenamiento column-major y vectores columna. `hx_mul_mat4(&a, &b, &out)` calcula `a × b`; `hx_make_mat4_trs` compone traslación, rotación y escala en ese orden.

## Desarrollo

- Convenciones: `.clang-format` (LLVM, 4 espacios, llaves Allman, 120 columnas).
- Agrega pruebas C/C++ en `tests/` y registra nuevos ejecutables con CTest en `CMakeLists.txt`.
- La guía de contribución y dependencias está en `CONTRIBUTING.md` y `docs/DEPENDENCIES.md`.
