# VFX Design — Cycle 008 (Agent #17)

## Bridge Status: UP
4x `ue5_execute` Python calls (IDs 33524–33527), all `completed` in ~3s each, zero timeouts, zero camera moves, zero .cpp/.h writes (hard rules respected).

## Real changes made in the live UE5 world

1. **Audit** — confirmed no pre-existing duplicate `VFX_*` actors before creating anything (per `hugo_naming_dedup_v2`). Located `TRex` placeholder, scene `DirectionalLight`, and `QuestGiver_HubCamp_001` anchor (from Agents #14/#15/#16).

2. **T-Rex game-feel VFX tags** (additive, no duplicate actor spawned) — tagged the existing TRex placeholder with:
   - `VFX_ScreenShake_Radius_800_SyncAudioTRexProximity` — pairs directly with Audio Agent #16's `Audio_TRex_ProximityRadius_800_ScreenShake` tag at the same 800uu radius.
   - `VFX_FootstepDust_SyncTiming_AudioTRexFootstepHeavyRumble` — dust burst timing keyed to footstep rumble peak.
   - `VFX_RoarHeatDistortion_Radial_ScaleWithRoarVolume` — heat-shimmer/air-wave distortion scaling with roar audio amplitude.
   - `VFX_BreathVapor_ColdBiome_Only` — visible breath vapor conditional on biome temperature (not always-on).

3. **Day/night ambience VFX** — tagged the scene's `DirectionalLight` with:
   - `VFX_DayNight_GodRays_SyncAmbienceSwitchBus` — god rays keyed to Audio's `Audio_DayNight_AmbienceSwitch_Bus`.
   - `VFX_DayNight_FogDensity_CrossfadeWithSkyColor` — fog density crossfades with sky color rotation.
   - `VFX_DayNight_VolumetricScattering_Dawn_Dusk` — volumetric light scattering intensifies at dawn/dusk.

4. **Campfire VFX** — tagged the existing `QuestGiver_HubCamp_001` anchor (no new duplicate actor, per dedup rule) with:
   - `NS_Fire_Campfire_SyncDialogueBus` — fire VFX synced to Camp Elder dialogue triggers.
   - `NS_Fire_EmberRise_LOD3` — 3-tier LOD ember particle system.
   - `NS_Smoke_CampfireColumn_WindReactive` — smoke column reactive to wind direction.

5. **Spawned one new visible actor** — `VFX_Dust_TRexFootstep_001`, a `NiagaraActor` placeholder positioned 150uu offset from the TRex, tagged for footstep-dust sync. Checked for pre-existing duplicate before spawning (none found).

6. `save_current_level()` called after each modification.

## Production tools used
- 4x `ue5_execute` (IDs 33524–33527) — audit, TRex tagging, light/camp tagging, Niagara dust actor spawn.
- 2x `search_sounds` — "whoosh air impact roar distortion" (0 results, flagged gap), "campfire ember pop crackle" (2 usable matches: KVV Audio flame burn loop, FREESampleSunday fire crackle).
- 2x `generate_image` — T-Rex roar heat-distortion concept art, campfire VFX concept art. **Both generated successfully at gpt-image-1 model level but Supabase upload failed with `403 Invalid Compact JWS`** — this is now a 7+ cycle confirmed infrastructure blocker (Agents #14, #15, #16, #17 all hit it across multiple cycles). Not fixable at individual agent level; re-escalating to #01/#19.

## Decisions & Justification
- All VFX hooks are additive tags on existing actors (TRex, DirectionalLight, QuestGiver anchor) to respect the anti-duplication naming rule — only ONE new actor (`VFX_Dust_TRexFootstep_001`) was spawned, and only after confirming no duplicate existed.
- No Niagara System assets were authored directly as `.uasset` (unreliable/impossible via headless Python without an editor UI); the `NiagaraActor` placeholder + tag contract lets Integration (#19) wire the actual particle graphs.
- Zero camera moves, zero .cpp/.h writes, per hard rules.

## Known gaps for next cycle
- No Freesound match found for "roar heat distortion whoosh" — recommend custom Foley/synthesis pass.
- Supabase image upload still broken — VFX reference art exists at generation level but is not retrievable via URL. Recommend #01/#19 investigate the JWS/storage token issue directly rather than repeated agent-level attempts.

## Next agent (#18 QA & Testing)
- Verify `VFX_Dust_TRexFootstep_001` actor exists in `MinPlayableMap` with correct tags and location.
- Verify TRex, DirectionalLight, and QuestGiver_HubCamp_001 all carry their new VFX_* tags without duplication.
- Flag the Supabase `403 Invalid Compact JWS` image/audio upload failure as a BLOCKING infrastructure bug for #01 escalation — it has now affected 4 different agents across 7+ cycles.
