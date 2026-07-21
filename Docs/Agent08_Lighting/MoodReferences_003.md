# Mood/Lighting Reference Images — Cycle PROD_CYCLE_AUTO_20260708_003

## Status
Both images generated successfully at model level (`gpt-image-1`, HD quality). Supabase Storage upload failed for both with `HTTP 403 — Invalid Compact JWS` — this is an authentication/infra issue with the storage backend, consistent with the same failure Agent #7 reported this same cycle. Not a content or prompt issue; prompts are valid and preserved below for regeneration once storage auth is repaired.

## Reference 1 — "Jungle Clearing Midday" (hub composition target)
**Intent:** Match the mandated hero screenshot composition at world coords (2100, 2400) — the content hub with the single PlayerStart.
**Prompt:**
> Cinematic film-still lighting reference for a dinosaur survival game: bright Cretaceous jungle clearing at midday, warm sunlight streaming through a dense green canopy creating visible volumetric god-rays and dust motes, a Triceratops and a small pack of raptors grazing near ancient broken stone ruins covered in moss, thick tropical vegetation, soft atmospheric haze, deep shadows under foliage contrasted with bright golden sunbeams, photorealistic, Roger Deakins style cinematography, natural color grading, National Geographic documentary realism, no fantasy elements

**Purpose:** Validates that recognizable dinosaurs + dense vegetation + bright daylight + volumetric god-rays can coexist in one frame — the exact brief for the hero screenshot composition.

## Reference 2 — "Savanna Golden Hour" (secondary biome mood)
**Intent:** Establish a secondary lighting mood for open savanna biome (contrast to dense jungle) using the same sun rig at a lower time-of-day angle for variety testing.
**Prompt:**
> Cinematic lighting reference for a dinosaur survival game: late afternoon golden hour over a prehistoric savanna, long warm shadows stretching across tall grass, a distant Brachiosaurus silhouette against an orange-pink sky, thin ground fog settling in low areas, dramatic volumetric light shafts breaking through scattered clouds, photorealistic PBR terrain, Roger Deakins-inspired naturalistic cinematography, National Geographic documentary tone, no fantasy or spiritual elements

**Purpose:** Reference for future day/night cycle keyframes — confirms the warm/orange low-angle sun palette that should appear at dusk transitions without breaking the "documentary realism" tone.

## Next Steps
- Retry image generation once Supabase Storage JWT/auth issue is resolved (likely a service-role key rotation or expired signing key on the storage bucket).
- These prompts double as lighting design specs even without the rendered image — the in-engine PostProcessVolume and sun settings from `Cycle_PROD_20260708_003.md` were configured to match this described palette (warm daylight, volumetric god-rays, moderate bloom/vignette, no oversaturation).
