# Rename Mapping Table — Phase 1 Vocabulary Alignment

This table documents all renames from the old API to the new strict vocabulary convention.
Use this to update existing code and bindings.

## Legend
- **Old Name** → **New Name** (Action: Added/Modified/Removed)
- Functions follow `hx_<verb>_<object>` pattern
- Verbs: `make`, `load`, `drop`, `add`, `draw`, `move`, `spin`, `size`, `look`, `set`, `get`, `snap`, `say`, `tween`, `on`

---

## Lifecycle & Boot

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_boot()` | `hx_boot(const HxCfg*)` | Modified (added config) |
| `hx_quit()` | `hx_quit()` | Unchanged |
| `hx_version()` | `hx_version()` | Unchanged |
| — | `hx_get_backend()` | Added |
| — | `hx_get_backend_name()` | Added |
| — | `hx_get_gpu_vendor()` | Added (was `hx_gpu_vendor`) |
| — | `hx_get_gpu_renderer()` | Added (was `hx_gpu_renderer`) |
| — | `hx_get_gpu_version()` | Added (was `hx_gpu_version`) |

---

## Window

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_make_win()` | `hx_make_win()` | Unchanged |
| `hx_win_from_foreign()` | `hx_make_win_foreign()` | Renamed (verb `make`) |
| `hx_drop_win()` | `hx_drop_win()` | Returns `HxResult` now |
| `hx_win_tick()` | `hx_tick()` | Renamed (verb `tick` on win) |
| `hx_win_show()` | `hx_show()` | Renamed (verb `show` on win) |
| `hx_win_alive()` | `hx_get_win_alive()` | Renamed (verb `get`) |
| `hx_win_focused()` | `hx_get_win_focused()` | Renamed (verb `get`) |
| `hx_win_minimized()` | `hx_get_win_minimized()` | Renamed (verb `get`) |
| `hx_win_size()` | `hx_get_win_size()` | Renamed (verb `get`) |
| `hx_win_dpi_scale()` | `hx_get_win_dpi_scale()` | Renamed (verb `get`) |
| `hx_win_dt()` | `hx_get_win_dt()` | Renamed (verb `get`) |
| `hx_win_time()` | `hx_get_win_time()` | Renamed (verb `get`) |
| `hx_win_set_title()` | `hx_set_win_title()` | Renamed (verb `set`) |
| `hx_win_set_size()` | `hx_set_win_size()` | Renamed (verb `set`) |
| `hx_win_set_vsync()` | `hx_set_win_vsync()` | Renamed (verb `set`) |
| `hx_win_set_fullscreen()` | `hx_set_win_fullscreen()` | Renamed (verb `set`) |
| — | `hx_snap_win()` | Added (capture to PNG) |

---

## Events (Unified Callback System)

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_win_set_key_cb()` | `hx_on()` | Replaced |
| `hx_win_set_mouse_btn_cb()` | `hx_on()` | Replaced |
| `hx_win_set_mouse_move_cb()` | `hx_on()` | Replaced |
| `hx_win_set_mouse_wheel_cb()` | `hx_on()` | Replaced |
| `hx_win_set_resize_cb()` | `hx_on()` | Replaced |
| `hx_win_set_close_cb()` | `hx_on()` | Replaced |
| — | `HxEvent` struct | Added |
| — | `HxEventType` enum | Added |
| — | `HxEventCallback` type | Added |

---

## Input — Keyboard

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_key_state()` | `hx_get_key_state()` | Renamed (verb `get`) |
| `HX_KEY_UP` | `HX_KEY_STATE_UP` | Renamed (prefix `key_state_`) |
| `HX_KEY_DOWN` | `HX_KEY_STATE_DOWN` | Renamed (prefix `key_state_`) |
| `HX_KEY_REPEAT` | `HX_KEY_STATE_REPEAT` | Renamed (prefix `key_state_`) |

---

## Input — Mouse

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_mouse_btn()` | `hx_get_mouse_btn()` | Renamed (verb `get`) |
| `hx_mouse_pos()` | `hx_get_mouse_pos()` | Renamed (verb `get`) |
| `hx_mouse_delta()` | `hx_get_mouse_delta()` | Renamed (verb `get`) |
| `hx_mouse_wheel()` | `hx_get_mouse_wheel()` | Renamed (verb `get`) |

---

## Input — Gamepad

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_pad_connected()` | `hx_get_pad_connected()` | Renamed (verb `get`) |
| `hx_pad_buttons()` | `hx_get_pad_buttons()` | Renamed (verb `get`) |
| `hx_pad_stick()` | `hx_get_pad_stick()` | Renamed (verb `get`) |
| `hx_pad_trigger()` | `hx_get_pad_trigger()` | Renamed (verb `get`) |
| `HX_PAD_LB` | `HX_PAD_BUMP_L` | Renamed (clear name) |
| `HX_PAD_RB` | `HX_PAD_BUMP_R` | Renamed (clear name) |
| `HX_PAD_LT` | `HX_PAD_TRIGGER_L` | Renamed (clear name) |
| `HX_PAD_RT` | `HX_PAD_TRIGGER_R` | Renamed (clear name) |
| `HX_PAD_LS` | `HX_PAD_STICK_L` | Renamed (clear name) |
| `HX_PAD_RS` | `HX_PAD_STICK_R` | Renamed (clear name) |

---

## World / Scene

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_make_world()` | `hx_make_world()` | Unchanged |
| `hx_drop_world()` | `hx_drop_world()` | Returns `HxResult` now |
| `hx_world_add()` | `hx_add_mesh()` | Renamed (verb `add`, object `mesh`) |
| `hx_world_drop()` | *(removed)* | Removed — `hx_drop_mesh(mesh)` auto-removes from world |
| `hx_world_clear()` | `hx_clear_world()` | Renamed (verb `clear`, object `world`) |

---

## Mesh (Geometry)

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_make_mesh()` | `hx_make_mesh()` | Unchanged |
| `hx_make_cube()` | `hx_make_cube()` | Unchanged |
| `hx_make_sphere()` | `hx_make_sphere()` | Unchanged |
| `hx_make_plane()` | `hx_make_plane()` | Unchanged |
| `hx_make_quad()` | `hx_make_quad()` | Unchanged |
| `hx_drop_mesh()` | `hx_drop_mesh()` | Returns `HxResult` now; auto-removes from world |
| — | `hx_move_mesh()` | Added (translate) |
| — | `hx_spin_mesh()` | Added (rotate radians) |
| — | `hx_size_mesh()` | Added (scale) |

---

## Skin (Material)

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_make_skin()` | `hx_make_skin()` | Unchanged |
| `hx_make_skin_tex()` | `hx_make_skin_tex()` | Unchanged |
| `hx_make_skin_pbr()` | `hx_make_skin_pbr()` | Unchanged |
| `hx_drop_skin()` | `hx_drop_skin()` | Returns `HxResult` now |

> **Note:** `skin` = material. For skeletal animation, use `rig` (not `skin`).

---

## Camera

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_make_cam3d()` | `hx_make_cam3d()` | Unchanged |
| `hx_make_cam2d()` | `hx_make_cam2d()` | Unchanged |
| `hx_cam_look()` | `hx_look()` | Renamed (verb `look`) |
| `hx_cam_persp()` | `hx_set_cam_persp()` | Renamed (verb `set`) |
| `hx_cam_ortho()` | `hx_set_cam_ortho()` | Renamed (verb `set`) |
| `hx_cam2d_move()` | `hx_move_cam2d()` | Renamed (verb `move`) |
| `hx_cam2d_zoom()` | `hx_size_cam2d()` | Renamed (verb `size` = zoom) |
| `hx_cam_view()` | `hx_get_cam_view()` | Renamed (verb `get`) |
| `hx_cam_proj()` | `hx_get_cam_proj()` | Renamed (verb `get`) |
| `hx_cam_view_proj()` | `hx_get_cam_view_proj()` | Renamed (verb `get`) |
| `hx_drop_cam()` | `hx_drop_cam()` | Returns `HxResult` now |

---

## Light (Lamp)

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_make_lamp()` | `hx_make_lamp()` | Unchanged |
| `hx_lamp_sun_dir()` | `hx_set_lamp_dir()` | Renamed (verb `set`) |
| `hx_lamp_range()` | `hx_set_lamp_range()` | Renamed (verb `set`) |
| `hx_lamp_spot_angles()` | `hx_set_lamp_spot()` | Renamed (verb `set`) |
| `hx_drop_lamp()` | `hx_drop_lamp()` | Returns `HxResult` now |

---

## Texture (GPU)

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_make_tex()` | `hx_make_tex()` | Unchanged |
| `hx_make_tex_cube()` | `hx_make_tex_cube()` | Unchanged |
| `hx_load_tex()` | `hx_load_tex()` | Unchanged |
| `hx_drop_tex()` | `hx_drop_tex()` | Returns `HxResult` now |

---

## Picture (CPU Image) — NEW

| Old Name | New Name | Action |
|----------|----------|--------|
| — | `HxPic` type | Added |
| — | `hx_load_pic()` | Added |
| — | `hx_save_pic()` | Added |
| — | `hx_get_pic_size()` | Added |
| — | `hx_get_pic_pixels()` | Added |
| — | `hx_drop_pic()` | Added |

---

## Shader

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_load_shader()` | `hx_load_shader()` | Unchanged |
| `hx_drop_shader()` | `hx_drop_shader()` | Returns `HxResult` now |

---

## Buffer (GPU)

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_make_buffer()` | `hx_make_buffer()` | Unchanged |
| `hx_buffer_write()` | `hx_write_buffer()` | Renamed (verb `write`) |
| `hx_buffer_read()` | `hx_read_buffer()` | Renamed (verb `read`) |
| `hx_drop_buffer()` | `hx_drop_buffer()` | Returns `HxResult` now |

---

## Draw

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_draw_world()` | `hx_draw_world()` | Unchanged |
| `hx_draw_mesh()` | `hx_draw_mesh()` | Unchanged |
| `hx_begin_pass()` | `hx_begin_pass()` | Unchanged |
| `hx_end_pass()` | `hx_end_pass()` | Unchanged |

---

## Math Helpers — Consistent Prefixes

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_mat4_identity()` | `hx_make_mat4_identity()` | Renamed (verb `make`) |
| `hx_mat4_mul()` | `hx_mul_mat4()` | Renamed (verb `mul`) |
| `hx_mat4_translate()` | `hx_make_mat4_translate()` | Renamed (verb `make`) |
| `hx_mat4_rotate()` | `hx_make_mat4_rotate()` | Renamed (verb `make`) |
| `hx_mat4_scale()` | `hx_make_mat4_scale()` | Renamed (verb `make`) |
| `hx_mat4_trs()` | `hx_make_mat4_trs()` | Renamed (verb `make`) |
| `hx_mat4_inverse()` | `hx_inverse_mat4()` | Renamed (verb `inverse`) |
| `hx_mat4_transpose()` | `hx_transpose_mat4()` | Renamed (verb `transpose`) |
| `hx_quat_identity()` | `hx_make_quat_identity()` | Renamed (verb `make`) |
| `hx_quat_mul()` | `hx_mul_quat()` | Renamed (verb `mul`) |
| `hx_quat_from_axis_angle()` | `hx_make_quat_axis_angle()` | Renamed (verb `make`) |
| `hx_quat_from_euler()` | `hx_make_quat_euler()` | Renamed (verb `make`) |
| `hx_quat_slerp()` | `hx_slerp_quat()` | Renamed (verb `slerp`) |
| `hx_quat_rotate_vec()` | `hx_rotate_vec_quat()` | Renamed (verb `rotate`) |

---

## 2D / Sprite (Stubs)

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_make_atlas()` | `hx_make_atlas()` | Unchanged |
| `hx_atlas_add()` | `hx_add_atlas()` | Renamed (verb `add`) |
| `hx_atlas_build()` | `hx_build_atlas()` | Renamed (verb `build`) |
| `hx_drop_atlas()` | `hx_drop_atlas()` | Returns `HxResult` now |
| `hx_load_font()` | `hx_load_font()` | Unchanged |
| `hx_drop_font()` | `hx_drop_font()` | Returns `HxResult` now |
| — | `hx_say()` | Added (text rendering stub) |

---

## Animation (Stubs)

| Old Name | New Name | Action |
|----------|----------|--------|
| — | `HxTween` type | Added |
| — | `hx_make_tween()` | Added |
| — | `hx_get_tween_value()` | Added |
| — | `hx_get_tween_done()` | Added |
| — | `hx_drop_tween()` | Added |
| — | `HxRig` type | Added (skeletal rig) |
| — | `hx_load_rig()` | Added |
| — | `HxClip` type | Added (animation clip) |
| — | `hx_load_clip()` | Added |
| — | `hx_play_clip()` | Added |
| — | `hx_drop_rig()` | Added |
| — | `hx_drop_clip()` | Added |

> **Note:** `anim` is used only for the concept. Clips are `clip`. Skeletal skinning uses `rig`, not `skin`.

---

## Post-Process Effects (Stubs)

| Old Name | New Name | Action |
|----------|----------|--------|
| — | `HxFx` type | Added |
| — | `hx_add_fx()` | Added |
| — | `hx_drop_fx()` | Added |
| — | `HX_FX_BLOOM` | Added |
| — | `HX_FX_SSAO` | Added |
| — | `HX_FX_FXAA` | Added |
| — | `HX_FX_TONEMAP` | Added |

---

## Headless / Offscreen

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_render_headless()` | `hx_render_headless()` | Unchanged |

---

## Logging

| Old Name | New Name | Action |
|----------|----------|--------|
| `hx_set_log_cb()` | `hx_set_log_cb()` | Unchanged |

---

## Error Codes — New

| Old Name | New Name | Action |
|----------|----------|--------|
| — | `HX_ERR_ALREADY_DROPPED` | Added (safe double-drop) |

---

## Constants — Renamed for Consistency

| Old Name | New Name | Action |
|----------|----------|--------|
| `HX_WIN_NONE` | `HX_WIN_NONE` | Unchanged |
| `HX_SKIN_NONE` | `HX_SKIN_NONE` | Unchanged |
| `HX_BUF_NONE` | `HX_BUF_NONE` | Unchanged |
| `HX_TEX_NONE` | `HX_TEX_NONE` | Unchanged |

---

## Cross-Language Mapping Updates

| C (Old) | C (New) | C++ | C# | Python | Rust | JS/WASM | Go | Java | Lua |
|---------|---------|-----|-----|--------|------|---------|----|------|-----|
| `hx_win_tick()` | `hx_tick()` | `hx::tick()` | `Hx.Tick()` | `hx.tick()` | `hx::tick()` | `helix.tick()` | `hx.Tick()` | `Hx.tick()` | `hx.tick()` |
| `hx_win_show()` | `hx_show()` | `hx::show()` | `Hx.Show()` | `hx.show()` | `hx::show()` | `helix.show()` | `hx.Show()` | `Hx.show()` | `hx.show()` |
| `hx_win_alive()` | `hx_get_win_alive()` | `hx::get_win_alive()` | `Hx.GetWinAlive()` | `hx.get_win_alive()` | `hx::get_win_alive()` | `helix.getWinAlive()` | `hx.GetWinAlive()` | `Hx.getWinAlive()` | `hx.get_win_alive()` |
| `hx_world_add()` | `hx_add_mesh()` | `hx::add_mesh()` | `Hx.AddMesh()` | `hx.add_mesh()` | `hx::add_mesh()` | `helix.addMesh()` | `hx.AddMesh()` | `Hx.addMesh()` | `hx.add_mesh()` |
| `hx_world_drop()` | *(removed)* | — | — | — | — | — | — | — | — |
| `hx_cam_look()` | `hx_look()` | `hx::look()` | `Hx.Look()` | `hx.look()` | `hx::look()` | `helix.look()` | `hx.Look()` | `Hx.look()` | `hx.look()` |
| `hx_cam_persp()` | `hx_set_cam_persp()` | `hx::set_cam_persp()` | `Hx.SetCamPersp()` | `hx.set_cam_persp()` | `hx::set_cam_persp()` | `helix.setCamPersp()` | `hx.SetCamPersp()` | `Hx.setCamPersp()` | `hx.set_cam_persp()` |
| `hx_mat4_mul()` | `hx_mul_mat4()` | `hx::mul_mat4()` | `Hx.MulMat4()` | `hx.mul_mat4()` | `hx::mul_mat4()` | `helix.mulMat4()` | `hx.MulMat4()` | `Hx.mulMat4()` | `hx.mul_mat4()` |
| `hx_quat_slerp()` | `hx_slerp_quat()` | `hx::slerp_quat()` | `Hx.SlerpQuat()` | `hx.slerp_quat()` | `hx::slerp_quat()` | `helix.slerpQuat()` | `hx.SlerpQuat()` | `Hx.slerpQuat()` | `hx.slerp_quat()` |

---

## Summary of Breaking Changes

1. **All `hx_win_*` → `hx_*` or `hx_get_win_*` / `hx_set_win_*`**
2. **All `hx_world_*` → `hx_*_mesh` / `hx_*_world`**
3. **All `hx_cam_*` → `hx_look` / `hx_set_cam_*` / `hx_move_cam2d` / `hx_size_cam2d` / `hx_get_cam_*`**
4. **All `hx_lamp_*` → `hx_set_lamp_*`**
5. **All `hx_buffer_*` → `hx_write_buffer` / `hx_read_buffer`**
6. **All `hx_mat4_*` / `hx_quat_*` → consistent `make_`/`mul_`/`inverse_`/`transpose_`/`slerp_`/`rotate_` prefixes**
7. **Callbacks replaced by single `hx_on()`**
7. **Gamepad constants renamed for clarity (`lb`→`bump_l`, `ls`→`stick_l`, etc.)**
8. **All `drop` functions return `HxResult` (safe double-drop)**
9. **Boot takes `HxCfg*` with `HxGpu` enum (separate from `HxBackend`)**

---

## Migration Script (Conceptual)

```bash
# Example sed commands for automated migration (review before running!)
sed -i 's/hx_win_tick/hx_tick/g' *.c *.h
sed -i 's/hx_win_show/hx_show/g' *.c *.h
sed -i 's/hx_win_alive/hx_get_win_alive/g' *.c *.h
sed -i 's/hx_world_add/hx_add_mesh/g' *.c *.h
# NOTE: hx_world_drop was removed — hx_drop_mesh(mesh) now auto-removes from world
sed -i 's/hx_cam_look/hx_look/g' *.c *.h
sed -i 's/hx_cam_persp/hx_set_cam_persp/g' *.c *.h
sed -i 's/hx_mat4_mul/hx_mul_mat4/g' *.c *.h
sed -i 's/hx_quat_slerp/hx_slerp_quat/g' *.c *.h
# ... etc.
```

**Always run tests after migration:** `ctest --output-on-failure`