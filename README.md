# Transpersonal Game Studio — Jurassic Survival Project

## Visão Geral
Jogo de sobrevivência em mundo aberto ambientado no período Jurássico/Cretáceo. O jogador é um paleontologista transportado no tempo que deve sobreviver num mundo dominado por dinossauros.

## Conceito Central
- **Medo constante** — o jogador é sempre a presa, nunca o predador
- **Dinossauros com vidas próprias** — rotinas independentes do jogador
- **Domesticação gradual** — alguns herbívoros podem ser domesticados
- **Cada dinossauro é único** — variações físicas identificáveis

## Estrutura do Projeto

```
Source/
├── TranspersonalGame/          # Código principal do jogo
│   ├── Core/                   # Sistemas fundamentais
│   ├── Dinosaurs/              # Sistema de dinossauros e IA
│   ├── Player/                 # Sistema do jogador
│   ├── World/                  # Geração procedural e ambiente
│   ├── Survival/               # Mecânicas de sobrevivência
│   └── UI/                     # Interface do usuário
├── TranspersonalGameEditor/    # Ferramentas de editor
└── Plugins/                    # Plugins customizados

Content/
├── Blueprints/                 # Blueprints do jogo
├── Maps/                       # Níveis e mapas
├── Art/                        # Assets visuais
├── Audio/                      # Assets de áudio
├── Materials/                  # Materiais e texturas
└── VFX/                        # Efeitos visuais

Config/                         # Configurações do projeto
Binaries/                       # Executáveis compilados
Intermediate/                   # Arquivos temporários de build
Saved/                          # Dados salvos e logs
```

## Tecnologias Core
- **Engine:** Unreal Engine 5.4+
- **Rendering:** Lumen (iluminação global)
- **World:** World Partition + PCG (Procedural Content Generation)
- **Animation:** Motion Matching + IK
- **AI:** Mass AI (até 50.000 agentes simultâneos)
- **Audio:** MetaSounds + ElevenLabs
- **VFX:** Niagara com LOD chain

## Build Status
- **Última Build:** Em desenvolvimento
- **Target Platforms:** Windows (primário), Console (futuro)
- **Performance Target:** 60fps PC / 30fps Console

## Cadeia de Produção
Este projeto segue uma cadeia de 19 agentes especializados:
1. Studio Director → 2. Engine Architect → ... → 19. Integration & Build Agent

## Começar Desenvolvimento
1. Clone o repositório
2. Instale Unreal Engine 5.4+
3. Abra `TranspersonalGame.uproject`
4. Compile o projeto

---
*Transpersonal Game Studio — Março 2026*