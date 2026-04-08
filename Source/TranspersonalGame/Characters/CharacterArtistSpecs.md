# CHARACTER ARTIST SPECIFICATIONS
## Transpersonal Game Studio — Jurassic Survival
## Agent #09 — Character Artist Agent
## Production Cycle: PROD_JURASSIC_001

---

## TECHNICAL FOUNDATION

### MetaHuman Integration Pipeline
- **Primary Tool:** MetaHuman Creator (UE5.7 integrated)
- **Export Format:** Native UE5 MetaHuman assets
- **LOD Strategy:** Automatic Nanite-based scaling
- **Animation Compatibility:** Motion Matching ready skeletons
- **Texture Resolution:** 4K base, with 2K/1K LOD chain

### Character Categories

#### 1. PROTAGONIST — Dr. [Name TBD]
**Archetype:** Academic Survivor
**Age Range:** 35-45 years
**Physical Traits:**
- Weathered but intelligent features
- Signs of recent survival stress
- Academic background visible in posture/expression
- Adaptable clothing (torn modern gear transitioning to primitive)

#### 2. TRIBAL NATIVES
**Archetype:** Indigenous Survivors
**Age Range:** 18-60 years
**Physical Traits:**
- Adapted to prehistoric environment
- Tribal markings and primitive clothing
- Strong, survival-hardened physiques
- Diverse ethnic backgrounds

#### 3. OTHER TIME TRAVELERS
**Archetype:** Lost Explorers
**Age Range:** 25-55 years
**Physical Traits:**
- Various modern backgrounds
- Different stages of adaptation
- Improvised survival gear
- Psychological stress markers

#### 4. BACKGROUND NPCS
**Archetype:** Environmental Population
**Age Range:** 20-50 years
**Physical Traits:**
- Simplified but unique features
- Crowd-ready optimization
- Modular clothing system

---

## VISUAL DESIGN PRINCIPLES

### 1. AUTHENTICITY OVER BEAUTY
Every face tells a survival story. No "perfect" characters.

### 2. ENVIRONMENTAL ADAPTATION
Clothing, skin, hair show interaction with prehistoric world.

### 3. INDIVIDUAL IDENTITY
No clones. Each NPC has unique facial structure and expression.

### 4. PERFORMANCE OPTIMIZATION
LOD system ensures 60fps with crowds of 50+ characters.

---

## TECHNICAL SPECIFICATIONS

### MetaHuman Setup
```
Base Template: MetaHuman Creator UE5.7
Skeleton: UE5 Mannequin Compatible
Animation BP: ABP_MetaHuman_PostProcess
Physics Asset: Cloth simulation ready
Collision: Capsule + detailed mesh for interactions
```

### Material System
```
Skin Shader: Subsurface scattering enabled
Clothing Materials: Physically based, weather-damaged
Hair System: Strand-based with wind interaction
Dirt/Damage Overlays: Procedural weathering masks
```

### Performance Targets
```
LOD0 (Close): 15,000-20,000 triangles
LOD1 (Medium): 8,000-12,000 triangles  
LOD2 (Far): 3,000-5,000 triangles
LOD3 (Crowd): 1,000-2,000 triangles
Texture Memory: 8MB per character (all LODs)
```

---

## PRODUCTION PIPELINE

### Phase 1: Core Characters (Week 1)
1. Protagonist base design
2. 3 Tribal archetypes
3. 2 Time traveler variants

### Phase 2: Population Expansion (Week 2)
1. 10 Background NPC variants
2. Crowd optimization
3. Modular clothing system

### Phase 3: Specialization (Week 3)
1. Unique named NPCs
2. Quest-specific characters
3. Cinematic quality variants

### Phase 4: Integration (Week 4)
1. Animation system integration
2. Performance optimization
3. Mass AI compatibility testing

---

## QUALITY STANDARDS

### Visual Fidelity
- Photorealistic skin rendering
- Believable facial expressions
- Authentic period-appropriate details
- Consistent art direction

### Technical Performance
- Stable 60fps with 20+ characters on screen
- Seamless LOD transitions
- Memory budget compliance
- Animation system compatibility

### Narrative Integration
- Visual storytelling through character design
- Clear archetype recognition
- Emotional resonance with player
- World consistency

---

*Document Version: 1.0*
*Created: Production Cycle PROD_JURASSIC_001*
*Character Artist Agent #09*