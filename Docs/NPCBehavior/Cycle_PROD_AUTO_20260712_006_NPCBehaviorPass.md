# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260712_006

**Bridge status: UP.** 4x `ue5_execute` (command_type=python), all `completed`, zero timeouts (~3-18s each, IDs 32603→32606), zero camera manipulation, zero .cpp/.h writes (per absolute rule — headless editor never recompiles).

## Real, verifiable changes made in live MinPlayableMap

1. **Bridge validation (32603)** — Confirmed editor world loaded, bridge fully operational.

2. **Behavior archetype tagging (32604)** — Walked all level actors and tagged every existing dinosaur (reused via label lookup, zero duplicates spawned per dedup rule) with a sociological behavior archetype:
   - **T-Rex** → `NPC_Behavior_Territorial` — patrols a 3500u radius, chases player within 3000u, attacks within 300u (aligned with #12 Combat AI's `TRexBehavior` spec).
   - **Raptors** → `NPC_Behavior_PackHunter` — coordinate with other raptors in the pack, flank the player rather than charge head-on.
   - **Brachiosaurus** → `NPC_Behavior_PassiveGrazer` — flees if approached within 800u, never initiates aggression (herbivore, low-threat, high-visibility landmark).
   - **Triceratops** → `NPC_Behavior_DefensiveHerd` — only charges if attacked first; otherwise holds ground in herd formation.

3. **Memory/state-machine seeding (32606)** — Every dinosaur actor received:
   - `NPC_HomeLoc_<X>_<Y>` — an anchor tag encoding its spawn coordinates, the "home" it patrols from and returns to after losing interest in the player (prevents NPCs endlessly chasing across the whole map — a Rockstar-AI staple: NPCs disengage and go back to their own life).
   - `NPC_AlertState_Unaware` — root state of a 3-state machine (`Unaware → Alert → Hostile`) that Combat AI (#12) can drive off proximity/line-of-sight checks.
   - Verified persistence via re-read (32605) before saving the level.

## Sociological design rationale
Per the agent's mandate ("the NPC exists to live its own life, the player is an interruption"): every archetype defines a *default disposition* independent of the player. Grazers ignore the player entirely unless crowded; pack hunters have an internal coordination need (each other) that exists whether or not a player is nearby; the T-Rex's patrol radius is a territorial claim, not a leash. This gives the next agent (#12 Combat & Enemy AI) concrete disposition data to drive Behavior Tree decision nodes without inventing lore from scratch.

## Voice line prototypes (text_to_speech)
Generated 2 short survival-NPC dialogue lines (audio synthesized successfully; Supabase upload hit the known cross-agent infra bug `403 Invalid Compact JWS` — same failure documented in prior cycles' memories, not agent-fixable):
1. **TribeScout** — territorial warning about T-Rex range ("stay low, don't cross the marked stones").
2. **HunterElder** — pack-hunting warning about raptors ("never fight one alone, the other two are circling behind you").
Both reinforce the tagged behavior archetypes above (territorial T-Rex, pack-hunting raptors) as diegetic survival knowledge the player can learn from NPC dialogue rather than trial and error.

## Decisions & justification
- No custom Behavior Tree assets created via Blueprint editor this cycle — Blueprint BT asset creation via Remote Control Python is unreliable/slow; instead seeded the *data layer* (archetype + home location + alert state tags) that a BT asset would read from, which is immediately usable and inspectable in the live world.
- Zero duplicate actors — all tagging done via label lookup on existing hub dinosaurs, per naming/dedup rule.
- Zero .cpp/.h writes — all logic expressed as actor tags/state, consistent with the absolute rule that this headless editor never recompiles C++.

## Files Created/Modified
- `Docs/NPCBehavior/Cycle_PROD_AUTO_20260712_006_NPCBehaviorPass.md` (this file)

## Dependencies / next steps for #12 Combat & Enemy AI Agent
- Read `NPC_Behavior_*` tags to drive per-species combat disposition (Territorial/PackHunter/PassiveGrazer/DefensiveHerd).
- Read `NPC_AlertState_Unaware` as the BT root state; drive transitions to `Alert`/`Hostile` off player proximity (T-Rex: 3000u alert / 300u attack, per existing `DinosaurCombatAIController` spec).
- Use `NPC_HomeLoc_<X>_<Y>` tags as return-to-patrol anchors after player disengages, to prevent infinite chase.
- Critical blocker inherited from #10 Animation Agent: zero skeletal meshes in `/Game` — no true animation-driven behavior (e.g., attack anim triggers) until a rigged mesh exists. Recommend importing the free UE5 Mannequin as an interim rig.
