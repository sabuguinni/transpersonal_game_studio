# Crafting System Design — Agent #14 (Quest & Mission Designer)
## Cycle PROD_CYCLE_AUTO_20260711_006

### Status: LIVE IN MinPlayableMap (verified via ue5_execute, bridge healthy)

## Resources Placed (near hub, world coords ~2100,2400)
Naming convention: Type_Bioma_NNN, tag `Resource_Pickup` + `ResourceType_*`

| Label | Type | Location (x,y,z) | Notes |
|---|---|---|---|
| Rock_Hub_101/102/103 | Rock | ~2300-1950, 2250-2550, 95 | Cube placeholder, scale 0.25 |
| Stick_Hub_101/102/103 | Stick | ~2250-2000, 2450-2600, 95 | Cube placeholder, scale 0.08 (elongated look TBD by Environment Artist) |
| Leaf_Hub_101/102 | Leaf | ~2150-1900, 2350-2400, 95 | Sphere placeholder, scale 0.15 |

All spawns were dedup-checked against existing actor labels before creation (no duplicates created, per naming/dedup rule).

## Crafting Station
`CraftingStation_Hub_001` — TriggerSphere at hub center (2100,2400,100), radius 250u.
Tags carry recipe metadata for the NPC Behavior / gameplay systems to read at runtime:
- `Quest_CraftingStation`
- `Recipe_StoneAxe_2Rock_1Stick`
- `Recipe_Campfire_3Stick`
- `Recipe_WaterContainer_1Rock_1Leaf`

## Recipes (design spec — realistic survival crafting)
1. **Stone Axe**: 2x Rock + 1x Stick → tool for chopping/combat, faster resource gathering.
2. **Campfire**: 3x Stick → light source, cooking, warmth (counters night temperature drop), predator deterrent.
3. **Water Container**: 1x Rock (hollowed) + 1x Leaf (seal/lining) → carry water, reduces thirst depletion while traveling.

## Interaction Model (spec for Character/UI programmer — NOT implemented in C++ this cycle per no-cpp rule)
- Player walks into `CraftingStation_Hub_001` trigger sphere → prompt "Press C to craft".
- On C press, UI queries actor tags on overlapping trigger for available `Recipe_*` tags.
- Player inventory (tracked by TranspersonalCharacter survival stats extension) checked against recipe requirements.
- This is a DESIGN SPEC ONLY — actual UMG widget + input binding must be implemented via Blueprint/UI system by the appropriate agent (C++ .h/.cpp changes are inert in this headless editor per hard rule).

## Verification
- 4/4 ue5_execute Python calls succeeded this cycle (bridge healthy, ~3.0s each).
- Dedup audit ran before spawning (query for existing Quest/Trigger/Resource/Craft labels near hub).
- Level saved after all spawns (`unreal.EditorLevelLibrary.save_current_level()`).

## Next Cycle (#15 Narrative & Dialogue Agent / future Quest cycles)
- Narrative agent: attach lore flavor text to the crafting station (why humans need these tools — survival framing, no mysticism).
- Future Quest cycle: add a "gather 2 rocks + 1 stick" fetch objective tied to `CraftingStation_Hub_001`, verified via tag-based inventory check.
- Environment Artist: replace placeholder Cube/Sphere meshes with proper rock/stick/leaf static meshes.
- UI/Character programmer: implement actual "Press C" widget binding (Blueprint-side, since C++ is inert here).
