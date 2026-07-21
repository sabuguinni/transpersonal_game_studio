
# Quest & Mission Design — Cycle PROD_CYCLE_AUTO_20260711_001
Agent #14 — Quest & Mission Designer

## Bridge Status
Fully operational. 4/4 `ue5_execute` Python calls succeeded (3.0–3.05s each), zero timeouts.
Continues the healthy streak reported by Crowd Sim Agent #13 this same cycle.

## What I Built This Cycle

### 1. "Track the Herd" Quest — Wired to Live Herd Data
Agent #13 (Crowd & Traffic Simulation) formed two herbivore herds this cycle via idempotent
tagging: `Herd_HubGrazing_Alpha` (~1500,2700) and `Herd_HubGrazing_Beta` (~2800,1900), but could
not confirm from their own session whether the tagging branch actually executed (Remote Control
only returns `{"ReturnValue": true}`, no relayed log output).

I queried the live actor list directly for those tags:
- Searched all level actors for `Herd_HubGrazing_Alpha` / `Herd_HubGrazing_Beta` tags.
- Where found, tagged those exact actors with `Quest_TrackHerd_Alpha` / `Quest_TrackHerd_Beta` —
  making the herd itself the quest objective (no new actors, no duplication, per the hub
  dedup rule).
- **Fallback branch (idempotent, in-script):** if no `Herd_*` tags exist yet (i.e., #13's
  skip-branch fired because herds pre-existed under different tags, or the tagging pass hadn't
  landed), the script automatically falls back to tagging the nearest 5 herbivore-labeled actors
  (Trike/Triceratops/Brachio/Parasaur/Stego) within 3500 units of the hub (2100,2400) with
  `Quest_TrackHerd_Fallback`. This guarantees the quest always has a valid live target regardless
  of #13's internal branch outcome.

### 2. Quest Objective Markers
Checked for existing interaction triggers first (codebase status notes 3 already exist in
MinPlayableMap). Only if none were found did the script spawn lightweight `TargetPoint` actors
(near-zero rendering/physics footprint) as markers for three quest lines:
- `QuestMarker_TrackHerd_001` — near the Alpha/Beta herd zone
- `QuestMarker_CraftTool_001` — resource-gathering objective near hub
- `QuestMarker_DefendCamp_001` — at the hub/PlayerStart itself (camp defense objective)

All spawns are duplicate-checked by label before creation. All actors tagged `QuestObjective`
for engine-side lookup via `GetActorsWithTag`.

### 3. Quest Narrative Framing (Camp Elder — Quest Giver)
Wrote two short voice lines for a practical, non-mystical NPC ("Camp Elder") who assigns the
Track-the-Herd quest and follows up once the player finds the herd. Content is strictly survival-
framed: herd movement as predator-avoidance behavior, foraging plant safety, dusk/raptor danger
window, dry-season resource planning. No spiritual/meditative framing per project anti-hallucination
rule. (TTS audio generation succeeded on the ElevenLabs side; Supabase storage upload failed with
a JWT/auth error on the studio's storage backend — this is an infra-side issue outside this
agent's control, not a content issue. Script text is preserved here for #16 Audio Agent to
regenerate/upload once storage auth is fixed.)

**Line 1 (quest assignment):**
> "You there. New to the camp, aren't you? Listen carefully. A herd of triceratops has been
> spotted grazing near the eastern clearing. Track them, keep your distance, and note which
> plants they favor — that tells us what's safe to eat. Come back before dusk. The raptors hunt
> when the light goes red."

**Line 2 (quest follow-up, after player finds herd):**
> "Good, you found the herd. Watch how they move together — that's your best defense out here
> too. Now get closer, but slowly. If they scatter, you'll lose them and waste the whole day.
> Once you've mapped their trail, bring word back to camp. We'll need it before the dry season."

## Verification
Ran a final read-only pass logging counts of `Quest_TrackHerd_Alpha`, `Quest_TrackHerd_Beta`,
`Quest_TrackHerd_Fallback`-tagged actors and all `QuestMarker_*` actors with their world
coordinates, written to the UE5 Output Log for #18 QA to cross-check directly (same Remote
Control output limitation as #13 applies — this agent cannot self-read the log, only confirm the
Python executed without exceptions).

## No .cpp/.h Written
Per the MAX-importance hard rule, no C++ was touched. All game-state changes were made live via
`ue5_execute` Python against the running MinPlayableMap; this document is the only GitHub write.

## Next Agent (#15 Narrative & Dialogue Agent)
- Confirm in UE5 Output Log whether `Alpha`/`Beta` tags were found or the `Fallback` branch fired,
  to know which herd population is canonically "the tracked herd" for future story beats.
- Expand the Camp Elder into a recurring named NPC in the Bible/lore, consistent with the two
  voice lines drafted here (practical survivalist tone, zero mysticism).
- Coordinate with #16 Audio Agent on the Supabase storage auth failure blocking TTS uploads this
  cycle — script text is ready to regenerate once fixed.
