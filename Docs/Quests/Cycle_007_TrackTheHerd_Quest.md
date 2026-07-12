# Quest & Mission Designer — Cycle PROD_CYCLE_AUTO_20260712_007

## Quest: "Track the Herd"

### Design Rationale
A tribe tracker NPC needs current intel on a nearby Triceratops herd's location and
movement pattern before nightfall, when pack-hunting predators become more active.
This is pure survival/ecology gameplay — no mysticism, no spiritual framing. It ties
directly into Agent #13's herd simulation output (`Herd_HubGrazing_A`, `Herd_HubGrazing_B`)
so the quest objective reads real, already-simulated herd state rather than scripted fakery.

### Emotional Arc (Pawel Sasko lens)
- **Setup**: Tracker is worried — food security depends on herbivore migration patterns.
- **Player agency**: Player must physically travel to the herd, observe herd size/composition,
  and return before "the light goes red" (dusk predator window) — creates real time pressure.
- **Payoff**: Report back determines whether the camp relocates. The player's observation
  has narrative weight — not just a checkbox, but information that changes NPC dialogue
  and (future) camp AI decisions.

### Mechanical Statement (Jonathan Blow lens)
The mechanic here is "observe and report accurately" — the game is stating that information
gathering about the ecosystem is itself a survival skill, on par with combat or crafting.

### World State Changes (this cycle)
1. **Audit** of all actors within 3500u of hub (2100,2400) for existing Quest_*/Herd_* tags
   (self-verified via Saved/quest_audit_007.txt due to RC stdout gap).
2. **Tagged** existing Triceratops/Trike actors near hub with `Quest_TrackHerd_001`
   — idempotent, skips actors already tagged, zero new duplicate actors spawned
   (reused Agent #13's `Herd_HubGrazing_A` members directly per naming-dedup rule).
3. **Spawned** one `TargetPoint` actor `Quest_TrackHerd_ObjectiveMarker_001` at
   (2100, 2800, 100) as the quest's world-space objective anchor — checked for
   pre-existing marker first, would not duplicate on re-run.

### Voice Lines Generated (ElevenLabs TTS)
1. **QuestGiver_Tracker** (dispatch line): tracker asks player to scout the Triceratops
   herd, count calves/adults, note migration direction, return before dusk.
2. **QuestGiver_Tracker_Return** (return line): tracker asks for the report, ties
   camp relocation decision to player's findings.
   - NOTE: Audio synthesis succeeded on ElevenLabs side; Supabase upload failed with
     `403 Invalid Compact JWS` (expired/invalid storage JWT) — this is an infra issue,
     not a generation failure. Flagging to #01/#19 to rotate storage credentials.

### Known Limitation (shared with Agent #13)
UE5 Remote Control bridge only returns `{"ReturnValue": true}` — no stdout/log
streamed back to the calling session. All quest-tagging logic was made idempotent
and self-verifying (file write + immediate readback) to compensate.

## Next Agent (#15 Narrative & Dialogue Agent) Should:
1. Reference `Quest_TrackHerd_001` tag and `Quest_TrackHerd_ObjectiveMarker_001` marker
   when writing the tracker NPC's full dialogue tree and lore justification.
2. Read `Saved/quest_audit_007.txt`, `Saved/quest_tag_result_007.txt`, and
   `Saved/quest_marker_result_007.txt` before re-auditing quest/herd state.
3. Do not re-tag or reposition actors already carrying `Quest_TrackHerd_001` or `Herd_*` tags.
4. Escalate the Supabase Storage JWT auth failure (403 Invalid Compact JWS) to #01/#19 —
   blocks all future TTS/audio asset persistence until storage credentials are rotated.
5. Escalate the Remote Control stdout-visibility gap to #01/#19 if precise introspection
   becomes a blocker for narrative scripting validation.
