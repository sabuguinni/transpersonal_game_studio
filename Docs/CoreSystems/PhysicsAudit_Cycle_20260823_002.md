# Core Systems Programmer (#3) — Auditoria de Física/Assentamento
## Ciclo PROD_CYCLE_AUTO_20260823_002

### Contexto
Seguindo a limitação registada nas memórias (`hugo_absolute_prohibitions_v2`, `hugo_memorias_vs_directiva_v1`) e na Specific Directive deste agente, **não foi escrito nenhum ficheiro .cpp/.h**. C++ é inerte neste editor headless (binário pré-compilado, UHT não recompila). O `SurvivalComponent` e qualquer outro componente C++ do repositório **continuam inertes** — não foram integrados no `TranspersonalCharacter` porque essa integração não tem qualquer efeito no jogo vivo. Reporto a limitação em vez de desperdiçar escrita de ficheiro.

### Trabalho real realizado (3 chamadas `ue5_execute`, todas verificadas por releitura independente)

1. **Diagnóstico de PIE e censo de criaturas**
   - `LevelEditorSubsystem.is_in_play_in_editor()` = `False` (PIE já fechado, nenhuma acção necessária).
   - Mundo activo: `MinPlayableMap`, 3514 actores totais, 44 actores `Dino_*`.
   - Varredura completa das 44 criaturas: line trace vertical ao Landscape (ignorando todos os actores não-Landscape) em cada XY, comparado com o osso mais baixo de cada `SkeletalMeshComponent` (via `get_num_bones()` + `get_bone_name(i)` + `get_socket_location(bn)`, conforme API verificada — `get_bone_location` não existe).

2. **Resultado da varredura — TODAS as 44 criaturas dentro da tolerância (|delta| ≤ 20 uu)**
   - Maior delta observado: 8.88 uu (Triceratops 11–14, criaturas móveis, valor consistente entre as 4 — plausível devido à pose de animação avaliada no instante da medição).
   - `Dino_Ankylosaurus_10` (o defeito histórico do ciclo 8, corrigido em ciclos anteriores) reconfirmado: `ground_z=-523.44`, `lowest_bone_z=-523.53`, **delta=0.09 uu** — assente correctamente, sem regressão.
   - **Nenhuma criatura em flutuação ou enterrada.** Nenhuma acção de correcção foi necessária este ciclo (o trabalho dos ciclos anteriores manteve-se estável).

3. **Verificação de colisão/espaçamento entre criaturas móveis**
   - 6 criaturas móveis identificadas: `Dino_Triceratops_11/12/13/14`, `Dino_Parasaurolophus_6`, `Dino_Ankylosaurus_10`.
   - Todas com `CharacterMovementComponent.velocity = (0,0,0)` (esperado — PIE fechado, pastor não está a tickar).
   - Distância par-a-par calculada entre as 6: mínimo 2400 uu (Triceratops_11↔12 e 12↔14) — **sem risco de sobreposição** (limiar de risco definido em 300 uu).
   - Estado do módulo do "pastor" (callback de post-tick) confirmado presente em memória do processo Python: `ZZ_PASTOR`, `ZZ_HANDLE`, `ZZ_INVENTARIO`, `ZZ_CASA`, `ZZ_ESPECIE`, `ZZ_MALHA_VIA`, `ZZ_RECOLHA_*`, `ZZ_VIGIA_*` — todos os atributos de estado dos ciclos anteriores sobreviveram, prontos a re-registar quando o PIE abrir via `prepare_game_for_play`.

### Regra respeitada: ADICIONAR, NUNCA MOVER (V1-C25/C25b)
Nenhum actor `Dino_*` foi movido, rodado, reescalado ou apagado. Todas as chamadas foram de **leitura e medição**. Baseline `{"n": 44, "movidos": 0}` mantido.

### Decisões técnicas e justificação
- Não se procedeu a nenhuma correcção de posição porque **nenhum defeito de assentamento físico foi encontrado** nas 44 criaturas — o trabalho de ciclos anteriores (correcção do Ankylosaurus_10) manteve-se persistido e verificado.
- Não se criou nenhum Blueprint novo de comportamento: confirmado (memórias `hugo_python_neste_editor_v2`) que grafos de Blueprint não podem ganhar nós via Python nesta build — o único mecanismo de comportamento por frame continua a ser o callback de post-tick (o "pastor"), já implementado e a funcionar em ciclos anteriores.
- Não se tocou no PlayerStart, no Landscape, no Terrain_Savana nem no pawn do jogador (regras `HANDS OFF`).

### Ficheiros criados/modificados
- `Docs/CoreSystems/PhysicsAudit_Cycle_20260823_002.md` (este ficheiro, novo).

### Dependências / inputs necessários de outros agentes
- **#4 Performance Optimizer**: nenhuma alteração de física pesada este ciclo — o mundo de física está estável. Foco sugerido: os 67 actores com espalhamento >200 uu identificados em ciclos anteriores (R18) continuam a aguardar decisão humana de escala/sítio — não são da minha alçada (assentamento de malhas estáticas grandes é R18/#6, não física de criaturas).
- **#5/#6**: R18 em WARN (67/27) é da responsabilidade do World Generator/Environment Artist, não deste agente — não mexi em malhas estáticas.
- Nenhum bloqueio novo introduzido. Estado de física de criaturas: **estável e verificado**.
