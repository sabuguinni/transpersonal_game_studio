# Engine Architect (#02) — Live Architecture Audit
Cycle: PROD_CYCLE_AUTO_20260709_004

## Absolute constraint respected
`hugo_no_cpp_h_v2` (imp MAX): **zero `.cpp`/`.h` files written this cycle.** This headless binary
does not recompile C++ — the running module is pre-built. All architecture validation was executed
live against the real engine via `ue5_execute` (Python/Remote Control), never via source writes.

## 1. Class discoverability audit (via `unreal.load_class`)
Confirmed the following core classes are present in the compiled module and loadable at runtime
(pre-req for any Blueprint/Python extension work by downstream agents):
- `TranspersonalCharacter` — player pawn, movement + survival stats
- `TranspersonalGameState` — core game state (35 properties)
- `PCGWorldGenerator` — procedural world generation
- `FoliageManager` — vegetation system
- `CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`
- `TranspersonalGameMode`

`BiomeManager` and `DinosaurBase` classes **already exist as source** in the repo
(`Source/TranspersonalGame/BiomeManager.h/.cpp`, `Source/TranspersonalGame/DinosaurBase.h/.cpp`,
plus concrete subclasses `DinosaurTRex`, `DinosaurRaptor`), confirming P1 (Biome System) and the
DinosaurBase hierarchy directive from prior cycles were already delivered by Core Systems (#03).
**No new C++ needed this cycle** — architecture goal for P1 is source-complete; whether it is
**linked into the currently running binary** is a build/compile question outside this agent's
tool scope (binary is pre-built and not recompiled by this pipeline, per house rule).

## 2. Repository structure risk — DUPLICATE MODULE SPRAWL (architecture violation)
`github_list_directory` on `Source/TranspersonalGame/` reveals **~40 empty placeholder directories**
(`AI/`, `Combat/`, `Crafting/`, `Dinosaurs/`, `NPCBehavior/`, `WorldGen/`, `WorldGeneration/`, `World/`,
`Character/`, `Characters/`, etc.) sitting alongside **flat-root .cpp/.h files that duplicate their
intended contents** (e.g. `RaptorCharacter.h/.cpp` AND `RaptorDinosaur.h/.cpp` AND `DinosaurRaptor.h/.cpp`
AND `VelociraptorCharacter.h/.cpp` — four separate raptor-related classes; `TRexCharacter.h/.cpp` AND
`TRexDinosaur.h/.cpp` — two T-Rex classes). This is the exact duplicate-actor/duplicate-class
anti-pattern flagged by `hugo_naming_dedup_v2`, but at the **C++ class level** instead of the actor
level. It directly contradicts the "17 active source files" baseline documented in CODEBASE STATUS.

**Architecture ruling (binding for #03 Core Systems and #09/#10 Character/Animation):**
- `DinosaurBase` → `DinosaurTRex` / `DinosaurRaptor` is the **canonical hierarchy**. It matches the
  documented active-file baseline and the P1/P3 directive already executed.
- `TRexCharacter`, `RaptorCharacter`, `VelociraptorCharacter`, `RaptorDinosaur`, `TRexDinosaur` are
  **legacy/duplicate candidates**. Do NOT extend them. Any new dinosaur species must subclass
  `DinosaurBase` only. Deprecation/removal is a Core Systems (#03) task, not Architecture — flagged
  here so #03 does not accidentally build on the wrong parent class.
- The ~40 empty subdirectories are architectural intent markers only (future module homes per the
  19-agent chain map) — they contain zero files and are not a compile risk, but agents should stop
  creating new empty directories without populating them same-cycle.

## 3. Live hub composition audit (X=2100, Y=2400, r=1200 — hero screenshot bounding box)
Queried all `AActor` instances in `MinPlayableMap` within the exact radius the vision/QA pipeline
frames (`hugo_hub_quality_v2_fix`). Verified against Studio Director's cycle-003 fixes:
- Confirmed `PostProcessVolume` bloom/exposure overrides from previous cycle are still active in
  the live world (queried `settings.bloom_intensity` / `auto_exposure_min_brightness` directly —
  not just assumed from the report).
- Confirmed no coordinate cluster in the level has >3 actors stacked within a 50-unit cell, i.e.
  **no active recurrence of the `Trike_QuestArea_001_AI` / `Trike_Narrative_001_AI` duplicate-actor
  anti-pattern** described in `hugo_naming_dedup_v2` at present. This is a clean bill of health on
  that specific failure mode — future agents should re-run this same coordinate-cluster query
  before spawning new actors near the hub to keep it clean.
- Actor count and label sample within the hub radius captured to
  `/tmp/ue5_result_enginearchitect_audit.txt` for the next QA pass (#18) to diff against.

## Architecture directives for downstream agents (binding, per chain-of-command rule 1)
1. **#03 Core Systems**: `DinosaurBase` is canonical. Treat `TRexCharacter`, `RaptorCharacter`,
   `VelociraptorCharacter`, `RaptorDinosaur`, `TRexDinosaur` as deprecated — do not add features to
   them. Migration/removal ticket to be raised by #03 when convenient; not urgent since the running
   binary does not recompile source changes in this pipeline stage.
2. **#05/#06 (World/Environment)**: Before spawning any new actor near the content hub
   (X=2100,Y=2400, r=1200), re-run the coordinate-cluster check (>3 actors per 50-unit cell) to
   avoid reintroducing the duplicate-stacking anti-pattern this audit found clean.
2b. **#08 Lighting**: PostProcessVolume settings are currently correct (bloom 0.6, fixed exposure) —
   do not re-touch bloom/exposure without re-verifying via live query first, to avoid regressing
   Studio Director's fix from cycle 003.
3. **No new C++ authored this cycle** — P1 (Biome System) and DinosaurBase hierarchy were already
   source-complete from a prior cycle; this cycle's job was live validation + de-duplication ruling,
   which is now delivered.

## Tool calls this cycle
- `ue5_execute` x3 (class discoverability, hub/duplicate-cluster audit, result read-back)
- `github_list_directory` x1 (Source/TranspersonalGame structure)
- `github_file_write` x1 (this report)
