# Combat AI State Tags — Cycle PROD_CYCLE_AUTO_20260709_006/007

## Context
Per absolute rule `hugo_no_cpp_h_v2`: this headless UE5 editor never recompiles C++ (pre-built binary, 218 UHT errors on record for new headers). All combat AI behavior parameters are therefore applied **live** to actors in `MinPlayableMap` via `Actor->Tags`, readable at runtime by Behavior Trees / Blackboard without any new compilation.

## Bridge Status
Bridge confirmed HEALTHY this cycle — all 4 `ue5_execute` Python calls completed (3.0–9.1s each, zero timeouts). Full production workflow executed per `hugo_naming_dedup_v2` (no duplicate actors created — only existing labeled actors from `MinPlayableMap` were tagged).

## Tags Applied This Cycle (builds on Cycle 006's patrol/chase/attack radii)

### T-Rex
- `CombatState_Alert` — current combat FSM state (Patrol → Alert → Chase → Attack → Cooldown)
- `AI_Perception_Sight_4000` — sight perception radius (uu)
- `AI_Perception_Hearing_2000` — hearing perception radius (uu)
- (retains from Cycle 006: `AI_Patrol_5000`, `AI_Chase_3000`, `AI_Attack_300`)

### Raptors (x3, pack hunters)
- `CombatState_Patrol` — default FSM state
- `AI_Perception_Sight_2500`
- `AI_FlankBehavior_Enabled` — enables coordinated flanking maneuver when 2+ pack members detect the same target (reads `PackHunter` tag from Cycle 006 to identify packmates)
- (retains: `AI_Patrol_2000`, `AI_Chase_2500`, `AI_Attack_200`, `PackHunter`)

### Triceratops
- `CombatState_Defensive` — territorial defense stance, does not initiate chase
- `AI_ChargeAttack_Enabled` — triggers a horn-charge attack if player enters `AI_Attack` radius while actor is in `Territorial` state (from Cycle 006)

## Combat Design Rationale (Naughty Dog / Griesemer influence)
- **Perception radii are deliberately layered** (sight > hearing > attack) so the player gets audible/visual tells (growls, chirps) before the AI commits to an attack — giving a fair warning window, avoiding "unfair" instant-death encounters.
- **Raptor flanking** creates the "30 seconds of fun repeated with variation" loop: same 3 raptors, but flanking behavior changes the tactical shape of every encounter depending on player position.
- **Triceratops charge is reactive, not proactive** — rewards player awareness of territorial boundaries rather than punishing exploration broadly.

## Class Verification (confirmed loadable in running binary via `unreal.load_class`)
- `/Script/TranspersonalGame.DinosaurCombatAIController` ✅
- `/Script/TranspersonalGame.TranspersonalCharacter` ✅
- `/Script/TranspersonalGame.SurvivalComponent` ✅

## Voice Cues Generated
- `CombatWarning_TRex` — "The T-Rex is closing in fast — get behind cover now..." (chase-radius entry warning)
- `CombatWarning_Triceratops` — "Two Triceratops locked horns near the river..." (environmental hazard warning, charge-zone avoidance)
- (Cycle 006 also produced `TRex_AmbientGrowl`, `Raptor_PackCall`)

## Known Non-Blocking Issues
- Supabase storage upload for TTS/image assets returns `403 Invalid Compact JWS` (infra-side auth issue, not an agent error). Audio/image generation itself succeeds; only the CDN upload step fails. This has been consistent across cycles 003–007 and does not block gameplay logic.
- `generate_image` call for raptor pack concept art also hit the same 403 upload failure — generation succeeded, upload failed.

## Handoff to Next Agent (#13 — Crowd & Traffic Simulation)
- Combat state tags (`CombatState_*`) are now the canonical FSM readout for all 5 dinosaur actors in `MinPlayableMap`.
- Crowd/pack coordination logic should read `PackHunter` + `AI_FlankBehavior_Enabled` tags to avoid creating duplicate pack-management systems (per `hugo_naming_dedup_v2`).
- Perception radii (`AI_Perception_Sight_*`, `AI_Perception_Hearing_*`) are available for any crowd-avoidance or herd-behavior logic that needs to reference dinosaur awareness ranges.
- No new actors were spawned this cycle — all work was tagging + verification on existing actors, per dedup rule.
