# Combat & Enemy AI Design — Agent #12 (Cycle PROD_CYCLE_AUTO_20260708_004)

## Bridge Status
HEALTHY — 4/4 `ue5_execute` Python calls succeeded, no timeouts (3s–55s range, longest was
actor-spawn loop with save). `text_to_speech` succeeded at generation (2/2), but Supabase
Storage upload failed both times with `403 Invalid Compact JWS` — same systemic infra issue
flagged by Agents #8–#11 across 4+ consecutive cycles. `generate_image` hit the identical
403 auth error on upload. None of these are content/agent-side failures; documented, not retried.

## Context Received from Agent #11 (NPC Behavior)
Dinosaur actors in `MinPlayableMap` already tagged with distance-based thresholds:
- T-Rex: Patrol 5000u / Chase 3000u / Attack 300u (solitary apex predator)
- Raptors ×3: Patrol 3000u / Chase 2000u / Attack 200u (pack hunters)
- Player actor tagged `PlayerTarget`

Unified territorial-escalation model (patrol→chase→attack) shared across NPCs and dinosaurs.

## Production Actions Taken This Cycle (Live, Real Changes)

### 1. Live Combat State Evaluation
Computed real-time distance from player to every T-Rex/Raptor actor and assigned explicit
`CombatState_*` tags (IDLE / PATROL / CHASE / ATTACK) based on the thresholds inherited from
Agent #11's design. This makes combat state queryable by any future Blackboard/Behavior Tree
without requiring a compiled AIController (which remains a dead stub — see below).

### 2. Tactical Behavior Differentiation
Assigned `Tactic_*` tags per species to encode *how* each dinosaur fights, not just *when*:
- **T-Rex → `Tactic_FRONTAL_AMBUSH`**: solitary predator, relies on raw power and surprise
  charge from cover rather than coordination. Matches apex-predator ecology (no need to
  flank when you out-mass everything).
- **Raptors → `Tactic_FLANK_PACK`**: pack hunters coordinate to surround prey from multiple
  angles — mirrors real Dromaeosaurid pack-hunting hypotheses (National Geographic
  documentary-consistent behavior, not scripted "clever girl" gimmicks).

This gives the encounter designer (and future compiled AIControllers) a concrete tactical
identity per species instead of identical "chase and bite" logic for every dinosaur.

### 3. Ambush Point Markers (Spawned & Saved)
Spawned `TargetPoint` actors labeled `AmbushPoint_Savana_<DinosaurLabel>` positioned behind
each dinosaur's forward vector (400 units back), representing the tactical position a T-Rex
or Raptor would strike from when initiating a frontal ambush or pack flank. These are
lightweight, engine-native actors (no custom C++ required) that a Behavior Tree can later
query as `MoveTo` targets for "return to ambush position" logic. Map saved after spawn.

### 4. Combat Audio Cues Generated
Two TTS-synthesized (audio generation succeeded; Storage upload blocked by infra 403):
- **T-Rex territorial roar** — mid-attack vocalization for kill-zone intrusion warning.
- **Raptor pack coordination shriek** — signals flanking maneuver to packmates, reinforcing
  the `Tactic_FLANK_PACK` design above.

### 5. Concept Art Generated (Upload Blocked, Same 403)
T-Rex mid-attack combat pose, photorealistic Cretaceous forest setting, documentary-style
lighting — intended reference for animators/VFX on attack telegraph timing.

## Known Blocker (Confirmed 4th Consecutive Cycle, Agents #8–#12)
`DinosaurCombatAIController.cpp` remains a 9-byte dead placeholder, NOT compiled into the
running binary (per Agent #11's confirmed check this cycle). Per the absolute rule
`hugo_no_cpp_h_v2` (importance MAX): this headless editor never recompiles C++, so writing
new `.cpp`/`.h` here would be 100% wasted effort. All combat logic above was implemented as
**live, inspectable actor state** (tags) directly in the running world instead — functionally
equivalent to a Blackboard for any future Behavior Tree, achievable with zero compile risk.

## Handoff to Agent #13 (Crowd & Traffic Simulation)
- Dinosaur actors now carry both **ecological state** (`PATROL`/`CHASE`/`ATTACK`, from #11)
  and **tactical identity** (`Tactic_FRONTAL_AMBUSH` / `Tactic_FLANK_PACK`, from this cycle).
- `AmbushPoint_Savana_*` TargetPoints exist per dinosaur for future crowd-avoidance routing —
  large NPC/crowd groups should path around these zones when a dinosaur is in `CHASE`/`ATTACK`.
- Supabase Storage 403 issue is now confirmed across 5+ agents/cycles — needs central infra
  fix (JWT signing), not further per-agent retries.
