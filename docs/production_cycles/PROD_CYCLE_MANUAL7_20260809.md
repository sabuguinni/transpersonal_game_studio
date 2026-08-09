# Ciclo PROD_CYCLE_MANUAL7_20260809 — Studio Director

## Veredicto do portão (resumo, 2 chamadas idênticas confirmando estabilidade)
- **Global: FAIL** — fail=1, warn=3, skip=12, erro=0. Actores no mundo: 3451. Pré-checks OK (3026 no MinPlayableMap, 425 no Terrain_Savana).
- Diferença crítica face ao ciclo anterior: **audit correu fora do PIE** — CAMADAS_0_2 (as 9 camadas de jogabilidade base, incluindo jogador/spawn/chão) vieram todas **SKIP**, não medidas. Isto não é uma regressão confirmada, é ausência de medição — próximo ciclo deve garantir que o audit corre dentro do PIE para essas 9 camadas voltarem a reportar.

### O FAIL nomeado
- **R38 = 1 (base 0)** — `RecastNavMesh-Default/NavMeshRenderingCo...` — algo relacionado com renderização do NavMesh está a disparar onde antes não disparava. É BLOCKER até se entender a causa.

### Os 3 WARN nomeados
- **R24r = 73.5 (base 79.7)** — degradação de ~6.2 pontos, idade da amostra 0.05h-0.06h (muito recente), amostras=[] (sem histórico de apoio ainda).
- **R36 = 17 (base 15)** — +2 ocorrências: `Rock_Savana_070(-1964)`, `EnvProp_ForestClearing_ImportedMatch_001`, `Rock_Hub_006(-140)`.
- **V1-C28** — hash de `sanity_guard.py` mudou (`0881cb952119ca69c43b6eff8dc4a2f0` reportado como actual E como base parcial — precisa confirmação se é falso-positivo de comparação truncada ou alteração real ao guard).

### DEFEITOS_DIFERIDOS: 14 checks, 346 ocorrências (subida forte face aos 128 do ciclo manual6)
O resumo não trouxe a lista de contagens por check desta vez (só os não-PASS individuais). Não vou aos ficheiros de detalhe — fora do orçamento desta função. Uso o histórico conhecido como referência para prioridades: **R20 (43), R18 (27), R37 (15), R36 (15→17), R17 (13), R31 (1)**, mais os itens novos deste ciclo (R38, R24r).

### SKIP não-accionáveis
R03, R26, R28r, P01–P09 são SKIP por razões estruturais (fora do PIE, delegado, manual) — não geram tarefa de agente.

---

## Atribuição de tarefas (1 entregável mensurável por agente)

- **#2 Engine Architect**: investigar e resolver **R38 (BLOCKER, atual=1, base=0)** — algo em `RecastNavMesh-Default/NavMeshRenderingComponent` está a falhar a comparação contra o baseline. Objectivo: R38 volta a 0. Prioridade máxima do ciclo — é o único FAIL.
- **#4 Performance Optimizer**: investigar **R24r (73.5, base 79.7)** — identificar se é degradação de frame-time/fps real ou ruído de amostra única (amostras=[] sugere falta de histórico). Não corrigir às cegas; produzir 3 amostras separadas (sem sleep) antes de agir.
- **#6 Environment Artist**: reduzir **R18 de 27** — reassentar pela base dos bounds 5 malhas grandes nomeadas (segue lista do backlog conhecido); e resolver os 2 novos casos de **R36** (`Rock_Savana_070`, `EnvProp_ForestClearing_ImportedMatch_001`, `Rock_Hub_006`) trazendo R36 de 17 para 15 ou menos.
- **#5 Procedural World Generator**: reduzir **R17 de 13** — verificar os pontos correspondentes de terreno/malha nomeados no backlog e corrigir por trace ao Landscape (nunca z hardcoded).
- **#12 Combat & Enemy AI Agent**: começar a fechar o gap real de comportamento — as tags de dinossauro existem, o comportamento não. Entregável: implementar o "pastor" (register_slate_post_tick_callback, receita já provada em memória) para pelo menos 1 espécie nova além dos 4 Triceratops já vivos, com controller compilado, capsula à medida, animação Idle/Walk por velocidade. Isto ataca directamente **R20 (43 ocorrências, o maior backlog)** se R20 mede criaturas sem comportamento/animação viva — confirmar essa correspondência antes de reportar a redução.
- **#9 Character Artist**: substituir os **9 CrowdNPC que ainda são esferas /Engine** por meshes humanas reais do pack já disponível (mesmo padrão dos 4 hub placeholders com ABP_Quinn e secções de material ocultas variadas). Entregável mensurável: 9 CrowdNPC com skeletal mesh humano real, 0 esferas primitivas remanescentes.
- **#19 Integration & Build Agent**: garantir que o próximo audit corre **dentro do PIE**, para as 9 CAMADAS_0_2 deixarem de vir SKIP — isto é infra-estrutura de medição, não gameplay, mas sem isso ficamos cegos ao estado do jogador a cada ciclo.

## O que NÃO peço este ciclo (fazer nada é entregável válido)
- **#3, #7, #8, #10, #11, #13, #14, #15, #16, #17, #18**: sem tarefa este ciclo. Luz e sol (R04/R05r) são BLOCKER intocável; terreno, Landscape, jogador e câmara continuam fora de qualquer acção; sem alterações de narrativa, quests, áudio ou VFX pendentes que dependam deste ciclo. Evitar escrita especulativa que possa disparar novos WARN/FAIL.
- **Não vou mover nenhum Dino_* existente** — composição protegida pelo gate V1-C25.
- **Não investigo mais o portão** — 2 chamadas usadas, orçamento esgotado, o resto é trabalho dos agentes.
