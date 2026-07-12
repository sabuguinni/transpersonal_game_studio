# Combat & Enemy AI Agent #12 — Cycle PROD_CYCLE_AUTO_20260712_006

**Bridge status: UP.** 4x `ue5_execute` (command_type=python), IDs 32607→32610, all `completed`, 3s–9s each, zero timeouts, zero camera manipulation, zero .cpp/.h writes (per absolute rule).

## Anti-duplication audit (per brain memory `hugo_combat_label_consistency_v1`)
Before creating any `CombatZone_*` / `BehaviorTag_*` actor, queried all actors within 3500u of the hub (2100, 2400) for existing labels of these prefixes, grouped by creature type (TRex / Raptor / Trike-Triceratops / Brachio).

- **Call 1** — bridge health validation (`bridge_ok True`).
- **Call 2** — full audit of existing `CombatZone_*` and `BehaviorTag_*` labels near hub + inventory of dinosaur actors present (TRex, Raptors, Triceratops, Brachiosaurus).
- **Call 3** — first attempt at idempotent tag creation returned `ReturnValue: false` (likely a runtime exception inside the script, e.g. missing component on a text render actor) — logged, not blindly retried with a duplicate-risk script.
- **Call 4** — rewritten with full try/except + per-actor null-checks, executed successfully (`ReturnValue: true`), and saved the level.

## What was created (idempotent — one pair per creature TYPE, not per instance)
Per brain memory `hugo_naming_dedup_v2`, this pass tags per **creature type** (TRex / Raptor / Trike / Brachio), not per individual dinosaur instance, to avoid the flood pattern (191 duplicate labels observed in earlier cycles). If a `CombatZone_<Type>` or `BehaviorTag_<Type>` already existed from a prior cycle, it was reused/skipped — only missing types got a new pair.

Combat disposition data authored per type:
- **TRex** — `Territorial-Ambush`: ground-pound AOE 300u, roar-stagger 500u, chase radius 3000u.
- **Raptor** — `PackHunter-Flank`: 3-unit coordination, bleed-DoT bite, flees if isolated from pack.
- **Triceratops/Trike** — `DefensiveCharge`: retaliate-only (never initiates), gore-charge 400u, high poise/armor.
- **Brachiosaurus** — `PassiveFlee`: no combat AI, tail-swipe only if cornered <150u, lowest target priority for AI aggro tables.

Each type also received a `BehaviorTag_<Type>` actor encoding the combat state machine: `Idle -> Alert -> Engage -> Retreat (HP < 20%)`.

This builds directly on Agent #11's `NPC_Behavior_*` / `NPC_AlertState_*` sociological tags — combat AI treats `Alert`/`Hostile` NPC states as the trigger to transition into the `Engage` combat state, and `NPC_HomeLoc_*` anchors are the retreat/return points once a fight ends or the creature's HP drops below threshold.

## Audio & concept art (generated server-side, storage blocked)
- 2x `text_to_speech` — TRex territorial roar vocalization cue, Raptor pack hunting-call cue. Both synthesized successfully server-side (13–14s audio) but hit the known cross-agent Supabase `403 Invalid Compact JWS` storage bug — same infra failure documented by Agent #12 in cycles 002/004 and by other agents. Not agent-fixable; scripts/text are preserved below for regeneration once storage is fixed.
- 1x `generate_image` — Velociraptor pack ambush concept art (HD, 1792x1024, documentary paleoart style). Generated successfully by the model but hit the identical storage 403 error on upload.

### Preserved prompts for re-generation once Supabase storage auth is fixed
- **TRex_CombatVocalization**: "RAAAAGH! *territorial roar* The ground trembles as the Tyrannosaur charges — thirty tons of muscle and teeth closing the distance in seconds. Its jaws promise no negotiation, only impact."
- **Raptor_PackCallout**: "*sharp hunting shriek, three-tone call* The raptor pack signals the flank — one distracts from the front while two circle wide through the undergrowth. Silence after the call means they are already moving."
- **Enemy concept art prompt**: "Realistic National-Geographic-documentary style concept art of a tactical combat encounter in a dense Cretaceous forest clearing: a lean, muscular Velociraptor pack of three animals mid-ambush, flanking from tall ferns, feathers visible on arms and tail, low afternoon sunlight filtering through canopy, dynamic action poses (one lunging, two circling), realistic scaly-feathered skin texture, dirt and scars from territorial fights, photorealistic paleoart style, no fantasy or magical elements, grounded survival-thriller mood, wide cinematic composition."

## Decisions
- Data-layer combat tagging (TextRenderActor labels + text payload) chosen over live Blueprint Behavior Tree assets — Remote Control cannot reliably author BT/BB assets; this gives a directly inspectable, idempotent disposition layer that a future C++ `UCombat_AIController`/Behavior Tree implementation can read at BeginPlay.
- Tagged per creature TYPE not per instance, per `hugo_naming_dedup_v2` — prevents the historical flood of ~1680+ duplicate label pairs.
- Zero .cpp/.h files written — this headless editor does not recompile new C++ per `hugo_no_cpp_h_v2`; all combat logic authored as data (text tags) consumable once real Combat AI C++ classes exist and are compiled into a future editor build.

## Files Created/Modified
- `Docs/CombatAI/Cycle_PROD_AUTO_20260712_006_CombatAIPass.md` (this file)

## Next steps for #13 Crowd & Traffic Simulation
- Consume `CombatZone_*` / `BehaviorTag_*` type-level dispositions to avoid triggering mass-panic/crowd-flee logic redundantly per-instance — read once per creature type.
- When real Combat AI C++ (Behavior Tree + AIController) is eventually compiled, wire `NPC_AlertState_*` (from #11) → `BehaviorTag_<Type>` state machine → actual attack/flee montages once skeletal meshes exist (blocker inherited: zero skeletal meshes in `/Game`, dinosaurs are primitive-shape placeholders).
- Regenerate the 2 audio cues + 1 concept art once the Supabase storage `403 Invalid Compact JWS` bug is fixed studio-wide (not fixable by this agent).
