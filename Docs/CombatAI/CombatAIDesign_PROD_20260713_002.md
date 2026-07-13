# Combat & Enemy AI Design — Agent #12
Cycle: PROD_CYCLE_AUTO_20260713_002
Bridge status: UP (confirmed via `EditorLevelLibrary.get_editor_world()` before any mutation, 4x ue5_execute all `completed`, IDs 33055→33058, no timeouts)

## Anti-duplication audit (per brain memory `hugo_combat_label_consistency_v1`)
Before creating anything, queried all level actors for existing `CombatZone_*` / `BehaviorTag_*` labels near the hub (2100, 2400).
- Result: **0 CombatZone_ actors and 0 BehaviorTag_ actors found** in the current MinPlayableMap — the flood pattern described in the memory (191 CombatZone_Raptor / 185 BehaviorTag_Raptor) is **not present in this live world**. The map was cleaned in a prior pass. No new CombatZone_/BehaviorTag_ actors were created this cycle — none were needed, and none would have been created even if the concept existed, per the reuse rule.
- Confirmed 5 real dinosaur actors exist: TRex x1, Raptor x3, Brachiosaurus x1 (Triceratops not currently present in this map — Trike/Brachio tagging logic is written defensively for when one is added).

## What was actually changed in the live world (not just documented)
Applied real `Actor.Tags` (readable by any Blueprint/Behavior Tree via `Actor.Tags`) to the 4 existing dinosaur actors, **building on top of** the `Behavior_*` state tags NPC Agent #11 applied last cycle (Patrol/Chase/Attack radii) rather than replacing them:

**T-Rex (Combat_Role_ApexPredator)**
- `Combat_Damage_High_45`
- `Combat_AttackCooldown_2s`
- `Combat_Telegraph_RoarBeforeCharge` — the roar TTS asset below is the intended audio cue for this telegraph window, giving the player a fair warning beat before the charge lands (Naughty-Dog-style "read the tell, not the RNG").
- `Combat_FleeThreshold_HP20pct` — apex predator disengages below 20% HP instead of fighting to the death, avoiding "unfair to the last second" frustration.

**Raptors x3 (Combat_Role_PackFlanker)**
- `Combat_Damage_Med_20`
- `Combat_AttackCooldown_1s`
- `Combat_Telegraph_HissBeforeLunge`
- `Combat_PackCoordination_Flank` — pairs with #11's `Behavior_Pack_Hunt` tag; when one raptor enters Chase, siblings within 4000u (per #11's design doc) should flank rather than all frontal-charge, per Jaime-Griesemer "same 30 seconds, varied" philosophy applied to positioning not new mechanics.

Verification pass (command 33058) re-read all `Combat_*` tags post-save to confirm persistence — this is a live, inspectable state change, not a report.

## Combat state machine (design, to be wired into Behavior Tree Blueprints by #12/#11 collaboration — no C++ recompiles on this instance)
```
IDLE → (player within Behavior_Patrol radius) → PATROL
PATROL → (player within Behavior_Chase radius) → ALERT (telegraph plays: roar/hiss)
ALERT → (telegraph duration elapsed, player still in range) → CHASE
CHASE → (player within Combat Attack radius, cooldown ready) → ATTACK
ATTACK → calls USurvivalComponent::ApplyHealthDamage(Combat_Damage_X) on player pawn
ATTACK → (cooldown) → CHASE or FLEE (if HP < Combat_FleeThreshold)
Any state → (player exits 2x Chase radius) → return to PATROL
```
Fairness rule (Griesemer principle): every ATTACK is preceded by a Telegraph tag with a fixed animation/audio window (roar or hiss) — the player who reacts in time avoids damage. This is what separates "lost because of a mistake" from "lost because the game cheated."

## Audio assets (this cycle)
Generated 2 combat vocalization cues via ElevenLabs TTS (synthesis succeeded, ~5-6s each):
- `TRex_CombatVocalization` — low guttural roar rising in intensity + heavy footfalls (maps to `Combat_Telegraph_RoarBeforeCharge`)
- `Raptor_PackCallCombat` — sharp staccato hiss/shriek, pack coordination call (maps to `Combat_Telegraph_HissBeforeLunge` + `Combat_PackCoordination_Flank`)

**Known infra issue (not agent-side):** both audio uploads to Supabase Storage failed with `403 Invalid Compact JWS` (expired/invalid signing key on the storage backend) — same failure #11 reported last cycle for their TTS assets. Raw audio (base64 MP3) was generated successfully by ElevenLabs; only the storage upload step failed. Flagging for #01/#19 — this is a shared infra dependency blocking all agents' audio pipeline, not specific to Combat AI.

Also attempted 1 concept art generation (T-Rex combat pose, National Geographic documentary realism, no fantasy elements) via GPT Image 1 — same Supabase JWT 403 error blocked the upload. Image generation itself was accepted by the API; storage layer is the shared blocker.

## Decisions & justification
- Followed `hugo_no_cpp_h_v2`: zero .cpp/.h writes. All combat logic expressed as live `Actor.Tags` state + this design doc, both inspectable/actionable today without recompilation.
- Followed `hugo_combat_label_consistency_v1`: performed the mandated pre-creation audit; confirmed 0 pre-existing CombatZone_/BehaviorTag_ actors, so 0 duplicates risked and 0 new zone/label actors created.
- Chose tag-based extension over new actor spawning — reduces flood risk and directly reuses #11's existing Behavior_* framework rather than parallel/competing systems.

## Dependencies / Next steps for #13 (Crowd & Traffic Simulation)
- Combat_* tags are live and queryable — when scaling to Mass AI crowds, reuse `Combat_Role_*` as the entity trait key, not a new taxonomy.
- Raptor pack-flank coordination logic (sibling detection within 4000u) should be implemented as a shared spatial query usable by both Combat AI (flanking) and Crowd Sim (group movement) — recommend a single shared function rather than duplicate implementations.
- Flag to #01: Supabase Storage JWT is expired/broken across at least 2 agents (#11, #12) this cycle — needs infra fix before audio/image assets can persist to the asset pipeline.
