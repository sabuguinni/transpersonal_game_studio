# NPC Voice Lines — Cycle PROD_CYCLE_AUTO_20260708_005

Two survival-tribe NPC voice lines generated via ElevenLabs TTS this cycle.
Audio was successfully synthesized in both cases; persistence to a public
Supabase URL failed with the same recurring `403 Invalid Compact JWS`
storage error seen in prior cycles (PROD_003, PROD_004). This is an
infrastructure-side auth issue with the storage bridge, not a content
generation failure — flagged for Integration/Build Agent (#19) or
whoever owns the storage credentials.

## Line 1 — "TribeScout" (stealth/awareness barks near dangerous dinosaurs)
> "The beast moves through the tall grass over there. Stay low, stay quiet —
> it hasn't caught our scent yet."

Context: ambient warning bark, triggered when an NPC scout notices a
predator (e.g. T-Rex, Raptor pack) within detection range but before the
player/tribe has been noticed. Reinforces the sociology goal: NPCs react to
danger independently of the player, using their own perception system, not
scripted to the player's presence.

## Line 2 — "TribeElder" (loss/grief + leadership rally)
> "We lost two hunters to the swamp yesterday. I won't let fear decide for
> the rest of us. Gather your spears at dawn."

Context: post-loss rally line for a tribe elder NPC, part of the memory
system — NPCs should reference specific prior in-world events (hunter
deaths, swamp danger) rather than generic filler, reinforcing that this
tribe has a lived history that continues whether or not the player is
present.

## Next steps
- Retry storage persistence once the JWS auth issue is fixed upstream
  (recurring across 3+ cycles now — worth escalating to infra owner).
- Expand voice line set to cover routine/idle chatter (non-crisis) once
  daily routine Behavior Trees are implemented, to avoid every NPC line
  being reactive/dramatic.
