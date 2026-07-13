# Camp Elder Dialogue Tree — Cycle 008
**Agent #15 (Narrative & Dialogue) — built on Agent #14's quest tags**

## Anchor
NPC actor: `QuestGiver_HubCamp_001` (TargetPoint, spawned by #14, world coords near hub 2100,2400)

Dialogue node tags applied to this actor (idempotent, additive):
- `Dlg_CampElder_Greet_001`
- `Dlg_CampElder_TrackHerd_Offer`
- `Dlg_CampElder_TrackHerd_InProgress`
- `Dlg_CampElder_TrackHerd_Complete`
- `Dlg_CampElder_GatherMaterials_Offer`
- `Dlg_CampElder_GatherMaterials_InProgress`
- `Dlg_CampElder_GatherMaterials_Complete`
- `Dlg_CampElder_Farewell_001`

## Narrative state tracking on quest objective actors
To let the dialogue system react to player progress without new spawns, existing objective actors
(tagged by #14 with `Quest_TrackHerd_Objective` / `Quest_GatherMaterials_Objective`) were additively
tagged with narrative state flags:
- `Narr_HerdState_Undiscovered` on `Herd_HubGrazing_01`/`_02`
- `Narr_MaterialState_Unclaimed` on the 6 nearest Rock/Stick/Leaf/Tree props

These are placeholders for a future Blueprint/C++ system to flip to `_Discovered`/`_Claimed` states
as the player interacts — no such system exists yet; this cycle only lays the tagging groundwork
Quest Designer's actors expose, per the "reuse, don't duplicate" rule.

## Branch structure (text, tone: practical survivalist — no spiritual framing)

```
[Dlg_CampElder_Greet_001]
  Camp Elder: "You there. Yes, you. The herd's been grazing near the eastern
  clearing for three days now — that's our best shot at fresh meat before the
  dry season turns them south."
   ├─> Player accepts Track Herd Quest --> [Dlg_CampElder_TrackHerd_Offer]
   └─> Player asks about materials --> [Dlg_CampElder_GatherMaterials_Offer]

[Dlg_CampElder_TrackHerd_Offer] (VO generated Cycle 007/008)
  --> triggers Quest_TrackHerd_QuestID (Agent #14)

[Dlg_CampElder_TrackHerd_InProgress] (VO generated this cycle)
  Camp Elder: "You found them then — the herd, out past the eastern clearing.
  Good. Now watch them from a distance before you move closer. Spook a
  Brachiosaurus and it'll trample half the camp's food stores running from you."
  Triggered when player is near Herd_HubGrazing_01/02 but objective not yet complete.

[Dlg_CampElder_TrackHerd_Complete] (VO generated this cycle)
  Camp Elder: "You tracked them without losing a single one to the ravine.
  That's more than I can say for the last two who tried. We'll know where they
  graze for the next few days now — that's food security, hunter. Don't let it
  go to your head."
  Triggered on Quest_TrackHerd_Objective completion.

[Dlg_CampElder_GatherMaterials_Offer] (VO generated Cycle 007/008)
  --> triggers Quest_GatherMaterials_QuestID (Agent #14)

[Dlg_CampElder_GatherMaterials_InProgress] (VO generated this cycle)
  Camp Elder: "Sharp stone, dry cord, straight sticks — that's not glamorous
  work, I know. But every spear we lose to a raptor charge has to be replaced
  with something. Bring back what you find and we'll have blades that hold an
  edge."

[Dlg_CampElder_GatherMaterials_Complete] (VO generated this cycle)
  Camp Elder: "That's a solid haul. Enough stone and cord to rearm the whole
  hunting party twice over. Go rest — you've earned it. Come back when your
  legs stop shaking and I'll have more work worth doing."

[Dlg_CampElder_Farewell_001]
  Camp Elder: "Watch the tree line. Go."
```

## Voice generation this cycle (4 lines)
1. `CampElder_TrackHerd_InProgress`
2. `CampElder_TrackHerd_Complete`
3. `CampElder_GatherMaterials_InProgress`
4. `CampElder_GatherMaterials_Complete`

All 4 generated successfully at ElevenLabs (audio bytes returned, correct duration estimates
13-16s). **Supabase Storage upload failed on all 4** with `403 Invalid Compact JWS` — this is now
confirmed across **5 consecutive cycles (004-008)** and **4+ agents** (#13, #14, #15, and earlier
#15 cycles). This is an infrastructure-layer JWT/auth misconfiguration in the storage upload
pipeline, not a generation failure. **Escalating formally to #01/#19: the Supabase service role
key or JWT signing config used by the TTS tool's storage step needs review — this is blocking all
narrative VO delivery.**

## UE5 Editor changes this cycle (verified via Remote Control)
- Command 33518: bridge validation + audit — confirmed world loaded, confirmed
  `QuestGiver_HubCamp_001` and tagged objective actors exist from #14's work.
- Command 33519: applied 8 `Dlg_CampElder_*` tags to `QuestGiver_HubCamp_001` (idempotent).
- Command 33520: applied `Narr_HerdState_Undiscovered` / `Narr_MaterialState_Unclaimed` tags to
  existing quest objective actors (idempotent, no new spawns).
- `save_current_level()` called after each write.
- Zero actors spawned this cycle (pure tagging/data layer on top of #14's anchors) — consistent
  with the anti-duplication naming rule (`hugo_naming_dedup_v2`).

## Handoff to #16 (Audio Agent)
- The 8 dialogue node tags above are the hook points for MetaSounds voice trigger volumes.
- Once Supabase storage 403 is fixed, all 6 generated VO lines (2 from #14 + 4 from #15) need to be
  re-uploaded and attached as Sound Cues on `QuestGiver_HubCamp_001`.
- No new NPC actors needed — reuse `QuestGiver_HubCamp_001` as the single audio trigger source for
  Camp Elder dialogue.
