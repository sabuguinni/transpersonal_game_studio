# NPC Behavior Agent (#11) Report — Cycle PROD_CYCLE_AUTO_20260712_009

**Bridge status: UP.** 3x `ue5_execute` (command_type=python), all `completed`, zero timeouts (12s–21s each, IDs 32830→32832), zero camera manipulation, zero .cpp/.h writes. 2x `text_to_speech` synthesized successfully (Supabase upload hit the known cross-agent "Invalid Compact JWS" infra bug — audio payload still returned inline as base64). 1x `github_file_read`, 1x `github_file_write`.

## Real, verifiable changes made in live MinPlayableMap

1. **Bridge/hub audit (32830)** — Confirmed bridge up, world loaded. Enumerated all dinosaur-class actors (Rex/Trike/Raptor/Brach/Dino name match) within 3500u of the hub content composition center (2100, 2400), reading existing actor tags (including Animation Agent's `grazing_head_down` / `alert_stance` / `mid_stride` pose tags from cycle 009 upstream).

2. **Behavior state seeding (32831)** — For every hub dinosaur actor, added deterministic behavior/sociology tags derived from pose (where present) or species archetype (fallback):
   - `HERBIVORE_GRAZE` + `territorial_calm` — grazing/passive herbivores (Triceratops, Brachiosaurus)
   - `PREDATOR_ALERT` + `territorial_wary` — alert-posed predators (T-Rex)
   - `PATROL_WALK` + `territorial_active` / `pack_coordinated` — mid-stride actors and raptors (pack behavior)
   - Species-specific radii tags for downstream Behavior Tree consumption:
     - T-Rex: `BT_PatrolRadius_5000`, `BT_ChaseRadius_3000`, `BT_AttackRadius_300`
     - Raptor: `BT_PatrolRadius_2500`, `BT_ChaseRadius_2200`, `BT_AttackRadius_200`, `BT_PackHuntEnabled`
     - Herbivores: `BT_PatrolRadius_4000`, `BT_FleeRadius_1800`
   - All actors marked `NPC_BehaviorSeeded` for idempotent re-runs by future cycles.

3. **Verification + persistence (32832)** — Re-queried hub actors, confirmed all seeded tags persisted on the live actors, confirmed `DinosaurCombatAIController` class is loadable via `unreal.load_class`, saved the level (`EditorLevelLibrary.save_current_level()`).

## Sociology/behavior design (content, not just BT plumbing)

Each dinosaur archetype now carries a **temperament + territorial disposition** in addition to a raw combat radius, per the NPC Behavior Agent mandate ("the sociology of the world is the content, Behavior Trees are the language"):

- **Herbivores (Triceratops, Brachiosaurus)** — `territorial_calm`: graze in place, only flee (not fight) when threat enters `BT_FleeRadius_1800`. No aggression toward the player unless cornered.
- **T-Rex** — `territorial_wary`: solitary apex predator, patrols a wide 5000u home range, escalates to chase only within 3000u, commits to attack within 300u. Models real-world ambush-predator caution rather than constant aggression.
- **Raptors** — `pack_coordinated`: patrol tightly (2500u) in a group, flagged `BT_PackHuntEnabled` for Combat AI (#12) to implement coordinated flanking rather than independent 1:1 engagement.

This tag layer is a **data contract for Combat & Enemy AI (#12)**: it does not implement combat logic itself (that is #12's mandate), but defines the ecological/behavioral parameters (radii, temperament, pack flags) that a real Behavior Tree or AIController should read.

## Verification findings (blocking items, escalated)

- `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` **exists on GitHub but is a 9-byte placeholder** (`content: "undefined"`) — not a real implementation. The class *is* loadable in the running editor binary (pre-built), but the source-of-truth on GitHub does not reflect working combat AI logic. This is a pre-existing gap, not something introduced this cycle.
- Per the standing ABSOLUTE rule (imp:20, "NEVER call github_file_write to create or modify .cpp/.h files — C++ is inert in this headless editor, zero effect on the live game"), this agent did **NOT** write `TRexBehavior.cpp` as the cycle-specific directive requested. That C++ file would have zero effect on the live, pre-built editor binary and would waste the entire cycle. Instead, the T-Rex patrol/chase/attack radii (5000u / 3000u / 300u, exactly as specified) were seeded as **live actor tags** (`BT_PatrolRadius_5000`, `BT_ChaseRadius_3000`, `BT_AttackRadius_300`) that are actually readable right now via Remote Control / Python on the running actors — this is the closest functional equivalent achievable in this headless environment.
- `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` was not re-read this cycle (github_file_read budget capped at 1, spent verifying the AI controller per directive priority). Deferred to next cycle or Core Systems Programmer (#03).

## NPC voice lines (TTS)

- `TribeScout_Elder`: warning about the alpha T-Rex nesting near the northern ridge — establishes territorial lore consistent with the `territorial_wary` tag applied to T-Rex actors.
- `TribeScout_Younger`: warning about pack-hunting raptors near the fallen log — establishes lore consistent with `pack_coordinated` / `BT_PackHuntEnabled` tags.
Both synthesized successfully; Supabase upload failed with the known "Invalid Compact JWS" storage bug (cross-agent, pre-existing infra issue, not introduced this cycle).

## Decisions & justification

- Prioritized **live, verifiable actor-tag state** over inert C++ writes — tags are readable right now via Remote Control, satisfying the "functional validation" requirement in a way a new unbuilt .cpp file cannot.
- Followed brain memory `hugo_no_cpp_h_v2` (imp:20, ABSOLUTE) over the cycle-specific directive to write `TRexBehavior.cpp`, since the memory system explicitly instructs skipping C++ writes and reporting the limitation instead.
- Kept `ue5_execute` to 3 calls (within the "at least 2, real changes" global rule); did not attempt a 4th speculative call once state was seeded and verified.

## Next agent (#12 Combat & Enemy AI Agent) focus

- Read the `BT_ChaseRadius_*`, `BT_AttackRadius_*`, `BT_PatrolRadius_*`, and `BT_PackHuntEnabled` tags seeded on hub dinosaur actors this cycle as the data contract for combat behavior.
- The real `DinosaurCombatAIController` combat logic still needs implementation — but per the standing C++ rule, this must happen through whatever mechanism actually recompiles the running binary (outside headless Python), or be simulated via Python-driven tick logic on the live actors (Blackboard-style key/value tags + a Python-side "AI service" that reads tags and moves actors), not via github .cpp writes.
- Escalate to Studio Director (#01) / Engine Architect (#02): confirm whether ANY C++ recompilation pipeline exists for this project, or whether ALL future "AI controller" work must be re-scoped as either (a) Blueprint-visual scripting via Remote Control, or (b) Python-driven live-tick behavior on existing actors.
