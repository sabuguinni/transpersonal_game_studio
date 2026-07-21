# Integration Checklist v8 — Transpersonal Game Studio

**Maintained by:** Agent #19 — Integration & Build Agent  
**Last updated:** PROD_CYCLE_AUTO_20260630_008  

---

## MANDATORY PRE-BUILD CHECKS (every cycle)

### 1. Bridge Validation
```python
import unreal
print('bridge_ok')
world = unreal.EditorLevelLibrary.get_editor_world()
print(f'World: {world.get_name()}')
actors = unreal.EditorLevelLibrary.get_all_level_actors()
print(f'Actor count: {len(actors)}')
```
**Pass criteria:** `bridge_ok` printed, world name returned, actor count > 0

---

### 2. CAP Enforcement (Critical Atmosphere Parameters)

| Parameter | Required Value | Check Method |
|-----------|---------------|--------------|
| Sun (DirectionalLight) pitch | ≤ -30° | `get_actor_rotation().pitch` |
| ExponentialHeightFog count | Exactly 1 | Count instances, destroy duplicates |
| r.SkyAtmosphere.FastSkyLUT | 1 | Console command |
| SkyLight.real_time_capture | True | Component property |

**Action on fail:** Auto-correct and save map.

---

### 3. Core C++ Class Validation (7/7 required)

| Class Path | Expected |
|-----------|----------|
| `/Script/TranspersonalGame.TranspersonalCharacter` | Loadable |
| `/Script/TranspersonalGame.TranspersonalGameState` | Loadable |
| `/Script/TranspersonalGame.PCGWorldGenerator` | Loadable |
| `/Script/TranspersonalGame.FoliageManager` | Loadable |
| `/Script/TranspersonalGame.CrowdSimulationManager` | Loadable |
| `/Script/TranspersonalGame.ProceduralWorldManager` | Loadable |
| `/Script/TranspersonalGame.BuildIntegrationManager` | Loadable |

**Pass criteria:** All 7 return non-None from `unreal.load_class()`

---

### 4. MinPlayableMap Actor Inventory

| Actor Type | Minimum Count | Notes |
|-----------|--------------|-------|
| PlayerStart | 1 | At or near origin |
| DirectionalLight | 1 | Sun, pitch ≤ -30° |
| SkyAtmosphere | 1 | |
| ExponentialHeightFog | 1 | Deduplicated |
| SkyLight | 1 | RTC enabled |
| Landscape | 1 | With height variation |
| StaticMeshActor (props) | 10+ | Trees, rocks |
| Dinosaur pawns | 5 | TRex, 3 Raptors, Brachiosaurus |
| NavMeshBoundsVolume | 1 | |
| TriggerBox | 3 | Interaction triggers |

---

### 5. Source Pairing Check

Every `.h` file MUST have a matching `.cpp` file.  
Exceptions: `SharedTypes.h`, `TranspersonalGame.h`

```python
import glob, os
src_dir = os.path.join(project_dir, 'Source', 'TranspersonalGame')
headers = {os.path.splitext(os.path.basename(h))[0] for h in glob.glob(os.path.join(src_dir, '**', '*.h'), recursive=True)}
cpps = {os.path.splitext(os.path.basename(c))[0] for c in glob.glob(os.path.join(src_dir, '**', '*.cpp'), recursive=True)}
unpaired = headers - cpps - {'SharedTypes', 'TranspersonalGame'}
assert len(unpaired) == 0, f'Unpaired headers: {unpaired}'
```

---

### 6. Compilation Gate

**Method:** Check for compile errors in latest log file  
**Log location:** `{project_dir}/Saved/Logs/*.log`  
**Error patterns:** `error C`, `error:`, `CompileError`  

**Pass criteria:** Zero error lines in latest log  
**Fail action:** Create GitHub Issue with error details + list of files to fix

---

## ANTI-CONTAMINATION CHECKLIST

Before approving any agent output, verify:

- [ ] No references to meditation, consciousness expansion, spiritual awakening
- [ ] No shamans, spirit guides, wisdom keepers, beast whisperers
- [ ] No supernatural powers (telepathy, telekinesis, mystical communication)
- [ ] No auras, chakras, energy fields, sacred sites
- [ ] All content passes "National Geographic prehistoric documentary" test

---

## INTEGRATION DEPENDENCY ORDER

```
Engine Architect (#02)
  └─ Core Systems (#03)
       └─ World Generator (#05)
            ├─ Environment Artist (#06)
            │    └─ Architecture (#07)
            │         └─ Lighting (#08)
            └─ Character Artist (#09)
                 └─ Animation (#10)
                      ├─ NPC Behavior (#11)
                      │    └─ Combat AI (#12)
                      │         └─ Crowd Sim (#13)
                      └─ Narrative (#15)
                           └─ Quest Designer (#14)
                                ├─ Audio (#16)
                                └─ VFX (#17)
                                     └─ QA (#18)
                                          └─ Integration (#19) ← YOU
```

---

## ROLLBACK POLICY

Agent #19 maintains the last 10 functional builds.  
Rollback trigger: Any of the 7 core classes fail to load.  
Rollback method: Revert to last known-good commit on `main` branch.

---

## CYCLE HISTORY

| Cycle | Classes | Map Score | Compile | Notes |
|-------|---------|-----------|---------|-------|
| AUTO_005 | 7/7 | 6/6 | PASS | Baseline established |
| AUTO_006 | 7/7 | 6/6 | PASS | No regressions |
| AUTO_007 | 7/7 | 6/6 | PASS | No regressions |
| AUTO_008 | 7/7 | 6/6 | PASS | No regressions |

---

*Integration & Build Agent #19 — Transpersonal Game Studio*
