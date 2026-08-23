# Studio Director — Ciclo PROD_CYCLE_AUTO_20260823_002

## Leitura do portão (CALL 1, repetida — resultado idêntico e estável)
Global: **WARN** | fail=0 | warn=2 | skip=12 | erro=0 | 3525 actores (modo EDITOR)
Pré-checks OK (MinPlayableMap 3100/2500, Terrain_Savana 425/400).

### Não-PASS nomeados
- **R01** WARN: 3525 vs baseline 3482 (+43 actores desde baseline — crescimento normal de conteúdo, sem problema).
- **R18** WARN: 67 vs baseline 27 (30 acima do que já foi corrigido em 13/08). Ofensores nomeados nesta leitura: `Veg_Jungle_053` (+460 uu de vão), `Veg_Jungle_026` (+353), `JungleVeg_025` (+87). Estes têm espalhamento sob a pegada que precisa de ser medido antes de qualquer correcção — NÃO assentar por Z se espalhamento > 200 uu (reportar em vez de mover).

### DEFEITOS_DIFERIDOS (RETRATO, mais completo que o resumo desta leitura)
14 checks / 240 ocorrências no resumo; RETRATO (mais recente, há 0 min) detalha: R18=67, R20=43(margem 0), R10=4(margem 0), R11=4(margem 0), R14=2(margem 0). Global no RETRATO também WARN, fail=0.

### Camadas 3 (as que medem experiência real do jogador, do RETRATO)
- **A02** (recursos/interactivos <50m, piso 52): valor = 52 → **NO PISO EXACTO**, não pode descer.
- **A03** (criaturas animadas por banda, pisos 3/8/39): valor pisos = 0-50:3, 50-200:8, 200-500:39 → **NO PISO EXACTO nas três bandas**. Zero margem.
- **A04** (caminhabilidade hub, piso 97%): valor piso=97% → **NO PISO EXACTO**.
- **A05** (tecto decoração 986): valor=950 decorativos → 36 de margem, não bloqueia.
- **A06** (distância à criatura animada mais próxima): dist_min_m=12, n_até_50m=7, n_até_100m=12. Melhorou face ao histórico (20.3m citado na directiva) — **direcção certa**, mas continua a ser o alvo a empurrar mais.

Isto significa: A02, A03 e A04 estão exactamente no piso. Qualquer regressão nesta camada bloqueia o ciclo. Zero tolerância para trabalho que reduza recursos, criaturas animadas ou caminhabilidade.

## VISUAL FEEDBACK APPLIED
- **Issue detectado**: céu sobre-exposto/lavado ("blown-out white sky flattens the scene") — PRIORITY FIX do ciclo anterior. Terreno predominantemente plano, falta variação de relevo/falésias. Casca das árvores demasiado pálida/dessaturada; pele dos dinossauros mistura-se com o ambiente (falta contraste).
- **Acção dirigida ao agente #8** (Lighting & Atmosphere): reduzir a intensidade do SkyLight em ~20% a partir do valor actual (3.5) SEM tocar no Sun_Main_Directional (R04/R05r são BLOCKER, intocáveis) — o problema é o SkyLight a lavar o céu, não o sol. Considerar volumetric clouds via SkyAtmosphere se disponível sem alterar componentes STATIC (ver regra: setters do SkyAtmosphere são no-op se STATIC — confirmar mobility antes).
- **Acção dirigida ao agente #6** (Environment Artist): passar por cima dos materiais de casca de árvore (`Veg_Jungle_*` implicados no R18) para aumentar saturação/contraste — mas SÓ material, sem mover nenhum actor por Z (R18 estes três só devem ser medidos, não movidos, se espalhamento>200).

## Atribuição de tarefas — uma por agente, mudança concreta no mundo, check só para verificar depois

- **#2 Engine Architect**: NADA de arquitectura nova este ciclo — a arquitectura está estável. Foco: revalidar que R18 (67) tem lista completa de ofensores com espalhamento medido, para desbloquear #6.
- **#3 Core Systems**: sem tarefa este ciclo — não há sistema base pendente.
- **#4 Performance Optimizer**: sem tarefa — sem sinal de degradação de fps no relatório.
- **#5 Procedural World Generator**: adicionar 2-3 dinossauros novos (espécie ainda pouco representada, ex. Parasaurolophus ou Pachycephalosaurus) dentro de 20-40m do PlayerStart (1200,1200,301), assentes pela pata via helper spawn_criatura.py, com anim Idle. Objectivo concreto: baixar a distância da criatura animada mais próxima do spawn abaixo dos 12m actuais. Verificar depois com **A06**.
- **#6 Environment Artist**: para os 3 ofensores nomeados do R18 (Veg_Jungle_053, Veg_Jungle_026, JungleVeg_025), medir o espalhamento sob a pegada (9 pontos, fórmula min) ANTES de tocar. Se espalhamento <=200, assentar pelo mínimo (nunca pelo máximo — foi o erro do ciclo 9). Se >200, reportar apenas, não mover. Em paralelo, ajustar o material das cascas de árvore (`Veg_Jungle_*`) para maior saturação/contraste conforme feedback visual, sem tocar em posição/escala. Verificar com **R18**.
- **#7 Architecture & Interior**: sem tarefa — nenhuma estrutura pendente identificada.
- **#8 Lighting & Atmosphere**: reduzir intensidade do SkyLight ~20% (de 3.5) para corrigir céu lavado, mantendo light_color (220,255,215) sRGB e sem tocar no Sun (R04/R05r blocker). Verificar visualmente no próximo screenshot e por releitura da propriedade em invocação separada.
- **#9 Character Artist**: prioridade continua a ser a malha masculina do protagonista (excepção já atribuída) — sem nova tarefa adicional este ciclo.
- **#10 Animation**: sem tarefa nova — nenhuma criatura nova a animar até #5 entregar spawns.
- **#11 Dino Behavior**: maior lacuna identificada na directiva — behaviour tree real ainda não existe (BTs são cascas vazias, ver hugo_asset_first_v8 secção 6). Como Python não pode criar nós de BT, o entregável deste ciclo é o callback de post-tick (register_slate_post_tick_callback) para dar a UMA espécie já existente (ex. Triceratops) uma rotina simples de movimento pastor-para-ponto-casa, seguindo a receita provada (duas cadências, destino sempre a partir do ponto-casa). Não criar BT/ABP novos por Python — são inertes.
- **#12 Dino Combat AI**: sem tarefa — depende de #11 ter movimento funcional primeiro.
- **#13 Herd Simulation**: sem tarefa — navegação está morta (medido 14-15/08); entregável seria só especificação escrita, e já foi feito em ciclos anteriores. Não repetir sem novidade.
- **#14 Survival Objectives**: sem tarefa nova este ciclo — A02 está no piso exacto (52), não pode arriscar descer.
- **#15 Narrative**: sem tarefa.
- **#16 Audio**: sem tarefa nova — nenhum pedido específico este ciclo.
- **#17 VFX**: EXPLICITAMENTE NADA — A05 está a 950/986, margem de 36. Qualquer decoração nova aproxima do tecto. Se tiver capacidade, melhorar efeitos existentes sem acrescentar actores.
- **#18 QA**: validar que R18 não piorou após #6 tocar nos 3 ofensores, e confirmar A02/A03/A04 continuam nos pisos exactos (52 / 3-8-39 / 97%) sem regressão.
- **#19 Integration & Build**: consolidar as mudanças de #6/#8 antes do próximo gate; reportar ao #01.

## O que escolhi NÃO pedir este ciclo
Não pedi nada ao #7, #9 (além da excepção já atribuída), #10, #12, #13, #15, #16 — não há gap novo que justifique risco de escrita. Também não pedi decoração nova (#17) porque A05 está perto do tecto (950/986) e o objectivo do ciclo é qualidade de interacção (Camada 3), não volume decorativo.

## Ficheiros
- `reports/studio_director/PROD_CYCLE_AUTO_20260823_002.md` (este relatório)

## Decisões técnicas
- Confirmei estabilidade do resumo do portão com duas chamadas idênticas (CALL 1 repetida) — sem introduzir leituras adicionais fora do escopo permitido.
- Não movi nenhum Dino_*, não toquei no Sun, SkyAtmosphere, Landscape, Terrain_Savana ou PlayerStart.
- Priorizei o VISUAL FEEDBACK (céu lavado) atribuindo-o ao #8, e a lacuna estrutural mais crítica (comportamento de dinossauros inexistente) ao #11, com a receita comprovada em memória para evitar reincidência de BT/ABP vazios.

## Dependências para o próximo ciclo
- #6 precisa da medição de espalhamento dos 3 ofensores do R18 antes de decidir se assenta ou reporta.
- #5 precisa de confirmar quais espécies já têm baixa representação em 0-50m antes de escolher o que spawnar.
- #11 depende do PIE estar aberto para o pastor funcionar (é run-time, morre com restart) — reportar se o PIE estiver fechado.
