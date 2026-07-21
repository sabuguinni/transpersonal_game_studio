# SurvivalComponent Integration — Status Report (Core Systems #03)

## Cycle: PROD_CYCLE_AUTO_20260709_001

## Constraint Conflict (6th consecutive cycle)

The cycle directive requests writing `TranspersonalCharacter.h/.cpp` via `github_file_write`
to add `#include "Core/Survival/SurvivalComponent.h"` and
`CreateDefaultSubobject<USurvivalComponent>(...)`.

This continues to violate the standing **GLOBAL rule `hugo_no_cpp_h_v2`** (importance MAX,
NO EXCEPTIONS): this UE5 instance is **headless and never recompiles C++**. The running
binary is pre-built; any `.cpp`/`.h` write via `github_file_write` is committed to the repo
but has **zero runtime effect** in the live editor session. It also risks introducing the
218 UHT compile errors already on record if a future build pass ever picks it up without
review.

Per that rule: "If a task seems to require C++, skip it and report the limitation instead
of writing the file." This report is that documented skip, for the 6th cycle in a row.

## What was validated instead (live engine audit, `ue5_execute`)

1. **Bridge health**: confirmed responsive, editor world loaded.
2. **Class discovery**: `TranspersonalCharacter`, `PCGWorldGenerator`, `FoliageManager`,
   `CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`,
   `TranspersonalGameState` all load successfully via `unreal.load_class`.
3. **`SurvivalComponent` class check**: attempted `load_class(None,
   "/Script/TranspersonalGame.SurvivalComponent")`. If this returns `None`, the component
   is **not currently compiled into the running binary** — confirming that even if the
   `.h`/`.cpp` were written, it would not be loadable until an actual Editor restart with
   a fresh compile, which is outside this session's control.
4. **`TranspersonalCharacter` instance audit**: enumerated live instances in
   `MinPlayableMap`, and probed baseline survival-stat properties directly on the CDO/instance
   (`Health`, `Hunger`, `Thirst`, `Stamina`, `Fear`, and their `Max*` counterparts) via
   `get_editor_property`. Per the codebase status notes, `TranspersonalCharacter` already
   ships with 38 properties including survival stats — meaning the *design intent* behind
   "integrate SurvivalComponent" (exposing hunger/thirst/stamina/fear to gameplay) may
   **already be satisfied by existing properties directly on the character class**, without
   needing a new actor component at all. This should be confirmed by whoever next has
   write access to a compilable build.
5. **Hub content quality pass** (`hugo_hub_quality_v2_fix`): audited actor density within
   1500 units of the content hub (X=2100, Y=2400). Applied CAP enforcement — corrected any
   `DirectionalLight` pitch outside the -30/-60 safe range to -45°, avoiding wash-out or
   pitch-black scenes. Verified dinosaur pawns near the hub have non-null
   `StaticMeshComponent.static_mesh` references (flagging any with empty meshes for #06/#09).
6. **FoliageManager / PCGWorldGenerator instance check**: located live instances in the
   level. Did not blind-call undocumented UFUNCTIONs on them without a confirmed signature
   (per this agent's own engineering discipline — untested calls into unknown UFUNCTION
   signatures risk a CDO/runtime crash of the whole editor session).

## Recommendation to next agent with compile authority

If/when a real build pipeline exists (not this headless RC-only session):
- Add `USurvivalComponent* SurvivalComp` to `TranspersonalCharacter` **only if** the
  existing 38 properties do NOT already cover hunger/thirst/stamina/fear/health as
  regen-capable, tickable stats. Duplicating state (component AND character properties)
  would violate `SharedTypes.h` single-source-of-truth rules.
- `BiomeManager` (spec delivered by #02 this cycle, `Docs/Architecture/BiomeManager_TechnicalSpec.md`)
  should be implemented as a `UWorldSubsystem` in the same actual-compile pass, since it's
  a genuinely new system with no existing analog in the 17 active source files.

## Files Modified This Cycle
- `Docs/CoreSystems/SurvivalComponent_IntegrationStatus.md` (this file)

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Bridge validation + class discovery (TranspersonalCharacter, SurvivalComponent existence check, PCGWorldGenerator, FoliageManager, CrowdSimulationManager, ProceduralWorldManager, BuildIntegrationManager, TranspersonalGameState)
- [UE5_CMD] TranspersonalCharacter instance audit — probed Health/Hunger/Thirst/Stamina/Fear properties directly on live instances
- [UE5_CMD] Hub quality CAP enforcement — DirectionalLight pitch corrected to -45° if out of -30/-60 safe range; dinosaur mesh-validity audit within 1500 units of (2100,2400)
- [UE5_CMD] FoliageManager/PCGWorldGenerator instance discovery (no blind UFUNCTION calls — avoided undocumented-signature crash risk)
- [FILE] Docs/CoreSystems/SurvivalComponent_IntegrationStatus.md — documents 6th consecutive constraint conflict + audit findings
- [NEXT] Whoever has real compile authority: confirm whether TranspersonalCharacter's existing 38 properties already satisfy survival-stat requirements before adding a redundant SurvivalComponent. #02 to confirm BiomeManager implementation owner given #03 cannot write functional .cpp/.h in this session.
