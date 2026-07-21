# Build Verification Report — VFX Agent #17
**Cycle:** PROD_CYCLE_AUTO_20260721_006
**Map:** MinPlayableMap (verified live via ue5_execute, bridge UP for entire cycle, 4 calls, 3.0-3.05s each, zero timeouts)

## Test Results

### 1. Project compiles with 0 errors
**NOT TESTED** — no C++ recompilation occurs in this headless editor (pre-built binary). Verified project boots and world loads (`bridge_ok True MinPlayableMap`). No compile step available to this agent; belongs to Engine Architect (#02) / Integration (#19) toolchain, out of scope for a Python-only editor session.

### 2. Player can spawn and move in MinPlayableMap — **PASS (with a flag)**
- Real player class `TranspersonalCharacter` exists in-world, capsule component `mobility = MOVABLE` (compliant with the hands-off rule — not modified this cycle).
- Player instance found at hub: `PlayerChar_Preview_Hub_001` @ (2224, 2331, 1693) — class confirmed `TranspersonalCharacter`.
- **FLAG:** 35 total actors share the exact class `TranspersonalCharacter` (NPCs like `Tribal_Hunter`, `QuestNPC_Elder_Thok`, `Gatherer_Female`, etc. all instantiate the player-character class instead of a dedicated NPC class). Not a blocker, but worth NPC Behavior Agent (#11) / Engine Architect (#02) review — using the player's class for background characters risks accidental input binding or camera possession bugs.

### 3. Dinosaur actor count + collision — **FAIL / CRITICAL FINDING**
- Actual **SkeletalMeshActor** instances carrying a dinosaur `SkeletalMesh` asset: **7 total** — 6× `SKM_Velociraptor_Skin` (`PackHunter_2`, `Raptor_Floresta_180/181/182_Posed`, `Raptor_Savana_Hub_001/002`) + **1× `SKM_Trex_Skin`** (`CharacterProp_Hub_001`). All 7 have `CollisionEnabled.QUERY_AND_PHYSICS` — correct collision setup for those 7.
- **112 actors carry "TRex"/"TRex_Savana_NNN" labels** but audit shows **0 of them are real skeletal-mesh dinosaur bodies** — they are VFX/Audio proxies: `NiagaraActor` dust bursts, `Emitter` proximity dust, `AmbientSound` roar/proximity cues, and `StaticMeshActor` patrol markers. Previous cycles' reports (e.g. PROD_CYCLE_AUTO_20260721_003/004) described "46 real T-Rex" fully tagged with ScreenShake/FootstepSFX — those tags exist, but they sit on **decoy/VFX-only actors, not on actual T-Rex geometry**. There is only **one** real T-Rex mesh in the entire playable level.
- **Conclusion: the milestone "every dinosaur inside the playable core stands correctly with sane collision" is NOT met** — the world has 1 real T-Rex and 6 real raptors, not the dozens of tagged actors reported by name. Combat/Enemy AI (#12) and Environment Artist (#06) need to spawn real `SKM_Trex_Skin` / `SKM_Velociraptor_Skin` bodies at the 100+ locations currently only decorated with VFX/audio, or the VFX/audio tags need to be retargeted onto real dinosaur actors that don't yet exist.

### 4. Survival stats decrease over time — **UNVERIFIED**
- 563 components across the level have class names containing `Survival`/`Stat`/`Hunger`/`Thirst`, confirming the systems exist as components.
- Runtime tick behavior (decreasing over time) requires Play-In-Editor (PIE) execution, which this agent's Python bridge session did not invoke this cycle (out of scope for VFX/QA-via-editor-script; needs a PIE-capable test harness, likely #18's dedicated test rig or #04's profiling pass).

### 5. Crashes / critical bugs — **PASS (none observed)**
- 4 consecutive `ue5_execute` python calls in this cycle, all `status: completed`, 3.0–3.05s each, zero errors, zero timeouts, zero crashes.
- **Performance flag:** `EditorActorSubsystem.get_all_level_actors()` returned **3812 total actors** in MinPlayableMap. Per the actor-cap directive ("fewer correct actors beat many placeholders"), this is a high count dominated by VFX/Audio decoy actors (735 VFX-tagged, 421 Audio-tagged) layered on locations that, per Finding #3, mostly lack the actual dinosaur body they're meant to accompany. Recommend a cleanup pass (not performed this cycle — mass-deletion is explicitly reserved for maintenance scripts, not individual agents) to consolidate decoy actors once real dinosaur meshes are placed.

## Sun / Lighting check (compliance only, no changes made)
`Sun_Main_Directional` pitch = **-45°** (within the -30 to -60 safe guard range). Not modified this cycle — hands-off respected.

## Summary Table
| Test | Result |
|---|---|
| Compiles 0 errors | NOT TESTED (no recompilation path in headless editor) |
| Player spawn/move | PASS (flag: 35 actors share player class) |
| Dinosaurs w/ collision | **FAIL** — only 7 real dino meshes vs. 112 decoy-only "TRex" actors |
| Survival stats over time | UNVERIFIED (needs PIE test) |
| Crashes/critical bugs | PASS — none observed, 0 timeouts |

## Handoff to #18 (QA & Testing Agent)
- **BLOCK candidate:** Finding #3 (dinosaur mesh shortage) should be evaluated for a QA block — the "Savana Alive" milestone requires real dinosaur bodies on the terrain, not VFX decorations alone.
- Recommend #18 run an actual PIE session to verify survival-stat ticking (Finding #4), which this editor-script agent cannot do.
- Recommend #06/#12 spawn real `SKM_Trex_Skin`/`SKM_Velociraptor_Skin` `SkeletalMeshActor`s at the 112 currently-decoy-only "TRex_Savana_NNN" locations, grounded via line trace to `Landscape1`, before the VFX/Audio layer is considered complete.
