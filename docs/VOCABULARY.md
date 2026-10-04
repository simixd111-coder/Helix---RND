# Helix Vocabulary — Token Dictionary

**Version:** 0.1.0 (Phase 1)
**Status:** Living document — updated each phase
**Source of truth:** `include/helix.h` — this document must match the header exactly.

---

## Naming Rules

| Rule | Description |
|------|-------------|
| **Language** | Simple English, short readable words; compounds with `_` allowed (`double_sided`, `render_headless`) |
| **No Spanish** | Never use literal Spanish words |
| **One concept = one word** | No synonyms in API |
| **Pattern** | `verb + object` (e.g., `make_win`, `drop_mesh`, `spin_mesh`) |
| **Verbs (fixed set)** | `make`, `load`, `drop`, `add`, `draw`, `move`, `spin`, `size`, `look`, `set`, `get`, `snap`, `say`, `tween`, `on` |
| **Prefixes** | `hx_` (C functions), `Hx` (types), `HX_` (constants), `helix.` (other languages) |
| **Abbreviations (allowed)** | `cam` (camera), `tex` (texture), `pic` (picture/image), `mat` (matrix), `vec` (vector), `quat` (quaternion), `prim` (primitive), `buf` (buffer), `pad` (gamepad), `dt` (delta time), `fov` (field of view), `uv` (texture coords), `srgb`, `r8`, `rgba16f`, `d24s8`, `bump` (bumper), `stick` (thumbstick), `trigger` |

### Language Style Convention

| Language | Style | Example |
|----------|-------|---------|
| C | `snake_case` | `hx_make_win()` |
| C++ | `snake_case` | `hx::make_win()` |
| Rust | `snake_case` | `hx::make_win()` |
| Python | `snake_case` | `hx.make_win()` |
| Lua | `snake_case` | `hx.make_win()` |
| C# | `PascalCase` | `Hx.MakeWin()` |
| Go | `PascalCase` | `hx.MakeWin()` |
| Java | `camelCase` | `Hx.makeWin()` |
| JavaScript/WASM | `camelCase` | `helix.makeWin()` |

---

## Core Tokens (Phase 1)

### Lifecycle
| Token | C API | Meaning |
|-------|-------|---------|
| `boot` | `hx_boot(const HxCfg*)` | Initialize engine with config |
| `quit` | `hx_quit()` | Shutdown engine, destroy all |
| `version` | `hx_version()` | Query runtime version |

### Boot Configuration
| Token | C API | Meaning |
|-------|-------|---------|
| `cfg` | `HxCfg` | Boot configuration struct |
| `gpu` | `HxGpu` | GPU backend preference |
| `gpu_auto` | `HX_GPU_AUTO` | Auto-select: Vulkan → OpenGL/Metal → Software |
| `gpu_vk` | `HX_GPU_VK` | Force Vulkan |
| `gpu_gl` | `HX_GPU_GL` | Force OpenGL |
| `gpu_metal` | `HX_GPU_METAL` | Force Metal (macOS/iOS) |
| `gpu_soft` | `HX_GPU_SOFT` | Force Software rasterizer |
| `backend` | `HxBackend` | Backend actually in use |
| `backend_vulkan` | `HX_BACKEND_VULKAN` | Vulkan active |
| `backend_opengl` | `HX_BACKEND_OPENGL` | OpenGL active |
| `backend_metal` | `HX_BACKEND_METAL` | Metal active |
| `backend_software` | `HX_BACKEND_SOFTWARE` | Software active |
| `get_backend` | `hx_get_backend()` | Query active backend |
| `get_backend_name` | `hx_get_backend_name()` | Backend name string |
| `get_gpu_vendor` | `hx_get_gpu_vendor()` | GPU vendor string |
| `get_gpu_renderer` | `hx_get_gpu_renderer()` | GPU renderer string |
| `get_gpu_version` | `hx_get_gpu_version()` | API version string |

### Error Handling
| Token | C API | Meaning |
|-------|-------|---------|
| `last_error` | `hx_last_error()` | Get last error string (thread-local) |
| `ok` | `HX_OK` | Success result code |
| `err` | `HX_ERR_*` | Error result codes |
| `err_already_dropped` | `HX_ERR_ALREADY_DROPPED` | Double drop attempted (safe) |

---

### Window
| Token | C API | Meaning |
|-------|-------|---------|
| `win` | `HxWin`, `hx_make_win()` | Window handle & creation |
| `make_win` | `hx_make_win()` | Create window |
| `make_win_foreign` | `hx_make_win_foreign()` | Wrap existing native handle |
| `drop_win` | `hx_drop_win()` | Destroy window (safe to call twice) |
| `tick` | `hx_tick()` | Poll events, advance frame |
| `show` | `hx_show()` | Present / swap buffers |
| `get_win_alive` | `hx_get_win_alive()` | Window not closed |
| `get_win_focused` | `hx_get_win_focused()` | Has keyboard focus |
| `get_win_minimized` | `hx_get_win_minimized()` | Iconified / occluded |
| `get_win_size` | `hx_get_win_size()` | Framebuffer size (pixels) |
| `get_win_dpi_scale` | `hx_get_win_dpi_scale()` | HiDPI scale factor |
| `get_win_dt` | `hx_get_win_dt()` | Delta time (seconds) |
| `get_win_time` | `hx_get_win_time()` | Total time since boot |
| `set_win_title` | `hx_set_win_title()` | Change window title |
| `set_win_size` | `hx_set_win_size()` | Resize window |
| `set_win_vsync` | `hx_set_win_vsync()` | Toggle vsync |
| `set_win_fullscreen` | `hx_set_win_fullscreen()` | Toggle fullscreen |
| `snap_win` | `hx_snap_win()` | Capture window to PNG |

#### Window Flags
| Token | Constant | Meaning |
|-------|----------|---------|
| `none` | `HX_WIN_NONE` | Default |
| `fullscreen` | `HX_WIN_FULLSCREEN` | Exclusive fullscreen |
| `borderless` | `HX_WIN_BORDERLESS` | No decorations |
| `resizable` | `HX_WIN_RESIZABLE` | User can resize |
| `vsync` | `HX_WIN_VSYNC` | Sync to vblank |
| `hidpi` | `HX_WIN_HIDPI` | High-DPI backing store |
| `hidden` | `HX_WIN_HIDDEN` | Create invisible |
| `headless` | `HX_WIN_HEADLESS` | Offscreen only |
| `foreign` | `HX_WIN_FOREIGN` | Wrap native handle |

---

### Events — Unified Callback System
| Token | C API | Meaning |
|-------|-------|---------|
| `on` | `hx_on()` | Register event callback (replaces all set_*_cb) |
| `event` | `HxEvent` | Event structure |
| `event_type` | `HxEventType` | Event type bitmask |

#### Event Types
| Token | Constant | Meaning |
|-------|----------|---------|
| `key_down` | `HX_EV_KEY_DOWN` | Key pressed |
| `key_up` | `HX_EV_KEY_UP` | Key released |
| `key_repeat` | `HX_EV_KEY_REPEAT` | Key repeat |
| `mouse_btn_down` | `HX_EV_MOUSE_BTN_DOWN` | Mouse button pressed |
| `mouse_btn_up` | `HX_EV_MOUSE_BTN_UP` | Mouse button released |
| `mouse_move` | `HX_EV_MOUSE_MOVE` | Mouse moved |
| `mouse_wheel` | `HX_EV_MOUSE_WHEEL` | Mouse wheel scrolled |
| `resize` | `HX_EV_RESIZE` | Window resized |
| `close` | `HX_EV_CLOSE` | Close requested |
| `focus_gained` | `HX_EV_FOCUS_GAINED` | Window gained focus |
| `focus_lost` | `HX_EV_FOCUS_LOST` | Window lost focus |
| `pad_connected` | `HX_EV_PAD_CONNECTED` | Gamepad connected |
| `pad_disconnected` | `HX_EV_PAD_DISCONNECTED` | Gamepad disconnected |
| `pad_btn_down` | `HX_EV_PAD_BTN_DOWN` | Gamepad button pressed |
| `pad_btn_up` | `HX_EV_PAD_BTN_UP` | Gamepad button released |
| `pad_stick` | `HX_EV_PAD_STICK` | Gamepad stick moved |
| `pad_trigger` | `HX_EV_PAD_TRIGGER` | Gamepad trigger changed |

---

### Input — Keyboard
| Token | C API | Meaning |
|-------|-------|---------|
| `key` | `HxKey` | Key code (GLFW-aligned values, but Helix does NOT depend on GLFW) |
| `get_key_state` | `hx_get_key_state()` | Up / Down / Repeat |

#### Key States
| Token | Constant |
|-------|----------|
| `key_state_up` | `HX_KEY_STATE_UP` |
| `key_state_down` | `HX_KEY_STATE_DOWN` |
| `key_state_repeat` | `HX_KEY_STATE_REPEAT` |

---

### Input — Mouse
| Token | C API | Meaning |
|-------|-------|---------|
| `mouse_btn` | `HxMouseBtn` | Mouse button index |
| `get_mouse_btn` | `hx_get_mouse_btn()` | Button pressed? |
| `get_mouse_pos` | `hx_get_mouse_pos()` | Cursor position |
| `get_mouse_delta` | `hx_get_mouse_delta()` | Movement since last tick |
| `get_mouse_wheel` | `hx_get_mouse_wheel()` | Scroll accumulation |

#### Mouse Buttons
| Token | Constant |
|-------|----------|
| `mouse_left` | `HX_MOUSE_LEFT` |
| `mouse_right` | `HX_MOUSE_RIGHT` |
| `mouse_middle` | `HX_MOUSE_MIDDLE` |

---

### Input — Gamepad
| Token | C API | Meaning |
|-------|-------|---------|
| `pad` | `hx_get_pad_*()` | Gamepad (0–3) |
| `get_pad_connected` | `hx_get_pad_connected()` | Is plugged in? |
| `get_pad_buttons` | `hx_get_pad_buttons()` | Button bitmask |
| `get_pad_stick` | `hx_get_pad_stick()` | Left/right stick XY |
| `get_pad_trigger` | `hx_get_pad_trigger()` | LT/RT analog |

#### Pad Buttons (bitmask) — Clear Names
| Token | Constant | Meaning |
|-------|----------|---------|
| `pad_a` | `HX_PAD_A` | A button |
| `pad_b` | `HX_PAD_B` | B button |
| `pad_x` | `HX_PAD_X` | X button |
| `pad_y` | `HX_PAD_Y` | Y button |
| `pad_bump_l` | `HX_PAD_BUMP_L` | Left bumper (LB) |
| `pad_bump_r` | `HX_PAD_BUMP_R` | Right bumper (RB) |
| `pad_trigger_l` | `HX_PAD_TRIGGER_L` | Left trigger (LT) |
| `pad_trigger_r` | `HX_PAD_TRIGGER_R` | Right trigger (RT) |
| `pad_back` | `HX_PAD_BACK` | Back button |
| `pad_start` | `HX_PAD_START` | Start button |
| `pad_stick_l` | `HX_PAD_STICK_L` | Left stick press |
| `pad_stick_r` | `HX_PAD_STICK_R` | Right stick press |
| `pad_up` | `HX_PAD_UP` | D-pad up |
| `pad_down` | `HX_PAD_DOWN` | D-pad down |
| `pad_left` | `HX_PAD_LEFT` | D-pad left |
| `pad_right` | `HX_PAD_RIGHT` | D-pad right |

---

### World / Scene
| Token | C API | Meaning |
|-------|-------|---------|
| `world` | `HxWorld`, `hx_make_world()` | Scene root |
| `make_world` | `hx_make_world()` | Create world |
| `drop_world` | `hx_drop_world()` | Destroy world |
| `add_mesh` | `hx_add_mesh()` | Add mesh+skin+transform to world |
| `clear_world` | `hx_clear_world()` | Remove all mesh instances (does NOT destroy resources) |

---

### Mesh (Geometry)
| Token | C API | Meaning |
|-------|-------|---------|
| `mesh` | `HxMesh`, `hx_make_mesh()` | Geometry handle |
| `make_mesh` | `hx_make_mesh()` | Create mesh from raw data |
| `make_cube` | `hx_make_cube()` | Unit cube |
| `make_sphere` | `hx_make_sphere()` | UV sphere |
| `make_plane` | `hx_make_plane()` | XZ plane |
| `make_quad` | `hx_make_quad()` | Unit quad at Z=0 |
| `drop_mesh` | `hx_drop_mesh()` | Destroy mesh (auto-removes from world) |
| `move_mesh` | `hx_move_mesh()` | Translate mesh |
| `spin_mesh` | `hx_spin_mesh()` | Rotate mesh (radians) |
| `size_mesh` | `hx_size_mesh()` | Scale mesh |

#### Vertex Attributes (flags)
| Token | Constant | Meaning |
|-------|----------|---------|
| `vert_pos` | `HX_VERT_POS` | Position (required) |
| `vert_normal` | `HX_VERT_NORMAL` | Normal |
| `vert_tangent` | `HX_VERT_TANGENT` | Tangent |
| `vert_uv0` | `HX_VERT_UV0` | Texcoord 0 |
| `vert_uv1` | `HX_VERT_UV1` | Texcoord 1 |
| `vert_color` | `HX_VERT_COLOR` | Vertex color |
| `vert_joints` | `HX_VERT_JOINTS` | Skin joints |
| `vert_weights` | `HX_VERT_WEIGHTS` | Skin weights |

#### Primitive Types
| Token | Constant |
|-------|----------|
| `prim_triangles` | `HX_PRIM_TRIANGLES` |
| `prim_lines` | `HX_PRIM_LINES` |
| `prim_points` | `HX_PRIM_POINTS` |
| `prim_tri_strip` | `HX_PRIM_TRI_STRIP` |

---

### Skin (Material) — NOT Skeletal Skinning
| Token | C API | Meaning |
|-------|-------|---------|
| `skin` | `HxSkin`, `hx_make_skin()` | Material handle |
| `make_skin` | `hx_make_skin()` | Create material from color |
| `make_skin_tex` | `hx_make_skin_tex()` | Create textured material |
| `make_skin_pbr` | `hx_make_skin_pbr()` | Create PBR material |
| `drop_skin` | `hx_drop_skin()` | Destroy material |

#### Skin Flags
| Token | Constant | Meaning |
|-------|----------|---------|
| `skin_none` | `HX_SKIN_NONE` | Default |
| `skin_unlit` | `HX_SKIN_UNLIT` | No lighting |
| `skin_wireframe` | `HX_SKIN_WIREFRAME` | Wireframe |
| `skin_double_sided` | `HX_SKIN_DOUBLE_SIDED` | No backface cull |
| `skin_transparent` | `HX_SKIN_TRANSPARENT` | Alpha blend |
| `skin_masked` | `HX_SKIN_MASKED` | Alpha test (cutout) |
| `skin_emissive` | `HX_SKIN_EMISSIVE` | Emissive color |

> **Note:** `skin` = material. For skeletal animation, use `rig` (not `skin`).

---

### Camera
| Token | C API | Meaning |
|-------|-------|---------|
| `cam` | `HxCam`, `hx_make_cam3d()`, `hx_make_cam2d()` | Camera handle |
| `make_cam3d` | `hx_make_cam3d()` | Perspective camera |
| `make_cam2d` | `hx_make_cam2d()` | Orthographic camera |
| `look` | `hx_look()` | Look-at (target, eye, up) |
| `set_cam_persp` | `hx_set_cam_persp()` | Perspective params |
| `set_cam_ortho` | `hx_set_cam_ortho()` | Ortho params |
| `move_cam2d` | `hx_move_cam2d()` | 2D pan |
| `size_cam2d` | `hx_size_cam2d()` | 2D zoom (scale) |
| `get_cam_view` | `hx_get_cam_view()` | View matrix |
| `get_cam_proj` | `hx_get_cam_proj()` | Projection matrix |
| `get_cam_view_proj` | `hx_get_cam_view_proj()` | Combined VP |
| `drop_cam` | `hx_drop_cam()` | Destroy camera |

#### Camera Types
| Token | Constant |
|-------|----------|
| `cam_3d` | `HX_CAM_3D` |
| `cam_2d` | `HX_CAM_2D` |

---

### Light (Lamp)
| Token | C API | Meaning |
|-------|-------|---------|
| `lamp` | `HxLamp`, `hx_make_lamp()` | Light handle |
| `make_lamp` | `hx_make_lamp()` | Create light |
| `lamp_sun` | `HX_LAMP_SUN` | Directional light |
| `lamp_point` | `HX_LAMP_POINT` | Point light |
| `lamp_spot` | `HX_LAMP_SPOT` | Spot light |
| `set_lamp_dir` | `hx_set_lamp_dir()` | Sun direction |
| `set_lamp_range` | `hx_set_lamp_range()` | Point/spot range |
| `set_lamp_spot` | `hx_set_lamp_spot()` | Spot cone angles |
| `drop_lamp` | `hx_drop_lamp()` | Destroy light |

---

### Texture (GPU Resource)
| Token | C API | Meaning |
|-------|-------|---------|
| `tex` | `HxTex`, `hx_make_tex()` | Texture handle |
| `make_tex` | `hx_make_tex()` | Create texture from raw pixels |
| `make_tex_cube` | `hx_make_tex_cube()` | Create cubemap |
| `load_tex` | `hx_load_tex()` | Load texture from file (GPU upload) |
| `drop_tex` | `hx_drop_tex()` | Destroy texture |

#### Texture Flags
| Token | Constant | Meaning |
|-------|----------|---------|
| `tex_none` | `HX_TEX_NONE` | Default |
| `tex_srgb` | `HX_TEX_SRGB` | sRGB → linear |
| `tex_mipmaps` | `HX_TEX_MIPMAPS` | Generate mips |
| `tex_repeat` | `HX_TEX_REPEAT` | Wrap repeat |
| `tex_mirror` | `HX_TEX_MIRROR` | Wrap mirror |
| `tex_linear` | `HX_TEX_LINEAR` | Linear filter |

#### Texture Formats
| Token | Constant |
|-------|----------|
| `tex_fmt_r8` | `HX_TEX_FMT_R8` |
| `tex_fmt_rg8` | `HX_TEX_FMT_RG8` |
| `tex_fmt_rgb8` | `HX_TEX_FMT_RGB8` |
| `tex_fmt_rgba8` | `HX_TEX_FMT_RGBA8` |
| `tex_fmt_bgra8` | `HX_TEX_FMT_BGRA8` |
| `tex_fmt_r16f` | `HX_TEX_FMT_R16F` |
| `tex_fmt_rg16f` | `HX_TEX_FMT_RG16F` |
| `tex_fmt_rgb16f` | `HX_TEX_FMT_RGB16F` |
| `tex_fmt_rgba16f` | `HX_TEX_FMT_RGBA16F` |
| `tex_fmt_r32f` | `HX_TEX_FMT_R32F` |
| `tex_fmt_rg32f` | `HX_TEX_FMT_RG32F` |
| `tex_fmt_rgb32f` | `HX_TEX_FMT_RGB32F` |
| `tex_fmt_rgba32f` | `HX_TEX_FMT_RGBA32F` |
| `tex_fmt_d24s8` | `HX_TEX_FMT_D24S8` |
| `tex_fmt_d32f` | `HX_TEX_FMT_D32F` |

---

### Picture (CPU-side Image)
| Token | C API | Meaning |
|-------|-------|---------|
| `pic` | `HxPic`, `hx_load_pic()` | Image in CPU memory |
| `load_pic` | `hx_load_pic()` | Load image file to CPU |
| `save_pic` | `hx_save_pic()` | Save to PNG |
| `get_pic_size` | `hx_get_pic_size()` | Get dimensions |
| `get_pic_pixels` | `hx_get_pic_pixels()` | Get RGBA8 pixels |
| `drop_pic` | `hx_drop_pic()` | Destroy picture |

---

### Shader
| Token | C API | Meaning |
|-------|-------|---------|
| `shader` | `HxShader`, `hx_load_shader()` | Shader program (embedded bytecode) |
| `load_shader` | `hx_load_shader()` | Load embedded shader by name |
| `drop_shader` | `hx_drop_shader()` | Destroy shader |

---

### Buffer (GPU)
| Token | C API | Meaning |
|-------|-------|---------|
| `buf` | `HxBuffer`, `hx_make_buffer()` | GPU buffer |
| `make_buffer` | `hx_make_buffer()` | Create buffer |
| `write_buffer` | `hx_write_buffer()` | CPU → GPU |
| `read_buffer` | `hx_read_buffer()` | GPU → CPU |
| `drop_buffer` | `hx_drop_buffer()` | Destroy buffer |

#### Buffer Flags
| Token | Constant | Meaning |
|-------|----------|---------|
| `buf_none` | `HX_BUF_NONE` | Default |
| `buf_vertex` | `HX_BUF_VERTEX` | Vertex buffer |
| `buf_index` | `HX_BUF_INDEX` | Index buffer |
| `buf_uniform` | `HX_BUF_UNIFORM` | Uniform/constant buffer |
| `buf_storage` | `HX_BUF_STORAGE` | Storage buffer (SSBO) |
| `buf_dynamic` | `HX_BUF_DYNAMIC` | Frequent updates |
| `buf_map_write` | `HX_BUF_MAP_WRITE` | Persistent write map |
| `buf_map_read` | `HX_BUF_MAP_READ` | Persistent read map |

---

### Draw
| Token | C API | Meaning |
|-------|-------|---------|
| `draw_world` | `hx_draw_world()` | Render world + cam → window |
| `draw_mesh` | `hx_draw_mesh()` | Single mesh + skin + transform |
| `begin_pass` | `hx_begin_pass()` | Begin custom render pass |
| `end_pass` | `hx_end_pass()` | End custom render pass |

---

### Math (POD types + helpers)
| Token | Type | Meaning |
|-------|------|---------|
| `vec2` | `HxVec2` | 2D vector |
| `vec3` | `HxVec3` | 3D vector |
| `vec4` | `HxVec4` | 4D vector / color |
| `mat4` | `HxMat4` | 4×4 matrix (column-major) |
| `quat` | `HxQuat` | Quaternion (xyz=vector, w=scalar) |
| `color` | `HxColor` | Linear RGBA (0–1) |

#### Math Helpers — Consistent `make_` / `mul_` / `inverse_` / `transpose_` / `slerp_` / `rotate_` Prefixes
| Token | C API | Meaning |
|-------|-------|---------|
| `make_mat4_identity` | `hx_make_mat4_identity()` | Identity matrix |
| `mul_mat4` | `hx_mul_mat4()` | Multiply A×B |
| `make_mat4_translate` | `hx_make_mat4_translate()` | Translation matrix |
| `make_mat4_rotate` | `hx_make_mat4_rotate()` | Rotation from quat |
| `make_mat4_scale` | `hx_make_mat4_scale()` | Scale matrix |
| `make_mat4_trs` | `hx_make_mat4_trs()` | T×R×S combined |
| `inverse_mat4` | `hx_inverse_mat4()` | Inverse |
| `transpose_mat4` | `hx_transpose_mat4()` | Transpose |
| `make_quat_identity` | `hx_make_quat_identity()` | Identity quat |
| `mul_quat` | `hx_mul_quat()` | Multiply |
| `make_quat_axis_angle` | `hx_make_quat_axis_angle()` | Axis-angle |
| `make_quat_euler` | `hx_make_quat_euler()` | Euler XYZ |
| `slerp_quat` | `hx_slerp_quat()` | Spherical lerp |
| `rotate_vec_quat` | `hx_rotate_vec_quat()` | Rotate vector |

#### Common Colors
| Token | Value |
|-------|-------|
| `white` | `HX_WHITE` |
| `black` | `HX_BLACK` |
| `red` | `HX_RED` |
| `green` | `HX_GREEN` |
| `blue` | `HX_BLUE` |
| `yellow` | `HX_YELLOW` |
| `cyan` | `HX_CYAN` |
| `magenta` | `HX_MAGENTA` |
| `orange` | `HX_ORANGE` |
| `gray` | `HX_GRAY` |

---

### 2D / Sprite (Stubs — Phase 5)
| Token | C API | Meaning |
|-------|-------|---------|
| `atlas` | `HxAtlas`, `hx_make_atlas()` | Texture atlas builder |
| `make_atlas` | `hx_make_atlas()` | Create atlas |
| `add_atlas` | `hx_add_atlas()` | Add sub-texture |
| `build_atlas` | `hx_build_atlas()` | Pack & return texture |
| `drop_atlas` | `hx_drop_atlas()` | Destroy atlas |
| `font` | `HxFont`, `hx_load_font()` | TTF font |
| `load_font` | `hx_load_font()` | Load font |
| `drop_font` | `hx_drop_font()` | Destroy font |
| `say` | `hx_say()` | Draw text (stub) |

---

### Animation (Stubs — Phase 5/6)
| Token | C API | Meaning |
|-------|-------|---------|
| `tween` | `HxTween`, `hx_make_tween()` | Simple float tween |
| `make_tween` | `hx_make_tween()` | Create tween |
| `get_tween_value` | `hx_get_tween_value()` | Current value |
| `get_tween_done` | `hx_get_tween_done()` | Finished? |
| `drop_tween` | `hx_drop_tween()` | Destroy tween |
| `rig` | `HxRig`, `hx_load_rig()` | Skeletal rig |
| `load_rig` | `hx_load_rig()` | Load rig from glTF |
| `clip` | `HxClip`, `hx_load_clip()` | Animation clip |
| `load_clip` | `hx_load_clip()` | Load clip |
| `play_clip` | `hx_play_clip()` | Play animation |
| `drop_rig` | `hx_drop_rig()` | Destroy rig |
| `drop_clip` | `hx_drop_clip()` | Destroy clip |

> **Note:** `anim` is used only for the concept. Clips are `clip`. Skeletal skinning uses `rig`, not `skin`.

---

### Post-Process Effects (Stubs — Phase 7)
| Token | C API | Meaning |
|-------|-------|---------|
| `fx` | `HxFx`, `hx_add_fx()` | Post-process effect |
| `add_fx` | `hx_add_fx()` | Add effect to window |
| `drop_fx` | `hx_drop_fx()` | Remove effect |
| `fx_bloom` | `HX_FX_BLOOM` | Bloom |
| `fx_ssao` | `HX_FX_SSAO` | SSAO |
| `fx_fxaa` | `HX_FX_FXAA` | FXAA |
| `fx_tonemap` | `HX_FX_TONEMAP` | Tonemapping |

---

### Headless / Offscreen
| Token | C API | Meaning |
|-------|-------|---------|
| `render_headless` | `hx_render_headless()` | Render to memory buffer |

---

### Logging
| Token | C API | Meaning |
|-------|-------|---------|
| `log` | `hx_set_log_cb()` | Set log callback |

#### Log Levels
| Token | Constant |
|-------|----------|
| `log_trace` | `HX_LOG_TRACE` |
| `log_debug` | `HX_LOG_DEBUG` |
| `log_info` | `HX_LOG_INFO` |
| `log_warn` | `HX_LOG_WARN` |
| `log_error` | `HX_LOG_ERROR` |
| `log_fatal` | `HX_LOG_FATAL` |

---

## Ownership & Lifetime

| Handle | Creator | Owner | Drop Function | Double Drop |
|--------|---------|-------|---------------|-------------|
| `HxWin` | `hx_make_win` / `hx_make_win_foreign` | Caller | `hx_drop_win` | Safe → `HX_ERR_ALREADY_DROPPED` |
| `HxWorld` | `hx_make_world` | Caller | `hx_drop_world` | Safe |
| `HxMesh` | `hx_make_mesh` / `hx_make_cube` etc. | Caller | `hx_drop_mesh` | Safe |
| `HxSkin` | `hx_make_skin` / `hx_make_skin_tex` / `hx_make_skin_pbr` | Caller | `hx_drop_skin` | Safe |
| `HxCam` | `hx_make_cam3d` / `hx_make_cam2d` | Caller | `hx_drop_cam` | Safe |
| `HxLamp` | `hx_make_lamp` | Caller | `hx_drop_lamp` | Safe |
| `HxTex` | `hx_make_tex` / `hx_load_tex` | Caller | `hx_drop_tex` | Safe |
| `HxPic` | `hx_load_pic` | Caller | `hx_drop_pic` | Safe |
| `HxShader` | `hx_load_shader` | Caller | `hx_drop_shader` | Safe |
| `HxBuffer` | `hx_make_buffer` | Caller | `hx_drop_buffer` | Safe |
| `HxAtlas` | `hx_make_atlas` | Caller | `hx_drop_atlas` | Safe |
| `HxFont` | `hx_load_font` | Caller | `hx_drop_font` | Safe |
| `HxTween` | `hx_make_tween` | Caller | `hx_drop_tween` | Safe |
| `HxRig` | `hx_load_rig` | Caller | `hx_drop_rig` | Safe |
| `HxClip` | `hx_load_clip` | Caller | `hx_drop_clip` | Safe |
| `HxFx` | `hx_add_fx` | Caller | `hx_drop_fx` | Safe |

**On `hx_quit()`:** All remaining handles are destroyed automatically. No need to manually drop everything before quit, but explicit drops are recommended for deterministic resource management.

---

## Cross-Language Mapping (Updated)

| C | C++ | C# | Python | Rust | JS/WASM | Go | Java | Lua |
|---|-----|----|--------|------|---------|----|------|-----|
| `hx_boot()` | `hx::boot()` | `Hx.Boot()` | `hx.boot()` | `hx::boot()` | `helix.boot()` | `hx.Boot()` | `Hx.boot()` | `hx.boot()` |
| `hx_make_win()` | `hx::make_win()` | `Hx.MakeWin()` | `hx.make_win()` | `hx::make_win()` | `helix.makeWin()` | `hx.MakeWin()` | `Hx.makeWin()` | `hx.make_win()` |
| `hx_tick()` | `hx::tick()` | `Hx.Tick()` | `hx.tick()` | `hx::tick()` | `helix.tick()` | `hx.Tick()` | `Hx.tick()` | `hx.tick()` |
| `hx_draw_world()` | `hx::draw_world()` | `Hx.DrawWorld()` | `hx.draw_world()` | `hx::draw_world()` | `helix.drawWorld()` | `hx.DrawWorld()` | `Hx.drawWorld()` | `hx.draw_world()` |
| `hx_spin_mesh()` | `hx::spin_mesh()` | `Hx.SpinMesh()` | `hx.spin_mesh()` | `hx::spin_mesh()` | `helix.spinMesh()` | `hx.SpinMesh()` | `Hx.spinMesh()` | `hx.spin_mesh()` |
| `hx_on()` | `hx::on()` | `Hx.On()` | `hx.on()` | `hx::on()` | `helix.on()` | `hx.On()` | `Hx.on()` | `hx.on()` |
| `HxVec3` | `hx::Vec3` | `HxVec3` | `hx.Vec3` | `hx::Vec3` | `helix.Vec3` | `hx.Vec3` | `HxVec3` | `hx.vec3` |
| `HX_OK` | `hx::OK` | `HxResult.OK` | `hx.OK` | `hx::OK` | `helix.OK` | `hx.OK` | `HxResult.OK` | `hx.OK` |

---

## Reserved for Future Phases

| Phase | Tokens (planned) |
|-------|------------------|
| 2 | Complete window, software backend, triangle demo |
| 3 | `vk` (Vulkan backend), `spirv`, `pipeline`, `descriptor` |
| 4 | `gl` (OpenGL backend), `auto` (backend selection) |
| 5 | `sprite`, `layer`, `particle`, `tilemap`, `say` (text), `atlas` (full) |
| 6 | `gltf`, `obj`, `rig`, `clip`, `shadow`, `skybox` |
| 7 | `fx` (bloom, ssao, fxaa, tonemap), `profile` |
| 8 | `cocoa`, `wasm`, `webgpu` |
| 9 | `bind` (binding generators), `pkg` (packaging) |
| 10 | `cli`, `doctor`, `new`, `run`, `demo`, `update`, `pack` |

---

## Changelog

| Version | Date | Changes |
|---------|------|---------|
| 0.1.0 | 2026-10-03 | Phase 1: Complete vocabulary aligned with helix.h |