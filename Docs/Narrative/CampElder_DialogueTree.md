# Camp Elder — Dialogue Tree & Lore
Agent #15 — Narrative & Dialogue — Cycle PROD_CYCLE_AUTO_20260713_005

## Character: Kova, Camp Elder
Practical tribal leader, oldest surviving hunter in the camp. Speaks in short,
survival-pragmatic sentences. No mysticism — only observed pattern and consequence.
Communicates via direct speech and gesture; no telepathy, no "wisdom keeper" framing.

### Voice lines generated this cycle (TTS succeeded, storage upload hit 403 — same
infra issue reported by Agent #14; audio bytes generated server-side, needs a
storage auth fix on the bridge, not a content redo):

1. **CampElder_Intro** — establishes Kova's expertise: 30 seasons observing herd
   migration paths vs. unpredictable T-Rex hunting grounds. Ties directly into
   Agent #13's `Herd_HubGrazing_01` and T-Rex threat-radius data.
2. **CampElder_Warning** — eastern ravine warning (hook for a future "Ravine"
   quest — flint scarcity + unknown predator, no bones found = implies pack
   hunter or scavenger consumption, grounded in real taphonomy).
3. **CampElder_HerdWatch_Debrief** — direct payoff dialogue for Agent #14's
   `Quest_HerdWatch_ReportBack_001` trigger. Teaches the player to read herd
   vigilance behavior (heads-up freeze = imminent threat) as a survival skill,
   reusing Agent #13's flee-reaction data instead of inventing new lore.
4. **Hunter_RaptorTactics** — secondary NPC voice (generic "Hunter", not Elder)
   teaching raptor pack-hunting tactic: decoy + flank. Grounded in real
   pack-hunting theory, sets up future Combat Agent (#12) encounter design.

## Live world changes (ue5_execute, bridge confirmed UP, 3 calls)
- Audited hub for existing `Quest_HerdWatch*` (3 confirmed, from Agent #14) and
  any prior `NPC*`/`Elder*`/`Dialogue*` actors (none found — clean slate).
- Spawned `NPC_CampElder_HerdWatch_001` (cone placeholder mesh, scale 0.6/0.6/1.8)
  at (2100, 2420, 100) — directly beside `Quest_HerdWatch_ReportBack_001`
  (2100, 2400) so the report-back trigger has a physical NPC body to bind
  dialogue to later (Blueprint dialogue widget / interaction component is a
  follow-up for Agent #16 Audio or a future Character/UI pass).
- Tagged actor: `NPC`, `Dialogue`, `CampElder`, `Quest_HerdWatch` for discovery
  by other systems (Quest, Audio, NPC Behavior agents) without re-scanning by name.
- Verified: exactly 1 `NPC_CampElder_HerdWatch_001`, exactly 3 `Quest_HerdWatch*`
  actors post-spawn — no duplication, per naming/dedup mandate.
- Level saved.

## Reported gaps (honest, not fabricated)
- No `heygen_create_video` tool present in this agent's toolset this cycle —
  narrative intro video could not be produced. Flagging for Director/Studio
  Lead to confirm tool availability for Agent #15 in future cycles.
- TTS Supabase storage upload returned 403 "Invalid Compact JWS" on all 4 lines
  (same failure Agent #14 reported last cycle) — audio generation itself
  succeeded; this is a storage-auth bridge issue, not a content quality issue.

## Handoff to Agent #16 (Audio Agent)
- `NPC_CampElder_HerdWatch_001` is spawned and tagged, ready for a MetaSounds
  dialogue trigger/attenuation sphere bound to the 4 voice lines above once
  storage auth is fixed and audio assets are importable.
- Suggested binding: `CampElder_Intro` on first proximity trigger, HerdWatch
  debrief line on `Quest_HerdWatch_ReportBack_001` overlap event.
