# Helix RND — instrucciones del proyecto

## Propósito y stack
Biblioteca/API de renderizado 2D/3D con API pública C, núcleo C++20 y rasterizador por software; Vulkan es opcional y algunos backends/features siguen en fase inicial.
Stack: C11, C++20, CMake 3.20+, clang-format; dependencias C/C++ vendorizadas.

## Estructura
- `include/`: API pública C (`helix.h`).
- `src/core/`: ciclo de vida, handles, errores, recursos, GPU y entidades.
- `src/math/`: tipos y operaciones matemáticas.
- `src/platform/`: ventanas/entrada y adaptadores por sistema operativo.
- `src/render/software/`: renderer por software y rasterización.
- `src/thirdparty/`: dependencias vendorizadas; no editar salvo necesidad.
- `tests/`: pruebas C/C++ para API y subsistemas.
- `demos/`: ejemplos ejecutables, incluido triángulo headless y mini juego visual Windows.
- `bindings/`: vacío; wrapper C# está en `packaging/nuget/HelixRND/`.
- `packaging/`, `pkg/`: configuración y artefactos de distribución.
- `docs/`: arquitectura, decisiones, dependencias, vocabulario y publicación.
- `scripts/`: automatización auxiliar.
- `.github/workflows/`: CI y empaquetado de releases.

## Comandos
- Configurar: `cmake -S . -B build -G Ninja -DHX_BUILD_TESTS=ON -DHX_BUILD_HEADLESS_DEMO=ON`
- Compilar: `cmake --build build --config Release --parallel`
- Probar: `ctest --test-dir build --output-on-failure`
- Suite integral en Windows: `.\scripts\verify-v2.ps1 -BuildDir build-test -Configuration Debug`
- Test gráfico interactivo Windows: configurar con `-DHX_BUILD_VISUAL_TEST=ON`, compilar `helix_visual_smoke` y ejecutar `build-test\Debug\helix_visual_smoke.exe`.
- Formatear: `clang-format -i <archivos C/C++ modificados>`

## Convenciones
- Mantener C11 en la interfaz y C++20 en la implementación; respetar API y handles opacos.
- Aplicar `.clang-format`: LLVM, 4 espacios, llaves Allman, 120 columnas, sin tabs.
- Usar targets/opciones CMake existentes; cubrir cambios funcionales con pruebas en `tests/`.
- Matrices column-major: `hx_mul_mat4(a, b, out)` calcula `a × b`; `hx_make_mat4_trs` usa T×R×S.
- Mantener separadas las capas core, plataforma y render; no modificar código vendorizado sin motivo.

## Problemas/mejoras detectadas (prioridad)
1. **P1** Render headless soporta triángulos y textura 2D; faltan alfa, iluminación PBR y topologías adicionales.
2. **P1** Windows/Linux X11 son las plataformas objetivo; Wayland/WASM no están soportados y macOS queda fuera del alcance actual.
3. **P2** Implementar o recortar de la API pública texto/fuentes, rig/clip, FX, shader y dibujo a ventana.
4. **P2** Ampliar la referencia visual del rasterizador y mantener cobertura de captura PNG headless en CI Windows/Linux.
5. **P3** Binding Python solo si vuelve al alcance; README documenta C/C# como APIs previstas.
