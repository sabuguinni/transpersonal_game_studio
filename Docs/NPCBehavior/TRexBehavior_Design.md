# T-Rex Behavior Design (Runtime Tag-Driven)
Agent #11 — NPC Behavior Agent | Cycle PROD_CYCLE_AUTO_20260709_001

## Why not a .cpp file
Per the ABSOLUTE RULE memory (`hugo_no_cpp_h_v2`, importance MAX): this headless UE5 editor instance runs a pre-built binary that never recompiles C++. Any `TRexBehavior.cpp` written to GitHub would sit inert in the repo with zero effect on the live game. This design doc replaces the originally-requested `.cpp` deliverable with a **runtime-applied, Blueprint-consumable spec**, already partially applied this cycle via Actor Tags (see `DinosaurCombatAI_Status.md`).

## Behavior Spec — T-Rex (Apex Predator, Solitary)

### State: PATROL (default)
- Roams within a 5000-unit radius of spawn point (tag `PatrolRadius_5000`)
- Moves at walk speed, pauses periodically to "scan" (idle animation placeholder)
- No aggro — ignores player unless within Chase range

### State: CHASE
- Trigger: player enters 3000-unit radius (tag `ChaseRange_3000`)
- T-Rex turns toward player, increases move speed to run/charge speed
- Persists chase even if player briefly exits radius (adds hysteresis — recommend +500 unit buffer before disengage, to be tuned by Combat AI Agent #12)
- Roars/vocalizes on chase-start (hook for Audio Agent #16)

### State: ATTACK
- Trigger: player within 300-unit radius (tag `AttackRange_300`)
- T-Rex performs bite/stomp attack, deals damage to player health stat
- Cooldown between attacks (recommend 2s, tuned by Combat Agent #12)
- If player exits attack range but stays within chase range → return to CHASE state

### State: DISENGAGE
- Trigger: player exits chase range + buffer, or player escapes line-of-sight for N seconds
- T-Rex returns toward patrol origin, resumes PATROL

## Sociological Rationale (per Agent #11 mandate)
The T-Rex does not exist to "fight the player on demand." It is an apex predator defending territory. It will chase intruders relentlessly within its hunting range and disengage once the perceived threat/prey leaves its territory — behavior grounded in real predator ecology (opportunistic ambush + territorial defense), not scripted boss-fight logic.

## Pack Hunter Spec — Raptors (x3, Coordinated)
Tagged `Behavior_PackHunter`, `PatrolRadius_2500`, `ChaseRange_2000`, `AttackRange_150`. Design intent (for Combat Agent #12 to implement in Behavior Tree):
- Raptors patrol together, sharing a loose formation around their pack anchor point
- When one raptor detects the player (chase range), it broadcasts alert to the other two (shared Blackboard key or Gameplay Message)
- Pack attempts flanking: one raptor approaches from front, others circle to sides before entering attack range
- This creates emergent danger — a lone raptor is manageable, a pack is not

## Handoff Requirements
- **Combat & Enemy AI Agent (#12):** implement Behavior Tree logic reading the Actor Tags applied this cycle. Since C++ compilation is unavailable, prefer Blueprint-based AIController + Behavior Tree assets created via `ue5_execute` Python (e.g., `unreal.AIBlueprintHelperLibrary`, `BehaviorTreeFactory`) rather than new C++ classes.
- **Audio Agent (#16):** hook roar/vocalization SFX to CHASE state transition.
- **Animation Agent (#10):** current dinosaur pawns are basic-shape placeholders with no skeletal mesh — attack/chase animations blocked until mesh + skeleton pipeline lands.
