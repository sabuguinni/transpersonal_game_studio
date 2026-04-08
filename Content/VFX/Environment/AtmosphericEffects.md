# ATMOSPHERIC EFFECTS — NIAGARA SYSTEMS

## DESIGN PHILOSOPHY
Os efeitos atmosféricos devem ser **subtis mas constantes**, criando uma camada de tensão subconsciente que reforça a sensação de que o jogador é sempre uma presa no mundo dos dinossauros.

## CORE ATMOSPHERIC SYSTEMS

### 1. DUST_MOTES_AMBIENT
**Propósito:** Partículas de poeira que flutuam constantemente no ar
**Sensação:** Mundo antigo, pesado, opressivo
**Implementação Niagara:**
- Sprite Renderer com material translúcido
- Spawn Rate: 5-15 partículas/segundo
- Lifetime: 20-40 segundos
- Velocity: Noise-driven, muito lento
- LOD: 3 níveis (High: 15/s, Medium: 8/s, Low: 3/s)

### 2. POLLEN_DRIFT
**Propósito:** Pólen e esporos que derivam no vento
**Sensação:** Natureza primitiva, ar denso, respiração difícil
**Implementação Niagara:**
- Mesh Renderer com pequenas esferas
- Spawn Rate: 3-8 partículas/segundo
- Lifetime: 30-60 segundos
- Velocity: Wind-influenced, flutuação suave
- LOD: 3 níveis com mesh complexity reduction

### 3. INSECT_SWARM_DISTANT
**Propósito:** Enxames de insetos distantes, quase imperceptíveis
**Sensação:** Vida em movimento, algo sempre acontecendo
**Implementação Niagara:**
- Sprite Renderer com textura de enxame
- Spawn Rate: 1-3 partículas/segundo
- Lifetime: 10-20 segundos
- Velocity: Swarm behavior pattern
- LOD: Apenas em High quality

### 4. HEAT_SHIMMER
**Propósito:** Ondas de calor que distorcem o ar
**Sensação:** Calor opressivo, mundo hostil
**Implementação Niagara:**
- Material com distortion
- Spawn Rate: Contínuo
- Lifetime: Infinito (looping)
- Velocity: Vertical rise com noise
- LOD: Disabled em Low quality

## WIND SYSTEM INTEGRATION

### WIND_LEAVES_RUSTLE
**Propósito:** Folhas que caem e são levadas pelo vento
**Sensação:** Movimento constante, natureza viva mas ameaçadora
**Implementação Niagara:**
- Mesh Renderer com folhas variadas
- Spawn Rate: 2-10 partículas/segundo (wind-dependent)
- Lifetime: 15-30 segundos
- Velocity: Physics-based com wind zones
- LOD: 3 níveis com mesh count reduction

### GRASS_SWAY_PARTICLES
**Propósito:** Partículas que simulam movimento de erva alta
**Sensação:** Cobertura, esconderijo, mas também perigo oculto
**Implementação Niagara:**
- Sprite Renderer com blade textures
- Spawn Rate: 20-50 partículas/segundo
- Lifetime: 5-10 segundos
- Velocity: Synchronized sway pattern
- LOD: High detail apenas perto do jogador

## LIGHTING INTERACTION EFFECTS

### SUNBEAM_DUST
**Propósito:** Poeira iluminada em raios de sol
**Sensação:** Beleza perigosa, luz que revela e esconde
**Implementação Niagara:**
- Sprite Renderer com light interaction
- Spawn Rate: Light-dependent
- Lifetime: 10-20 segundos
- Velocity: Brownian motion
- LOD: Apenas quando luz directa disponível

### SHADOW_MOVEMENT
**Propósito:** Partículas que simulam movimento nas sombras
**Sensação:** Paranoia, algo sempre observando
**Implementação Niagara:**
- Dark sprite particles
- Spawn Rate: 1-5 partículas/segundo
- Lifetime: 5-15 segundos
- Velocity: Subtle, erratic movement
- LOD: Medium e High apenas

## PERFORMANCE GUIDELINES

### LOD IMPLEMENTATION
- **LOD 0 (High):** Todos os efeitos ativos, máxima qualidade
- **LOD 1 (Medium):** Redução de 50% no spawn rate, simplificação de materiais
- **LOD 2 (Low):** Apenas efeitos essenciais, 25% do spawn rate original
- **LOD 3 (Culled):** Todos os efeitos desativados

### DISTANCE CULLING
- **0-500m:** LOD 0
- **500-1500m:** LOD 1
- **1500-3000m:** LOD 2
- **3000m+:** LOD 3

### PERFORMANCE BUDGET
- Target: 0.5ms per frame para todos os efeitos atmosféricos
- Maximum: 50 partículas ativas simultaneamente por sistema
- Fallback: Disable em hardware de baixa performance

## INTEGRATION WITH GAMEPLAY

### STEALTH MECHANICS
Efeitos atmosféricos devem reagir à presença do jogador:
- Dust motes se dispersam quando o jogador passa
- Insect swarms fogem quando há movimento brusco
- Wind effects intensificam durante momentos de tensão

### DINOSAUR PRESENCE INDICATION
Efeitos subtis que indicam presença de dinossauros:
- Birds suddenly taking flight (particle burst)
- Insects going silent (effect pause)
- Dust patterns changing direction (predator movement)

---

*Atmospheric Effects Design Document*
*VFX Agent — Transpersonal Game Studio*
*Março 2026*