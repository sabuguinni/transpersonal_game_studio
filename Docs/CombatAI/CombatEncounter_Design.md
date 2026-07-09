# Combat & Enemy AI — Tactical Design (Agent #12)
Cycle: PROD_CYCLE_AUTO_20260709_001

## Context
Per `hugo_no_cpp_h_v2` (imp:MAX): this headless UE5 build never recompiles C++.
All combat AI iteration this cycle was delivered as **live runtime Actor Tags**
applied directly to existing MinPlayableMap actors via `ue5_execute` (Python),
consumed downstream by Blueprint AIControllers / Behavior Trees — NOT new .cpp/.h.

Builds directly on Agent #11's patrol/chase/attack tag foundation
(`Behavior_TRexAggressive`, `Behavior_PackHunter`) — this cycle adds the
**combat-specific tactical layer** on top (what happens once engagement starts).

## Tags Applied This Cycle

### T-Rex (solo apex predator — ambush + intimidation choreography)
- `CombatAI_Solo` — no pack coordination, single-target focus
- `CombatAI_AmbushBehind` — prefers flanking from behind foliage/terrain before first strike
- `CombatAI_HealthThreshold_30_Retreat` — disengages and retreats when HP drops below 30%
- `CombatAI_RoarIntimidate` — triggers intimidation roar (fear stat impact on player) before charging

### Raptors x3 (pack tactical hunters — coordinated flanking)
- `CombatAI_PackFlank` — surrounds target from multiple angles, not frontal rush
- `CombatAI_CallToPack_Range1500` — alerts other raptors within 1500 units to converge
- `CombatAI_HealthThreshold_40_Flee` — pack member flees at 40% HP if isolated from pack
- `CombatAI_CircleTarget` — circles player to force reaction/panic before committing to attack

### Interaction Triggers → Combat Zones
- `CombatZone_TensionRamp` — audio/lighting cue ramp on zone entry (hooks into Agent #08/#16)
- `CombatZone_MaxConcurrentEnemies_2` — caps simultaneous attackers to keep fights readable/fair
- `CombatZone_EscapeRouteRequired` — design constraint: every combat zone must have a clear player escape path (Griesemer principle — losing should feel like a mistake, not unfairness)

## Design Philosophy Applied
- **Naughty Dog influence:** T-Rex roar-intimidate is a "reaction beat" — the player should feel hunted before the fight starts, not ambushed by an invisible damage tick.
- **Griesemer "30 seconds repeated with variation":** the Raptor pack loop (circle → call-to-pack → flank → retreat-if-isolated) is a single tactical unit repeatable across encounters, varied by number of raptors present and terrain.
- **"Player believes they lost because of their own mistake":** retreat/flee thresholds ensure enemies never fight to the death blindly — an AI that disengages reads as intelligent, not as a rubber-banding damage sponge.

## Class Status Verified This Cycle
- `DinosaurCombatAIController` — class loads via `unreal.load_class`, still a dead placeholder (no Blueprint Behavior Tree wired to it yet). This is the **hard blocker** for real combat AI in this project.
- `TranspersonalCharacter` — loads correctly, confirmed as the single character class (health/hunger/thirst/stamina/fear stats live here, per Agent #11 audit).
- `TranspersonalGameState` — loads correctly.

## Blocker (unchanged from Agent #11 report, now confirmed independently)
Combat AI cannot advance beyond tag-driven runtime configuration until:
1. A Blueprint Behavior Tree is authored and assigned to `DinosaurCombatAIController` (requires editor UI work outside Python remote control's practical reach, or a future C++ recompile window).
2. Dinosaur pawns get skeletal meshes + attack animations (Agent #10 blocker — still basic-shape placeholders).

## Audio/Visual Assets (generated, storage upload failed — known infra issue)
- `TRexRoar_CombatSFX` — deep guttural bellow, generated successfully via ElevenLabs, Supabase upload hit `403 Invalid Compact JWS` (systemic, 6th+ consecutive cycle with this exact error across multiple agents — flagged, not retried per protocol).
- `RaptorSnarl_CombatSFX` — pack shriek/snarl, same generation success / same storage failure.
- Raptor pack hunting concept art (1792x1024, HD) — generated successfully via GPT Image 1, same storage failure.

## Next Steps for Agent #13 (Crowd & Traffic Simulation)
- Combat zones (`CombatZone_MaxConcurrentEnemies_2`) should inform crowd density caps near danger areas — don't route large NPC crowds through active combat zones.
- Raptor `CombatAI_CallToPack_Range1500` tag can be reused as a template for crowd panic-propagation radius (prey animals fleeing when a predator call is heard).
