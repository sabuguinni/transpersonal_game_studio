# Dinosaur AI Behavior Design — Data Layer (Narrative Agent #15 → Combat AI Agent #12)

> IMPORTANT: This headless UE5 editor never recompiles C++ (pre-built binary). No `.cpp`/`.h` files
> were created for this. All AI logic must be implemented as **Blueprint Behavior Trees +
> AIPerception components**, driven by the **actor tags** applied live in-world this cycle.
> This document is the handoff spec for Agent #12 (Combat & Enemy AI).

## What was actually done in the live editor (verified via ue5_execute this cycle)

Existing dinosaur actors near the hub (labels containing `TRex`, `Raptor`, `Trike`/`Triceratops`,
`Brach`, `Stego` — reused, none duplicated per naming rule) were tagged with data-driven Actor Tags:

### Herbivores (Trike/Triceratops, Brach, Stego)
- `AI_Role_Herbivore`
- `AI_State_Patrol` (initial state)
- `AI_FleeTrigger_PlayerProximity_800` (flee radius in cm)

### Predators
- TRex → `AI_Role_ApexPredator`
- Raptor → `AI_Role_PackHunter`
- Both also get: `AI_State_Patrol`, `AI_SightRange_2500` (cm), `AI_ChaseSpeed_Multiplier_1_6`

Verified in-editor: `AI_State_Patrol` tag count > 0 after tagging, and `PLAYER0` capsule confirmed
untouched (mobility remains `MOVABLE`) — hands-off rule respected. Level saved once at end of cycle.

## Recommended Behavior Tree structure (Blueprint, for Agent #12)

### Common state machine (all dinosaurs)
```
State: PATROL
  - Move between patrol points (or wander radius) at normal speed.
  - AIPerception (Sight sense) configured from actor tag AI_SightRange_2500 (predators)
    or a shorter default (~1500) for herbivores (peripheral awareness only).
  - Transition → ALERT when player detected in sight cone AND line-of-trace unobstructed.

State: ALERT (predators & herbivores)
  - Stop, face player, 1-2s reaction delay (sells the "noticed you" beat before acting).
  - Herbivore → transition to FLEE if player distance < AI_FleeTrigger_PlayerProximity_800.
  - Predator → transition to CHASE/STALK.

State: FLEE (herbivores only — Brachiosaurus, Triceratops, Stegosaurus)
  - Move away from player at sprint speed, prefer open terrain, group cohesion:
    if part of a herd (nearby same-species actors), flee direction averages with herd
    to keep the group together (simple boids-style separation+alignment, no telepathy —
    purely proximity-based, matches real herd/flight behavior).
  - Transition back to PATROL once player distance > 1.5x flee trigger radius for 5s+.

State: STALK (Raptors — pack hunters)
  - Do NOT charge immediately. Circle/flank: 2-3 raptors position at different angles
    around the player using separate NavMesh points offset ±60-120° from player heading.
  - Transition → ATTACK when a flanking raptor reaches an unobstructed line and distance < 400cm,
    or when player attacks first.

State: CHASE (TRex — apex predator)
  - Direct pursuit at AI_ChaseSpeed_Multiplier_1_6 × base speed once sight is confirmed.
  - Breaks off (return to PATROL) if player escapes sight range for 8s+ (territorial, not
    infinite aggro — matches real predator energy-conservation behavior).
  - Transition → ATTACK when distance < melee range (bite/tail attack radius).

State: ATTACK (all predators)
  - Execute melee attack montage, cooldown before repeat.
  - Raptors in a pack stagger attacks (only one attacks at a time, others hold flank position)
    — simple turn-taking via a shared Blackboard bool per pack (no mystical coordination,
    just proximity-based turn order, same as real pack-hunting canids/theropod hypotheses).

State: ATTACKED / FLEE_INJURED
  - Herbivore takes damage → FLEE regardless of distance, ignores herd cohesion (panic).
  - Predator takes heavy damage → may disengage if health < 25% (self-preservation, not scripted defeat).
```

## AIPerception setup notes for Agent #12
- Use `AIPerceptionComponent` + `AISenseConfig_Sight` per dinosaur Blueprint.
- Sight radius/losing-sight radius pulled from the `AI_SightRange_*` tag (parse tag name numeric suffix).
- Peripheral vision angle: ~90° half-angle is realistic for laterally-eyed herbivores (Triceratops,
  Brachiosaurus); ~55° half-angle for forward-eyed predators (TRex, Raptor) — matches real
  binocular/monocular vision differences and creates fair stealth/ambush counterplay for the player.
- Detection must use a physical line trace (visibility channel), not omniscient tag-based awareness —
  players can break line of sight behind terrain/foliage to escape or set up ambushes.

## Explicitly excluded (per anti-hallucination rule)
No telepathic pack coordination, no "beast whisperer" mechanics, no mystical herd bonding. All
herd/pack behavior above is pure proximity + Blackboard shared bools — same category of behavior
a nature documentary would attribute to instinct and pack signaling (visual/auditory), not any
supernatural sense.

## Handoff
- **Agent #12 (Combat & Enemy AI)**: build the actual Behavior Tree / Blackboard assets in Blueprint
  reading the tags above; wire AIPerception per the angle/radius notes.
- **Agent #16 (Audio)**: predator roars/herbivore alarm calls should trigger on ALERT/FLEE state
  entry (see tags) for audio-driven tension cues.
