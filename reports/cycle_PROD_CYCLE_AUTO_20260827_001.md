# Studio Director — Ciclo PROD_CYCLE_AUTO_20260827_001

## Estado do portão (via /root/playability_audit.py --resumo)
- Global: **WARN** | fail=0 warn=2 skip=12 erro=0 | 3555 actores (modo EDITOR)
- Pré-checks OK (MinPlayableMap 3130/2500, Terrain_Savana 425/400)
- Não-PASS nomeados:
  - **R01** WARN: 3555 vs base 3482 (+73, crescimento normal, não bloqueante)
  - **R18** WARN: 67 vs base 27 — ofensores nomeados: `Veg_Jungle_053` (+460 uu vão), `Veg_Jungle_026` (+353), `JungleVeg_025` (+87). Vãos sob a pegada de vegetação, espalhamento provavelmente >200 (não mover por Z sem medir espalhamento primeiro).
- DEFEITOS_DIFERIDOS: 14 checks, 240 ocorrências acumuladas (R18=67, R20=43, R42=~65, e outros menores conforme RETRATO).
- CAMADAS_0_2: 8 de 9 não mensuráveis fora do PIE (normal, não é falha).
- PIE estava fechado no início do ciclo — verificado e mantido fechado (nenhuma leitura cega).

## RETRATO automático (mais recente, prioridade sobre auto-relatos)
- A02 (recursos <50m): 52 — no piso (mínimo 52), sem margem.
- A03 (criaturas animadas por banda): 0-50m=3 (mínimo 3, sem margem), 50-200m=8 (mínimo 8, sem margem), 200-500m=39 (mínimo 39, sem margem). Layer 3 está TODA no limiar exacto — qualquer regressão bloqueia.
- A04 (caminhabilidade hub): 97% (mínimo 97, sem margem).
- A05 (tecto decoração): 951/986 — ainda há 35 de margem, mas perto do tecto.
- A06 (distância à criatura animada mais próxima): 12 m (medição mais recente do RETRATO; versões anteriores diziam 20.3m — a mais fresca vence). Continua a ser o alvo de maior valor: aproximar mais é a única forma de subir C01/A06 sem tocar em Dino_ existentes.

## VISUAL FEEDBACK APPLIED
- **Issue detectado:** o feedback visual do último screenshot aponta um erro de credibilidade científica — plantas de bananeira (género pós-Cretáceo) visíveis em todos os planos largos, ao lado de cicas/samambaias/coníferas correctas. Isto quebra o rigor científico que é pilar do jogo (ver REGRA ANTI-ALUCINAÇÃO / KB). Secundariamente: céu sobreexposto no horizonte, falta de neblina volumétrica, terreno macro demasiado plano.
- **Acção dirigida ao agente #6 (Environment Artist):** localizar e substituir TODAS as plantas do tipo banana/tropical anacrónico por espécies período-correctas do Cretáceo (cicadáceas, samambaias arbóreas, coníferas tipo Araucaria) já disponíveis nos packs do projecto. NÃO apagar por coordenada — localizar por asset/label, confirmar que não têm tags funcionais que as protejam de outra forma (ver hugo_asset_first_v8), e trocar a malha por uma espécie period-accurate. Reportar quantas trocou e onde.
- **Acção dirigida ao agente #8 (Lighting & Atmosphere):** ajustar densidade do ExponentialHeightFog para reduzir a sobreexposição do céu no horizonte e dar profundidade atmosférica. NÃO tocar no sol nem no SkyLight (R04/R05r são BLOCKER, resolvidos e protegidos) — apenas fog density/height falloff.
- Nota: terreno "demasiado plano" (macro relevo) NÃO é tarefa de nenhum agente este ciclo — o Landscape é intocável (regra hugo_terreno_e_assentamento_v1); reportar como limitação estrutural conhecida, não atribuir trabalho.

## Atribuições deste ciclo (uma entrega concreta por agente, verificável por check nomeado)

- **#5 Procedural World Generator:** NÃO fazer spawn de criaturas novas fora do corredor de showcase este ciclo — o gap real é aproximar vida do spawn (A06=12m), não aumentar contagem total. Se decidir agir, adicionar 1 criatura animada dentro do corredor visível (cone yaw 320±45 do PlayerStart, até ~150m) a menos de 12m de distância actual do spawn, respeitando espaçamento ≥2400uu de outros indivíduos. Verificar via A06 (deve descer abaixo de 12m) e A03 banda 0-50m (deve subir acima de 3).
- **#6 Environment Artist:** substituir plantas de bananeira por cicadáceas/samambaias arbóreas/Araucaria period-accurate (ver Visual Feedback acima). NÃO tocar em Veg_Jungle_053/026/JungleVeg_025 por Z sem antes medir o espalhamento sob a pegada (regra R18/R36) — se espalhamento >200uu, reportar em vez de mover. Verificar por inspecção visual e por R18 (não deve piorar de 67).
- **#7 Architecture & Interior:** sem tarefa este ciclo — nenhum gap identificado nesta camada.
- **#8 Lighting & Atmosphere:** ajustar ExponentialHeightFog (density/height falloff) para reduzir sobreexposição do céu no horizonte, sem tocar no sol (R04) nem SkyLight (R05r). Verificar por comparação visual do próximo screenshot (redução do "washed out" no horizonte).
- **#9 Character Artist:** sem tarefa nova este ciclo — a substituição da malha feminina por masculina realista continua atribuída mas não é a prioridade máxima; se tiver capacidade, avançar, senão não fazer nada.
- **#10 Animation:** sem tarefa este ciclo.
- **#11 Dino Behavior Agent:** este é o maior gap estrutural do projecto — existem tags de comportamento mas nenhuma Behavior Tree tem nós reais (27 de 28 BT são cabeçalhos vazios, confirmado em memória). Como BT/ABT não se constroem via Python (API de nós não exposta), a entrega concreta é: usar o padrão de post-tick callback (register_slate_post_tick_callback) para dar a UMA espécie já com anim (ex: Triceratops) uma rotina simples de "olhar para o jogador quando este se aproxima a <15m" ou pequeno deslocamento local, funcionando apenas em PIE. Reportar contagem de ticks e ausência de erros, sem mover posição registada dos Dino_ (respeitar V1-C25).
- **#12 Dino Combat AI:** sem tarefa este ciclo — depende de #11 estabelecer primeiro comportamento base.
- **#13 Herd Simulation:** sem tarefa de execução (navegação está morta, medido); se quiser produzir, entregar apenas especificação escrita de coesão/dispersão para quando houver navmesh funcional.
- **#14 Survival Objectives:** sem tarefa este ciclo.
- **#15 Narrative & Dialogue:** sem tarefa este ciclo.
- **#16 Audio:** sem tarefa este ciclo.
- **#17 VFX:** NADA de novo — tecto de decoração A05 está em 951/986, margem de 35. Se tiver capacidade, melhorar VFX existentes sem acrescentar actores novos.
- **#18 QA & Testing:** verificar que a troca de flora do #6 não introduziu novos actores acima do tecto A05, e que R18 não piorou.
- **#19 Integration & Build:** consolidar mudanças deste ciclo numa build, confirmar R18 e A06 no relatório final.

## O que escolhi NÃO pedir este ciclo
- Não pedi ao #6 para tentar corrigir os vãos de R18 (Veg_Jungle_053/026/JungleVeg_025) por deslocação em Z sem primeiro medir o espalhamento — repetir esse erro (ciclo 9, R18 de 30→173) é pior que não tocar.
- Não pedi spawn de decoração nova a ninguém — A05 está a 35 de margem do tecto e o objectivo real é interactividade, não mais efeitos.
- Não pedi terraformação macro (relevo/rolling hills) — o Landscape é intocável por regra absoluta; fica registado como limitação conhecida, não como tarefa.

## Ficheiros criados/modificados
- `reports/cycle_PROD_CYCLE_AUTO_20260827_001.md` (este relatório)

## Ferramentas usadas
- 2x `ue5_execute` (dentro do orçamento de 2): leitura do portão (`playability_audit.py --resumo`) e verificação/fecho de PIE (estava fechado).
