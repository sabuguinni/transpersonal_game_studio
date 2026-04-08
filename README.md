# Transpersonal Game Studio — Jurassic Survival Project

## Visão Geral
Jogo de survival em mundo aberto ambientado no período Jurássico/Cretáceo. O jogador é um paleontologista transportado no tempo que deve sobreviver num mundo dominado por dinossauros.

## Conceito Central
- **Género:** Survival de mundo aberto
- **Escala:** Regional — uma região selvagem do período pré-histórico
- **Sensação:** Medo constante — o jogador é sempre a presa, nunca o predador

## Características Únicas
1. **Dinossauros com vidas próprias** — rotinas independentes do jogador
2. **Domesticação de espécies não-ameaçadoras** — processo lento baseado em confiança
3. **Cada dinossauro é único** — variações físicas genuínas dentro da mesma espécie

## Estrutura do Projeto

### Diretórios Principais
- `Source/` — Código C++ do projeto
- `Content/` — Assets do jogo (Blueprints, materiais, meshes, etc.)
- `Config/` — Arquivos de configuração
- `Plugins/` — Plugins customizados
- `Build/` — Scripts de build e automação
- `Documentation/` — Documentação técnica e de design

### Agentes da Cadeia de Produção
Este projeto segue uma cadeia de produção com 19 agentes especializados:
1. Studio Director → 2. Engine Architect → 3. Core Systems → ... → 19. Integration & Build

## Requisitos Técnicos
- **Engine:** Unreal Engine 5.4+
- **Target Platforms:** Windows (primário), consolas (futuro)
- **Performance:** 60fps PC / 30fps consola
- **Tecnologias:** World Partition, Mass AI, Lumen, Niagara, MetaHuman

## Build e Integração
- **Source Control:** Git com GitHub
- **Build System:** Unreal Build Tool + custom scripts
- **CI/CD:** GitHub Actions
- **Packaging:** Automated builds para testing e distribuição

## Começar a Desenvolver
1. Clone o repositório
2. Instale Unreal Engine 5.4+
3. Execute `GenerateProjectFiles.bat`
4. Abra `TranspersonalGame.uproject`

---
*Transpersonal Game Studio — Março 2026*