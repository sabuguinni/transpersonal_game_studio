# CHARACTER CREATION PIPELINE
## Transpersonal Game Studio — Character Art Department

### OVERVIEW
Este documento define o pipeline completo para criação de personagens humanos no jogo Jurássico usando MetaHuman Creator integrado ao Unreal Engine 5.7.

### CORE PRINCIPLES
1. **Cada rosto conta uma história** — Nenhum personagem é genérico
2. **Variação procedural** — Sistema que gera diversidade automática
3. **Performance first** — LODs automáticos e otimização Nanite
4. **Modular clothing** — Sistema de roupas intercambiáveis

### TECHNICAL PIPELINE

#### 1. MetaHuman Creation
- **Tool**: MetaHuman Creator (UE5.7 integrated)
- **Base presets**: 12 arquétipos fundamentais
- **Variation system**: Procedural blend shapes
- **Export format**: Native UE5 assets

#### 2. Character Variants
- **Facial diversity**: 50+ unique face combinations
- **Body types**: 8 base body shapes (4 masculine, 4 feminine)
- **Age ranges**: Young adult (20-30), Adult (30-50), Mature (50+)
- **Ethnicity**: Global representation

#### 3. Clothing System
- **Base layer**: Primitive clothing (animal skins, plant fibers)
- **Modern layer**: Contemporary clothing (for time travelers)
- **Damage states**: Clean, worn, torn, dirty
- **Seasonal variants**: Hot climate, cold climate adaptations

#### 4. Performance Optimization
- **LOD system**: Automatic Nanite scaling
- **Texture streaming**: Virtual textures for skin/clothing
- **Animation LODs**: Distance-based complexity reduction
- **Crowd optimization**: Mass AI compatible meshes

### ASSET NAMING CONVENTION
```
CH_{Type}_{Archetype}_{Variant}_{LOD}
Example: CH_NPC_Tribal_Elder_01_LOD0
```

### MEMORY BUDGETS
- **Hero characters**: 50MB max (full detail)
- **Important NPCs**: 25MB max
- **Crowd NPCs**: 10MB max
- **LOD fallback**: 2MB max

### INTEGRATION WITH OTHER SYSTEMS
- **Animation**: Compatible with Motion Matching system
- **AI Behavior**: Facial expression support for emotions
- **Narrative**: Character-specific dialogue animations
- **Combat**: Damage visualization on clothing/skin