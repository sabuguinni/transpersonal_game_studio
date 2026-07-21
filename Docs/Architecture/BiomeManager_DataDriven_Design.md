# BiomeManager — Data-Driven Architecture (P1 World Generation)

**Author:** Engine Architect #02
**Cycle:** PROD_CYCLE_AUTO_20260711_011
**Status:** VALIDATED LIVE via ue5_execute — zero .cpp/.h written (per `hugo_no_cpp_h_v2`)

## Why data-driven instead of a new `UBiomeManager` C++ class

The headless UE5 instance running this project is a **pre-built binary that never recompiles**.
Any new `.h`/`.cpp` (e.g. `BiomeManager.h/.cpp`) would be:
- invisible to Remote Control (no UHT pass, no reflection data),
- invisible to `unreal.load_class(...)`,
- dead code sitting in GitHub with zero runtime effect,
- a guaranteed FAIL on the "Class Existence" functional validation check.

Per `hugo_no_cpp_h_v2` (imp:MAX, absolute rule, no exceptions), all engine-facing changes for this
cycle were done live via `ue5_execute` Python against the running Editor, and the architecture
was designed to be **enforceable without new compiled types**.

## Architecture: Actor Tag-Based Biome System

Every content actor (terrain features, foliage, dinosaurs, props) carries a `Name` tag of the
form `Biome_<BiomeID>`, e.g.:

- `Biome_Forest_Cretaceous` — dense vegetation zone, includes the content hub clearing at
  world coords X=2100, Y=2400 (the hero-screenshot composition zone).
- `Biome_Plains_Cretaceous` — open terrain, sparser vegetation, larger dinosaur roaming range.

This is queryable by **any** downstream system without requiring a shared C++ class:

```python
# Any agent/system can query biome membership like this:
actors_in_forest = [a for a in unreal.EditorLevelLibrary.get_all_level_actors()
                     if unreal.Name("Biome_Forest_Cretaceous") in a.tags]
```

### Zone Rule (current, v1)
```
if 1500 <= X <= 2700 and 1800 <= Y <= 3000:
    Biome_Forest_Cretaceous
else:
    Biome_Plains_Cretaceous
```
This rule intentionally covers the content-hub clearing (X=2100, Y=2400) so all trees, rocks and
dinosaurs already placed there resolve to `Biome_Forest_Cretaceous` — matching the mandated
"living Cretaceous forest" composition (`hugo_hub_quality_v2_fix`).

## Live Validation Performed This Cycle

1. **Class existence check** — confirmed via `unreal.load_class` that all 7 active gameplay
   classes (`TranspersonalCharacter`, `TranspersonalGameState`, `PCGWorldGenerator`,
   `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`,
   `BuildIntegrationManager`) load correctly from `/Script/TranspersonalGame.*`. This is the
   baseline the P1 biome system must not regress.
2. **Tag coverage audit** — scanned all level actors for existing `Biome_*` tags; found gaps on
   trees/rocks/dinosaurs placed by earlier cycles without biome classification.
3. **Tag backfill + save** — applied `Biome_Forest_Cretaceous` / `Biome_Plains_Cretaceous` tags
   to every previously untagged `Tree_`, `Rock_`, `TRex_`, `Raptor_`, `Brachiosaurus_`, `Trike_`
   labeled actor based on world position, then saved the level.

## Rules for Downstream Agents (Law of the Studio for P1)

- **#5 (Procedural World Generator):** any new terrain/PCG spawn MUST assign a `Biome_<ID>` tag
  matching its world-space zone. Do not spawn untagged content actors.
- **#6 (Environment Artist):** foliage density/species selection should read the `Biome_*` tag
  of the target zone before placing props — forest zones get denser packing than plains.
- **#8 (Lighting):** may key color temperature/fog per biome tag (e.g. warmer amber in
  `Biome_Forest_Cretaceous`) without needing a new subsystem.
- **#9/#10 (Character/Animation):** dinosaur pose/species selection can filter by biome tag to
  keep species plausible per zone (documentary-realism constraint).
- **Naming/dedup (`hugo_naming_dedup_v2`):** biome tags are additive metadata on **existing**
  actors — never a reason to spawn a duplicate. Look up by label first, tag second.

## Files Modified
- None (.cpp/.h) — per absolute rule.
- Live editor state: actor tags applied + level saved via `ue5_execute`.
- This design doc: `Docs/Architecture/BiomeManager_DataDriven_Design.md`.
