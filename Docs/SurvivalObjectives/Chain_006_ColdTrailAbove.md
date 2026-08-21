## Chain 006 — "Cold Trail Above" (completed this cycle)

### Estado antes deste ciclo
A cadeia QuestChain_006_ColdTrailAbove existia com 4 nós (QuestOrder_01 a 04),
todos sobre actores `Stick_`/`Rock_` reais já presentes no mundo, subindo em
altitude de z≈111 até z≈930 num arco Savana → Floresta. Não tinha remate: parava
a meio da subida sem apontar para lado nenhum. Nenhum NPC, nenhum diário —
apenas objectos com malha real e tags de progressão.

| Ordem | Actor (label real, malha existente) | Posição | Papel narrativo |
|---|---|---|---|
| 01 | Stick_Savana_101 | (2200, 2300, 111.1) | primeiro sinal: um ramo partido fora do padrão de queda natural |
| 02 | Rock_Floresta_403 | (2700, 2050, 225.3) | pedra deslocada, marca de passagem |
| 03 | Rock_Floresta_401 | (2600, 2250, 765.5) | a subida começa a ficar íngreme e fria |
| 04 | Rock_Floresta_402 | (2400, 2050, 930.4) | o rasto esfria — poucas pistas, vento mais forte |

### O que foi acrescentado neste ciclo
**QuestOrder_05 — Rock_Savana_571** (2500.6, 2616.3, 817.9), actor já existente
no mundo (malha de rocha real, sem criação de conteúdo novo), previamente sem
tag de cadeia. Ficou marcado com:
- `QuestChain_006_ColdTrailAbove`
- `QuestOrder_05`
- `Quest_ColdTrailSummit`
- `Quest_PointsToSnowyRockside`

Está a ~575 uu do nó anterior (Rock_Floresta_402), na mesma cornija elevada,
fechando o arco em vez de o deixar suspenso. É o ponto mais alto e mais frio
da cadeia — onde o rasto de pegadas desaparece na rocha nua e o jogador tem de
decidir, sozinho, olhar para cima e para norte, para a linha branca da
Snowy Rockside no horizonte. Não há texto, não há marcador luminoso: é o
próprio relevo (rocha desgastada pelo vento, ausência de vegetação, frio
crescente que o sistema de temperatura já regista) que comunica "o caminho
continua para lá".

### Arco emocional (Sasko)
Descoberta (o ramo partido no meio da savana, fora do sítio) → confirmação
(a pedra deslocada) → esforço (a subida íngreme, duas pedras em altitude
crescente) → revelação sem recompensa material — o remate desta cadeia não dá
um item, dá uma DIRECÇÃO. É a mesma promessa que qualquer side-quest bem
desenhada faz: o jogador não termina com um objecto, termina a saber mais
sobre o mundo do que sabia.

### Afirmação de mecânica (Blow)
O jogo nunca diz "vai para a Snowy Rockside". Diz-o através de um facto do
terreno: pistas ficam mais raras à medida que a altitude sobe e o frio
aumenta. A mecânica de rastreamento — cada vez menos evidente, cada vez mais
exigente — é a afirmação de que o mundo cretácico não facilita a jornada até
à segunda gema; tem de ser conquistada por leitura do terreno, não por
indicação.

### Verificação
Tags confirmadas por releitura independente do actor (chamada separada da
escrita): `['CraftIngredient_Rock', 'QuestChain_006_ColdTrailAbove',
'QuestOrder_05', 'Quest_ColdTrailSummit', 'Quest_PointsToSnowyRockside']`.
Nenhum actor `Dino_` foi tocado. Nenhuma malha nova foi criada — Rock_Savana_571
já existia no mundo com malha real; só foram acrescentadas tags.

### Cadeias existentes no mundo (inventário deste ciclo, não modificadas)
- QuestChain_001_CraftsmansAxe (4 nós, completa) — Rock→Rock→Stick→NPC_Anchor_CraftsmanOrin_001
- QuestChain_002_ElderGathering (4 nós, completa) — 3×Leaf→NPC_Anchor_ElderSana_001
- QuestChain_003_TrackerWatch (5 nós, completa) — 2×QuestMarker→Rock→Stick→NPC_Anchor_TrackerKael_001
- QuestChain_004_ImprovisedSpear (4 nós, completa) — Stick→Rock→Leaf→Rock (craft site)
- QuestChain_005_ReadingTheStrata (5 nós, completa) — 5×Rock em arco geológico crescente
- QuestChain_006_ColdTrailAbove (5 nós, **completada neste ciclo**)

As cadeias 001, 002 e 003 terminam em `NPC_Anchor_*` — resíduo de um conceito
anterior (aldeia tribal) marcado para remoção pela directiva deste ciclo. Não
foram tocadas por não fazerem parte do meu mandato (não movo/apago actores de
cadeias de ciclos anteriores sem indicação), mas o remate delas terá de ser
redesenhado quando os `NPC_Anchor_*` forem removidos — reportado como
limitação abaixo.

### Limitações reportadas (não resolvidas neste ciclo, fora do mandato)
1. **NPC_Anchor_CraftsmanOrin_001, NPC_Anchor_ElderSana_001,
   NPC_Anchor_TrackerKael_001** são o remate de 3 das 6 cadeias. Quando forem
   removidos (conforme directiva), essas 3 cadeias ficam sem nó final e
   precisam de um remate novo sobre objecto real (ex.: um local de fabrico,
   uma pilha de ossos, uma marca de garra) em vez de uma figura humana.
2. **24 QuestMarker_\*** existentes (Vantage_Point, Hunt_002, Water, HuntZone,
   Explore, Crafting, NestSite, EscapeRoute, Camp, Q1_FindTribe, Q1_TalkElder,
   Q3_GatherBerries, etc.) carregam tags `Narrative_Theme_WayHome` e
   referências a "Tribe"/"Elder" — resíduo do mesmo conceito descartado. Não
   foram usados como base de nenhuma cadeia nova, conforme a directiva. A sua
   remoção/renomeação é trabalho de outro agente (Narrative #15 ou QA #18).
3. **Dinossauros com HerdRole_Member** (Dino_Parasaurolophus_1-6) estão a
   17.000–24.000 uu do PlayerStart — tecnicamente alcançáveis a pé mas muito
   distantes para servir de âncora de uma missão de migração jogável neste
   ciclo. Uma cadeia de "seguir a manada" precisa de que o #05 (World
   Generator) ou o #13 (Herd Simulation) aproxime pontos de manada intermédios
   do corredor do spawn antes de eu poder ancorar objectivos legíveis nela.
4. Sem UI/HUD e sem diário de missões (por design), a única forma de o
   jogador "ler" uma cadeia é o próprio arranjo espacial dos objectos e a
   iconografia do terreno (altitude, temperatura, vegetação rareando) — não
   há forma de eu verificar em headless que a leitura funciona para um
   jogador humano; fica como pedido de validação ao QA (#18) em Pixel
   Streaming.
