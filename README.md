# Transpersonal Game Studio — Jurassic Survival Game

## Visão Geral
Jogo de sobrevivência em mundo aberto ambientado no período Jurássico/Cretáceo, onde um paleontologista transportado no tempo deve sobreviver entre dinossauros enquanto procura uma gema para regressar ao presente.

## Estrutura do Projecto

```
TranspersonalGame/
├── Source/                     # Código C++ do jogo
│   ├── TranspersonalGame/      # Módulo principal
│   ├── Core/                   # Sistemas core (física, colisão)
│   ├── World/                  # Geração procedural de mundo
│   ├── Characters/             # Personagens e NPCs
│   ├── AI/                     # Sistemas de IA
│   └── Audio/                  # Sistemas de áudio
├── Content/                    # Assets do jogo
│   ├── Maps/                   # Níveis e mapas
│   ├── Characters/             # Modelos de personagens
│   ├── Environment/            # Assets ambientais
│   ├── Audio/                  # Ficheiros de áudio
│   └── VFX/                    # Efeitos visuais
├── Config/                     # Configurações do projecto
├── Plugins/                    # Plugins customizados
├── Build/                      # Scripts de build
└── Documentation/              # Documentação técnica
```

## Build Pipeline

### Configurações Suportadas
- **Debug**: Desenvolvimento com símbolos completos
- **Development**: Testes e iteração rápida
- **Test**: QA e testes internos
- **Shipping**: Build final para distribuição

### Plataformas Alvo
- Windows (primária)
- Consolas (futuro)

## Integração Contínua

O projecto utiliza um sistema de integração baseado em:
1. **Source Control**: Git com GitHub
2. **Build Automation**: Scripts UAT customizados
3. **Testing**: Automated testing pipeline
4. **Packaging**: Multi-platform packaging system

## Comandos Rápidos

```bash
# Build completo
./Build/BuildGame.bat Development

# Cook content
./Build/CookContent.bat Windows

# Package para Windows
./Build/PackageGame.bat Windows Development

# Run tests
./Build/RunTests.bat
```

## Agentes da Cadeia de Produção

1. **#01 Studio Director** — Coordenação geral
2. **#02 Engine Architect** — Arquitectura técnica
3. **#03 Core Systems** — Sistemas base
4. **#04 Performance Optimizer** — Optimização
5. **#05 World Generator** — Geração procedural
6. **#06 Environment Artist** — Arte ambiental
7. **#07 Architecture Agent** — Estruturas
8. **#08 Lighting Agent** — Iluminação
9. **#09 Character Artist** — Personagens
10. **#10 Animation Agent** — Animações
11. **#11 NPC Behavior** — Comportamento NPCs
12. **#12 Combat AI** — IA de combate
13. **#13 Crowd Simulation** — Simulação de massas
14. **#14 Quest Designer** — Missões
15. **#15 Narrative Agent** — Narrativa
16. **#16 Audio Agent** — Áudio
17. **#17 VFX Agent** — Efeitos visuais
18. **#18 QA Agent** — Testes
19. **#19 Integration Agent** — Integração (este agente)

## Contacto
Transpersonal Game Studio — Março 2026