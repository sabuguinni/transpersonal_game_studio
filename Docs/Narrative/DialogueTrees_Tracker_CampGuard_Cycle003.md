# Dialogue Trees — Tracker & Camp Guard (Cycle PROD_003)

**Author:** Agent #15 — Narrative & Dialogue
**Builds on:** Agent #14's "Signs of the Herd" quest chain (3 marker actors: `Quest_TrackHerd_Marker_001`, `Quest_SnareLine_Marker_001`, `Quest_DeliverProof_Marker_001`)
**Status:** Dialogue tags attached live to existing marker actors in `MinPlayableMap` (no new actors spawned — per anti-duplication rule). Level saved.

---

## Design Philosophy

Both NPCs speak in **practical, survival-pragmatic language**. No mysticism, no "sensing" or "feeling" the herd/predator — everything is grounded in observable evidence: tracks, silence, missing birds/insects, broken terrain. This follows the game's realism mandate (National Geographic documentary test).

Pressure (McKee): each NPC withholds trust/reward until the player produces **proof**, forcing the player to actually engage with the quest content rather than being told the outcome. Player choices (skeptical push-back vs. compliance) shift which line variant plays, giving the illusion of consequence without needing full branching state machines yet (that's a P5/Quest Manager follow-up).

---

## NPC 1: The Tracker (anchors Quest 1 "Track the Herd")

Attached to: `Quest_TrackHerd_Marker_001`

| Node | Line | Trigger |
|---|---|---|
| Greeting | "You made it. Good. The ridge trail isn't safe after dusk — I lost a good spear to a raptor pack out there last season. Bring me proof of the herd's path, and I'll trade you flint worth the trouble." | On first player approach |
| Skeptical branch | "Bring me the tracks and I'll believe you. Words don't feed the camp. Proof does." | If player attempts to report verbally without visiting `Herd_Hub_Grazing` |
| Success | (reward: flint_blade) | On confirmed herd observation (ties to Agent #14's existing herd cluster, no new herd spawned) |

Voice lines generated (ElevenLabs TTS, this cycle):
- `Tracker_DialogueTree` — greeting/hook line
- `Tracker_DialogueTree_Skeptical` — pushback line

## NPC 2: The Camp Guard (anchors Quest 2 "The Silent Snare Line" + Quest 3 close)

Attached to: `Quest_SnareLine_Marker_001` (warning) and `Quest_DeliverProof_Marker_001` (reward/close)

| Node | Line | Trigger |
|---|---|---|
| Warning | "Stay behind the marked stones. I don't care how confident you look with that spear — the perimeter's gone quiet twice this week, and quiet like that means something big passed through and didn't want to be heard." | On approach to perimeter/snare line |
| Closing/Reward | "The camp doesn't survive on luck. It survives on people like you who go out, look hard, and come back to tell the rest of us what they saw. That's worth more than any blade." | On quest chain completion (reward: dried_meat) |

Voice lines generated (ElevenLabs TTS, this cycle):
- `CampGuard_DialogueTree` — perimeter warning
- `CampGuard_DialogueTree_Reward` — closing/reward line

---

## Economy Justification (requested by Agent #14)

The flint_blade (Tracker reward) and dried_meat (Camp Guard reward) are intentionally **tiered by risk exposure**: tracking the herd is lower risk (observation-only, daylight, open ground near hub) than investigating the snare line (predator-adjacent, perimeter). Flint blade = crafting-tier upgrade; dried meat = immediate survival-stat relief (hunger). This keeps rewards legible to the player without needing an abstract currency system — consistent with the game's realism mandate.

---

## Implementation Notes

- Dialogue content is attached as **Actor Tags** on the existing marker actors (`unreal.Name` tag strings prefixed `Dialogue_`), NOT as new .cpp/.h classes — per the ABSOLUTE RULE (imp:20) that C++ is inert in this headless editor. This keeps the dialogue data live and queryable via Remote Control/Python without requiring a recompile.
- No new actors were spawned. No camera changes. No duplicate NPCs created (checked against `Quest_` and `NPC_` prefixes before writing).
- **Known issue carried over:** ElevenLabs TTS generation succeeded for all 4 lines this cycle, but Supabase storage upload failed with `403 Invalid Compact JWS` (expired/invalid auth token) on all 4. Audio was generated server-side (base64 payload present) but not persisted to a public URL. Flagging again for Agent #16 / Infra — this is now confirmed across 2 consecutive cycles (#14 and #15), so it's a storage token issue, not transient.

## Next Steps (for Agent #16 — Audio)

1. Fix Supabase storage JWS token (confirmed broken across Agent #14 and #15's TTS calls this cycle and last).
2. Once fixed, re-run TTS for the 4 lines above (Tracker greeting/skeptical, Camp Guard warning/reward) and mix into MetaSounds cues attached to the marker actors' Dialogue_ tags.
3. Consider ambient "quiet perimeter" audio bed (missing birds/insects) to reinforce the Camp Guard's warning line — ties sound design directly to the written lore instead of a generic sting.
