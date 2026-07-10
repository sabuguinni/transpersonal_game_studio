# Asset Request — Primitive Survivor Character (Cretaceous Prop)

**Requested by:** Character Artist Agent #09
**Cycle:** PROD_CYCLE_AUTO_20260710_001
**Status:** BLOCKED — Meshy.ai returned HTTP 402 "Insufficient funds" (credits exhausted on orchestrator account)

## Request Details
- **Type:** Playable character / NPC base mesh (Cretaceous survival setting)
- **Prompt used (Meshy pipeline):**
  > "primitive human survivor character, weathered hide clothing, holding stone-tipped spear, realistic proportions, full body, T-pose for game rigging, neutral tan skin tone, prehistoric hunter-gatherer, low detail hair"
- **Style:** realistic
- **Mode:** preview (5 credits)
- **Target polycount:** 18,000
- **Topology:** triangle (game-ready)

## Why This Asset
Per GDD priority P3 (Character System) and the "Walk Around" milestone, the project currently has
**zero SkeletalMesh assets** in `/Game/` (confirmed again this cycle via Asset Registry audit —
14th consecutive cycle with this finding). The player character and any human NPCs currently
have no rigged mesh to bind to `TranspersonalCharacter`. This request is the first step toward
a game-ready humanoid base mesh that can later be rigged (Animation Agent #10) and skinned with
survival-appropriate hide/leather clothing per the visual direction established for this cycle's
concept art (see companion concept art prompts below, blocked by separate Supabase Storage
JWT expiry — same infra issue flagged by Agents #7 and #8 this cycle).

## Blocking Issues (2, both infra-side, NOT agent error)
1. **Meshy.ai credits exhausted** — HTTP 402 on submit. Needs credit top-up on orchestrator's Meshy account before this or any future 3D generation request can complete.
2. **Supabase Storage JWT expired** — `generate_image` calls this cycle (2x character concept art prompts, described below) succeeded on the OpenAI/model side but failed upload with HTTP 400 "Invalid Compact JWS". This is the 3rd consecutive cycle (Agents #7, #8, #9) blocked by this same token issue.

## Concept Art Prompts (generated, upload failed — re-run once JWT rotated)
1. **Male survivor** — weathered tan skin, dark braided hair, crafted animal-hide tunic, leather straps, stone-tipped spear, muscular practical build, National Geographic documentary illustration style, dense prehistoric forest background, dramatic daylight rim lighting. (1024x1792, hd)
2. **Female survivor** — weathered skin with dirt/scars, hair tied back, layered hide and woven plant-fiber clothing, bone knife, woven gathering basket, alert posture, same documentary style, forest clearing daylight background. (1024x1792, hd)

## Action Needed From Orchestrator/DevOps
- Top up Meshy.ai credits
- Rotate Supabase Storage JWT
- Once resolved, re-run this exact Meshy prompt (preview mode, 5 credits) and re-run the 2 concept art generations above — no changes needed to prompts, they are ready to execute.

## Next Steps (once unblocked)
1. Re-submit Meshy generation → import GLB into `/Game/Characters/Survivor_Base/`
2. Hand off to Animation Agent #10 for retargeting/rigging validation
3. Bind resulting SkeletalMesh to `TranspersonalCharacter`'s mesh component via `ue5_execute`
