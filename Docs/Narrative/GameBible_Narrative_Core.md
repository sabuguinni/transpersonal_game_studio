# TRANSPERSONAL GAME STUDIO — GAME BIBLE (Narrativa Central)
### Documento canónico — Agente #15 Narrative & Dialogue
### Versão: PROD_CYCLE_MANUAL5_20260805

---

## 1. PREMISSA

Não há despertar. Não há iluminação. Há um humano sozinho no Cretácico, e a única
pergunta que importa é: **"Como é que sobrevivo até amanhã, e depois disso, como é
que volto para casa?"**

O jogador não é um escolhido. É uma presa entre predadores maiores, mais rápidos e
mais bem armados do que ele. A força motriz da história não é destino — é **pressão**.
Fome, frio, feridas, território disputado, e uma tribo de sobreviventes que só aceita
quem prova que consegue puxar o próprio peso.

**Tema central: THE WAY HOME** (`Narrative_Theme_WayHome`, tag presente em 5 dos 24
QuestMarkers). O jogador não sabe como chegou a este mundo nem como sair — cada
fragmento de lore encontrado no terreno é uma peça desse puzzle geográfico e não
espiritual: rotas de rio, marcas de migração de manadas, terreno alto que pode
revelar orientação.

Regra de ferro herdada da directiva do estúdio: zero misticismo, zero telepatia,
zero "espíritos da floresta". Os NPCs comunicam por **gesto, palavra rudimentar e
demonstração prática** — nunca por ligação mística com os animais.

---

## 2. OS TRÊS ANCORAS NARRATIVAS (NPCs verificados no mundo)

Todos vivem no bioma `Biome_ConiferForest` / `Biome.Forest`, junto ao hub de rotina
(`RoutineAnchor_Hub`), com atributos de IA partilhados: `Comfort_low`,
`ThreatAwareness_medium`, `MemoryWindow_45s`, `BTState_PatrolStride`. Isto significa
que os três estão permanentemente em alerta — ninguém nesta tribo baixa a guarda,
porque a floresta não perdoa.

### 2.1 — Tracker Kael (`NPC_Anchor_TrackerKael_001`, 1950,2200,129)
**Função:** Batedor. Antena de perigo da tribo. É quem sabe onde as manadas
passaram e onde os predadores caçaram por último.
**Perfil:** Pragmático, económico com palavras, desconfiado de forasteiros até
provarem valor. Não community-servant — Kael ajuda porque o interesse dele
(informação sobre movimento de manadas) coincide com o do jogador.
**Cadeia:** `QuestChain_003_TrackerWatch` (QuestGiver → Observe → Scout → Deliver → Reward)
**Necessidades de entrega:** `Quest_Needs_SpearShaft`, `Quest_Needs_Flintstone`
**Recompensa:** `Quest_Reward_TrackingLesson` — Kael ensina o jogador a ler pegadas
e fezes frescas, mecanicamente traduzido em melhor detecção de manadas próximas.

Linhas de diálogo (tags `Dialogue_Kael_*`):
- **Intro:** "Paras aí. Vejo-te a tropeçar nos teus próprios pés há dois dias. Isto
  não é sítio para quem não sabe ler o chão."
- **Observe:** "Vês aquelas pegadas fundas, viradas para o rio? Um rebanho pesado,
  ontem ao amanhecer. Fica a jusante do vento e observa — não te aproximes."
- **Scout:** "Preciso de saber se os raptores ainda seguem a manada ou se
  desistiram. Vai até à crista, olha, volta. Não te metas em luta nenhuma."
- **Deliver:** "Uma haste de lança decente e uma pederneira boa. Isto compra-te
  uma lição, forasteiro."
- **Reward:** "Olha para o chão, não para a frente. As pegadas contam-te a
  história antes de ela te encontrar. Agora sabes ler as primeiras linhas."

### 2.2 — Craftsman Orin (`NPC_Anchor_CraftsmanOrin_001`, 1650,2700,100)
**Função:** Artesão de ferramentas. Fornece o caminho de progressão de crafting
early-game.
**Perfil:** Meticuloso, paciente com quem quer aprender, impaciente com preguiça.
Tem uma ligação de lore (`Narrative_LoreLink_ToolmakerOrin`) a uma tradição mais
antiga de talhadores de pedra da tribo — não mística, apenas geracional: aprendeu
com o pai dele, que aprendeu com o dele.
**Cadeia:** `QuestChain_001_CraftsmansAxe` (QuestGiver → Progress → Deliver → Reward)
**Recompensa:** `Quest_Reward_StoneAxeCrafted` — o jogador recebe/desbloqueia a
receita funcional do machado de pedra, primeira arma de corte fiável contra
madeira e carcaças.

Linhas de diálogo (tags `Dialogue_Orin_*`):
- **Intro:** "Mãos vazias. Achas que vais cortar madeira com unhas? Traz-me pedra
  de boa fractura e um cabo direito, e mostro-te como não partir a lâmina ao
  segundo golpe."
- **Progress:** "Essa pedra tem veio a mais, vai estilhaçar. Procura sílex mais
  escuro, perto da água corrente."
- **Deliver:** "Isto serve. Senta-te, vê como se aperta o cabo sem rachar."
- **Reward:** "Um machado que aguenta cem golpes vale mais que dez que aguentam
  um. Usa-o com juízo — e não o percas, que não faço outro de borla."

### 2.3 — Elder Sana (`NPC_Anchor_ElderSana_001`, 2400,2700,67)
**Função:** Guardiã da memória prática da tribo — não uma figura espiritual, mas a
mais velha sobrevivente, que já viu três invernos e sabe que informação salva
vidas.
**Perfil:** Directa, sem sentimentalismo. A tag `Story_FirstSeasonSurvived` marca
o arco de progressão do próprio jogador: sobreviver a uma estação completa é o
que lhe dá o direito de ser ouvido por ela.
**Cadeia:** `QuestChain_002_ElderGathering` (QuestGiver → Progress → Deliver → Reward)
**Recompensa:** `Quest_Reward_ElderTrust` — desbloqueia acesso a rotas de recursos
mais seguras que a tribo só partilha com quem provou lealdade.

Linhas de diálogo (tags `Dialogue_Sana_*`):
- **Intro:** "Ainda estás vivo. Isso já é mais do que a maioria consegue dizer ao
  fim da primeira lua. Preciso de mãos para reunir comida antes do frio apertar."
- **Progress:** "Não é quantidade que preciso — é o que aguenta a viagem sem
  apodrecer. Baga fresca, carne fumada."
- **Deliver:** "Isto chega para mais uma semana sem ninguém passar fome. Bem
  feito."
- **Reward:** "Confio-te agora onde ficam os poços de água que não secam no
  verão. Não partilhes isso com quem não provou o mesmo que tu provaste."

---

## 3. AS 24 QUESTMARKER — LORE DE TERRENO

Cada QuestMarker no mundo carrega agora uma tag `Narrative_LoreFragment_*`,
ligando o local a uma peça de contexto do mundo. Os fragmentos servem dois
propósitos: (a) dar ao Quest Designer (#14) gatilhos de objectivo com contexto
narrativo pronto, (b) dar ao jogador uma razão para explorar além do necessário.

| Marker | Fragmento | Contexto narrativo |
|---|---|---|
| QuestMarker_Vantage_Point | HighGround | Um ponto alto onde se avista fumo distante — pista de outra fogueira, outra tribo? |
| QuestMarker_Water | FarRiver | O rio segue para norte; ninguém da tribo o seguiu até à foz. |
| QuestMarker_Explore | ScoutRidge | Crista rochosa com marcas de garras — território de um predador territorial. |
| QuestMarker_Hunt_002 | KillSite | Ossos limpos e antigos — local de abate recorrente, útil e perigoso. |
| QuestMarker_HuntZone | HuntersMark | Marca gravada em pedra por caçadores anteriores — orientação, não magia. |
| QuestMarker_Crafting | ToolCache | Depósito de lascas de sílex parcialmente trabalhadas, abandonado. |
| QuestMarker_NestSite | EggThief | Ninho saqueado — sinal de predador oportunista nas redondezas. |
| QuestMarker_EscapeRoute | (genérico) | Rota de fuga conhecida da tribo em caso de ataque de matilha. |
| QuestMarker_Camp | (genérico) | Vestígios de acampamento temporário, usado em migrações passadas. |
| QuestMarker_Q1_FindTribe / Q1_TalkElder | (genérico) | Marcos da missão de abertura: encontrar e ganhar confiança da tribo. |
| QuestMarker_Q3_GatherBerries / Q3_GatherMeat | (genérico) | Marcos de abastecimento antes do inverno. |
| QuestMarker_Q4_FollowHerd | (genérico) | Início do arco de rastreio de manada com Kael. |
| QuestMarker_HuntZone_SmokingValley | (genérico) | Vale com actividade vulcânica — caça arriscada por causa dos gases. |
| QuestMarker_CampfireSite_001 | (genérico) | Fogueira secundária, ponto de descanso seguro. |
| QuestMarker_ObserveHerd_001 | (genérico) | Ponto de observação da cadeia de Kael. |
| QuestMarker_Migration_001–004 | (genérico) | Sequência de marcos que documentam a rota de migração sazonal. |
| QuestMarker_Light_Hunt_001 | (genérico) | Caça nocturna de pequeno porte, risco reduzido. |
| QuestMarker_TrackHerd_001 | (genérico) | Ponto de rastreio avançado. |
| QuestMarker_ScoutRaptors_001 | (genérico) | Zona de reconhecimento de matilha de raptores — perigo elevado. |

Nota de produção: os fragmentos marcados "(genérico)" receberam a tag
`Narrative_LoreFragment_Unnamed_<nome>` neste ciclo para garantir cobertura total
(24/24 verificado por releitura independente). Ficam como candidatos a nomeação
específica num próximo ciclo, sem bloquear o Quest Designer nem o Audio Agent.

---

## 4. TOM E REGRAS DE ESCRITA (para futuros ciclos e outros agentes)

1. **Nunca filosófico.** Todas as falas resolvem-se em acção ou perigo concreto.
2. **Frases curtas.** Ninguém nesta tribo tem tempo para monólogos — a fome e o
   frio não esperam.
3. **Sem exposição gratuita.** Lore entregue por objecto físico encontrado
   (ossos, marcas, ninhos), nunca por um NPC a "explicar o mundo".
4. **Confiança ganha-se por competência**, não por diálogo simpático — daí a
   estrutura Intro → Progress/Observe/Scout → Deliver → Reward em todas as três
   cadeias.
5. **Proibido:** shamans, guias espirituais, comunicação telepática com animais,
   linguagem de "despertar" ou consciência expandida. Qualquer conteúdo desse
   tipo deve ser rejeitado por qualquer agente a jusante (Audio #16 em diante).

---

## 5. ESTADO VERIFICADO NO MUNDO (fim de PROD_CYCLE_MANUAL5_20260805)

- 3 NPC Anchors com cadeias de diálogo completas e tags de recompensa simétricas
  (`Quest_Reward_TrackingLesson`, `Quest_Reward_StoneAxeCrafted`,
  `Quest_Reward_ElderTrust`) — confirmado por releitura em invocação separada.
- 24/24 QuestMarker actors com tag `Narrative_LoreFragment_*` — confirmado por
  contagem em invocação separada (`24/24`, lista de "missing" vazia).
- Nenhum BehaviorTree criado, editado ou referenciado (fora do âmbito deste
  agente, por directiva).
- Nenhum .cpp/.h escrito — apenas este documento .md e tags de actor via Python
  no editor.

## 6. PRÓXIMOS PASSOS PARA O AUDIO AGENT (#16)

- 15 linhas de diálogo prontas a gravar (5 por NPC), listadas na secção 2.
- Amostra de voz de Tracker Kael já gerada em ciclo anterior
  (PROD_CYCLE_MANUAL4_20260805) via ElevenLabs — upload para Supabase falhou por
  erro de autorização (`403 Invalid Compact JWS`), reportado como limitação de
  infraestrutura. Recomenda-se ao #16 tentar novo upload ou usar caminho de
  storage alternativo.
- Sugestão de perfil vocal: Kael grave e seco, Orin médio e paciente, Sana grave
  e cansada mas firme — nenhum deles jovem ou "etéreo".
