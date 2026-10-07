# HelixRND — NuGet package (C# bindings)

Installs the C# bindings for Helix RND via P/Invoke.

## Usage

The C# wrapper exposes the native APIs listed below. The per-window FPS cap is currently available through the C API, not this wrapper. Package version is controlled by HelixRND.csproj; update it before packing a release.

```bash
dotnet add package HelixRND
```

```csharp
using Helix;

var cfg = new Cfg { gpu = Gpu.Soft, headless = true };
if (Hx.Boot(cfg) != HxResult.Ok)
{
    Console.WriteLine(Hx.LastError);
    return 1;
}

using var win = Win.Make(320, 240, "My Engine", WinFlags.Headless);
if (win is null) { Hx.Quit(); return 2; }

var stats = Hx.MemoryStats;
Console.WriteLine($"live={stats.live_resources}");

Hx.Quit();
```

## Build the package

The native library must be built first and placed under `runtimes/`:

```bash
# 1. Build the native library (Windows example)
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DHX_BUILD_SHARED=ON
cmake --build build --config Release

# 2. Stage the native binaries
mkdir -p runtimes/win-x64/native
cp build/helix.dll runtimes/win-x64/native/        # Windows
# cp build/libhelix.so runtimes/linux-x64/native/   # Linux

# 3. Pack
dotnet pack -c Release
```

For release 2.5.0, the resulting `HelixRND.2.5.0.nupkg` can be pushed to nuget.org after updating the project version:

```bash
dotnet nuget push HelixRND.2.5.0.nupkg --api-key <KEY> --source https://api.nuget.org/v3/index.json
```

> Rebuild and re-stage the native DLL for every release
> (see `docs/PUBLISHING.md`, Paso 3).
