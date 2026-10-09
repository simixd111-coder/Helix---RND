// HelixRND — C# bindings for the Helix RND C API (Phase 1 surface).
// License: MIT
//
// Naming follows the Helix vocabulary convention: C# uses PascalCase
// (see docs/VOCABULARY.md). Handles are opaque pointers wrapped in
// safe structs; hx_quit() cleans up anything left over.

using System;
using System.Runtime.InteropServices;

namespace Helix
{
    /// <summary>Result codes returned by the Helix C API.</summary>
    public enum HxResult : int
    {
        Ok = 0,
        Err = -1,
        Oom = -2,
        InvalidArg = -3,
        InvalidHandle = -4,
        InvalidState = -5,
        NotImplemented = -6,
        Unsupported = -7,
        AlreadyDropped = -8,
        AlreadyBooted = -10,
        NotBooted = -11,
        BackendUnavailable = -12,
        WinCreate = -20,
        WinContext = -21,
        WinSurface = -22,
        ShaderCompile = -30,
        PipelineCreate = -31,
        TextureCreate = -32,
        BufferCreate = -33,
        MeshCreate = -34,
        DrawInvalid = -35,
        AssetNotFound = -40,
        AssetParse = -41,
        AssetUnsupported = -42,
        InputUnavailable = -50,
    }

    /// <summary>GPU backend requested at boot.</summary>
    public enum Gpu : byte
    {
        Auto = 0,
        Vk = 1,
        Gl = 2,
        Metal = 3,
        Soft = 4,
    }

    /// <summary>Backend actually in use after boot.</summary>
    public enum Backend : byte
    {
        Unknown = 0,
        Vulkan = 1,
        OpenGL = 2,
        Metal = 3,
        Software = 4,
    }

    /// <summary>Adapter preference when auto-selecting a GPU device.</summary>
    public enum GpuPreference : byte
    {
        Auto = 0,
        HighPerformance = 1,
        LowPower = 2,
    }

    /// <summary>Window creation flags (bitmask).</summary>
    [Flags]
    public enum WinFlags : uint
    {
        None = 0u,
        Fullscreen = 1u << 0,
        Borderless = 1u << 1,
        Resizable = 1u << 2,
        VSync = 1u << 3,
        HiDpi = 1u << 4,
        Hidden = 1u << 5,
        Headless = 1u << 6,
        Foreign = 1u << 7,
    }

    /// <summary>Boot configuration. Zero-init is valid (Auto GPU, headless off).</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Cfg
    {
        public Gpu gpu;
        [MarshalAs(UnmanagedType.U1)] public bool headless;
        public IntPtr app_name;      // UTF-8 string pointer, optional
        public IntPtr reserved;
        public GpuPreference gpu_preference;
        public uint gpu_device_index;
    }

    /// <summary>Tracked resource and memory accounting.</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct MemoryStats
    {
        public UIntPtr live_resources;
        public UIntPtr cpu_bytes;
        public UIntPtr gpu_bytes;
    }

    /// <summary>
    /// Safe wrapper over a Helix window handle (HxWin).
    /// Dispose drops the window; hx_quit() reclaims anything left over.
    /// </summary>
    public sealed class Win : IDisposable
    {
        internal IntPtr Handle;

        private Win(IntPtr handle) => Handle = handle;

        /// <summary>Create a window. Returns null on failure (check Hx.LastError).</summary>
        public static Win? Make(int width, int height, string title, WinFlags flags)
        {
            var h = Native.hx_make_win(width, height, title, flags);
            return h == IntPtr.Zero ? null : new Win(h);
        }

        /// <summary>Poll events. Returns false once the window asked to close.</summary>
        public bool Tick() => Native.hx_tick(Handle);

        /// <summary>Present the frame.</summary>
        public void Show() => Native.hx_show(Handle);

        public bool Alive => Native.hx_get_win_alive(Handle);
        public bool Focused => Native.hx_get_win_focused(Handle);
        public bool Minimized => Native.hx_get_win_minimized(Handle);
        public double Dt => Native.hx_get_win_dt(Handle);
        public double Time => Native.hx_get_win_time(Handle);

        public (int W, int H) Size
        {
            get
            {
                Native.hx_get_win_size(Handle, out var w, out var h);
                return (w, h);
            }
        }

        public void Dispose()
        {
            if (Handle != IntPtr.Zero)
            {
                Native.hx_drop_win(Handle);
                Handle = IntPtr.Zero;
            }
            GC.SuppressFinalize(this);
        }
    }

    /// <summary>
    /// Safe wrapper over a Helix font handle (HxFont).
    /// Dispose drops the font; hx_quit() reclaims anything left over.
    /// </summary>
    public sealed class Font : IDisposable
    {
        internal IntPtr Handle;

        private Font(IntPtr handle) => Handle = handle;

        /// <summary>Load a TTF font from a file path. Returns null on failure.</summary>
        public static Font? Load(string path, float size)
        {
            var h = Native.hx_load_font(path, size);
            return h == IntPtr.Zero ? null : new Font(h);
        }

        /// <summary>Load a TTF font from raw byte data. Returns null on failure.</summary>
        public static Font? Load(byte[] data, float size)
        {
            var h = Native.hx_load_font_mem(data, (UIntPtr)data.Length, size);
            return h == IntPtr.Zero ? null : new Font(h);
        }

        /// <summary>Measure a UTF-8 string, returning (width, height) in pixels.</summary>
        public (float W, float H) Measure(string text)
        {
            Native.hx_measure_text(Handle, text, out var w, out var h);
            return (w, h);
        }

        /// <summary>Draw UTF-8 text at (x, y) in screen coordinates (top-left origin).</summary>
        public void Draw(Win win, string text, float x, float y, in Color color)
        {
            Native.hx_draw_text(win.Handle, Handle, text, x, y, color);
        }

        public void Dispose()
        {
            if (Handle != IntPtr.Zero)
            {
                Native.hx_drop_font(Handle);
                Handle = IntPtr.Zero;
            }
            GC.SuppressFinalize(this);
        }
    }

    /// <summary>RGBA color with components in [0, 1].</summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Color
    {
        public float R;
        public float G;
        public float B;
        public float A;

        public Color(float r, float g, float b, float a = 1f)
        {
            R = r;
            G = g;
            B = b;
            A = a;
        }

        public static Color White => new Color(1f, 1f, 1f, 1f);
        public static Color Black => new Color(0f, 0f, 0f, 1f);
    }

    /// <summary>Entry points for the Helix engine lifecycle.</summary>
    public static class Hx
    {
        /// <summary>Initialize the engine. Call once before anything else.</summary>
        public static HxResult Boot(in Cfg cfg) => Native.hx_boot(in cfg);

        /// <summary>Shut down and destroy all remaining handles. Idempotent.</summary>
        public static void Quit() => Native.hx_quit();

        /// <summary>Human-readable string for the last error on this thread.</summary>
        public static string LastError => Native.hx_last_error();

        public static MemoryStats MemoryStats
        {
            get
            {
                Native.hx_get_memory_stats(out var stats);
                return stats;
            }
        }

        public static Backend Backend => Native.hx_get_backend();
        public static string BackendName(Backend b) => Native.hx_get_backend_name(b);

        public static (int Major, int Minor, int Patch) Version()
        {
            Native.hx_version(out var major, out var minor, out var patch);
            return (major, minor, patch);
        }
    }

    internal static class Native
    {
        private const string Lib = "helix";

        private const CharSet Utf8 = CharSet.Ansi; // marshalled as UTF-8 via BestFitMapping=false

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall, EntryPoint = "hx_boot")]
        internal static extern HxResult hx_boot(in Cfg cfg);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern void hx_get_memory_stats(out MemoryStats out_stats);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern void hx_quit();

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern void hx_version(out int major, out int minor, out int patch);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern Backend hx_get_backend();

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall, CharSet = Utf8)]
        internal static extern string hx_get_backend_name(Backend b);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall, CharSet = Utf8)]
        internal static extern string hx_last_error();

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall, CharSet = Utf8, BestFitMapping = false, ThrowOnUnmappableChar = true)]
        internal static extern IntPtr hx_make_win(int width, int height, string title, WinFlags flags);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern HxResult hx_drop_win(IntPtr win);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static extern bool hx_tick(IntPtr win);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern void hx_show(IntPtr win);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static extern bool hx_get_win_alive(IntPtr win);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static extern bool hx_get_win_focused(IntPtr win);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.U1)]
        internal static extern bool hx_get_win_minimized(IntPtr win);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern void hx_get_win_size(IntPtr win, out int w, out int h);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern double hx_get_win_dt(IntPtr win);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern double hx_get_win_time(IntPtr win);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall, CharSet = Utf8, BestFitMapping = false, ThrowOnUnmappableChar = true)]
        internal static extern IntPtr hx_load_font(string path, float size);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern IntPtr hx_load_font_mem(byte[] data, UIntPtr size, float pt_size);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall)]
        internal static extern HxResult hx_drop_font(IntPtr font);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall, CharSet = Utf8, BestFitMapping = false, ThrowOnUnmappableChar = true)]
        internal static extern void hx_measure_text(IntPtr font, string text, out float w, out float h);

        [DllImport(Lib, CallingConvention = CallingConvention.StdCall, CharSet = Utf8, BestFitMapping = false, ThrowOnUnmappableChar = true)]
        internal static extern void hx_draw_text(IntPtr win, IntPtr font, string text, float x, float y, in Color color);
    }
}
