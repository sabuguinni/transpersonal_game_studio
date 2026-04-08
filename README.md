# Transpersonal Game Studio — Jurassic Survival Game

## Visão Geral
Jogo de sobrevivência em mundo aberto ambientado no período Jurássico/Cretáceo. O jogador é um paleontologista transportado no tempo que deve sobreviver num mundo dominado por dinossauros.

## Conceito Central
- **Medo constante**: O jogador é sempre a presa, nunca o predador
- **Dinossauros com vida própria**: Rotinas independentes, comportamento realista
- **Domesticação gradual**: Alguns herbívoros podem ser domesticados com paciência
- **Individualidade**: Cada dinossauro é único e reconhecível

## Estrutura do Projeto

```
TranspersonalGame/
├── Source/                 # Código C++ do jogo
│   ├── TranspersonalGame/  # Módulo principal
│   ├── Core/              # Sistemas centrais
│   ├── AI/                # Sistemas de IA
│   ├── World/             # Geração procedural
│   └── Player/            # Sistemas do jogador
├── Content/               # Assets do jogo
│   ├── Blueprints/        # Blueprints
│   ├── Maps/              # Níveis/mapas
│   ├── Materials/         # Materiais
│   ├── Meshes/           # Modelos 3D
│   ├── Textures/         # Texturas
│   ├── Audio/            # Áudio
│   └── VFX/              # Efeitos visuais
├── Config/               # Configurações
├── Plugins/              # Plugins customizados
└── Build/                # Scripts de build
```

## Tecnologias Core
- **Engine**: Unreal Engine 5.4+
- **Rendering**: Lumen (iluminação global)
- **World**: World Partition + PCG
- **AI**: Mass AI + Behavior Trees
- **Animation**: Motion Matching + IK
- **Audio**: MetaSounds
- **VFX**: Niagara com LOD chain

## Performance Targets
- **PC**: 60 FPS (1080p/1440p)
- **Console**: 30 FPS estável
- **Crowd Simulation**: Até 50.000 agentes simultâneos

## Build Status
🔄 **Em Desenvolvimento** — Ciclo PROD_JURASSIC_001

## Agentes da Cadeia de Produção
1. Studio Director → 2. Engine Architect → 3. Core Systems → 4. Performance Optimizer → 5. World Generator → 6. Environment Artist → 7. Architecture & Interior → 8. Lighting & Atmosphere → 9. Character Artist → 10. Animation → 11. NPC Behavior → 12. Combat & Enemy AI → 13. Crowd Simulation → 14. Quest & Mission Designer → 15. Narrative & Dialogue → 16. Audio → 17. VFX → 18. QA & Testing → **19. Integration & Build** ← VOCÊ ESTÁ AQUI

---
*Transpersonal Game Studio — Março 2026*