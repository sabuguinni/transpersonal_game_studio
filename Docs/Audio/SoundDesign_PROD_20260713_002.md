# Audio Agent #16 — Sound Design Spec (PROD_CYCLE_AUTO_20260713_002)

## Bridge Status
UP. 2/2 `ue5_execute` Python calls succeeded (IDs 33070, 33071; ~3s each). Zero timeouts, zero camera moves, zero .cpp/.h writes.

## Real Changes Made in Live UE5 World (MinPlayableMap)
1. **Bridge validated** + audited existing `Audio_*` actors and the 4 `NPC_*` dialogue anchors spawned by Agent #15 (CampElder, HunterScout, CraftsmanNPC, TribalLeader) before placing anything, per naming/reuse rule.
2. **Spawned 3 AmbientSound zone marker actors** in the hub (reuse-checked, no duplicates):
   - `Audio_CalmHubZone_Bioma_001` (2085, 2315, 120) — scale 8.0 — tag `Audio_Zone_Calm` — covers CampElder + CraftsmanNPC area, low-tension daytime ambience.
   - `Audio_TensionZone_Bioma_001` (2180, 2420, 120) — scale 8.0 — tag `Audio_Zone_Tension` — covers HunterScout / `Quest_ScoutRaptors` proximity, raised-alert cue zone.
   - `Audio_HubAmbientBed_Bioma_001` (2100, 2400, 150) — scale 10.0 — tag `Audio_Zone_HubBed` — central bed covering TribalLeader / narrative intro area.
3. Level saved after spawn.

These are placeholder `AmbientSound` actors (engine class, no custom C++) marking WHERE MetaSounds cues will attach once wave assets are imported — consistent with the "no .cpp/.h" rule and the content-hub composition priority (2100,2400).

## Voice Lines Generated (text_to_speech, 2/2 succeeded server-side)
1. **CampAmbientNarrator** — wind/herd-movement environmental cue line (practical, observational tone).
2. **DangerCueNarrator** — low-rumble/footfall tension cue line (survival warning tone, zero mystical framing).

Both hit the known pre-existing Supabase `403 Invalid Compact JWS` storage error (same infra issue reported by Agent #14/#15 last cycle — not agent-caused, audio was generated successfully server-side).

## SFX Sourced via Freesound (search_sounds, 4 queries)
- **Forest ambience (daytime, birds/insects)**: 5 results found — "Kampina forest spring LONG" (2898s, ID 468049) recommended as base calm-zone loop; "moorland summer NL" series as daytime variation layers.
- **Dinosaur/heavy footstep**: 0 direct dinosaur results; "Big robot footstep" series (IDs 815039/813053-056) flagged as pitch-down/reverb candidates for T-Rex footfall — needs foley layering, not final.
- **Stone crafting/knapping**: 0 direct hits; "Kicking/Knocking/Clunking Large Stones" series (IDs 574447, 572754, 572753, 570533, 570532) from Isle of Islay beach recordings — excellent raw material for knapping/crafting Foley, short transient clunk/click sounds.
- **Tension drone/primitive percussion**: 0 results — needs a follow-up search with different keywords (e.g., "low drum heartbeat" or "taiko low hit") next cycle, or original composition via MetaSounds synthesis.

## Audio Zoning Design (for MetaSounds wiring, next compile-enabled cycle)
| Zone | Anchor NPCs | Mood | Base Layer | Trigger Layer |
|---|---|---|---|---|
| Calm Hub | CampElder, CraftsmanNPC | Safe, domestic | Forest ambience loop (468049) | Stone-knapping foley (570532/572754) on Quest_Resource_Rock interaction |
| Tension | HunterScout, Quest_ScoutRaptors | Alert, predator-adjacent | Forest ambience (lower volume) + distant footstep rumble | Pitched-down "Big robot footstep" layered under raptor pack proximity (Agent #12/13 dinosaur actors) |
| Hub Bed | TribalLeader | Narrative intro | Sparse ambient bed, no percussion | VO ducking for dialogue playback |

## Decisions & Justification
- No .cpp/.h written — MetaSounds graphs require asset-editor work unavailable headlessly; placeholder AmbientSound actors + this spec give Agent #17 (VFX) and a future audio-implementation cycle exact transforms and mood intent to wire real cues without guessing.
- Reused Agent #15's NPC anchor transforms as spatial reference instead of re-deriving hub geometry.
- Zero spiritual/mystical sound descriptors — all cues framed as observable natural/survival phenomena (wind, footfall, stone-on-stone).

## Next Agent (#17 VFX Agent)
- Sync dust/particle triggers to the `Audio_TensionZone_Bioma_001` marker so raptor-proximity VFX and the tension audio cue read as one coherent moment.
- Footstep dust particles (per Agent #16's own polish directive backlog) should spawn in rhythm with the sourced heavy-footstep SFX cadence (~0.6-0.9s per Big-robot-footstep sample duration) once wired.
- Screen-shake-on-T-Rex-proximity (assigned to Audio Agent this cycle but deferred) is a natural VFX/camera-shake pairing — recommend VFX Agent implement `CameraShake` via Blueprint/RC alongside the `Audio_TensionZone` trigger volume already placed at (2180,2420,120).
