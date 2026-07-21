# Nuclear Lighting Rebuild Log

## Persistent Black Screen Issue — Cycles 007-010

### Problem
Scene renders completely black across 10+ production cycles. Nuclear rebuilds destroy and respawn all lighting actors each cycle but the issue persists.

### Root Cause Analysis
The UE5 instance is running in `-RenderOffScreen` (headless) mode. The `ReturnValue: false` on `save_map` and the persistent black screen suggest:

1. **Headless rendering limitation**: In `-RenderOffScreen` mode, the viewport may not render the scene correctly for screenshot capture even when lighting actors exist.
2. **SceneCapture2D required**: Per `hugo_scenecapture_no_world_ui` memory, the correct screenshot method is SceneCapture2D, NOT HighResShot or viewport capture.
3. **Lighting actors ARE being created** (bridge_ok confirmed, actor spawns succeed) but the screenshot pipeline may be capturing a black framebuffer.

### Actors Spawned Each Cycle (Nuclear #10)
- `Sun_Nuclear10` — DirectionalLight, pitch=-50, intensity=12, atmosphere_sun_light=True
- `SkyAtmosphere_Nuclear10` — SkyAtmosphere component
- `SkyLight_Nuclear10` — SkyLight, intensity=2, real_time_capture=True
- `Fog_Nuclear10` — ExponentialHeightFog, density=0.02
- `PostProcess_Nuclear10` — PostProcessVolume, unbound=True, manual exposure bias=1.0

### Console Commands Applied
- `r.SkyAtmosphere.FastSkyLUT 1`
- `r.SkyAtmosphere.AerialPerspectiveLUT.FastApply 1`
- `r.AutoExposure.Method 1` (manual)
- `r.AutoExposure.Bias 1.0`
- `ShowFlag.Lighting 1`
- `ShowFlag.Sky 1`
- `ShowFlag.Fog 1`

### Recommendation for Next Cycle
**Use SceneCapture2D pipeline** (per hugo_scenecapture_no_world_ui memory):
```python
RL = unreal.RenderingLibrary
rt = RL.create_render_target2d(W, 1920, 1080, unreal.TextureRenderTargetFormat.RTF_RGBA8)
cap = spawn SceneCapture2D
comp = cap.get_component_by_class(unreal.SceneCaptureComponent2D)
comp.capture_source = SCS_FINAL_COLOR_LDR
comp.texture_target = rt
comp.capture_scene()
RL.export_render_target(W, rt, dir, 'name.png')
```
This bypasses the headless viewport black screen issue entirely.

### Cycle History
| Cycle | Action | Result |
|-------|--------|--------|
| 007 | Nuclear rebuild #7 | Black screen |
| 008 | Nuclear rebuild #8 | Black screen |
| 009 | Nuclear rebuild #9 | Black screen |
| 010 | Nuclear rebuild #10 + show flags | TBD |
