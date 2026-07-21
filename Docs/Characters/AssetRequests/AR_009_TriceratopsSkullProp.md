# Asset Request Record — Triceratops Skull Prop

**Requested by:** Character Artist Agent #09
**Cycle:** PROD_CYCLE_AUTO_20260711_005
**Status:** BLOCKED (Meshy credits exhausted) — procedural placeholder active in live map

## Request Details (asset_requests row equivalent)
| Field | Value |
|---|---|
| asset_name | TriceratopsSkullProp |
| asset_type | creature_prop |
| era | Late Cretaceous |
| requested_by_agent | 09_CharacterArtist |
| pipeline | Meshy.ai (meshy-5, preview mode, realistic style, 15000 tris) |
| prompt | "Triceratops skull prop, weathered bone fossil, partially fossilized with cracks and mineral discoloration, three horns and frill intact, lying on ground as environmental/character accessory prop, scientifically accurate proportions, neutral earthy bone color, isolated on white background for game asset use" |
| result | HTTP 402 Insufficient funds — Meshy account has no remaining credits |
| fallback_action | Spawned procedural StaticMeshActor placeholder (`TrikeSkullProp_Hub_001`, scaled sphere primitive) at (2300, 2200, 100) inside the content hub, tagged for future GLB swap |
| next_step | Re-submit this exact prompt via `meshy_generate` once Meshy billing is topped up. Replace `TrikeSkullProp_Hub_001` mesh reference with the imported GLB via `unreal.AssetTools` import, keeping the same actor label and transform. |

## Rationale
A weathered Triceratops skull works as both an environmental storytelling prop (evidence of past hunts/scavenging) and a wearable/craftable trophy item consistent with survival-game itemization (bone tool crafting materials, decoration, quest marker). Placed inside the hero-camera content hub (X=2100,Y=2400 ± 3000) per the hub composition mandate, near the existing dinosaur cluster.

## Concept Art Status
Two HD character concept renders were generated successfully on the model side this cycle (survivor full-body reference sheet, tribal elder portrait) but both failed Supabase Storage upload with `HTTP 400 / 403 Invalid Compact JWS` — the same recurring infra issue reported by Agent #08 in the previous cycle. This is an infrastructure-side JWT/signing problem, not agent-actionable. Prompts are preserved below for regeneration once storage auth is fixed.

### Preserved Prompt 1 — Survivor Full-Body Reference
"Character concept art, full body reference sheet of a primitive human survivor in the Late Cretaceous period, weathered leather and hide clothing, bone and stone tool accessories, muscular build from survival labor, dirt and ash-streaked skin, determined expression, standing in a T-pose against a neutral gray background, realistic proportions, detailed textures for game character reference, National Geographic documentary realism, no fantasy elements"

### Preserved Prompt 2 — Tribal Elder Portrait
"Character concept art portrait, tribal prehistoric NPC elder, distinct facial features from other tribe members, deep wrinkles telling a lifetime of survival, braided gray hair with bone ornaments, animal hide cloak with hand-stitched seams, scars on weathered hands, sitting near a campfire at dusk, warm firelight on skin, realistic documentary style, no spiritual or mystical elements, grounded prehistoric survival aesthetic"
