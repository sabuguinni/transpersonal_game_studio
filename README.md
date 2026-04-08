# Transpersonal Game Studio — Jurassic Survival Project

## Visão Geral
Jogo de sobrevivência em mundo aberto ambientado no período Jurássico/Cretáceo, onde um paleontologista transportado no tempo deve sobreviver entre dinossauros enquanto procura uma gema para retornar ao presente.

## Estrutura do Projeto

```
TranspersonalGame/
├── Source/                     # Código C++ do jogo
│   ├── TranspersonalGame/      # Módulo principal
│   ├── Core/                   # Sistemas centrais
│   ├── Gameplay/               # Sistemas de gameplay
│   └── AI/                     # Sistemas de IA
├── Content/                    # Assets do jogo
│   ├── Blueprints/             # Blueprints
│   ├── Maps/                   # Níveis/mapas
│   ├── Characters/             # Personagens
│   ├── Environment/            # Ambiente
│   ├── Audio/                  # Áudio
│   └── VFX/                    # Efeitos visuais
├── Config/                     # Configurações
├── Plugins/                    # Plugins customizados
├── Build/                      # Scripts de build
├── Documentation/              # Documentação técnica
└── Integration/                # Scripts de integração
```

## Status do Desenvolvimento

**Ciclo Atual:** PROD_JURASSIC_001
**Fase:** Infraestrutura Base
**Engine:** Unreal Engine 5.4+

## Agentes da Cadeia de Produção

1. **#01 — Studio Director**: Coordenação geral
2. **#02 — Engine Architect**: Arquitetura técnica
3. **#03 — Core Systems Programmer**: Sistemas base
4. **#04 — Performance Optimizer**: Otimização
5. **#05 — Procedural World Generator**: Geração procedural
6. **#06 — Environment Artist**: Arte ambiental
7. **#07 — Architecture & Interior Agent**: Arquitetura
8. **#08 — Lighting & Atmosphere Agent**: Iluminação
9. **#09 — Character Artist Agent**: Personagens
10. **#10 — Animation Agent**: Animações
11. **#11 — NPC Behavior Agent**: Comportamento NPCs
12. **#12 — Combat & Enemy AI Agent**: IA de combate
13. **#13 — Crowd & Traffic Simulation**: Simulação de massas
14. **#14 — Quest & Mission Designer**: Missões
15. **#15 — Narrative & Dialogue Agent**: Narrativa
16. **#16 — Audio Agent**: Áudio
17. **#17 — VFX Agent**: Efeitos visuais
18. **#18 — QA & Testing Agent**: Testes
19. **#19 — Integration & Build Agent**: Integração

## Build e Integração

### Requisitos
- Unreal Engine 5.4+
- Visual Studio 2022
- Git LFS para assets grandes
- Perforce (opcional, para equipes grandes)

### Build Rápido
```bash
# Clone do repositório
git clone https://github.com/sabuguinni/transpersonal_game_studio.git
cd transpersonal_game_studio

# Gerar arquivos de projeto
./GenerateProjectFiles.bat

# Build do projeto
./Build/BuildGame.bat Development
```

### Integração Contínua
- Builds automáticos a cada commit
- Testes de QA automáticos
- Deploy automático para staging
- Rollback automático em caso de falha

## Contato
**Studio Director:** Miguel Martins
**Integration Lead:** Agente #19

---
*Transpersonal Game Studio — Março 2026*