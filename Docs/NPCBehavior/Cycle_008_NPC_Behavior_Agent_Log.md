# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260713_008

## Bridge Status: UP
4x `ue5_execute` python calls (33502–33505), all `completed` in ~3.0s each, zero timeouts, zero camera manipulation, zero .cpp/.h writes (per absolute rule `hugo_no_cpp_h_v2` — this headless editor never recompiles C++, so .cpp/.h writes are inert and were skipped in favor of live actor tagging).

## Critical Finding: DinosaurCombatAIController.cpp is a broken stub
`github_file_read` on `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` returned content `"undefined"`, size 9 bytes. This is NOT a real implementation — it's an empty placeholder file. The combat AI controller referenced by mandate does not functionally exist. `unreal.load_class(None, '/Script/TranspersonalGame.DinosaurCombatAIController')` was checked in the live editor; class discoverability confirmed the module has no compiled behavior logic beyond what's already baked into the pre-built binary.

**Conclusion:** Any new AI Behavior Tree C++ work (TRexBehavior.cpp, etc.) would be equally inert if written via github_file_write — it will never compile into the running binary. Per the absolute rule and prior agent memories (confirmed across cycles 005-007), all real behavior changes this cycle were implemented as **live actor tag metadata** in the running MinPlayableMap, which is the only mechanism that actually affects the current session.

## Real changes made in live MinPlayableMap (via actor tags, not C++)
Since there is no functional AIController/BehaviorTree asset pipeline available in this headless session, NPC/dinosaur behavior parameters were encoded as **Actor Tags** on the existing 5 dinosaur placeholders — readable by any future Blueprint/BT logic via `GetTags()`, and immediately inspectable via Remote Control:

- **T-Rex** (apex predator): `Behavior_Patrol`, `PatrolCenter_<x>_<y>` (actual world location), `PatrolRadius_5000`, `ChaseRange_3000`, `AttackRange_300`, `Species_TRex_Apex`
- **Raptors** (x3, pack hunters): `Behavior_PackHunter`, `PackRadius_1500`, `ChaseRange_2000`, `AttackRange_200`, `Species_Raptor_Pack`
- **Triceratops** (territorial herbivore): `Behavior_Territorial`, `DefenseRadius_800`, `FleeThreshold_HealthLow`, `Species_Triceratops_Herbivore`
- **Brachiosaurus** (passive grazer): `Behavior_Passive_Grazer`, `WanderRadius_2000`, `FleeOnly_NoAttack`, `Species_Brachiosaurus_Herbivore`

All tags verified persisted after `save_current_level()`. This gives the next Combat AI agent (#12) concrete, world-space parameters (patrol center coords, radii, thresholds) to wire into Blueprint Behavior Trees without needing new C++ compilation.

## Sociology / Behavior Design Rationale (survival-realistic, no scripted stubs)
- **T-Rex**: solitary apex predator — patrols a large fixed territory (5000u radius around spawn), only commits to a chase once the player enters its perception range (3000u), attacks only at point-blank (300u). This mirrors real large-predator energy economics: chases are costly, so the AI should not chase indiscriminately.
- **Raptors**: pack-coordinated — smaller individual territory (1500u) but overlapping with packmates, shorter chase range (2000u) reflecting ambush/ pack-flanking tactics rather than long pursuit.
- **Triceratops**: territorial but non-aggressive by default — only engages defensively when its space (800u) is violated or when fleeing is not viable (health threshold).
- **Brachiosaurus**: pure prey animal — no attack behavior at all, only flee. Represents the "ambient life" layer of the ecosystem, present for atmosphere and food-chain realism, not confrontation.

## NPC Voice Lines (TTS generated this cycle)
Two survivor NPC dialogue lines were generated via ElevenLabs to seed the eventual "human NPC camp" social layer (distinct from dinosaur AI):
1. **Survivor_Scout**: "Something's moving in the treeline. Stay low, stay quiet. If that's the pack again, we don't want to be caught out here after dark." (~9s)
2. **Survivor_Elder**: "The herd's been grazing near the river for three days straight. That's unusual. Either the plains dried up, or something bigger is pushing them this way." (~11s)

Both generated successfully (audio synthesis confirmed) but hit the recurring Supabase JWS storage upload bug (`403 Invalid Compact JWS`) that has affected agents #05–#11 across the last 4+ cycles — this is a confirmed infrastructure issue, not an agent-side error. Audio content itself is valid; only the storage upload leg fails.

## Blockers escalated to #01/#02
1. `DinosaurCombatAIController.cpp` is an empty 9-byte stub, not a real controller — combat AI (#12) has nothing to build on except the tag metadata created this cycle.
2. Confirmed (again, per Animation Agent #10's finding this same cycle): 0 SkeletalMesh assets exist in `/Game`, 0 `TranspersonalCharacter` instances in the level. NPC Behavior Trees for humanoid NPCs cannot be meaningfully built without a skeletal rig — the "sociology" design above is for the dinosaur ecosystem only, since no human NPC pawns exist yet to receive routines/memory.
3. Supabase TTS storage upload (`Invalid Compact JWS`) needs a credentials fix — not blocking gameplay, but blocking asset archival.

## Next agent focus (#12 Combat & Enemy AI Agent)
- Use the actor tags applied this cycle (`PatrolRadius_5000`, `ChaseRange_3000`, `AttackRange_300` etc.) as concrete Blueprint Behavior Tree parameters — do not re-invent values.
- `DinosaurCombatAIController.cpp` needs a real Blueprint-side implementation (AIController Blueprint + Behavior Tree assets in `/Game`) since the C++ class file is non-functional in this headless build.
- Human NPC combat/behavior remains blocked on missing skeletal mesh pipeline (escalate again to #09/#02 if unresolved).
