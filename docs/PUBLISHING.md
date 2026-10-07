# Publicar Helix RND — Guía paso a paso

Este documento explica cómo publicar Helix RND en los tres canales de distribución soportados:

| Canal | Comando de usuario | Qué se publica |
|-------|-------------------|----------------|
| **Chocolatey** (Windows) | `choco install helix-rnd` | ZIP con `helix.h` + `helix-core.lib` + `helix-platform.lib` + `helix-render-software.lib` + `helix.dll` |
| **NuGet** (C#) | `dotnet add package HelixRND` | Paquete con bindings C# + `helix.dll` (runtimes/win-x64/native) |
| **CMake / FetchContent** (C/C++) | `find_package(helix)` | Config CMake instalado + headers + libs |

---

## Requisitos previos

- Cuenta en **GitHub** con permisos de push al repo `simixd111-coder/Helix---RND`
- Cuenta en **Chocolatey Community** (https://community.chocolatey.org)
- Cuenta en **NuGet.org** (https://www.nuget.org)
- **Chocolatey API Key** (desde https://community.chocolatey.org/account/apikey)
- En NuGet.org, Trusted Publishing configurado para `simixd111-coder/Helix---RND` y `.github/workflows/release.yml`; crear el secreto GitHub `NUGET_USER` con el nombre de usuario NuGet. ([guía oficial](https://learn.microsoft.com/en-us/nuget/nuget-org/trusted-publishing))
- Para publicar NuGet manualmente desde local, una **NuGet API Key** (desde https://www.nuget.org/account/apikeys).

---

## Paso 1 — Preparar el release local

### 1.1 Compilar librería estática (para Chocolatey)

```powershell
cd C:\Users\Simon\Desktop\Helix RND

# Limpiar y configurar
Remove-Item -Recurse -Force build-release -ErrorAction SilentlyContinue
cmake -B build-release -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DHX_STATIC_RUNTIME=ON `
  -DHX_BUILD_TESTS=OFF `
  -DHX_BUILD_HEADLESS_DEMO=OFF `
  -DHX_BACKEND_VULKAN=ON `
  -DHX_BACKEND_SOFTWARE=ON

# Compilar
cmake --build build-release --config Release --parallel
```

Verificar que se generaron:
```
build-release/Release/helix-core.lib
build-release/Release/helix-platform.lib
build-release/Release/helix-render-software.lib
```

### 1.2 Compilar librería compartida (para NuGet / C#)

```powershell
# Limpiar y configurar
Remove-Item -Recurse -Force build-shared -ErrorAction SilentlyContinue
cmake -B build-shared -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_BUILD_TYPE=Release `
  -DHX_BUILD_SHARED=ON `
  -DHX_BUILD_TESTS=OFF `
  -DHX_BUILD_HEADLESS_DEMO=OFF `
  -DHX_BACKEND_VULKAN=ON `
  -DHX_BACKEND_SOFTWARE=ON

# Compilar
cmake --build build-shared --config Release --parallel
```

Verificar que se generó:
```
build-shared/Release/helix.dll
build-shared/Release/helix.lib   # import library
```

### 1.3 Armar el ZIP para Chocolatey

El script de Chocolatey espera un ZIP con esta estructura:

```
helix-rnd-2.0.0-windows-x64/
├── include/
│   └── helix.h
├── lib/
│   ├── helix-core.lib
│   ├── helix-platform.lib
│   └── helix-render-software.lib
├── bin/
│   └── helix.dll
└── LICENSE
```

```powershell
$version = "2.0.0"
$zipName = "helix-rnd-$version-windows-x64"
$staging = "pkg/$zipName"

# Crear estructura
New-Item -ItemType Directory -Force -Path "$staging/include" | Out-Null
New-Item -ItemType Directory -Force -Path "$staging/lib" | Out-Null
New-Item -ItemType Directory -Force -Path "$staging/bin" | Out-Null

# Copiar archivos
Copy-Item include/helix.h "$staging/include/"
Copy-Item build-release/Release/helix-core.lib, build-release/Release/helix-platform.lib, build-release/Release/helix-render-software.lib "$staging/lib/"
Copy-Item build-shared/Release/helix.dll "$staging/bin/"
Copy-Item LICENSE "$staging/"

# Crear ZIP
Compress-Archive -Path "$staging" -DestinationPath "pkg/$zipName.zip"

# Calcular SHA256 (necesario para el nuspec)
$hash = (Get-FileHash "pkg/$zipName.zip" -Algorithm SHA256).Hash
Write-Host "SHA256: $hash"
"$hash  $zipName.zip" | Out-File "pkg/$zipName.zip.sha256" -Encoding ascii
```

### 1.4 Actualizar el nuspec con el SHA256 real

Edita `packaging/chocolatey/helix-rnd.nuspec` y reemplaza:

```xml
<checksum64>REPLACE_WITH_SHA256_OF_ZIP</checksum64>
```

por el hash que acabas de calcular.

---

## Paso 2 — Crear el GitHub Release

El workflow `.github/workflows/release.yml` hace esto automáticamente al pushear un tag `v*`:

```powershell
# Crear y pushear el tag
git tag -a v2.0.0 -m "Release 2.0.0"
git push origin v2.0.0
```

Esto disparará el workflow que:
1. Compila en Windows/Linux
2. Corre los tests
3. Arma los ZIPs con headers + libs
4. Calcula SHA256
5. Crea el **GitHub Release** con los assets subidos

> **Nota**: El workflow actual usa `softprops/action-gh-release@v2` que requiere el permiso `contents: write` en el token de GitHub Actions (ya configurado en el YAML).

---

## Paso 3 — Publicar en Chocolatey

### 3.1 Empaquetar el .nupkg

```powershell
cd packaging/chocolatey
choco pack helix-rnd.nuspec
```

Esto genera `helix-rnd.2.0.0.nupkg` en el directorio actual.

El instalador verifica el ZIP con el asset `.sha256` publicado junto al ZIP de Windows en GitHub Releases. Publica ambos assets antes de enviar el paquete a Chocolatey.

### 3.2 Subir a Chocolatey Community

```powershell
choco push helix-rnd.2.0.0.nupkg --api-key TU_CHAVE_API --source https://push.chocolatey.org/
```

> La primera versión pasa por **revisión manual** (puede tardar horas/días). Las actualizaciones posteriores son automáticas.

### 3.3 Verificar

```powershell
choco install helix-rnd --version 2.0.0 --source https://community.chocolatey.org/api/v2
```

---

## Paso 4 — Publicar en NuGet (C#)

### 4.1 Preparar el paquete con la DLL nativa

```powershell
cd packaging/nuget/HelixRND

# Crear estructura de runtimes
$runtimes = "runtimes/win-x64/native"
New-Item -ItemType Directory -Force -Path $runtimes | Out-Null

# Copiar la DLL compartida
Copy-Item "C:\Users\Simon\Desktop\Helix RND\build-shared\Release\helix.dll" "$runtimes/"

# Verificar estructura
# runtimes/win-x64/native/helix.dll
```

### 4.2 Empaquetar

```powershell
dotnet pack -c Release -o ../nupkg
```

Esto genera `HelixRND.2.0.0.nupkg` en `packaging/nuget/nupkg/`.

### 4.3 Subir a NuGet.org

```powershell
dotnet nuget push ../nupkg/HelixRND.2.0.0.nupkg --api-key TU_NUGET_API_KEY --source https://api.nuget.org/v3/index.json
```

### 4.4 Verificar

```powershell
dotnet add package HelixRND --version 2.0.0
```

---

## Paso 5 — Verificar consumo desde C/C++ (CMake)

Una vez publicado el release en GitHub, los usuarios de C/C++ pueden consumirlo vía `FetchContent` sin instalar nada:

```cmake
# En su CMakeLists.txt
include(FetchContent)

FetchContent_Declare(
  helix
  GIT_REPOSITORY https://github.com/simixd111-coder/Helix---RND.git
  GIT_TAG        v2.0.0
)

FetchContent_MakeAvailable(helix)

# Ahora pueden linkear contra helix::helix
target_link_libraries(mi_app PRIVATE helix::helix)
```

O si descargan el ZIP del release manualmente:

```cmake
# helix-config.cmake está en el ZIP bajo lib/cmake/helix/
find_package(helix REQUIRED PATHS /ruta/al/zip/extraido/lib/cmake/helix)
target_link_libraries(mi_app PRIVATE helix::helix)
```

---

## Paso 6 — Automatizar futuros releases

Para no repetir los pasos manuales, puedes:

1. **Añadir un job al workflow `release.yml`** que:
   - Descargue los artifacts del build
   - Arme el ZIP de Chocolatey
   - Calcule SHA256 y actualice el nuspec
   - Haga `choco pack` y `choco push` (necesita `CHOCOLATEY_API_KEY` como secret)

2. **Añadir un job que:**
   - Copie `helix.dll` a `packaging/nuget/HelixRND/runtimes/win-x64/native/`
   - Haga `dotnet pack` y `dotnet nuget push` (necesita `NUGET_API_KEY` como secret)

Ejemplo de snippet para el workflow:

```yaml
# En .github/workflows/release.yml, dentro del job release:
- name: Package Chocolatey
  if: runner.os == 'Windows'
  shell: pwsh
  run: |
    # ... (pasos 1.3 y 1.4 de arriba)
    choco pack packaging/chocolatey/helix-rnd.nuspec
    choco push helix-rnd.2.0.0.nupkg --api-key ${{ secrets.CHOCOLATEY_API_KEY }} --source https://push.chocolatey.org/

- name: Package NuGet
  if: runner.os == 'Windows'
  shell: pwsh
  run: |
    $runtimes = "packaging/nuget/HelixRND/runtimes/win-x64/native"
    New-Item -ItemType Directory -Force -Path $runtimes
    Copy-Item build-shared/Release/helix.dll "$runtimes/"
    dotnet pack packaging/nuget/HelixRND -c Release -o packaging/nuget/nupkg
    dotnet nuget push packaging/nuget/nupkg/HelixRND.2.0.0.nupkg --api-key ${{ secrets.NUGET_API_KEY }} --source https://api.nuget.org/v3/index.json
  env:
    CHOCOLATEY_API_KEY: ${{ secrets.CHOCOLATEY_API_KEY }}
    NUGET_API_KEY: ${{ secrets.NUGET_API_KEY }}
```

---

## Checklist de release

- [ ] `git tag -a vX.Y.Z -m "Release X.Y.Z"` y `git push origin vX.Y.Z`
- [ ] Workflow de release pasa (✅ en Actions)
- [ ] GitHub Release creado con assets (ZIPs + SHA256)
- [ ] ZIP de Windows y su asset `.sha256` publicados juntos en GitHub Releases
- [ ] `choco pack` + `choco push` → Chocolatey Community
- [ ] `helix.dll` copiada a `runtimes/win-x64/native/`
- [ ] `dotnet pack` + `dotnet nuget push` → NuGet.org
- [ ] Verificar `choco install helix-rnd` y `dotnet add package HelixRND`

---

## Troubleshooting común

| Problema | Solución |
|----------|----------|
| Chocolatey falla con "checksum mismatch" | Regenera el asset `.sha256` a partir del ZIP y vuelve a publicar ambos assets |
| `dotnet nuget push` falla con "version already exists" | Incrementa la versión en `HelixRND.csproj` (`<Version>`) |
| C# `DllImport` no encuentra `helix.dll` | Verifica que `helix.dll` esté en `runtimes/win-x64/native/` dentro del nupkg |
| Chocolatey revisión tarda mucho | Es normal en la primera versión; las siguientes son automáticas |
| Build compartido falla con símbolos CRT | Asegúrate de que `HX_BUILD_SHARED=ON` use `/MD` (ya corregido en CMakeLists) |

---

## Referencias

- [Chocolatey Package Creation](https://docs.chocolatey.org/en-us/create/create-packages)
- [NuGet Package Publishing](https://docs.microsoft.com/en-us/nuget/nuget-org/publish-a-package)
- [GitHub Releases API](https://docs.github.com/en/rest/releases/releases)
- [CMake FetchContent](https://cmake.org/cmake/help/latest/module/FetchContent.html)
