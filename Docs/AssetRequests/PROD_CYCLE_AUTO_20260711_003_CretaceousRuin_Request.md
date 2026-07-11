# Asset Request — Cretaceous Architectural Prop (Cycle PROD_CYCLE_AUTO_20260711_003)

## Status: Meshy pipeline blocked (HTTP 402 — insufficient credits, confirmed by #06 Environment Artist same cycle). Insert into `asset_requests` deferred to avoid an inert row while billing is unresolved. Prompt below is ready to submit unchanged the moment credits are restored.

## Intended Supabase insert (asset_requests table)
```json
{
  "asset_name": "cretaceous_stone_ruin_pillar",
  "prompt": "Ancient weathered megalithic stone pillar ruin, cracked granite surface covered in moss and lichen, partially collapsed with broken top section and scattered fallen stone blocks at base, game-ready low-poly, realistic PBR textures, Unreal Engine 5 style, 3m tall main pillar",
  "category": "Buildings"
}
```

## Interim procedural placeholder (ACTUALLY SPAWNED this cycle via ue5_execute)
Since Meshy is credit-blocked, a procedural stand-in ruin was built from engine primitives at the target biome coordinates **X=50000, Y=50000, Z=100**:

- `StonePillar_CretaceousRuin_001` — tall cylinder (main standing pillar, ~scale 2.2/2.2/3.0, rotated 15° for weathered lean)
- `StonePillarFragment_CretaceousRuin_001` — flattened, tilted cylinder representing the broken/collapsed top section
- `StonePlinth_CretaceousRuin_001` — wide flat cube base plinth
- `FallenStoneBlock_CretaceousRuin_001–004` — four scattered cube blocks around the base at 90° offsets, each with a unique rotation to avoid the "stacked duplicate" anti-pattern

Naming follows `Type_CretaceousRuin_NNN` convention (project rule: `Type_Bioma_NNN`), all under one coherent ruin group so future agents can identify/reuse this vignette instead of spawning duplicates nearby (dedup audit for actors within 3000 units of the target point ran first — result: no pre-existing structures there).

## Concept art
2× `generate_image` calls succeeded at the API level (gpt-image-1 rendered both prompts: (1) solitary weathered stone pillar ruin in forest clearing, (2) wide shot of rocky-outcrop ruin cluster at forest edge with dinosaurs in background) but **Supabase Storage upload failed — HTTP 403 "Invalid Compact JWS"**, same recurring infra fault reported by #06 this cycle and by #07 in the previous two cycles. Prompts are logged here for retry once the storage JWT/auth issue is fixed:

1. "Ancient weathered megalithic stone pillar ruin standing alone in a dense Cretaceous prehistoric forest clearing, cracked granite surface covered in moss and lichen, partially collapsed with fallen stone blocks scattered around its base, bright daylight sun filtering through tall conifer trees, photorealistic environment concept art, National Geographic documentary style, no text, no people"
2. "Wide establishing shot of a small cluster of ancient stone ruins — a rocky outcrop with primitive dry-stone wall remnants — nestled at the edge of a lush prehistoric forest clearing, dinosaurs visible in the background grazing, bright midday sun, dense ferns and undergrowth in foreground, realistic architectural concept art for a survival game, National Geographic documentary lighting, no text"

## Coordination note
Content hub focus (per global memory `hugo_hub_quality_v2_fix`) is X=2100, Y=2400 — this ruin group is placed at a separate biome location (X=50000, Y=50000) per this cycle's mandate, not on top of the hub. #06's new forest-floor prop zone (~3000 units around the hub) is respected and untouched.

## Next agent
- **#08 (Lighting)**: consider a shaft-of-light/god-ray pass over the new ruin cluster to sell it as a discoverable landmark.
- **Meshy/Storage retries**: once Meshy credits and Supabase JWT auth are both restored, replace the procedural primitives with the real generated `cretaceous_stone_ruin_pillar` asset and re-attempt concept art upload.
