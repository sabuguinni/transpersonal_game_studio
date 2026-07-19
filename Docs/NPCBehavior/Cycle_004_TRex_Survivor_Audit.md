# NPC Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260719_004

## Bridge Status
UP. 3x `ue5_execute` executed this cycle (2x `python`, 1x `run_console_command`), all returned `success:true`, `status:completed`, execution times ~3.0s each — no timeouts.

## Known Bridge Limitation (confirmed again this cycle)
The Remote Control endpoint used by `ue5_execute` (`command_type='python'`) only ever returns `{"ReturnValue": true/false}` in the `result` field — it does not surface `print()`, `unreal.log()`, exceptions, or file contents back to the calling agent. This was already reported by the Animation Agent (#10) in the previous cycle and is confirmed again here: I wrote audit results to `D:/npc_audit_004.txt` and `D:/npc_audit_004_final.txt` on Hugo's PC and called `unreal.log(...)`, but none of that text is retrievable through this tool's `result` field. Every call completed without a Python exception (`success:true`), which is the only signal available to this agent channel.

**Recommendation (repeated from #10):** Engine Architect (#02) or Integration Agent (#19) should investigate whether the Remote Control HTTP response can be extended to include captured stdout/stderr, or whether a dedicated "read file back" command type could be added to the bridge so agents can verify string/numeric results, not just boolean success.

## Actions Taken This Cycle
1. **Bridge validation + world audit**: queried `EditorLevelLibrary.get_all_level_actors()`, filtered for labels containing `TRex` and `Human`/`Survivor`, collected their live (x, y, z) positions. No exception raised — confirms actors are enumerable and the world is loaded.
2. **Grounding pass (GROUNDED rule, no hardcoded z)**: for every `TRex*` actor whose XY falls inside the playable core (x: -3000..5000, y: -1000..5500), performed a real `line_trace_single` against the Landscape channel (`TraceTypeQuery1`) from z+2000 to z-2000. Where the trace hit and the vertical delta between the actor's current z and the hit z exceeded 30 units, repositioned the actor to `hit.z + 5` via `set_actor_location`. This is a corrective pass on top of whatever the World Generator/Environment Artist already placed — it does not touch Landscape, foliage, or Terrain_Savana, only the TRex actor transforms.
3. **NPC routine tagging**: located `Survivor_Savana_001` (spawned/grounded by Animation Agent #10 in this same cycle) and applied an actor tag `Routine_Forage_Hub`, marking the start of a sociological routine layer (this NPC's default daily behavior state: foraging near the hub at 2100,2400) without touching its transform, mesh, or animation mode (already set by #10 to `ANIM_SINGLE_NODE`).
4. **Single save**: `save all` console command executed once, at the end of all changes, per the "one save at the end of your turn" rule.

## HANDS OFF Compliance
- PLAYER0: not read, not written, not moved.
- Landscape / foliage / Terrain_Savana sublevel: not touched (only used as trace target).
- Editor camera: not touched.
- vision_loop.py / sun: not touched.
- No Character/Pawn mobility changed.

## No .cpp/.h Files
Per absolute rule `hugo_no_cpp_h_v2`, no C++ source was written this cycle. The directive's request to create `TRexBehavior.cpp` is intentionally NOT fulfilled as a C++ file (this has been the consistent decision across cycles 001-003 of this agent) — C++ is inert in this headless, non-recompiling editor. If T-Rex patrol/chase/attack logic (5000u patrol radius, 3000u chase range, 300u attack range) needs to be authored, it should be expressed as a Blueprint Behavior Tree / Blackboard asset configured live via `ue5_execute` Python (e.g. `unreal.AIModule`, `BehaviorTree`, `BlackboardData` assets edited through the Python Editor API), not as a source file. This is proposed as next-cycle work if the Combat & Enemy AI Agent (#12) does not already own this via `DinosaurCombatAIController`.

## Verification Note
Because the bridge cannot return text/numeric audit data, the exact count of TRex/Survivor actors and the exact number of grounding fixes applied this cycle could not be captured in this report — only that all 3 `ue5_execute` calls completed successfully with no exceptions, which for a Python script that iterates all level actors and calls `set_actor_location` implies the operations executed without crashing the editor.

## Handoff to #12 (Combat & Enemy AI Agent)
- `Survivor_Savana_001` now carries tag `Routine_Forage_Hub` — a first sociological data point (this NPC's baseline routine state) that future Behavior Tree / Blackboard work can key off.
- Any `TRex*` actor inside the playable core that was more than 30 units off the traced terrain height has been repositioned to sit on the Landscape surface (`hit.z + 5`) this cycle — Combat AI's chase/attack logic should now have a more reliable ground reference for these actors.
- Repeated ask to #02/#19: fix the bridge's inability to return non-boolean results — this blocks precise verification for every agent using `ue5_execute` with `python`, not just this one.
