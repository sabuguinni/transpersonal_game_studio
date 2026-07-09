# Biome System Architecture — Engine Architect #02
## Cycle: PROD_CYCLE_AUTO_20260709_008

## CRITICAL CONSTRAINT ACKNOWLEDGED
Per brain memory `hugo_no_cpp_h_v2` (importance MAX, zero exceptions): **this headless UE5 editor
runs a pre-built binary that never recompiles C++.** Writing a `BiomeManager.h`/`.cpp` pair via
`github_file_write` would be 100% inert — the code would sit in the repo but never execute,
never register as a UCLASS, and never be discoverable via `unreal.load_class()`.

Live audit this cycle confirmed via `ue5_execute`:
- `/Script/TranspersonalGame.BiomeManager` → **MISSING** (does not exist in the running binary)
- Active classes confirmed loadable: `TranspersonalGameState`, `TranspersonalCharacter`,
  `PCGWorldGenerator`, `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`,
  `BuildIntegrationManager` — all FOUND.

## ARCHITECTURE DECISION: Data-Driven Biome Zoning (no new C++ type)
Since a true `UBiomeManager` UObject/AActor subsystem cannot be compiled into this binary,
the enforced architecture for biome regions in this editor session is:

1. **Zone definition = spatial radius + Actor Tag**, applied live via `ue5_execute` Python,
   not via a new class. Tag convention: `Biome_<BiomeName>` (e.g. `Biome_CretaceousForest`).
2. **Hub zone established this cycle**: center (X=2100, Y=2400), radius 1500 units — matches
   the mandated hero-screenshot content hub (`hugo_hub_quality_v2_fix`). All actors within
   radius were tagged `Biome_CretaceousForest` live in the editor (tagging pass executed via
   `ue5_execute`, level saved).
3. **Future biome regions** (savanna, swamp, volcanic) follow the same pattern: define
   center + radius, tag actors in range, no new UCLASS required until a real build pipeline
   (with UBT recompilation) exists.
4. **When C++ compilation becomes available again** (future infrastructure fix), this
   architecture upgrades cleanly to a real `UBiomeManager` subsystem:
   - `UBiomeManager : public UWorldSubsystem`
   - `TMap<FName, FBiomeZoneData>` keyed by tag name, `FBiomeZoneData` in `SharedTypes.h`
     (fields: `FVector Center`, `float Radius`, `TArray<TSoftClassPtr<AActor>> SpeciesWhitelist`,
     `FLinearColor FogTint`, `float TemperatureRange`)
   - Existing tags become the migration seed — no data loss, actors already carry zone identity.

## RULES ESTABLISHED FOR ALL DOWNSTREAM AGENTS (#3–#19)
1. **DO NOT** write `BiomeManager.h`/`.cpp` — it will not compile/execute in this session.
2. **DO** use `ue5_execute` Python to tag/query actors by `Biome_<Name>` tag for any
   biome-conditional logic (e.g., #6 Environment Artist should only spawn ferns/cycads
   inside `Biome_CretaceousForest` tagged radius; #12 Combat AI should scale dinosaur
   aggression by biome tag, not by a new C++ enum).
2. **One hub, one biome, for now** — per Studio Director priority, do not fragment
   density across multiple biomes until the Cretaceous Forest hub hero-shot composition
   (recognizable dinosaurs in pose + dense vegetation, daylight) is confirmed as PASS by
   the reflection/vision loop.
3. `SharedTypes.h` remains the single source of truth for any FUTURE struct/enum once
   compilation is restored — no agent should pre-emptively define `FBiomeZoneData` in a
   header now, since it cannot be verified to compile.

## Live Validation Performed (ue5_execute, this cycle)
1. Class discoverability audit — 7/7 active classes FOUND, `BiomeManager` confirmed MISSING.
2. Level actor composition audit — actor label prefix histogram + terrain actor count,
   used to size the hub radius and confirm terrain presence before tagging.
3. Biome zone tagging pass — applied `Biome_CretaceousForest` tag to all actors within
   1500 units of (2100, 2400); level saved.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Class discoverability audit (7 active classes confirmed FOUND, BiomeManager confirmed MISSING — informs no-new-class decision)
- [UE5_CMD] Level actor composition audit (label histogram, terrain actor count) to size the hub biome zone
- [UE5_CMD] Applied `Biome_CretaceousForest` actor tag to all actors within 1500 units of (2100,2400); saved level
- [FILE] Docs/Architecture/BiomeManager_Architecture.md — documents data-driven biome zoning pattern, migration path to real UBiomeManager subsystem once compilation is restored, and rules for downstream agents
- [NEXT] #3 Core Systems: when/if a build pipeline is restored, implement `UBiomeManager : public UWorldSubsystem` using this doc's `FBiomeZoneData` spec in `SharedTypes.h`. Until then: #6/#9 use `Biome_CretaceousForest` tag to gate vegetation/dinosaur spawn density at the hub; #8 confirm PostProcessVolume bloom fix (BloomIntensity=0.5) persists within this same zone for the hero screenshot.
