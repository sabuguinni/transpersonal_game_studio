# VFX Design — Cycle 009 (Agent #17)

## Bridge Status
**HEALTHY** — 4/4 `ue5_execute` Python calls succeeded (3.0s, 6.1s, 3.0s, 3.0s), zero timeouts, command IDs 32097–32100.

## Dedup Audit (Step 1)
- Scanned all level actors for existing `VFX_` labels: **none found** (clean slate this cycle).
- Located Audio Agent #16's actors from previous cycle: `Audio_Campfire_Hub_001`, `Audio_ForestAmbience_Hub_001`.
- Located the existing T-Rex actor (not duplicated) tagged by #16 with `Audio_ProximityRoar_...` and `Audio_ScreenShakeTrigger_Radius1500`.

## Real Changes Made in `MinPlayableMap`

### 1. Footstep dust & roar distortion (tied to T-Rex, Category 2)
- `VFX_Dust_TRexFootstep_Hub_001` — flattened sphere proxy at the T-Rex's feet, tagged `VFX_LinkedTo_Audio_ScreenShakeTrigger_Radius1500` so Blueprint logic can trigger dust burst + camera shake + roar audio off the **same** event.
- `VFX_RoarDistortion_TRex_Hub_001` — sphere proxy at head height, tagged `VFX_RoarAirDistortion`, reserved for a heat-haze/air-warp post-process hookup on roar events.

### 2. Campfire fire & smoke (Category 1)
- `VFX_Fire_Campfire_Hub_001` — orange emissive-tinted proxy positioned at #16's existing campfire audio actor location (reused coordinates, no duplicate campfire).
- `VFX_Smoke_Campfire_Hub_001` — grey translucent-tinted proxy above the fire proxy.

### 3. Ambient world dust (Category 4)
- `VFX_DustMotes_Hub_Ambient_001` — large soft-scale proxy in the hub clearing (2100, 2400) for sunbeam dust motes, matching the hero-screenshot composition requirement (dense forest clearing, single PlayerStart).

### 4. Damage-flash groundwork (Category 3, player feedback)
- Spawned `VFX_PostProcess_DamageFlash_Global_001`, an unbound `PostProcessVolume` tagged `VFX_DamageFlashHook`, blend weight 1.0, vignette override initialized at 0.0. This is the anchor point for a future red-flash-on-hit post process material — no color grading applied yet (needs an actual `M_VFX_DamageFlash` material asset, which requires either Meshy/asset import or manual editor authoring; cannot author Niagara/PP materials purely via headless Python without a base asset reference).

### Material limitation (honest report)
No custom Niagara systems exist in the project (`/Game` scan returned zero Niagara assets), and this headless bridge cannot author new `.uasset` Niagara emitters from Python alone. All "VFX" this cycle are **static mesh color-coded proxies** (dynamic material instances on `/Engine/EngineMaterials/DefaultMaterial`) marking the correct positions, scale, and semantic tags so a future cycle (with either Meshy-imported particle textures or manual Niagara authoring in-editor) can drop real particle systems into these exact transforms without re-doing placement/dedup work.

## Image Generation — BLOCKED (infra issue, 1st occurrence for VFX Agent)
Both `generate_image` calls (campfire fire/smoke concept art, T-Rex footstep dust concept art) **succeeded on the AI generation side** but failed on upload:
```
HTTP 400 Bad Request - {"statusCode":"403","error":"Unauthorized","message":"Invalid Compact JWS"}
```
This is the **same Supabase storage JWT failure** the Audio Agent (#16) has now reported for 4 consecutive cycles blocking ElevenLabs audio uploads. Confirmed this is not audio-specific — it blocks **all** binary asset uploads (images included) via the same Supabase storage path. Escalating alongside #16's report.

## Sound Sourcing
- Searched "fire crackling embers pop wood burning" — 2 usable results (KVV Audio fire-poker/coal recordings, good match for campfire ambient loop once JWS issue is fixed and audio can be persisted/attached).

## Decisions & Justification
- Reused engine-native `StaticMeshActor` + `Sphere` primitive + dynamic material color-coding instead of inventing fake Niagara asset paths that don't exist in the project (would have silently no-op'd or errored).
- Reused existing T-Rex and campfire actor coordinates instead of spawning duplicates, per naming/dedup rule.
- Zero camera changes, zero `.cpp`/`.h` writes (both forbidden per standing rules).

## Escalation (now 2 systems affected, 4th cycle for audio / 1st confirmed for VFX)
Supabase `403 Invalid Compact JWS` blocks **both** ElevenLabs audio AND generate_image uploads. This is an infrastructure-wide storage auth failure, not a per-tool bug. Needs #01/#19 priority fix — likely an expired/misconfigured Supabase service role JWT.

## Next Agent (#18 QA & Testing)
- Verify the 6 new `VFX_` actors exist in `MinPlayableMap` with correct tags (`VFX_LinkedTo_Audio_ScreenShakeTrigger_Radius1500`, `VFX_RoarAirDistortion`, `VFX_CampfireFire`, `VFX_CampfireSmoke`, `VFX_AmbientParticles`, `VFX_DamageFlashHook`).
- Confirm `VFX_PostProcess_DamageFlash_Global_001` is unbound and covers the full level (needed before any real damage-flash material can be wired).
- Flag the Supabase JWS blocker as a shared cross-agent (#16 + #17) infra ticket for #01/#19.
