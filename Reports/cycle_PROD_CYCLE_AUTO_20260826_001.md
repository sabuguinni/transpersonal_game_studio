# Studio Director — Ciclo PROD_CYCLE_AUTO_20260826_001

## Leitura do portão (RETRATO/resumo, 2 chamadas ue5_execute)
- Global: **WARN**, fail=0, warn=2, skip=12, erro=0. 3527 actores (modo EDITOR). PIE fechado (confirmado — não bloqueia ninguém este ciclo).
- **R01** WARN: 3527 vs base 3482 (+45, crescimento normal, não accionável).
- **R18** WARN: 67 vs base 27 — ofensores nomeados no relatório: `Veg_Jungle_053` (+460), `Veg_Jungle_026` (+353), `JungleVeg_025` (+87). Estes são vãos sob a pegada em MALHAS ESTÁTICAS DE VEGETAÇÃO com espalhamento provavelmente > 200 uu (já reportado em ciclos anteriores como pendente de decisão humana de escala/sítio — 66 dos 67 estão fora de alcance de correcção por Z segundo a regra da pegada). NÃO mandar mover por Z sem medir o espalhamento primeiro.
- DEFEITOS_DIFERIDOS: 14 checks, 240 ocorrências (maior massa: R18=67, e outros não detalhados neste resumo — não fui aos ficheiros de detalhe, conforme directiva).
- CAMADAS_0_2: 8 de 9 não medíveis fora do PIE (normal, não é falha); spawns_stable=True.

Do RETRATO automático (mais recente que o meu próprio resumo, portanto prevalece nos números de composição):
- A02 (recursos <50m): 52 — no limiar mínimo (52), sem margem.
- A03 (criaturas com anim por banda): 0-50m=3 (mínimo 3, no limiar), 50-200m=8 (mínimo 8, no limiar), 200-500m=39 (mínimo 39, no limiar). TUDO no limiar exacto — qualquer regressão dispara FAIL.
- A04 (caminhabilidade hub): 97% (mínimo 97% — no limiar).
- A05 (decoração): 950 decorativos vs tecto 986 — 36 de margem.
- A06 (proximidade da criatura animada mais próxima): 12 m (retrato) / relatos anteriores indicavam 20,3 m — MELHOROU, é a direcção certa. Continuar a aproximar sem tocar nos Dino_ existentes.

## VISUAL FEEDBACK APPLIED
- Issue detectado: screenshot do último ciclo mostra atmosfera "washed out"/sobre-exposta, céu branco chapado sem profundidade, e sobretudo — **PRIORITY FIX** — os Triceratops parecem modelos low-poly/placeholder, sem detalhe de superfície (pele, normal maps, especular), com troncos de árvore uniformemente pálidos e sem textura.
- Acção dirigida ao agente #8 (Lighting & Atmosphere): NÃO tocar no sol/SkyLight (R04/R05r são BLOCKER e já resolvidos — sun 60000/5500K, SkyLight 3.5 verde 220,255,215 — está protegido, não mexer). Em vez disso, adicionar leve variação de cor atmosférica (ex.: ExponentialHeightFog com um tom âmbar subtil ao entardecer, dentro dos limiares do guard 8000-130000) para dar profundidade sem violar o clamp. Reportar o valor medido antes/depois, nunca mexer no sol directamente.
- Acção dirigida ao agente #6 (Environment Artist) / #9 (Character Artist, materiais): investigar os materiais de superfície da malha SKM_Triceratops (specular/normal map bindings) e dos troncos de árvore (Veg_Jungle_*) — sem trocar malha, sem mover actores Dino_*, apenas ajustar parâmetros de material instance existentes via MaterialEditingLibrary + update_material_instance (ver regra: setters de MI só têm efeito com update_material_instance chamado a seguir).

## Tarefas atribuídas (uma por agente, mudança concreta no mundo + check de verificação)

- **#2 Engine Architect**: nenhuma tarefa nova — arquitectura estável, sem conflitos a resolver este ciclo.
- **#3 Core Systems Programmer**: nada a fazer — sem regressão de física/colisão reportada.
- **#4 Performance Optimizer**: verificar fps com os +45 actores novos (R01 subiu de 3482→3527); reportar se há degradação. Verificar com: `stat fps` amostrado em duas invocações separadas.
- **#5 Procedural World Generator**: acrescentar UMA criatura animada nova (mesh real do Dinosaur_Pack, nunca mover as existentes) o mais perto possível do PlayerStart (1200,1200,301), respeitando espaçamento >=2400 uu dentro do corredor visível, para empurrar A06 (distância à criatura animada mais próxima) ainda mais para baixo dos 12 m actuais. Usar `/root/spawn_criatura.py <BP> <x> <y>`. Verificar via A06 no próximo retrato.
- **#6 Environment Artist**: NÃO mexer no R18 por Z (os 66 vãos remanescentes têm espalhamento >200 uu — decisão humana pendente, não os toques). Em vez disso, melhorar os material instances dos troncos de `Veg_Jungle_*`/`JungleVeg_*` próximos do corredor de showcase (specular/normal, sem trocar malha nem mover) — atende ao feedback visual "troncos uniformemente pálidos". Verificar por captura visual no próximo ciclo (C01/screenshot), não há check numérico dedicado.
- **#7 Architecture & Interior Agent**: nenhuma tarefa — sem gaps reportados em estruturas este ciclo.
- **#8 Lighting & Atmosphere Agent**: adicionar variação atmosférica subtil (fog tint âmbar) sem tocar no sol/SkyLight (R04/R05r protegidos). Ver secção VISUAL FEEDBACK acima. Verificar visualmente no próximo screenshot.
- **#9 Character Artist Agent**: melhorar os material instances (specular + normal map) da malha SKM_Triceratops via MaterialEditingLibrary — é o PRIORITY FIX do feedback visual. Não trocar a malha, não mover os actores Dino_Triceratops_*. Verificar por captura visual.
- **#10 Animation Agent**: nada este ciclo — A03 está no limiar exacto (3/8/39), qualquer trabalho deve ser aditivo (dar anim a criaturas existentes sem anim), coordenar com #5 se novo spawn precisar de idle.
- **#11 Dino Behavior Agent**: continuar a fechar o gap real identificado — não há comportamento nenhum além de tags. Escolher UM dinossauro tag'd sem behavior e implementar via callback de post-tick (register_slate_post_tick_callback) uma rotina mínima de idle/pastoreio, sem tocar nos Dino_ já registados como "movel" no registry. Verificar por observação em PIE (fora do escopo do gate numérico).
- **#12 Dino Combat AI Agent**: nada este ciclo — sem infra de combate a testar ainda (depende do #11).
- **#13 Herd Simulation Agent**: entregar apenas especificação escrita (navegação está morta, medido 14-15/08) — não tentar mover nada.
- **#14 Survival Objectives Designer**: nada este ciclo.
- **#15 Narrative & Dialogue Agent**: nada este ciclo.
- **#16 Audio Agent**: nada este ciclo — sem gap sonoro reportado.
- **#17 VFX Agent**: NADA — A05 (decoração) está a 950/986, margem de apenas 36; qualquer VFX novo aproxima do tecto. Se tiver de agir, só melhorar VFX existentes sem acrescentar actores.
- **#18 QA & Testing Agent**: validar que os 45 novos actores (R01) não introduziram colisão sobreposta em caminhos jogáveis (A04 está no limiar 97%, sem margem).
- **#19 Integration & Build Agent**: nada de build especial este ciclo — reportar de volta com estado consolidado.

## O que escolhi NÃO pedir este ciclo
- Não pedi a nenhum agente para mover, rodar ou apagar qualquer actor `Dino_*` (proibido, BLOCKER V1-C25).
- Não pedi correcção de R18 por ajuste de Z nos 66 vãos remanescentes — o espalhamento sob a pegada está acima de 200 uu nesses casos e mover por Z pioraria (regra R36/R18 documentada), fica para decisão humana de escala/sítio.
- Não pedi nenhum VFX/decoração nova (#17 fica parado) — A05 já está perto do tecto (950/986).
- Não fui ler o relatório JSON completo nem o baseline nem o creatures_registry — o resumo do audit já contém tudo o que preciso para decidir.

## Ficheiros criados/modificados
- `Reports/cycle_PROD_CYCLE_AUTO_20260826_001.md` (este relatório)

## Dependências para o próximo ciclo
- #5 precisa de confirmar a nova criatura no creatures_registry.json com `movel`, `ponto_casa`, `raio_casa` se for móvel.
- #8 e #9 devem reportar valores medidos antes/depois (não apenas "melhorei").
- #22 corre o gate no fim e reporta R18/A02-A06 actualizados — usar isso, não reler ficheiros a meio do ciclo.
