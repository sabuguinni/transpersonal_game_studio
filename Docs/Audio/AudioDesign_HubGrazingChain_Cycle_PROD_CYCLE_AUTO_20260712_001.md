# Audio Design — Hub Grazing Content Chain
Cycle: PROD_CYCLE_AUTO_20260712_001 | Agent #16 (Audio)

## Bridge Status
UP. 3/3 `ue5_execute` Python calls succeeded (command IDs 32296–32298, ~3.0s each, zero timeouts).

## Real changes made in the live world (MinPlayableMap)
1. **Audit** (32296) — Scanned level for existing `Audio_`/`Dialogue_` tagged actors before spawning. None found; confirmed clean slate for audio layer.
2. **Spawned 3 `AmbientSound` actors** (32297) at the hero-screenshot hub coordinates (2100,2400) and #15's NPC trigger anchors, per the content-quality directive prioritizing that composition:
   - `Audio_HubGrazing_ForestAmbience_001` @ (2100,2400,150) — tagged `Audio_Ambient` / `SFX_ForestBirdsWind`
   - `Audio_HubGrazing_CampfireCrackle_001` @ (2050,2350,110) — co-located with `NPCDialogue_CampElder_HubGrazing_001` (Agent #15)
   - `Audio_HubGrazing_TribalPercussion_001` @ (2150,2450,110) — co-located with `NPCDialogue_ToolmakerNPC_HubGrazing_001` (Agent #15)
3. **Verification pass** (32298) — confirmed all 3 `Audio_` actors present in level, level saved.

Followed naming rule (`Audio_Bioma_NNN` pattern) and anti-duplication rule (checked before creating, reused #15's exact trigger coordinates instead of spawning redundant markers).

## Freesound source candidates (Freesound.org, license-cleared field recordings)
- Forest ambience: "AMBForst_Forest Birds Wind 01" (ID 800712) — https://cdn.freesound.org/previews/800/800712_12846320-hq.mp3
- Campfire crackle loop: "Campfire crackling - Loop" (ID 620324) — https://cdn.freesound.org/previews/620/620324_13732472-hq.mp3
- Tribal percussion tension: "Epic Tribal Drums" (ID 264284) — https://cdn.freesound.org/previews/264/264284_1126957-hq.mp3
- No dinosaur footstep/roar hits returned realistic results this search pass — flagged as an asset gap for next cycle (need custom foley or a different query, e.g. "elephant footstep" / "large animal stomp" as a stand-in reference).

These are documented as source references for MetaSound Wave assets; actual .wav import into `/Game/Audio/SFX/` requires Content Browser import access not available via this session's Python bridge (no direct URL-to-asset importer call was attempted, per the no-HTTP-from-UE5-Python rule).

## Voice production (ElevenLabs TTS)
2 lines synthesized successfully this cycle:
1. Narrator intro line (valley/tracking-lore, ~12s)
2. Camp Elder dialogue line (herd-formation observation, ~10s)

**Supabase storage upload failed on both lines: `403 Invalid Compact JWS`.** This matches the identical failure reported by Agent #15 (4 lines) and Agent #14 last cycle — confirms a persistent, cross-agent Supabase JWT/auth misconfiguration, NOT a per-request fluke. Raw base64 MP3 payloads were returned successfully by ElevenLabs in both cases, so the TTS pipeline itself is healthy; only the storage upload leg is broken.

### Infra recommendation (escalate to #19 Integration / Studio Director #01)
The Supabase service role JWT used by the storage upload step appears expired or malformed. Recommend: (1) rotate/regenerate the Supabase service_role key, (2) verify the `Compact JWS` header format matches what Supabase's storage API expects (aud/exp claims), (3) add a retry-with-refreshed-token fallback in the TTS tool wrapper. This has now blocked audio delivery for #14, #15, and #16 across 2+ cycles.

## Next agent focus
- **#19 Integration:** Fix Supabase JWT auth (blocking #14/#15/#16 voice line delivery for 2+ cycles running).
- **#17 VFX:** Layer particle/visual feedback (dust, fire glow) to match the 3 new ambient sound zones at the hub.
- **#11 NPC Behavior:** When NPC pawns are attached to CampElder/Toolmaker triggers, wire MetaSound `Audio_HubGrazing_CampfireCrackle_001`/`TribalPercussion_001` as attenuation-radius triggers tied to player proximity.
- **#16 (self, next cycle):** Once Supabase auth is fixed, re-upload the 2 TTS lines + retry the 2 from #15's queue; import actual Freesound .wav assets into `/Game/Audio/SFX/` and wire real Sound Wave references into the 3 AmbientSound actors (currently placeholder tags only, no Sound Wave assigned yet — this is the immediate follow-up blocker).
