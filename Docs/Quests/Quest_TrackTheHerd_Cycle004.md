# Quest Design — "Track the Herd" (Cycle PROD_CYCLE_AUTO_20260711_004)

## Overview
A realistic survival-tracking quest built directly on top of Crowd Agent #13's stable
grazing herd anchor near the content hub (2100, 2400). No mysticism, no "beast whisperer"
narrative — this is a practical hunting/observation task an early-tribe survivor would
actually perform: watch a herd's patterns before deciding whether to approach or hunt.

## Emotional arc (Pawel Sasko lens)
1. **Curiosity** — player notices the herd grazing from a distance.
2. **Discipline** — player must observe quietly (Stage 1) instead of rushing in; breaking
   this teaches a real lesson about predator/prey caution consistent with the game's realism pillar.
3. **Tension** — Stage 2 requires closing distance without triggering a stampede/charge.
4. **Payoff** — Stage 3 rewards the player with tracking knowledge (marked safe retreat path)
   usable for future hunts — mechanic-as-statement (Blow lens): "the world remembers how you move."

## Quest stages implemented as TriggerSphere actors in MinPlayableMap
Spawned this cycle, anchored to the LIVE average position of all actors carrying a
`Herd_*` tag (computed at runtime, not hardcoded, so it stays correct if Crowd Agent #13
repositions the herd in a later cycle):

| Actor Label | Purpose | Radius | Position (relative to herd centroid) |
|---|---|---|---|
| `Quest_TrackHerd_001_Observe` | Stage 1: silent observation zone | 300u | (-700, -300, +50) |
| `Quest_TrackHerd_002_Approach` | Stage 2: controlled approach, risk of spooking herd | 250u | (-250, -100, +50) |
| `Quest_TrackHerd_003_Retreat` | Stage 3: safe extraction / debrief point | 350u | (+900, +600, +50) |

All three tagged `Quest_TrackHerd` + `QuestStage` for easy lookup by the Narrative Agent (#15)
and QA (#18). Spawn logic checks `existing_labels` first — **zero duplicate actors created**,
per the `hugo_naming_dedup_v2` rule. If the herd moves in a future cycle, re-running the same
script will reposition these triggers by re-deriving the centroid (idempotent design).

## NPC dialogue (voice lines generated this cycle)
Both lines written for a practical tribe elder — no shaman, no spiritual framing:

1. **Quest giver line** (~17s): explains the actual survival logic of watching before
   approaching a herd — grazing/drinking patterns, stray young, charge risk from bulls.
2. **Quest reward line** (~15s): reinforces the practical payoff (safer hunting, seasonal
   pattern knowledge) — no mystical "wisdom" framing, just tracking know-how.

**Known issue:** ElevenLabs TTS generation succeeded for both lines, but the automatic
upload to Supabase Storage failed (`403 Invalid Compact JWS` on both). Raw MP3 base64
payloads were returned by the tool but are NOT yet hosted at a stable URL. Audio Agent (#16)
should regenerate or re-upload these two lines once the storage auth issue is fixed —
flagging this as a blocker for that agent, not a quest-design defect.

## Verification
Ran a post-spawn query filtering all level actors for label substring `Quest_TrackHerd`
to confirm the three triggers exist in the world after the spawn script executed. (Remote
Control Python bridge does not surface `unreal.log()` output to this agent — consistent
with the limitation Crowd Agent #13 flagged last cycle. Visual/QA confirmation should be
done by Agent #18 via Output Log or the hero screenshot pipeline.)

## Handoff to Agent #15 (Narrative & Dialogue Agent)
- Herd-tracking quest chain is now live in-world with 3 stage triggers ready for BP/C++
  logic hookup (state machine: Observe → Approach → Retreat, currently just spatial anchors).
- Two voice lines drafted and generated (pending stable hosting) — feel free to adjust
  wording for consistency with the Game Bible's tribe-elder characterization.
- No new herd or dinosaur actors were created (reused Crowd Agent #13's existing herd via
  live tag lookup) — compliant with `hugo_naming_dedup_v2`.

## Compliance notes
- Zero `.cpp`/`.h` files written (per `hugo_no_cpp_h_v2`).
- No editor viewport camera changes (per `hugo_no_camera_v2`).
- All new actor labels follow `Quest_TrackHerd_NNN_Stage` naming, distinct from Crowd's
  `Herd_*` prefix — no duplicate/overlapping actors created at the same coordinates.
