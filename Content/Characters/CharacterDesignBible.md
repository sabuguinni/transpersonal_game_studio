# Character Design Bible — Transpersonal Game Studio
## Prehistoric Survival Game — Character Art Direction

**Last Updated:** Cycle PROD_CYCLE_AUTO_20260705_001  
**Agent:** #09 — Character Artist  
**Status:** DEGRADED MODE (UE5 bridge DOWN, OpenAI API 401, Meshy insufficient funds)

---

## DESIGN PHILOSOPHY

Characters in this game are **biographical portraits**. Every scar, every piece of clothing, every posture tells a survival story. No two characters look alike — the world is harsh, and it shows on every face.

Inspired by:
- **Caravaggio**: faces as maps of lived experience — shadow, tension, survival
- **Rockstar Games**: no NPC clones; every character has visual identity
- **National Geographic realism**: if it wouldn't appear in a prehistoric documentary, it doesn't exist

---

## PLAYER CHARACTER — MALE SURVIVOR

### Physical Profile
- **Age:** 25-35 (weathered, not old)
- **Build:** Athletic, lean muscle — not gym-built, survival-built
- **Height:** 175-180cm
- **Skin:** Tanned, weathered, scarred from encounters
- **Hair:** Dark, matted, tied back with sinew cord

### Clothing & Equipment
- **Upper body:** Animal hide vest (raptor leather, rough-stitched)
- **Lower body:** Wrapped fur leggings, primitive sandals with bone clasps
- **Accessories:** Bone necklace (trophies from hunts), wrist wraps of sinew
- **War paint:** Ochre and charcoal markings — tribal identification, not decoration
- **Primary weapon:** Stone-tipped spear (2m, ash wood shaft)
- **Secondary:** Flint hand axe on belt loop

### Color Palette
- Base: Warm ochre, raw umber, charcoal grey
- Accent: Blood red (war paint), bone white (trophies)
- Material: Matte, rough — no shine except wet conditions

---

## PLAYER CHARACTER — FEMALE SURVIVOR

### Physical Profile
- **Age:** 22-32
- **Build:** Lean and fast — a tracker, not a brawler
- **Height:** 165-170cm
- **Skin:** Olive-toned, sun-darkened, with ritual scarification on shoulders
- **Hair:** Braided with raptor feathers and bone beads

### Clothing & Equipment
- **Upper body:** Wrapped leather chest binding, shoulder guard from large reptile scale
- **Lower body:** Hide skirt with utility pouches, bare legs for mobility
- **Accessories:** Feather braids, bone ring on right hand
- **Primary weapon:** Flint knife (25cm, obsidian edge)
- **Secondary:** Short bow with stone-tipped arrows (5 carried)

### Color Palette
- Base: Deep brown, slate grey, forest green (plant dye)
- Accent: White (bone), deep red (ochre ritual marks)

---

## NPC ARCHETYPES

### 1. Elder Scout — "The Tracker"
- **Role:** Gives hunting knowledge, warns of territorial dinosaurs
- **Visual:** Elderly male, 55+, deeply lined face, missing two fingers (raptor encounter)
- **Clothing:** Layered furs, carved bone staff, extensive trophy necklace
- **Voice sample:** Recorded — `PrimitiveHuman_Scout` (Supabase TTS)
  - URL: `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783217948529_PrimitiveHuman_Male.mp3`

### 2. Young Hunter — "The Reckless"
- **Role:** Companion NPC, follows player, dies if player is careless
- **Visual:** Male, 18-20, eager expression, fresh scars, oversized spear
- **Clothing:** Minimal — just hide wrap and sandals, no trophies yet

### 3. Tribe Mother — "The Keeper"
- **Role:** Camp base NPC, manages crafting, gives survival tips
- **Visual:** Female, 40-50, strong build, elaborate bone jewelry, calm authority
- **Clothing:** Full hide robe, decorated with dried flowers and shells

### 4. Injured Survivor — "The Warning"
- **Role:** Found in the wilderness, warns of specific dinosaur dangers
- **Visual:** Either gender, torn clothing, visible wounds, fear in eyes
- **Clothing:** Shredded, bloodied — tells the story of what attacked them

---

## TECHNICAL SPECIFICATIONS

### Mesh Requirements
- **Player character:** 25,000-40,000 polygons (LOD0)
- **LOD1:** 12,000 polygons (medium distance)
- **LOD2:** 4,000 polygons (far distance)
- **LOD3:** 800 polygons (crowd/background)

### Texture Maps
- **Albedo:** 4096x4096 (player), 2048x2048 (NPCs)
- **Normal:** 4096x4096 (player), 2048x2048 (NPCs)
- **Roughness/Metallic/AO:** Packed into single 4096x4096 ORM map
- **Subsurface:** 2048x2048 for skin realism

### Rigging Requirements
- UE5 compatible skeleton (compatible with UE5 Mannequin proportions)
- Facial blend shapes: 52 FACS-based shapes for dialogue
- Hand IK bones for weapon holding
- Foot IK bones for terrain adaptation (feeds into Agent #10 Animation)

### MetaHuman Integration
- Player characters: Full MetaHuman pipeline (high fidelity)
- Main NPCs: MetaHuman pipeline
- Background NPCs: Simplified mesh, MetaHuman-inspired proportions

---

## DIVERSITY MATRIX

The tribe represents a single prehistoric population but with natural variation:
- **Skin tones:** Range from deep brown to medium olive (sun exposure variation)
- **Body types:** 3 male builds (lean/medium/stocky), 3 female builds (lean/medium/strong)
- **Facial features:** 8 base face shapes, randomized per NPC instance
- **Scars:** Procedural scar system — 0-5 scars per character, placed logically
- **Age:** 18-60 range, aging visible in skin texture and posture

---

## VOICE AUDIO ASSETS

| Character | File | Duration | URL |
|-----------|------|----------|-----|
| PrimitiveHuman_Male | 1783217948529_PrimitiveHuman_Male.mp3 | ~9s | [Supabase](https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783217948529_PrimitiveHuman_Male.mp3) |
| PrimitiveHuman_Scout | (pending this cycle) | ~12s | TBD |

---

## ASSET REQUEST LOG

| Cycle | Asset | Pipeline | Status | Notes |
|-------|-------|----------|--------|-------|
| AUTO_20260705_001 | Prehistoric Male Survivor 3D | Meshy | FAIL (insufficient funds) | Fallback: use UE5 mannequin placeholder |
| AUTO_20260705_001 | Male Character Concept Art | generate_image | FAIL (401 API key) | Blocked by API issue |
| AUTO_20260705_001 | Female Character Concept Art | generate_image | FAIL (401 API key) | Blocked by API issue |

---

## HANDOFF TO AGENT #10 (Animation)

### What's Ready
- Character design specifications (this document)
- Voice samples for dialogue timing reference
- Skeleton requirements defined

### What Animation Needs From This
1. Final mesh with UE5-compatible skeleton
2. Facial blend shape list (52 FACS shapes)
3. Weapon attachment socket positions
4. Cloth simulation regions (fur, hide strips)

### Blocking Issues
- UE5 bridge DOWN — cannot verify TranspersonalCharacter mesh in editor
- Meshy API insufficient funds — no 3D character mesh generated this cycle
- OpenAI API 401 — no concept art generated this cycle

### Recommended Next Steps for Agent #10
- Use UE5 mannequin (SK_Mannequin) as placeholder for animation work
- Set up Motion Matching database with locomotion states: idle, walk, run, sprint, crouch
- Implement foot IK for terrain adaptation
- Reference voice sample timing for dialogue animation sync

---

## WORLD COORDINATE TARGETS

Per global memory directive, character placement priority:
- **Hub center:** X=2100, Y=2400 (PlayerStart location)
- **Camera viewpoint:** ~(6500, 6500)
- Player character spawns at PlayerStart
- NPCs clustered within 3000 units of hub center

---

*Character Artist Agent #09 — Cycle PROD_CYCLE_AUTO_20260705_001*  
*DEGRADED MODE: UE5 bridge DOWN, visual APIs unavailable*  
*Deliverables: 2x TTS voice samples, 1x Character Design Bible*
