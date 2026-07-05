# Character Design Bible — Transpersonal Game Studio
## Prehistoric Survival Game — Character Artist Agent #09

---

## PLAYER CHARACTER: "The Survivor"

### Concept Overview
The player character is a primitive human survivor in the Cretaceous era. No supernatural abilities, no mystical powers — pure survival instinct, physical adaptation, and learned skill.

### Visual Profile — Male Variant
- **Build**: Lean and muscular, 5'10" (178cm). Hardened by constant physical labor and survival.
- **Skin**: Deep tan, weathered, with visible scars from previous encounters. Sun-darkened on exposed areas.
- **Face**: Strong jaw, prominent brow ridge, short dark hair matted with sweat and dirt. Tribal ochre markings on cheekbones — functional camouflage, not decoration.
- **Eyes**: Dark brown, constantly scanning. Alert, never relaxed.

### Visual Profile — Female Variant
- **Build**: Athletic and wiry, 5'6" (168cm). Fast and agile.
- **Skin**: Similar weathering, slightly lighter base tone. Braided dark hair tied back for practicality.
- **Face**: Sharp features, determined expression. Minimal ochre markings on forehead.

### Costume / Equipment Layers

#### Layer 0 — Base (always visible)
- Rough-cut hide loincloth/shorts, tied with sinew cord
- Wrapped hide sandals with bone buckles
- Sinew wrist wraps (functional — protect from bowstring and rope burns)

#### Layer 1 — Torso (craftable)
- **Tier 1**: Open hide vest, raw edges, single shoulder strap
- **Tier 2**: Stitched leather vest with bone toggles, better coverage
- **Tier 3**: Layered hide armor with carved bone plates on shoulders

#### Layer 2 — Legs (craftable)
- **Tier 1**: Bare legs with hide knee wraps
- **Tier 2**: Wrapped leather leg guards, mid-thigh to ankle
- **Tier 3**: Full hide leggings with carved bone shin guards

#### Layer 3 — Head (craftable)
- **Tier 1**: None (bare head)
- **Tier 2**: Feathered headband (from killed dinosaurs)
- **Tier 3**: Carved bone helmet with dinosaur scale trim

### Weapon Loadout (visual representation)
- **Primary slot**: Stone-tipped spear (visible on back when holstered)
- **Secondary slot**: Bone knife (hip sheath)
- **Ranged slot**: Primitive bow (carried in hand or slung)
- **Tool slot**: Stone hand-axe (belt loop)

---

## NPC ARCHETYPES

### 1. The Elder (Tribe Leader)
- **Role**: Quest giver, knowledge source, tribe authority
- **Visual**: Older male/female, 60+, deeply wrinkled skin, white hair, elaborate bone jewelry
- **Distinguishing feature**: Carries a carved staff topped with a dinosaur skull
- **Clothing**: Most elaborate in the tribe — layered hides, feathers, carved bone decorations
- **Expression**: Calm authority, never afraid

### 2. The Hunter (Combat NPC)
- **Role**: Patrol companion, combat ally, trainer
- **Visual**: Prime physical condition, 25-35, multiple visible scars
- **Distinguishing feature**: Trophy necklace of dinosaur teeth and claws
- **Clothing**: Practical — minimal coverage for mobility, no decorative elements
- **Expression**: Focused, calculating

### 3. The Gatherer (Civilian NPC)
- **Role**: Resource trader, crafting teacher
- **Visual**: Lighter build, 20-50, hands stained from plant processing
- **Distinguishing feature**: Carries woven grass baskets and pouches
- **Clothing**: Simpler hide wraps, more coverage for plant work
- **Expression**: Cautious but curious

### 4. The Outcast (Hostile NPC)
- **Role**: Bandit/enemy human
- **Visual**: Unkempt, aggressive posture, asymmetric scarring
- **Distinguishing feature**: Wears bones and teeth from other humans — intimidation display
- **Clothing**: Ragged, mismatched hides from multiple sources
- **Expression**: Aggressive, unpredictable

---

## DINOSAUR CHARACTER PROFILES (for AI/animation reference)

### T-Rex (Tyrannosaurus rex)
- **Scale**: 12m long, 4m tall at hip
- **Visual priority**: Massive head, tiny arms, powerful legs
- **Color scheme**: Dark olive green with brown striping, pale underbelly
- **Behavioral tells**: Head tilts when detecting sound, tail counterbalances when running
- **UE5 spawn target**: X=2100, Y=2400, Z=terrain+600 (hub area)

### Velociraptor
- **Scale**: 1.8m long, 0.9m tall at hip (accurate — NOT Jurassic Park size)
- **Visual priority**: Feathered body, sickle claw, intelligent eyes
- **Color scheme**: Mottled brown/green with cream underbelly, iridescent feather tips
- **Behavioral tells**: Head bobs when curious, pack members mirror each other's movements
- **UE5 spawn target**: Cluster of 3 near X=2100, Y=2400 hub

### Triceratops
- **Scale**: 9m long, 3m tall at shoulder
- **Visual priority**: Three-horned frill, massive bulk, armored hide
- **Color scheme**: Grey-brown with orange frill highlights
- **Behavioral tells**: Paws ground when threatened, frill flushes red when aggressive
- **UE5 spawn target**: X=2100, Y=2400 hub, grazing pose

### Brachiosaurus
- **Scale**: 26m long, 13m tall
- **Visual priority**: Extreme height, long neck, gentle giant presence
- **Color scheme**: Warm grey-brown, lighter underbelly
- **Behavioral tells**: Slow deliberate movements, neck sweeps side to side when feeding
- **UE5 spawn target**: X=2100, Y=2400 hub, visible from distance

### Parasaurolophus
- **Scale**: 10m long, 4m tall
- **Visual priority**: Distinctive hollow crest on head, duck-bill
- **Color scheme**: Earthy red-brown with cream stripes along flanks
- **Behavioral tells**: Vocalizes through crest (low resonant calls), herds together
- **UE5 spawn target**: Group of 2-3 near hub

---

## VISUAL CONSISTENCY RULES

1. **No anachronistic materials** — no metal, no woven fabric (only hide, bone, stone, sinew, feathers)
2. **Dirt and wear always present** — no clean characters. Everyone is dirty, weathered, used.
3. **Injuries persist** — scars, bruises, and wounds should be visible on NPCs based on their history
4. **Scale reference** — always model characters against a 6-foot human silhouette first
5. **Color palette** — earth tones dominate: ochre, sienna, umber, raw hide, bone white, forest green
6. **No magical glows, auras, or energy effects** — all visual effects must have physical explanations

---

## METAHUMAN CONFIGURATION TARGETS

### Player Character (Male)
```
Body Type: Athletic
Height: 178cm
Skin: Dark tan, high roughness
Hair: Short, dark brown, matted
Eyes: Dark brown
Face Preset: Strong jaw, prominent brow
Blend Shapes: Determined expression as neutral
```

### Player Character (Female)
```
Body Type: Athletic-Lean
Height: 168cm
Skin: Medium tan, high roughness
Hair: Long dark, braided/tied back
Eyes: Dark brown
Face Preset: Sharp features, alert neutral
Blend Shapes: Alert expression as neutral
```

---

## PRODUCTION STATUS

| Asset | Status | Priority |
|-------|--------|----------|
| Male Survivor — Concept | DESIGNED (bible) | P1 |
| Female Survivor — Concept | DESIGNED (bible) | P1 |
| Elder NPC — Concept | DESIGNED (bible) | P2 |
| Hunter NPC — Concept | DESIGNED (bible) | P2 |
| T-Rex visual profile | DESIGNED (bible) | P1 |
| Velociraptor visual profile | DESIGNED (bible) | P1 |
| Triceratops visual profile | DESIGNED (bible) | P2 |
| Brachiosaurus visual profile | DESIGNED (bible) | P2 |
| UE5 Mannequin placeholder | PENDING bridge | P1 |
| MetaHuman integration | PENDING bridge | P3 |

---

*Character Artist Agent #09 — Cycle PROD_CYCLE_AUTO_20260705_002*
*Bridge status: DOWN (timeout) — UE5 commands queued for next cycle*
*APIs: generate_image 401, meshy 402 (insufficient funds)*
*Deliverable: Full character design bible committed to GitHub*
