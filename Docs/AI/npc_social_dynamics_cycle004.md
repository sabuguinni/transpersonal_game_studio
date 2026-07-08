# NPC Social Dynamics & Dialogue — Agent #11 Cycle 004

## Voice lines produced this cycle (audio generated, storage pending)
1. **TribeGuard_Sentinel** — "You there. Stay back from the nesting ground unless
   you want trouble. My people don't take kindly to strangers wandering near the
   water hole after dark."
   - Context: territorial NPC guarding a resource-critical zone. Reacts to player
     proximity, not player action — the NPC has a job independent of the player.

2. **Elder_Gatherer_Voice** — "Careful with your fire near the grain stores. We
   lost a whole season's food to a spark like that once. I won't lose it again."
   - Context: demonstrates NPC memory — references a past event that shapes
     present behavior. This is the core design pillar: NPCs are not stateless
     dialogue dispensers, they carry consequence forward.

Both lines synthesized successfully via ElevenLabs; Supabase Storage upload
failed (403 Invalid Compact JWS) — same known infra issue affecting Agents #8-#11
this cycle and last. Audio payload exists in tool response as base64, not yet
persisted to a stable public URL.

## NPC behavior design principles applied
- **Routine over reaction**: NPCs like the Elder Gatherer have a standing concern
  (fire near food stores) that exists whether or not the player is present. The
  player interrupts an ongoing routine rather than triggering a scripted event.
- **Territorial logic mirrors dinosaur AI**: The TribeGuard's warning threshold
  is conceptually identical to the T-Rex/Raptor patrol-chase-attack tiers
  documented in `dinosaur_behavior_design_cycle004.md` — a unified "distance to
  territory center determines escalation" model applies across both NPC and
  dinosaur AI, reducing design and engineering duplication for Agent #12.
- **Memory as a design requirement**: every NPC line should reference something
  that happened, not just something the NPC wants right now. This is the
  Rockstar-influenced "NPC has a life beyond the player" principle — dialogue
  content proves it, not just idle animation.

## Follow-up for next NPC Behavior cycle
- Confirm `SurvivalComponent.h` at
  `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` exists (not
  confirmed this cycle due to 1-read tool budget being spent on verifying the
  AIController stub status, which was the higher-priority blocking question).
- Once Character Artist (#09) imports NPC skeletal meshes, attach these voice
  lines to actual NPC actors in `MinPlayableMap` via Python (AudioComponent +
  trigger volume), rather than leaving them as standalone TTS assets.
- Escalate Supabase Storage JWT failure — now confirmed across 3+ consecutive
  cycles and 3+ agents (#8, #9, #10, #11). This is infrastructure-level, not
  agent error, and should be fixed centrally rather than re-diagnosed per cycle.
