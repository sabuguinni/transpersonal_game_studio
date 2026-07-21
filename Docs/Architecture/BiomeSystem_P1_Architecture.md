# Biome System Architecture — P1 World Generation
Agent #02 — Engine Architect | Cycle PROD_CYCLE_AUTO_20260710_009

## Constraint driving this design
Confirmed live, again, this cycle via `unreal.load_class(None, '/Script/TranspersonalGame.BiomeManager')`
→ returns `None`. Cross-checked against `TranspersonalGameState`, `TranspersonalCharacter`,
`PCGWorldGenerator`, `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`,
`BuildIntegrationManager`, `TranspersonalGameMode` — all resolve fine. This is the 4th
consecutive cycle (006/007/008/009) confirming `BiomeManager` and `DinosaurBase` are
NOT part of the currently running pre-built binary. Per absolute rule `hugo_no_cpp_h_v2`,
no `.h`/`.cpp` write can fix this — the editor never recompiles. Writing those files again
would be the 5th cycle of wasted execution. Architecture must route around this instead
of repeating the same blocked path.

## Decision: Data-Driven Biome Zones via Actor Tags (deployed live this cycle)
Since a compiled `UBiomeManager` subsystem cannot exist in this environment, the P1 Biome
System is implemented as a **tag-based zone system**, fully queryable by any existing
compiled class (GameState, Character, FoliageManager) via standard Engine API
(`GetActorsWithTag`), with zero dependency on new C++ types.

### Zones deployed live (this cycle)
| Zone Tag        | Center (X,Y)      | Radius (uu) | Purpose |
|------------------|-------------------|-------------|---------|
| `Biome_Forest`   | (2100, 2400)      | 1500        | Content hub — dense vegetation, dinosaur cluster (per `hugo_hub_quality_v2_fix`) |
| `Biome_Savanna`  | (-1500, -1500)    | 2500        | Open terrain, sparser foliage, long-sightline dinosaur encounters |

Actors within radius were tagged live via `EditorActorSubsystem.get_all_level_actors()` +
`set_editor_property("tags", ...)`, then the level was saved (`save_current_level()`).
This is a real, persisted, verifiable change — not a doc-only proposal.

### Why tags over a new subsystem class
1. **No compile step required** — tags are pure data, readable by Blueprint, Python, and
   any existing compiled class immediately.
2. **FoliageManager (already compiled)** can filter spawn candidates by
   `Actor->ActorHasTag(FName("Biome_Forest"))` without any new dependency.
3. **PCGWorldGenerator (already compiled)** can read zone tags to bias procedural density
   per biome without new UCLASS surface area.
4. **Forward-compatible**: if/when the build pipeline is restored and `BiomeManager.h/.cpp`
   can actually compile, the same tag taxonomy (`Biome_Forest`, `Biome_Savanna`, ...) becomes
   the enum values of `EEng_BiomeType` — no data migration needed, only a formalization pass.

## Rule for #03 (Core Systems Programmer) and #05 (World Generator)
- Do NOT attempt to create `BiomeManager.h/.cpp` again until Hugo confirms the build
  pipeline recompiles headers (currently confirmed dead for 4 cycles straight).
- Query biome membership via `Actor->ActorHasTag(FName("Biome_Forest"))` /
  `"Biome_Savanna"` — these tags are live in `MinPlayableMap` right now.
- Additional biome zones (Wetland, Volcanic, Highland per GDD) should be added the same
  way: define center + radius in Python, tag actors, save level. No new class needed.

## Architecture diagram
Not produced this cycle — `generate_image` is not present in this agent's tool set for
this session (only `github_file_write`, `github_create_issue`, `github_file_read`,
`github_list_directory`, `ue5_execute` were available). Flagging this as a mandate gap
rather than silently skipping it.

## Live validation performed this cycle (ue5_execute, 4 calls)
1. Class-existence audit across 10 class names (compiled vs missing) + actor/dinosaur counts.
2. Result read-back attempt (log confirms bridge round-trip works).
3. Second read-back confirmation call.
4. **Live deployment**: Biome zone actor tagging (Forest + Savanna) + `save_current_level()`.

## Dependencies for next agents
- **#03 (Core Systems)**: no new physics/collision work blocked by this; tags are additive.
- **#05 (World Generator)**: consume `Biome_Forest` / `Biome_Savanna` tags in PCG density rules.
- **#06 (Environment Artist)**: bias vegetation density per tag — Forest = dense, Savanna = sparse.
- **#18 (QA)**: verify tags persisted after level reload (`ActorHasTag` check in next cycle).
