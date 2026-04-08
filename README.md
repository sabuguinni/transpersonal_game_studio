# Transpersonal Game Studio — Jurassic Survival Project

## Visão Geral
Jogo de sobrevivência em mundo aberto ambientado no período Jurássico/Cretáceo, onde um paleontologista transportado no tempo deve sobreviver entre dinossauros.

## Conceito Central
- **Género:** Survival de mundo aberto
- **Escala:** Regional — uma região selvagem do período pré-histórico
- **Sensação:** Medo constante — o jogador é sempre a presa, nunca o predador

## Características Únicas
1. **Dinossauros com vidas próprias** — rotinas independentes do jogador
2. **Domesticação de espécies não-ameaçadoras** — processo lento baseado em confiança
3. **Cada dinossauro é único** — variações físicas genuínas dentro da mesma espécie

## Estrutura do Projeto

```
Source/
├── TranspersonalGame/           # Código principal do jogo
│   ├── Core/                    # Sistemas fundamentais
│   ├── World/                   # Geração procedural e ambiente
│   ├── Characters/              # Personagens e NPCs
│   ├── AI/                      # Sistemas de IA
│   ├── Survival/                # Mecânicas de sobrevivência
│   └── UI/                      # Interface do usuário
Content/
├── Maps/                        # Níveis e mapas
├── Characters/                  # Assets de personagens
├── Environment/                 # Assets ambientais
├── Audio/                       # Áudio e música
├── VFX/                         # Efeitos visuais
└── UI/                          # Interface
Config/                          # Configurações do projeto
Plugins/                         # Plugins customizados
```

## Build Status
- **Engine:** Unreal Engine 5.4+
- **Target Platforms:** Windows (primary), consoles (future)
- **Build Configuration:** Development (current)

## Agentes da Cadeia de Produção
Este projeto é desenvolvido por uma cadeia de 19 agentes especializados:

1. **#01 — Studio Director:** Coordenação geral e comunicação com o Miguel
2. **#02 — Engine Architect:** Arquitectura técnica e regras de desenvolvimento
3. **#03 — Core Systems Programmer:** Física, colisão, ragdoll, destruição
4. **#04 — Performance Optimizer:** Garantia de 60fps PC / 30fps consola
5. **#05 — Procedural World Generator:** Terrenos, biomas, PCG
6. **#06 — Environment Artist:** Vegetação, rochas, props, materiais
7. **#07 — Architecture & Interior Agent:** Edifícios e estruturas
8. **#08 — Lighting & Atmosphere Agent:** Ciclo dia/noite, clima, Lumen
9. **#09 — Character Artist Agent:** Personagens com MetaHuman Creator
10. **#10 — Animation Agent:** Motion Matching e IK de pés
11. **#11 — NPC Behavior Agent:** Behavior Trees e rotinas diárias
12. **#12 — Combat & Enemy AI Agent:** IA de combate tático
13. **#13 — Crowd & Traffic Simulation:** Mass AI até 50.000 agentes
14. **#14 — Quest & Mission Designer:** Missões e objectivos
15. **#15 — Narrative & Dialogue Agent:** Bible do Jogo e história
16. **#16 — Audio Agent:** MetaSounds e ElevenLabs
17. **#17 — VFX Agent:** Niagara com LOD chain
18. **#18 — QA & Testing Agent:** Testes e validação
19. **#19 — Integration & Build Agent:** Integração final e builds

## Workflow de Integração
- **Ordem de dependências:** Engine → Core → World → Environment → Characters → AI → Audio → VFX → QA → Integration
- **Builds mantidas:** Últimas 10 builds funcionais para rollback
- **QA tem poder de bloqueio:** Nenhuma build avança sem aprovação do QA

## Próximos Passos
1. Aguardar Arquitectura Técnica do Agente #02
2. Implementar sistemas base conforme especificações
3. Integrar outputs dos agentes criativos após aprovação da Bible do Jogo
4. Manter builds funcionais para teste contínuo

---
*Transpersonal Game Studio — Março 2026*