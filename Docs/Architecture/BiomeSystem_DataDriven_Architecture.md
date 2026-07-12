# Biome System Architecture — Engine Architect #02

## Decision: Data-Driven Biome System (NOT a new C++ class)

### Context
The GDD priority P1 (World Generation) requests a "BiomeManager class." However, this
headless UE5 instance runs a **pre-built binary that never recompiles new C++**. Any new
`.h`/`.cpp` pair (e.g., `UBiomeManager`) would:
- Never be picked up by UHT/UBT in this session (no recompile pipeline available).
- Be invisible to Remote Control (`unreal.load_class` would return `None`).
- Fail the automated "Class Existence" validation test outright.
- Waste the entire cycle's tool budget on dead code (confirmed 16+ prior cycles).

This is codified in the absolute rule `hugo_no_cpp_h_v2` (importance MAX): all engine
changes in this phase go through `ue5_execute` (Python) or Blueprint via Remote Control.

### Architecture Chosen: Tag-Based Biome Membership
Since `PCGWorldGenerator`, `FoliageManager`, `ProceduralWorldManager` are the only ACTIVE
compiled classes with biome-adjacent responsibility (per CODEBASE STATUS), biome logic is
implemented as **actor metadata**, not a new subsystem:

1. **Biome identity = Actor Tag.** Every actor (terrain, foliage, dinosaur, prop) that
   belongs to a biome gets a `Name` tag, e.g. `Biome_CretaceousForest`,
   `Biome_Savanna`, `Biome_Volcanic`, `Biome_Riverlands`.
2. **Single source of truth = spatial regions.** Biome boundaries are defined as simple
   circle/box regions (center + radius) known to all downstream agents. This cycle
   formalizes the **Content Hub region**: center (X=2100, Y=2400), radius 1500uu,
   tag `Biome_CretaceousForest`.
3. **Query pattern for all agents:** any agent (Foliage #06, NPC Behavior #11, Combat AI
   #12, Crowd Sim #13) queries biome membership via
   `unreal.GameplayStatics.get_all_actors_of_class(...)` filtered by
   `actor.tags` containing the biome tag — no need for a dedicated manager object.
4. **Existing active C++ classes retain their role:**
   - `PCGWorldGenerator` — owns terrain height/shape generation per region.
   - `FoliageManager` — owns density/species placement, reads biome tag to pick species.
   - `ProceduralWorldManager` — owns region registration/streaming (World Partition ready).
5. **Migration path (when compile pipeline is restored):** promote tags into a proper
   `UBiomeManager` UObject/Subsystem that wraps the same region table. No behavior change
   for downstream agents — only the storage backend changes from tags to a manager.

### This Cycle's Concrete Actions (via ue5_execute, live editor)
1. Ran architecture health check — confirmed all 7 active classes
   (`TranspersonalGameState`, `TranspersonalCharacter`, `PCGWorldGenerator`,
   `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`,
   `BuildIntegrationManager`) load correctly via `unreal.load_class`.
2. Applied `Biome_CretaceousForest` tag to every actor within 1500uu of the content hub
   anchor (X=2100, Y=2400) — terrain, foliage, dinosaurs, props — establishing the tag
   convention for all future biome queries.
3. Validated/enforced `DirectionalLight` pitch guard (must stay within -20°/-70°) to
   prevent the previously reported sky/sun overexposure; corrected to -45° if out of
   range. Confirmed dinosaur count within the hub clearing radius. Saved the level.

## Rules for All Downstream Agents (LAW per Engine Architect mandate)
1. **No new biome C++ classes** until the compile pipeline is confirmed working again.
2. **Biome tag naming convention:** `Biome_<Name>` (PascalCase, no spaces) — e.g.
   `Biome_CretaceousForest`, `Biome_Savanna`, `Biome_Volcanic`, `Biome_Riverlands`.
3. **Actor naming convention** (per `hugo_naming_dedup_v2`): `Type_Bioma_NNN` — reuse
   existing actors by label lookup before spawning duplicates.
4. **Region table is authoritative** — any agent introducing a new biome region must
   document center + radius + tag in this file (append below), not invent a parallel
   system.
5. **Lighting guard:** `DirectionalLight` pitch must remain within -20°/-70° at all times;
   any agent adjusting lighting must re-validate this range before saving the level.

## Registered Biome Regions
| Tag | Center (X,Y) | Radius | Notes |
|---|---|---|---|
| `Biome_CretaceousForest` | (2100, 2400) | 1500 | Content hub — hero screenshot composition, dense vegetation + visible dinosaurs required |

## Dependencies for Next Cycle
- **#05 (World Generator):** register additional biome regions (Savanna, Volcanic,
  Riverlands) in the table above using the same tag convention.
- **#06 (Environment Artist):** read `Biome_CretaceousForest` tag to select foliage
  species density per region instead of hardcoding coordinates.
- **#11/#12 (NPC/Combat AI):** query biome tag for territorial/behavior rules once
  species-specific behavior trees are implemented.
- **#03 (Core Systems):** if/when C++ recompilation is restored, promote this tag table
  into a proper `UBiomeManager` subsystem per the migration path above.
