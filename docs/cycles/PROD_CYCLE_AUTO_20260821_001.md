ytterling: substituído abaixo (conteúdo real do ciclo)

# Studio Director — Ciclo PROD_CYCLE_AUTO_20260821_001

## Leitura do portão (CALL 1 — `/root/playability_audit.py --resumo`)

- **Global: WARN** — fail=0, warn=2, skip=12, erro=0. Modo EDITOR. 3509 actores.
- Pré-checks OK: MinPlayableMap 3084/2500, Terrain_Savana 425/400.
- **CAMADAS_0_2**: fora do PIE, 8 de 9 não medíveis (não são falhas). A única medida no editor: spawns_stable=True.
- **DEFEITOS_DIFERIDOS**: 14 checks, 239 ocorrências.

### Não-PASS com números
- **R01** WARN — atual=3509, base=3482 (contagem de actores subiu 27; monitorizar, não é bloqueante).
- **R18** WARN — atual=67, base=27. Piorou desde a última leitura registada nas memórias (67 já era o valor conhecido em 18/08; manteve-se). Ofensores nomeados: `Veg_Jungle_053` (+460 uu de vão sob a pegada), `Veg_Jungle_026` (+353), `JungleVeg_025` (+87).
- R03, R24r skip; R26 manual; R28r delegado; P01-P09 skip (fora do PIE).

### RETRATO automático (mais recente, ganha sobre relatos de agentes)
- A02 (recursos ≤50m): 52 — no piso mínimo (52), sem margem.
- A03 (criaturas animadas por banda): 0-50m=3(mín 3, sem margem), 50-200m=8(mín 8, sem margem), 200-500m=39(mín 39, sem margem). Zero folga em toda a Layer 3.
- A04 (caminhabilidade): 97% — exactamente no piso.
- A05 (decoração): 950 vs tecto 986 — 36 de margem, NÃO gastar em VFX/decoração novos sem necessidade forte.
- A06 (distância à criatura animada mais próxima): 20.3 m — igual ao ciclo anterior, não melhorou.

## Feedback visual do último screenshot
PRIORITY FIX identificado: **texturas/materiais dos Triceratops parecem placeholders cinzentos** — falta PBR com detalhe de pele, normal maps, subsurface scattering. Terreno "flat" precisa de mais variação a média distância. Erva sem animação de vento.

## VISUAL FEEDBACK APPLIED
- Issue detectado: Triceratops (e por extensão outros Dino_*) com superfícies cinzentas tipo argila/placeholder, sem detalhe de pele nem normal maps; terreno com pouca variação a média distância; erva estática sem vento.
- Acção dirigida ao agente #6 (Environment Artist): sem tocar em NENHUM actor Dino_* (proibido mover/rodar/reescalar/apagar), aplicar ou melhorar os MATERIAL INSTANCES das malhas SKM_Triceratops (e das outras espécies do Dinosaur_Pack) — adicionar/activar normal map, roughness variation e, se o material base do pack suportar, subsurface scattering, usando MaterialEditingLibrary.update_material_instance() para garantir que a alteração chega ao renderer (ver hugo_python_neste_editor_v2: MI muda sem update_material_instance não pinta nada). Isto NÃO é mover geometria — é só parametrizar o material existente, portanto não viola a protecção dos Dino_*.
- Acção dirigida ao agente #8 (Lighting & Atmosphere): não mexer no sol/SkyLight (R04/R05r são BLOCKER, resolvidos). Se houver budget, avaliar apenas variação de vento na vegetação via material (World Position Offset), sem novos actores (respeitar tecto A05=986, margem 36).

## Tarefas atribuídas (uma por agente, acção concreta no mundo + check de verificação)

- **#2 Engine Architect**: nada de arquitectura nova este ciclo — a arquitectura está estável. Confirma apenas que nenhuma directiva nova contradiz REUSE FIRST / HANDS OFF. Verificar via: nenhum check específico, é revisão de conformidade.
- **#3 Core Systems**: nada este ciclo. Física e colisão base já servem o gate (R14=2, R11=4 nas margens conhecidas). Não escrever C++.
- **#4 Performance Optimizer**: medir o custo de frame actual (fps) com os 3509 actores e reportar se subiu desde os 3482 do R01 baseline — verificação: R01 (contagem de actores, contexto de custo).
- **#5 Procedural World Generator**: adicionar UMA criatura animada nova (não movida) dentro da banda 0-50 m do PlayerStart, usando `/root/spawn_criatura.py <BP> <x> <y>`, respeitando espaçamento >=2400 uu de vizinhos e assentando pela pata. Objectivo: aproximar a distância à criatura animada mais próxima abaixo dos actuais 20.3 m. Verificar via: **A06**.
- **#6 Environment Artist**: aplicar normal map / roughness / subsurface (se disponível) aos Material Instances das malhas de dinossauro do Dinosaur_Pack (sem tocar em nenhum actor Dino_*, só nos materiais), usando MaterialEditingLibrary.update_material_instance() para confirmar propagação ao renderer. Verificar via: inspecção visual (screenshot do próximo ciclo) — não há check numérico para qualidade de material, reportar antes/depois.
- **#6 Environment Artist (segunda parte, opcional se houver budget)**: reduzir o vão sob a pegada dos 3 actores nomeados no R18 (`Veg_Jungle_053`, `Veg_Jungle_026`, `JungleVeg_025`) usando a fórmula do mínimo dos 9 pontos (NUNCA o máximo — foi o erro do ciclo 9). Só assentar se o espalhamento sob a pegada for <=200 uu; caso contrário reportar sem mover. Verificar via: **R18** (esperar descer de 67 em direcção a 27).
- **#7 Architecture & Interior**: nada este ciclo — não há gap identificado na arquitectura/interiores.
- **#8 Lighting & Atmosphere**: nada em sol/SkyLight (BLOCKER, intocável). Se avançar, só variação de vento na vegetação via WPO de material, sem novos actores. Verificar via: A05 (tecto 986, não pode subir).
- **#9 Character Artist**: continuar a atribuição já registada de substituir a malha feminina do protagonista por malha masculina (única excepção às regras HANDS OFF do pawn). Verificar via: inspecção visual, sem check de gate dedicado.
- **#10 Animation Agent**: nada este ciclo — sem gap novo de animação identificado além do que já está coberto pelas criaturas móveis existentes.
- **#11 Dino Behavior**: maior gap real do jogo — não há comportamento nenhum além de tags. Especificar (documento, não Python de nó) um Behavior Tree funcional mínimo para pelo menos 1 espécie usando as tarefas já nomeadas, documentando quais classes de nó faltam no projecto (nenhuma BT actual tem nós reais — 27 de 28 vazias). Verificar via: nenhum check de gate mede isto directamente; é gap estrutural a resolver por especificação.
- **#12 Dino Combat AI**: nada este ciclo — depende do #11 ter uma BT funcional primeiro.
- **#13 Herd Simulation**: nada este ciclo — navegação está morta (medido), entregável continua a ser especificação escrita, já feita em ciclos anteriores.
- **#14 Survival Objectives**: nada este ciclo — sem novo recurso ou terreno para converter em objectivo.
- **#15 Narrative & Dialogue**: nada este ciclo.
- **#16 Audio Agent**: nada este ciclo.
- **#17 VFX Agent**: **NADA de novos actores decorativos** — A05 está a 950/986, margem de apenas 36, e a directiva do próprio agente diz que decoração nova acima do tecto bloqueia o ciclo. Se tiver trabalho, melhorar VFX existentes sem acrescentar contagem.
- **#18 QA & Testing**: verificar especificamente se R18 desceu após a intervenção do #6 (67 → alvo <60) e se A06 desceu após #5 (20.3m → <20.3m). Bloquear se algum Dino_* tiver sido movido (V1-C25).
- **#19 Integration & Build**: reportar de volta ao #01 com os valores reais medidos de R18 e A06 pós-ciclo, não adjectivos.

## O que escolhi NÃO pedir este cycle
Não pedi trabalho novo aos agentes #2, #3, #7, #10, #12, #13, #14, #15, #16 porque não há gap novo identificável nas suas áreas este ciclo — fazer trabalho sem gap concreto arrisca criar defeitos novos (como aconteceu nos ciclos 8 e 9 quando agentes "inventaram" trabalho para mover o número de um check). Doing nothing é entrega válida.

## Decisões técnicas
- Mantive o orçamento de 2 chamadas ue5_execute conforme directiva (1 leitura do audit + 1 log de confirmação, sem mutação de mundo pelo Agente #1 — o papel deste agente é coordenação, não execução directa no mundo).
- Prioridade dada à Layer 3 (A02/A03/A04/A05/A06), que mede a experiência real do jogador, sobre qualquer outro número.
- Traduzi o feedback visual em instrução concreta de material (não geometria), respeitando a protecção dos Dino_*.

## Ficheiros
- `docs/cycles/PROD_CYCLE_AUTO_20260821_001.md` (este relatório)

## Dependências para o próximo ciclo
- #6 precisa de confirmar se os materiais do Dinosaur_Pack expõem parâmetros de normal map / subsurface antes de tentar activá-los (MaterialEditingLibrary.get_material_instance_*_parameter_value primeiro).
- #5 precisa de confirmar via `/root/spawn_criatura.py` que a nova criatura ficou a menos de 20.3 m do PlayerStart antes de reportar sucesso em A06.
