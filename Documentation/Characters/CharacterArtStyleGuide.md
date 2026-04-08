# CHARACTER ART STYLE GUIDE
## Transpersonal Game Studio — Jurassic Survival Game
### Version 1.0 — Character Artist Agent

---

## VISUAL PHILOSOPHY

"Every face tells a story before a word is spoken. In our prehistoric world, survival has carved itself into every line, every scar, every weathered feature. These are not modern humans transported back in time — these are people shaped by their environment, their struggles, their triumphs."

### Core Principles

1. **AUTHENTICITY OVER BEAUTY** — Characters show the marks of their lives
2. **DIVERSITY IN UNITY** — Different tribes, different features, shared humanity
3. **ENVIRONMENTAL STORYTELLING** — Clothing, scars, posture tell their history
4. **BELIEVABLE AGING** — Time and hardship are visible on every face

---

## PROTAGONIST DESIGN

### Dr. [Name TBD] — The Paleontologist

**Base Concept:** Modern academic thrust into primitive survival

**Physical Traits:**
- Age: 35-45 years
- Build: Academic physique transitioning to survival-hardened
- Hands: Initially soft, becoming calloused and scarred
- Posture: Evolving from scholarly to alert/defensive

**Clothing Evolution:**
- **Phase 1:** Modern expedition gear (torn, dirty)
- **Phase 2:** Hybrid modern/primitive repairs
- **Phase 3:** Fully adapted primitive clothing

**MetaHuman Base:** Medium build, weathered features, intelligent eyes

---

## NPC ARCHETYPES

### 1. THE TRIBAL ELDER
**Role:** Wisdom keeper, spiritual guide
**Age:** 55-70 years
**Features:** Deep weathering, ritual scars, bone/tooth ornaments
**Clothing:** Elaborate hide garments, ceremonial elements
**Posture:** Dignified despite age, commanding presence

### 2. THE HUNTER
**Role:** Primary food provider, tracker
**Age:** 25-40 years
**Features:** Lean muscle, hunting scars, keen eyes
**Clothing:** Practical hide armor, tool belts
**Posture:** Alert, predatory grace

### 3. THE GATHERER
**Role:** Plant knowledge, medicine, crafting
**Age:** 20-50 years
**Features:** Stained hands from plants, detailed knowledge lines
**Clothing:** Practical with many pouches, plant decorations
**Posture:** Observant, gentle movements

### 4. THE WARRIOR
**Role:** Tribe protection, conflict resolution
**Age:** 20-35 years
**Features:** Battle scars, intimidating build, war paint
**Clothing:** Protective hide armor, weapon displays
**Posture:** Aggressive readiness, territorial stance

### 5. THE CHILD
**Role:** Future, innocence, learning
**Age:** 8-16 years
**Features:** Curious eyes, minor scrapes, growth markers
**Clothing:** Simple hides, learning tools
**Posture:** Energetic, constantly moving

### 6. THE OUTCAST
**Role:** Exile, dangerous knowledge, wildcard
**Age:** 30-50 years
**Features:** Severe scarring, wild appearance, haunted eyes
**Clothing:** Ragged, improvised, non-tribal
**Posture:** Defensive, unpredictable

---

## TECHNICAL SPECIFICATIONS

### MetaHuman Integration
- **Base Templates:** Use diverse MetaHuman presets as starting points
- **Customization:** Focus on weathering, scars, tribal modifications
- **LOD Strategy:** 3 levels — Cinematic, Gameplay, Distance
- **Performance Target:** 60fps with 20+ characters on screen

### Texture Requirements
- **Skin:** 4K base, 2K detail maps for scars/dirt
- **Clothing:** 2K diffuse, normal, roughness maps
- **Accessories:** 1K textures, optimized for repetition

### Polygon Budgets
- **Hero Characters:** 150K triangles (LOD0)
- **Standard NPCs:** 75K triangles (LOD0)
- **Crowd NPCs:** 25K triangles (LOD0)

---

## MATERIAL GUIDELINES

### Skin Materials
- **Base:** Physically accurate skin shader
- **Weathering:** Dirt, sun damage, exposure effects
- **Scars:** Procedural scar generation system
- **Tribal Marks:** Paint and tattoo layers

### Clothing Materials
- **Hide/Leather:** Worn, organic textures
- **Fur:** Realistic hair shader with wind response
- **Bone/Tooth:** Polished and weathered variants
- **Plant Fibers:** Woven textures with fraying

### Aging System
- **Young (15-25):** Clean features, minimal weathering
- **Adult (25-45):** Moderate wear, experience lines
- **Elder (45+):** Deep weathering, wisdom marks

---

## CULTURAL VARIATIONS

### Tribe A — Forest Dwellers
- **Features:** Darker skin, green/brown clothing
- **Decorations:** Leaf patterns, wood ornaments
- **Specialization:** Camouflage, tree climbing

### Tribe B — Plains Nomads
- **Features:** Sun-weathered, lighter clothing
- **Decorations:** Geometric patterns, bone tools
- **Specialization:** Distance travel, hunting

### Tribe C — Cave Dwellers
- **Features:** Pale skin, thick clothing
- **Decorations:** Stone carvings, mineral paints
- **Specialization:** Tool making, fire keeping

---

## IMPLEMENTATION NOTES

### Modular System
- **Base Bodies:** 3 male, 3 female variants
- **Heads:** 20+ unique faces per gender
- **Clothing:** Mix-and-match system
- **Accessories:** Procedural placement

### Animation Considerations
- **Facial Rigs:** Full MetaHuman compatibility
- **Body Deformation:** Realistic muscle/fat simulation
- **Clothing Physics:** Wind and movement response
- **Hair Systems:** Realistic primitive hairstyles

### Performance Optimization
- **Instancing:** Shared materials where possible
- **Culling:** Aggressive LOD and occlusion
- **Streaming:** Dynamic loading of character details

---

*Character Art Style Guide v1.0*
*Character Artist Agent — Transpersonal Game Studio*