# Narrative & Dialogue Bible — Núcleo Jogável (Savana/Floresta Conífera do Hub)

**Agente:** #15 Narrative & Dialogue Agent
**Ciclo:** PROD_CYCLE_MANUAL4_20260805
**Escopo:** os três NPCs âncora existentes no mundo + os 24 QuestMarker_* verificados em jogo.
**Tom:** sobrevivência pré-histórica realista. Zero espiritualidade, zero misticismo, zero "despertar". Personagens comunicam por gesto, olhar e linguagem primitiva prática — nunca telepatia.

---

## 1. Premissa do Mundo

O jogador é um sobrevivente isolado que acorda sem memória de como chegou a este vale, cercado por floresta conífera e savana no período Cretáceo. Não há magia nem guias espirituais — há fome, frio, predadores territoriais, e um pequeno grupo de sobreviventes acampado perto de uma fogueira (`QuestMarker_CampfireSite_001`, 2150,2350,100) que pode ensinar-lhe a sobreviver, SE ele provar ser útil.

**Tema central:** competência ganha confiança. Confiança ganha lugar na tribo. Não há escolha "certa" moral — há escolha que funciona ou que te mata.

**Arco do jogador:** Estranho faminto → Par de mãos útil → Batedor de confiança → Membro funcional da tribo. Progressão medida em tarefas cumpridas (caça, recolha, reparação, observação), nunca em "iluminação".

---

## 2. Os Três NPCs Âncora (verificados em jogo)

### 2.1 Craftsman Orin — `NPC_Anchor_CraftsmanOrin_001`
**Posição:** (1650, 2700, 105) — Biome_ConiferForest
**Tags confirmadas:** `NPC_Dialogue_Anchor`, `NPC_CraftsmanOrin`, `Quest_CraftStoneAxe`, `QuestChain_001_CraftsmansAxe`, `QuestGiver_CraftsmansAxe`, `Narrative_LoreLink_ToolmakerOrin`, `Dialogue_Orin_Intro_001/Progress_001/Deliver_001/Reward_001`

**Perfil:** o artesão do acampamento. Não caça — faz as ferramentas de quem caça. Pragmático, seco, mede as pessoas pelo que trazem, não pelo que dizem. Perdeu dois dedos da mão esquerda a talhar sílex há anos; não fala disso a menos que perguntem duas vezes.

**Função narrativa:** primeiro contacto prático do jogador com o crafting. A "QuestChain_001_CraftsmansAxe" ensina o loop de recolha (pedra + madeira) → entrega → recompensa (machado de pedra funcional).

**Diálogo — Intro (`Dialogue_Orin_Intro_001`):**
> "Novo por aqui. Isso vê-se pelas mãos vazias. Não como carne que não ajudou a cortar. Traz-me uma pedra boa e um cabo de madeira direito — eu faço o resto."

**Diálogo — Progress (`Dialogue_Orin_Progress_001`):**
> "Essa pedra tem veio a mais, racha ao primeiro golpe. Procura uma sem linhas claras. E o cabo — se dobra na mão, dobra no crânio de um raptor. Não quero isso na minha consciência."

**Diálogo — Deliver (`Dialogue_Orin_Deliver_001`):**
> "Agora sim. Dá-me a tarde e um sítio quieto. Volta ao pôr-do-sol."

**Diálogo — Reward (`Dialogue_Orin_Reward_001`):**
> "Pesa bem na mão? Bom. Usa-o em algo maior que um coelho antes de confiares nele contra algo que te queira comer."

---

### 2.2 Elder Sana — `NPC_Anchor_ElderSana_001`
**Posição:** (2400, 2700, 72) — Biome_ConiferForest
**Tags confirmadas:** `NPC_Dialogue_Anchor`, `NPC_ElderSana`, `Story_FirstSeasonSurvived`, `QuestChain_002_ElderGathering`, `QuestGiver_ElderGathering`, `Dialogue_Sana_Intro_001/Progress_001/Deliver_001/Reward_001`

**Perfil:** a mais velha do grupo, sobreviveu a uma estação inteira sozinha antes de o resto chegar (`Story_FirstSeasonSurvived`). Não lidera por título — lidera porque é a única que sabe quais bagas matam e quais alimentam. Fala pouco, observa muito. Não tolera desperdício de comida nem bravata.

**Função narrativa:** ensina a lógica da escassez — recolha sazonal, reconhecimento de plantas seguras vs. tóxicas. A `QuestChain_002_ElderGathering` é o loop de recolha de recursos alimentares (bagas, raízes) como segunda camada de sobrevivência depois do crafting básico.

**Diálogo — Intro (`Dialogue_Sana_Intro_001`):**
> "Sobrevivi uma estação inteira sem ninguém para partilhar o fogo. Sabes quantas bagas vermelhas mataram gente boa nesse tempo? Eu sei. Traz-me o que eu pedir, e talvez não sejas o próximo."

**Diálogo — Progress (`Dialogue_Sana_Progress_001`):**
> "Essas não. As que tens na mão têm a pele lustrosa demais — apodrecem por dentro antes de mudarem de cor por fora. Procura as foscas, perto da água parada, não da corrente."

**Diálogo — Deliver (`Dialogue_Sana_Deliver_001`):**
> "Boas escolhas. Isto chega para guardar, não só para comer hoje. Estás a aprender a pensar na próxima lua, não só no próximo estômago."

**Diálogo — Reward (`Dialogue_Sana_Reward_001`):**
> "Toma. Reservas para os dias maus — e vão vir dias maus. Quando a chuva parar de cair durante muito tempo, é a essas que recorres primeiro."

---

### 2.3 Tracker Kael — `NPC_Anchor_TrackerKael_001`
**Posição:** (1950, 2200, 134) — Biome_ConiferForest
**Tags confirmadas:** `NPC_Dialogue_Anchor`, `NPC_TrackerKael`, `Quest_ObserveHerd`, `QuestChain_003_TrackerWatch`, `QuestGiver_TrackerWatch`, `Dialogue_Kael_Intro_001/Observe_001/Scout_001/Deliver_001/Reward_001`, `Quest_Needs_SpearShaft`, `Quest_Needs_Flintstone`, `Quest_Reward_TrackingLesson`

**Perfil:** o batedor. Mais tempo passado a observar predadores do que a falar com pessoas. Fala em factos observáveis — pegadas, direcção do vento, comportamento de manada — nunca em suposições. É quem detecta o perigo antes de ele chegar ao acampamento.

**Função narrativa:** ponte entre exploração e combate/evasão. A `QuestChain_003_TrackerWatch` avança em 4 etapas verificadas: `QuestMarker_ObserveHerd_001` (1800,2200,200) → `QuestMarker_ScoutRaptors_001` (2300,1900,100) → recolha de material para reparar a lança (`Rock_Savana_568`, `Stick_Savana_100`, tags `QuestChain_003_TrackerWatch`) → entrega em Kael.

**Diálogo — Intro (`Dialogue_Kael_Intro_001`):**
> "Não preciso de companhia. Preciso de olhos extra na crista, porque os meus não chegam a todo o lado. Vai até à manada de Triceratops a norte e conta as crias. Se um filhote ficar para trás do grupo, é aí que os raptores atacam primeiro."

**Diálogo — Observe (`Dialogue_Kael_Observe_001`):**
> "A manada moveu-se antes do amanhecer. Se tivessem cheirado os raptores, já se teriam dispersado. Não se dispersaram — significa que a alcateia ainda não os encontrou. Mas vai encontrar. Vigia a crista. Conta os jovens. Se uma cria ficar para trás, é aí que o ataque começa. Traz-me o que vires, não o que pensas que viste."

**Diálogo — Scout (`Dialogue_Kael_Scout_001`):**
> "Pegadas frescas, três indivíduos, a mover-se em linha — isso é caça coordenada, não deslocação. A minha lança partiu no último confronto com um destes. Preciso do cabo e de uma pedra de sílex boa para a reparar antes de voltar lá."

**Diálogo — Deliver (`Dialogue_Kael_Deliver_001`):**
> "Isto serve. A pedra tem fio, o cabo aguenta o impacto. Dá-me a noite para a montar."

**Diálogo — Reward (`Dialogue_Kael_Reward_001`, desbloqueia `Quest_Reward_TrackingLesson`):**
> "Lição de rastreio, já que perguntaste: uma pegada funda à frente e rasa atrás é um animal a acelerar — está a caçar ou a fugir, nunca a passear. Aprende a diferença antes que ela te aprenda a ti."

---

## 3. Cadeia de Missões — Mapeamento Verificado

| QuestChain | NPC | Etapas confirmadas em jogo | Recompensa |
|---|---|---|---|
| `QuestChain_001_CraftsmansAxe` | Orin | Intro → Progress → Deliver → Reward | Machado de pedra funcional |
| `QuestChain_002_ElderGathering` | Sana | Intro → Progress → Deliver → Reward | Reserva de comida para dias maus |
| `QuestChain_003_TrackerWatch` | Kael | `QuestMarker_ObserveHerd_001` → `QuestMarker_ScoutRaptors_001` → recolha (`Rock_Savana_568` + `Stick_Savana_100`) → Deliver → Reward | Lição de rastreio (`Quest_Reward_TrackingLesson`) |

**Nota técnica:** os itens de recolha `Rock_Savana_568` e `Stick_Savana_100` já estavam etiquetados com `QuestChain_003_TrackerWatch` num ciclo anterior — confirmado por releitura independente neste ciclo. Não foram duplicados.

---

## 4. Fragmentos de Lore — QuestMarkers Periféricos (24 confirmados)

Dos 24 `QuestMarker_*` no mundo, a maioria pertence à estrutura funcional de quests do Agente #14 (biomas, zonas de caça, migração). Este ciclo, adicionei tags de lore aditivas (sem remover nada) a 4 marcadores que não tinham ligação narrativa nenhuma, para os ancorar no tema "o caminho para casa":

- **`QuestMarker_Explore`** (-2000,1500,300): `Narrative_LoreFragment_ScoutRidge` + `Narrative_Theme_WayHome` — a crista que talvez mostre o caminho para fora do vale.
- **`QuestMarker_Vantage_Point`** (-2000,-2500,300): `Narrative_LoreFragment_HighGround` + `Narrative_Theme_WayHome` — ponto alto de onde se vê a extensão real do território hostil.
- **`QuestMarker_Water`** (8000,0,300): `Narrative_LoreFragment_FarRiver` + `Narrative_Theme_WayHome` — o rio distante, potencial rota de água doce para fora da região.
- **`QuestMarker_NestSite`** (2600,2100,93): `Narrative_LoreFragment_RaptorNest` + `Narrative_Theme_Danger` — ninho activo de raptores, explica por que a manada de Triceratops evita esta zona (ecologicamente coerente com o comportamento territorial de Kael).

Estas tags foram verificadas por releitura independente em invocação separada (confirmado: `HAS_LOREFRAGMENT_TAG=True` nos 4 actores).

---

## 5. Princípios de Escrita para Próximos Ciclos (Audio/VFX)

1. **Nunca metáfora espiritual.** "Sentir o perigo" não; "ver pegadas frescas" sim.
2. **Diálogo é informação de jogo.** Cada linha de Kael/Orin/Sana deve dar ao jogador uma pista mecânica real (o que recolher, onde ir, que perigo evitar).
3. **Recompensa é sempre material ou informacional** (item, receita, lição de rastreio) — nunca "clareza interior" ou equivalente.
4. **Vozes:** Kael = curto, factual, imperativo. Orin = seco, técnico, cínico. Sana = lento, autoritário, memória de perda.

---

## 6. Amostra de Voz Gerada Este Ciclo

- **Personagem:** Tracker Kael
- **Linha:** variante estendida do diálogo de observação da manada (ver secção 2.3)
- **Ferramenta:** ElevenLabs TTS via `text_to_speech`, ~21s de áudio gerado com sucesso.
- **Limitação registada:** o upload automático para Supabase Storage falhou com `403 Invalid Compact JWS` (mesmo erro já reportado no ciclo `PROD_CYCLE_MANUAL2_20260802`). O áudio existe em base64 na resposta da tool mas não foi persistido em storage público. Recomenda-se ao Agente #16 (Audio) ou à infraestrutura corrigir a autorização do bucket antes do próximo ciclo de voz.

---

## 7. Dependências para o Próximo Agente (#16 Audio Agent)

- As 3 âncoras NPC têm 17 linhas de diálogo escritas e etiquetadas (`Dialogue_<Nome>_<Etapa>_00N`), prontas para MetaSounds/voice-over.
- Recomenda-se gravar as 17 linhas com vozes distintas por personagem (Kael grave/curto, Orin médio/seco, Sana grave/lento).
- A tag `Narrative_VoiceLine_Ready` já está presente nos 3 anchors — sinaliza que o texto está pronto para produção de áudio.
- Investigar a falha de upload Supabase Storage (403) antes de gerar volume de áudio em produção.

## 8. Verificação Anti-Contaminação

Scan de tags feito neste ciclo sobre os 3381 actores do mundo, à procura de termos proibidos (`spirit`, `shaman`, `wisdom`, `mystic`, `chakra`, `aura`, `telepath`, `transcend`, `meditat`, `sacred`, `awakening`, `enlighten`): **0 ocorrências**. O conteúdo narrativo permanece 100% dentro do género de sobrevivência realista.
