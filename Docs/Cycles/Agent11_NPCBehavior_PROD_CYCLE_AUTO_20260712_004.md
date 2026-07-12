# Cycle Summary — NPC Behavior Agent #11 (PROD_CYCLE_AUTO_20260712_004)

**Bridge status: UP.** 4x `ue5_execute` (command_type=python), all `completed`, zero timeouts (~3.0s each, IDs 32464→32467). 2x `text_to_speech` synthesized successfully (audio generated; Supabase upload hit the known cross-agent infra bug `403 Invalid Compact JWS` — not agent-fixable, base64 payload returned inline instead).

## Real changes made to the live MinPlayableMap

1. **Dinosaur audit (32464)** — Enumerated all level actors, found and logged every TRex/Raptor/Triceratops/Brachiosaurus actor with exact world coordinates, confirming ground truth before tagging.
2. **TRex identification + tagging (32465)** — Located the TRex actor and PlayerStart, applied behavior-radius tags directly to the actor: `TRex_PatrolRadius_5000`, `TRex_ChaseRadius_3000`, `TRex_AttackRadius_300`. These tags encode the exact directive requirement (patrol 5000u, chase within 3000u, attack within 300u) as queryable actor metadata usable by any future Behavior Tree / Blueprint AI controller.
3. **Distance-based state resolution + pack tagging (32466)** — Computed real 2D distance between TRex and PlayerStart, resolved current behavior state (PATROL/CHASE/ATTACK) from that live distance, and stamped the result as an additional tag (`BehaviorState_X`). Tagged all Raptor actors as coordinated pack hunters: `PackHunter`, `PackID_Alpha`, `PatrolRadius_2000` — establishing pack-hunting sociology (raptors act as a coordinated group, not solitary predators) distinct from the solitary-apex-predator TRex. Saved the level.
4. **Verification pass (32467)** — Re-queried all TRex/Raptor actors and printed their final tag arrays to confirm persistence of the behavior metadata after save.

## NPC sociology decisions (this agent's domain)
- **TRex = solitary apex predator**: territorial, patrols alone, escalates to chase/attack only when the player enters its threat radii. It does not need allies — the world doesn't revolve around the player, it just happens to overlap this predator's hunting ground.
- **Raptors = pack hunters**: tagged with a shared `PackID_Alpha` so a future Behavior Tree can coordinate converge/flank behavior instead of independent AI per-raptor. This reflects real velociraptor pack-hunting ecology, not scripted "boss fight" logic.
- Two TTS voice assets generated (TRex territorial growl, Raptor pack-call shriek) to seed future ambient/combat audio triggers tied to these same behavior states.

## Known infra issue (not blocking)
Supabase storage upload for `text_to_speech` returns `403 Invalid Compact JWS` on both calls this cycle — same recurring cross-agent bug reported in previous cycles. Audio was generated successfully server-side; only the persistent storage URL failed.

## Files created/modified
- `Docs/Cycles/Agent11_NPCBehavior_PROD_CYCLE_AUTO_20260712_004.md`

No .cpp/.h files written this cycle (absolute rule — headless editor never recompiles C++, would be 100% wasted effort). No camera manipulation. No lighting/hub composition changes (out of scope for this agent this cycle).

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Actor audit of all dinosaur actors near hub — confirmed ground truth (32464)
- [UE5_CMD] Tagged TRex actor with PatrolRadius_5000/ChaseRadius_3000/AttackRadius_300 behavior metadata (32465)
- [UE5_CMD] Resolved live distance-based behavior state (PATROL/CHASE/ATTACK) + tagged Raptor pack with PackID_Alpha/PatrolRadius_2000, saved level (32466)
- [UE5_CMD] Verified all behavior tags persisted correctly (32467)
- [AUDIO] TRex territorial growl + Raptor pack-hunt call TTS lines generated (Supabase upload blocked by known infra bug, not agent-fixable)
- [NEXT] Agent #12 (Combat & Enemy AI) should read these actor tags (`TRex_ChaseRadius_3000`, `PackHunter`, `PackID_Alpha`) to drive actual Behavior Tree / AIController combat logic — the sociological rules (solitary apex predator vs. coordinated pack) are now encoded as queryable metadata on the live actors, ready for a real BT asset to consume.
