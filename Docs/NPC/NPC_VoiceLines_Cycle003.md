# NPC Voice Lines — Cycle PROD_CYCLE_AUTO_20260708_003

Two tribesperson dialogue lines generated via ElevenLabs TTS this cycle, reflecting the
sociological principle that NPCs comment on the world (dinosaur territory, herd routines) as if
living in it, independent of the player triggering a quest.

## Line 1 — "Tribesman_Scout" (caution / threat awareness)
> "Something is moving through the trees. Keep your voice down and your spear ready — this
> territory belongs to something bigger than us."

Context: Barked when player approaches the T-Rex's 3000-unit alert radius (see
`DinosaurBehavior_TRex_Design.md`). Reinforces that the T-Rex owns this territory — the player is
the intruder, not the other way around.

Generation status: Audio synthesized successfully (~9s). Storage upload failed with the known
`403 Invalid Compact JWS` backend error (same issue reported in cycles 001/002) — audio data was
produced but not persisted to a retrievable URL. Not a content/voice issue; a storage-auth issue
to flag for the Audio Agent (#16) / infrastructure owner.

## Line 2 — "Tribesman_Elder" (herd routine / habituation knowledge)
> "The herd grazes here every morning before the heat rises. If we stay downwind and move slow,
> they won't even notice us."

Context: Ambient knowledge line for the herbivore "Graze"/"Wary" states described in the design
doc — communicates that herbivores have a daily routine (morning grazing) rather than existing
only when the player is nearby.

Generation status: Audio synthesized successfully (~8s). Same storage upload failure as Line 1.

## Recommendation
Flag to infrastructure/Director (#01): the ElevenLabs → Supabase Storage upload path has now
failed with an identical `403 Invalid Compact JWS` error across 3 consecutive cycles (001, 002,
003) for this agent alone. This is a systemic signing-key/auth issue on the storage upload step,
not a per-request fluke — worth a dedicated infrastructure fix rather than continued retries.
