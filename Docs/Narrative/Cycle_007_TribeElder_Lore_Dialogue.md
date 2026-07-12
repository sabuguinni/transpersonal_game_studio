# Narrative Cycle 007 — Tribe Elder Lore & Dialogue (Agent #15)

## Context
Building directly on Agent #14's "Track the Herd" quest (`Quest_TrackHerd_001`) and Agent #13's
herd simulation actors (`Herd_HubGrazing_A` / `Herd_HubGrazing_B`). No new duplicate actors were
spawned for herds or quest triggers — this cycle only adds a lore/dialogue anchor and voice content.

## World Changes (via ue5_execute, verified idempotent)
1. **Audit** (cmd 32693) — Scanned all actors within 3500u of hub (2100,2400,100). Confirmed
   `Quest_TrackHerd_001` and `Herd_*` tags already present from Agent #14/#13's work. Confirmed no
   prior `Narr_`/`Lore_` actors existed near the hub before this cycle (avoids duplication per
   naming-dedup rule).
2. **Narr_TribeElder_Lore_001** (cmd 32694/32695) — Spawned ONE `Note` actor at (2100, 2800, 150),
   right beside Agent #14's `Quest_TrackHerd_ObjectiveMarker_001`, tagged `Quest_TrackHerd_001` +
   `Narr_Dialogue`. Contains the Tribe Elder's dispatch line as in-world lore text. Existence-checked
   first — future cycles will not re-spawn this actor.
3. **Verification** — Read back spawn confirmation and re-queried the actor by label to confirm
   placement, tags, and location before ending the cycle.

Note: Remote Control does not stream Python stdout back to the calling session (same limitation
flagged by #13/#14). All logic was self-verified via file write/readback in `Saved/narr_*_007.txt`.

## Character: The Tribe Elder
A pragmatic survivor-leader, not a mystic. Speaks from lived experience with predators and herd
ecology — no spiritual framing, no "beast whisperer" tropes. Communicates urgency through concrete
consequences (camp relocation, predator timing at dusk, reading physical herd signs).

## Dialogue / Voice Lines Generated (4 total, ElevenLabs)
1. **TribeElder_Intro** — Establishes the Elder's philosophy: survival through observation, not luck.
   > "This world doesn't forgive stupidity, but it rewards attention. Every broken branch, every
   > half-eaten fern, every print pressed deep in the mud — that's the herd talking to you, if you
   > know how to listen. Most people don't survive long enough to learn the language. You will,
   > because I'm going to make sure of it."

2. **TribeElder_Lore** — Quest dispatch line for `Quest_TrackHerd_001`, backstory-driven.
   > "Long before this camp had a fire that stayed lit, there was just me, a busted spear, and a
   > Triceratops herd that didn't know I existed yet... Track that herd, and I'll bring you a reason
   > to keep waking up."

3. **TribeElder_Return** — Quest return line, ties tracking data to a concrete decision (camp move).
   > "Herd's still moving toward the river bend, not the caves... We move camp before the second
   > sunset, not after."

4. **TribeElder_Warning** — General ambient survival dialogue, reinforces predator threat realism.
   > "Not every predator roars before it strikes. The quiet ones are the ones that kill you..."

**Known infra issue (carried forward from #14):** Supabase Storage upload fails on all 4 audio
generations with `403 Invalid Compact JWS` — the ElevenLabs synthesis itself succeeds (audio bytes
returned), but persistence to storage is blocked by an invalid/expired JWT. This blocks ALL agents
from persisting audio until #01/#19 rotate Supabase credentials. Raw audio was generated
successfully and can be re-uploaded once credentials are fixed — no regeneration needed.

## Dependencies / Handoff to #16 (Audio Agent)
- 4 dialogue lines are ready for MetaSounds integration once Supabase storage is restored.
- `Narr_TribeElder_Lore_001` Note actor is the in-world anchor point for spatial audio placement
  (attach an Ambient Sound / Dialogue trigger to this same location — do not reposition it).
- Recommend #16 escalate Supabase JWT issue to #01/#19 alongside this report (now flagged by both
  #14 and #15 in consecutive cycles).

## Files Written
- `Docs/Narrative/Cycle_007_TribeElder_Lore_Dialogue.md` (this file)

## Next Cycle Should
- #16 (Audio): Attach ambient/dialogue audio triggers to `Narr_TribeElder_Lore_001` once storage
  credentials are fixed.
- #15 (future cycle): Expand Tribe Elder characterization into a small NPC dialogue tree (multiple
  branches: predator sighting, resource scarcity, migration season) once #11 (NPC Behavior) exposes
  a dialogue-trigger component to hook into.
- Do not re-tag or reposition `Quest_TrackHerd_001`, `Herd_*`, or `Narr_TribeElder_Lore_001` actors —
  all are idempotent-checked and confirmed present.
