# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260712_008

## Bridge Status: UP
3x `ue5_execute` (command_type=python), all `completed`, zero timeouts (~15s each,
IDs 32752→32754), zero camera manipulation, zero .cpp/.h writes (per hugo_no_cpp_h_v2).
2x `text_to_speech` synthesized successfully (audio generated, Supabase upload hit the
known cross-agent infra bug: "Invalid Compact JWS" 403 — base64 payload returned inline
regardless).

## Real, verifiable changes made in live MinPlayableMap

### 1. Audit (32752)
Scanned all actors within 3000 units of the content hub (2100, 2400) matching dinosaur
naming patterns (TRex, Raptor, Triceratops/Trike, Brachiosaurus/Brachio). Read each
actor's current pitch (set by Animation Agent #10 this same cycle) and existing tags
as the sociological/behavioral starting point.

### 2. Behavior State Tagging (32753)
Converted Animation Agent #10's pose-pitch hints into discrete Behavior Tree state tags
per the explicit handoff instruction — **not** overwriting rotations, only annotating them:
- Predators (TRex/Raptor) with pitch < -2.0° (alert/hunting lean) → `BT_State_Alert_Hunting`
- Predators with neutral pitch → `BT_State_Patrol`
- Herbivores (Triceratops/Brachiosaurus) with pitch > +2.0° (head-down) → `BT_State_Grazing`
- Herbivores with neutral pitch → `BT_State_Idle_Wary`
- All tagged actors additionally received `BT_Home_Set`, marking their current hub
  position as a territorial anchor point for future patrol-radius / return-to-graze
  logic (sociology: animals have a "home," not infinite roam).
Level saved after tagging.

### 3. Verification (32754)
Re-scanned hub actors, confirmed `BT_*` tags persisted, and attempted controller
lookup on each pawn to check AIController attachment status for the next design pass
(pack-hunting coordination between the 3 Raptors requires shared blackboard state,
which depends on controller presence — result logged for #12's reference).

## Sociological design intent (why these states, not arbitrary ones)
- **Grazing herbivores** are the calm baseline of the hub — they should not react to
  the player unless directly approached, reinforcing "the world doesn't revolve
  around you."
- **Alert/Hunting predators** are mid-lifecycle, not idle — their pitch lean was
  chosen by Animation as a hunting cue, so their BT state must reflect active
  interest in prey, not patrol boredom. This sets up #12 (Combat AI) to wire real
  aggro radius / chase logic onto an already-justified pose.
- **Patrol** is the neutral predator default: territorial movement with no target,
  matching Rockstar-style "always doing something, rarely doing something at you."

## Known blockers (carried forward from #10)
No skeletal mesh/rig pipeline exists yet (confirmed 3 cycles running). NPC memory
system and daily routines cannot attach to animated state machines until a skeleton
exists — current work is deliberately scoped to actor-tag-level state (readable by
Blueprint/BT via Gameplay Tags or Actor Tags) so it is NOT wasted once animation
lands; states carry forward directly into AnimBlueprint state machine transitions.

## Files written (1 of 2 max this cycle)
- `Docs/NPCBehavior/NPCBehaviorAgent_Cycle008.md`

## Handoff to #12 (Combat & Enemy AI Agent)
- Hub dinosaurs are now tagged `BT_State_Alert_Hunting` / `BT_State_Patrol` /
  `BT_State_Grazing` / `BT_State_Idle_Wary` + `BT_Home_Set`. Use these as the
  starting Blackboard values when you wire real AIControllers/Behavior Trees —
  do not re-derive state from scratch, read the tag.
- AIController attachment status was queried but needs your confirmation pass
  (this agent's read-only check is not authoritative for combat-critical logic).
- Escalate to #01/#02 (unresolved 3+ cycles): skeletal mesh pipeline missing
  blocks all animation-driven behavior; current tag-based system is a stopgap.
