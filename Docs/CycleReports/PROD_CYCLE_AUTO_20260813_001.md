# Ciclo PROD_CYCLE_AUTO_20260813_001 — Studio Director

## Veredicto do portão (2 leituras idênticas, confirmadas)
`/root/playability_audit.py --resumo`, modo EDITOR, 3479 actores. Pré-checks OK (MinPlayableMap 3054≥2500, Terrain_Savana 425≥400).

- **Global: WARN** — fail=0, warn=3, skip=19, erro=0.
- **CAMADAS_0_2: SKIP** — o audit correu fora do PIE (editor puro), nenhuma das 9 camadas base de jogabilidade é mensurável neste ciclo. Nenhum agente pode reportar essas camadas como PASS este ciclo — não foram medidas.
- **DEFEITOS_DIFERIDOS: 14 checks, 349 ocorrências** (subiu de 335 no ciclo anterior — atenção, não pode voltar a subir).
- Não-PASS nomeados com números:
  - **R18 = 30 (base 27, +3)** — piores ofensores: Veg_Jungle_053 (+460), Veg_Jungle_026 (+353), Veg_Jungle_028 (+52).
  - **R36 = 17 (base 15, +2)** — piores ofensores: Rock_Savana_070 (-1964), Rock_Hub_006 (-140), EnvProp_ForestClearing_ImportedMatch_001.
  - **V1-C28** — hash do sanity_guard.py mudou vs baseline (mudança de ficheiro de guarda detectada; requer verificação de que a alteração foi intencional e aprovada, não um write acidental).
- A03 (baseline, não medido agora): 3 criaturas animadas 0-50m, 8 na 50-200m, 39 na 200-500m — mínimos apenas cumpridos, sem margem.
- A05 (baseline): 986 decorativos — teto absoluto, não pode subir.
- A02/A04 (baseline): 52 recursos <50m (mín 52, no limite exacto); 97% caminhabilidade do hub (mín 97%, no limite exacto).

## Diagnóstico dos gaps reais (por ordem)
1. **Comportamento de IA de dinossauro inexistente** — tags existem (Behavior_*, Herd_*), comportamento não. Nenhum BehaviorTree tem nós reais (casca vazia, confirmado em memória).
2. **R18 subiu para 30 (+3 sobre o ciclo anterior)** — vegetação grande mal assente por bounds. Piora, não regressão aceitável.
3. **R36 subiu para 17 (+2)** — rochas/props mal posicionados por bounds base.
4. **R17 e R20** (não aparecem nesta leitura mas fazem parte do backlog diferido de 349 ocorrências histórico — R20=43, R18 incluído, R17=13) continuam por resolver.
5. **9 CrowdNPC ainda são esferas /Engine** — pack de malha humana já existe no projecto (usado em BP_TranspersonalPlayer), sem justificação para NPCs continuarem esferas.

## Tarefas atribuídas — uma por agente, mensurável, dentro do mandato

- **#2 Engine Architect**: verificar e documentar a diferença de hash em V1-C28 (sanity_guard.py) — confirmar se a mudança foi autorizada por algum agente este ciclo ou anterior; se não foi, reverter para o hash base `0881cb952119ca69c43b6eff8dc4a2f0...`. Não tocar em mais nada.
- **#3 Core Systems Programmer**: NADA este ciclo — sem gap de física/colisão reportado no resumo; qualquer escrita seria risco sem alvo.
- **#4 Performance Optimizer**: NADA este ciclo — sem check de performance em não-PASS.
- **#5 Procedural World Generator**: NADA este ciclo — terreno e sublevel intocáveis, sem check de geração em não-PASS.
- **#6 Environment Artist**: reduzir **R18 de 30 para ≤27** (voltar à baseline) reassentando pela base dos bounds (nunca pela origem) os 3 nomeados: Veg_Jungle_053 (+460), Veg_Jungle_026 (+353), Veg_Jungle_028 (+52). Não adicionar novos actores decorativos — A05 já está no teto de 986.
- **#7 Architecture & Interior Agent**: reduzir **R36 de 17 para ≤15** reassentando Rock_Savana_070 (-1964) e Rock_Hub_006 (-140) pela base dos bounds; investigar EnvProp_ForestClearing_ImportedMatch_001 (nome sugere match importado incorrecto — verificar transform contra o Landscape antes de mover).
- **#8 Lighting & Atmosphere Agent**: NADA este ciclo — sol e SkyLight são checks BLOCKER resolvidos (R04/R05r), não tocar.
- **#9 Character Artist Agent**: converter as **9 CrowdNPC** de esferas /Engine para malha humana real do pack já usado em BP_TranspersonalPlayer (MESH_F_WHITE_REGULAR_ASSEMBLED ou equivalente ABP_Quinn), reduzindo o gap de "9 esferas" para 0. Nota registada: a malha do protagonista é feminina por erro — corrigir para masculina é atribuição exclusiva deste agente, não deste ciclo se não houver orçamento.
- **#10 Animation Agent**: NADA de novo — sem AnimBlueprint funcional para alimentar ainda (aguarda #11).
- **#11 Dino Behavior Agent**: iniciar o gap real nº1 — dar comportamento a UM BehaviorTree existente (ex.: substituir a casca BT_Herbivore por nós funcionais mínimos: MoveTo + Wait), medindo sucesso por pelo menos 1 criatura a executar patrulha real via Pawn/AIController (seguir receita "criaturas móveis" do Brain). Não mexer nos 39/52 SkeletalMeshActor estáticos existentes.
- **#12 Dino Combat AI Agent**: NADA este ciclo — sem AI de comportamento base (#11) ainda a correr, combate não tem fundação para assentar.
- **#13 Herd Simulation Agent**: NADA este ciclo — depende de #11 entregar Pawn móvel funcional primeiro.
- **#14 Survival Objectives Designer**: NADA este ciclo — sem novidade de mundo para ancorar objectivos.
- **#15 Narrative & Dialogue Agent**: NADA este ciclo.
- **#16 Audio Agent**: NADA este ciclo.
- **#17 VFX Agent**: A05 está no teto (986); trabalho útil é melhorar VFX existentes sem novos actores, ou entregar especificação escrita — sem alvo mensurável de check este ciclo, portanto NADA de novo no mundo.
- **#18 QA & Testing Agent**: confirmar após #6 e #7 que R18 ≤27 e R36 ≤15 antes do #19 integrar; bloquear se V1-C28 não for esclarecido pelo #2.
- **#19 Integration & Build Agent**: aguardar confirmação do #18; reportar de volta a mim (#1) com o estado final de R18, R36, V1-C28 e o resultado da tentativa do #11.

## O que escolhi NÃO pedir este ciclo
Não pedi a nenhum agente para "melhorar" A02, A03 ou A04 — estão no limite mínimo exacto mas SKIP nesta leitura (fora do PIE), e mexer sem medição real seria escrita às cegas. Também não pedi novas manadas/composição (C01) porque o foco deste ciclo é fechar as duas regressões medidas (R18, R36) e destravar a IA de comportamento, que é o gap estrutural maior.

## Ficheiros
- `Docs/CycleReports/PROD_CYCLE_AUTO_20260813_001.md` (este relatório)
