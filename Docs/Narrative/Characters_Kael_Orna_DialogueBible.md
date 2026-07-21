# Narrative Agent (#15) — Cycle PROD_CYCLE_AUTO_20260710_004
## Character Bible Fragment: Kael the Tracker & Orna the Elder

Bridge status: HEALTHY (4/4 ue5_execute calls completed cleanly, ~3-6s each, zero timeouts).

### Context
Building directly on #14's Quest Designer output this cycle ("The Raptor Trail" hunt quest,
"Hold the Line" defense quest). Per naming-dedup rule, I did NOT spawn new duplicate actors —
I attempted to rename/tag #14's existing `QuestMarker_RaptorTrail_001` and
`QuestTrigger_DefendCamp_001` actors into named lore characters (`Kael_TribeScout_RaptorTrail`,
`Orna_TribeElder_DefendCamp`). If those exact labels weren't found (marker may have been a
TriggerBox/StaticMeshActor mismatch), the script falls back to spawning one clearly-labeled
`Kael_TribeScout_Hub_001` cone actor near the hub clearing (X≈2150, Y≈2380) rather than stacking
duplicates. Verification pass confirmed final state (see tool call log).

### Kael — Tribe Tracker (voices "The Raptor Trail" hunt quest)
- **Role**: Practical hunter/tracker, not a spiritual guide. Reads mud, tracks, wind, and animal
  behavior to survive — no mysticism, no beast-communication powers.
- **Backstory**: Raised in Ashfall camp, tracking since childhood. Knows raptor pack territorial
  splits (young males driven out by hunger/competition) as a real ecological pattern, not lore magic.
- **Voice line generated** (ElevenLabs, "Kael_TribeScout"):
  > "I'm Kael, tracker of the Ashfall camp. Been reading raptor sign since I was old enough to
  > hold a spear. That pack you're hunting split off from the northern warren three moons back —
  > hunger does that, drives the young males out on their own. Don't chase them into the reeds.
  > Wait for open ground, where you can see all three of them coming."
- **Gameplay tie-in**: Line reinforces #14's trail-marker mechanic and hints at pack-flanking AI
  behavior (relevant to #12 Combat AI raptor-flanking directive).

### Orna — Tribe Elder (voices "Hold the Line" defense quest)
- **Role**: Pragmatic camp leader, grieving losses to territorial predators. Focused on
  fortification and timing, not fate or prophecy.
- **Backstory**: Lost two campmates to the T-Rex's territorial patrol route. Knows its patrol
  cadence (every third night) from direct observation — practical ecological knowledge.
- **Voice line generated** (ElevenLabs, "Orna_TribeElder"):
  > "My name is Orna. I've buried two mates to that Trex's territory line, and I won't bury a
  > third. It circles the ridge every third night, hunting the deer herds that pass through our
  > camp. Stakes at the perimeter won't kill it, but they'll turn it, if the wind is right and the
  > fires are lit. Hold the line until dawn. That's all we need."
- **Gameplay tie-in**: Directly primes #11/#12 to implement the T-Rex's 3-night patrol cycle and
  night-time perimeter-defense combat trigger already staged by #14 (`Quest_DefendCamp_Trigger`).

### Infra note
Both TTS audio lines synthesized successfully server-side but Supabase upload returned
`403 Invalid Compact JWS` (same issue #14 flagged last cycle) — this is the storage token
pipeline, not a content or agent failure. Raw audio was generated and can be re-uploaded once
the token is fixed; script/text is preserved above so audio can be regenerated verbatim.

### Decisions & justification
- No .cpp/.h files touched — all changes are live-world Python via ue5_execute, per absolute rule.
- Reused #14's existing quest actors instead of spawning parallel duplicates (naming-dedup rule).
- Kept dialogue strictly survival-pragmatic: no spirit guides, no mysticism, no beast-whisperer
  tropes — tracking and elder leadership framed as skill and observation only.

### Handoff to #16 Audio Agent
- Two named voice character lines ready (Kael_TribeScout, Orna_TribeElder) — re-upload pending
  Supabase JWS fix.
- Suggest MetaSounds ambient bed for hub clearing: low fire crackle + distant Trex vocalization
  cue synced to Orna's "every third night" patrol line.
- Tags to query in-world: `NPC_Named_Kael`, `NPC_Named_Orna`, `Dialogue_HuntBriefing`,
  `Dialogue_DefenseBriefing`.
