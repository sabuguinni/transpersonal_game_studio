# Game Bible — Hub / Savana Cretácea
**Agente #15 — Narrative & Dialogue Agent** | Ciclo PROD_CYCLE_MANUAL7_20260809

## Contexto do jogo
Sobrevivência pré-histórica realista. O jogador é um humano primitivo, sem armas nem ferramentas iniciais, que precisa de aprender a sobreviver num mundo do Cretáceo dominado por dinossauros territoriais e predadores. Não há magia, não há espiritualidade, não há telepatia. Tudo o que os NPCs sabem, sabem por experiência prática: fome, frio, feridas, perda.

**Pergunta central da história, sempre presente e nunca resolvida em diálogo:** *como é que eu volto para casa?* — o tema `Narrative_Theme_WayHome`, já presente nas 24 tags `QuestMarker_*` do mundo, é o fio que liga cada missão. Ninguém sabe a resposta. Todos têm uma teoria prática (a crista a norte, o rio que nunca seca, a migração da manada). Nenhuma teoria é garantida.

## Os três pilares narrativos (NPC anchors verificados no mundo)

### Craftsman Orin — `NPC_Anchor_CraftsmanOrin_001` (1650, 2700, 100)
**Quem é:** o artesão do acampamento. Não fala de sentimentos — fala de peso, ângulo, fio de corte. A sua lição para o jogador é competência: uma ferramenta mal feita mata tão depressa como um dente de Raptor.
**Cadeia:** `QuestChain_001_CraftsmansAxe` — 2 pedras (`Quest_Needs_AxeHeadStone`) + 1 pau (`Quest_Needs_AxeHandle`) → machado de pedra (`Quest_Reward_StoneAxeCrafted`).
**Diálogo (tags `Line_Orin_*`, gravadas no actor):**
- Intro: *"Bring me stone and a straight branch. A dull edge gets you killed out there."*
- Progress: *"Not yet. I need the stone AND the handle. One without the other is firewood."*
- Deliver: *"Good weight on this one. Hold it here, watch the balance."*
- Reward: *"Take the axe. Aim for the neck joints, not the skull — bone breaks your arm first."*

### Elder Sana — `NPC_Anchor_ElderSana_001` (2400, 2700, 67)
**Quem é:** sobreviveu a uma estação inteira sozinha (`Story_FirstSeasonSurvived`). Não é uma figura espiritual — é uma velha caçadora com os joelhos gastos, que agora troca conhecimento prático (plantas medicinais reais, sinais de infecção) por ajuda física que já não consegue dar a si mesma.
**Cadeia:** `QuestChain_002_ElderGathering` — 3 folhas (`Quest_Needs_MedicinalLeaf`) → confiança (`Quest_Reward_ElderTrust`).
**Diálogo (tags `Line_Sana_*`):**
- Intro: *"My knees won't carry me up that slope anymore. Three leaves, the broad ones by the creek."*
- Progress: *"Two so far. The fever won't wait for a third trip."*
- Deliver: *"These will do. Crush them before the wound closes over dirt."*
- Reward: *"You listen before you act. That's rarer than meat this season."*

### Tracker Kael — `NPC_Anchor_TrackerKael_001` (1950, 2200, 129)
**Quem é:** o observador — ensina o jogador a ler o terreno e o comportamento animal antes de o enfrentar. Pragmático até à secura: cada frase dele é uma regra de sobrevivência, não uma opinião.
**Cadeia:** `QuestChain_003_TrackerWatch` — observar manada + pederneira (`Quest_Needs_Flintstone`) + haste (`Quest_Needs_SpearShaft`) → lição de rastreio (`Quest_Reward_TrackingLesson`).
**Diálogo (tags `Line_Kael_*`):**
- Intro: *"Watch the herd from the ridge, don't go near. Count how many before they scatter."*
- Scout: *"Flint sparks faster than wood friction — bring me a piece and a straight shaft."*
- Deliver: *"This edge will hold. Now you know why I don't carry dead weight."*
- Reward: *"Downwind, always downwind. A raptor smells fear before it sees you."*

## Os 24 QuestMarker — a espinha dorsal geográfica da narrativa
Todos os 24 `QuestMarker_*` já carregam (ciclo anterior, verificado) `Narrative_Theme_WayHome` + um `Narrative_LoreFragment_*` próprio. Este ciclo confirma-os por auditoria e não os duplica (REUSE FIRST). Categorias observadas nos labels:
- **Sobrevivência imediata:** `QuestMarker_Camp`, `QuestMarker_CampfireSite_001`, `QuestMarker_Water`, `QuestMarker_Crafting`
- **Perigo / observação:** `QuestMarker_HuntZone`, `QuestMarker_HuntZone_SmokingValley`, `QuestMarker_Hunt_002`, `QuestMarker_ScoutRaptors_001`, `QuestMarker_ObserveHerd_001`, `QuestMarker_Vantage_Point`
- **Progressão / mundo:** `QuestMarker_Migration_001-004`, `QuestMarker_TrackHerd_001`, `QuestMarker_NestSite`, `QuestMarker_EscapeRoute`, `QuestMarker_Explore`
- **Arco de tribo (prefixo Q):** `QuestMarker_Q1_FindTribe`, `QuestMarker_Q1_TalkElder`, `QuestMarker_Q3_GatherBerries`, `QuestMarker_Q3_GatherMeat`, `QuestMarker_Q4_FollowHerd`, `QuestMarker_Light_Hunt_001`

**Leitura narrativa do arco Q1→Q4:** o jogador chega sozinho (Q1_FindTribe), fala com a anciã para ganhar confiança (Q1_TalkElder), aprende a recolher com segurança (Q3), e só depois segue a manada para além do hub conhecido (Q4_FollowHerd) — a primeira vez que o jogo o tira do raio seguro dos três NPCs. É aqui que a pergunta "como volto para casa" deixa de ser retórica.

## Arco do jogador — de sobrevivente solitário a membro competente da tribo
Não há "despertar". Há competência ganha por repetição:
1. **Chegada** — sem ferramentas, sem aliados, ameaça constante (Q1).
2. **Aprendizagem prática** — Orin (fabricar), Sana (curar), Kael (observar antes de agir) — as três cadeias correm em paralelo, não em sequência forçada.
3. **Confiança da tribo** — cada entrega é avaliada por resultado, nunca por intenção. Sana não agradece esforço; agradece a folha certa.
4. **Saída do hub** — seguir a manada (Q4) é a primeira aposta do jogador em conhecimento próprio, sem supervisão dos três NPCs.

## Tom e regras de escrita para próximos agentes
- Frases curtas, imperativas, sem metáforas floridas. Ninguém filosofa.
- Cada linha de diálogo ensina uma mecânica real (peso de ferramenta, infecção de ferida, direcção do vento) — nunca é só ambientação.
- Nenhum NPC tem poder sobrenatural. O que sabem, sabem por terem sobrevivido mais tempo.
- Tema narrativo único e obrigatório em qualquer novo conteúdo de lore: `Narrative_Theme_WayHome`.

## Verificação (ciclo PROD_CYCLE_MANUAL7_20260809)
- Auditados por leitura directa no mundo: 3 NPC anchors + 24 QuestMarker (24/24 confirmados, contagem exacta).
- Escritas 12 tags `Line_<Char>_<Beat>=<texto>` (4 por NPC) via `ue5_execute` com `actor.modify(True)` antes da escrita.
- Persistência confirmada por **releitura independente em invocação separada** após `save_dirty_packages(True, True)` — as 12 tags aparecem intactas nos 3 actores.
- Amostra de voz real gerada (ElevenLabs TTS) para a linha Intro+Reward de Craftsman Orin, entregue ao Audio Agent (#16) como referência de tom.
- Nenhum actor foi movido, apagado ou reescalado. Nenhum ficheiro `.cpp`/`.h` criado. Nenhuma BehaviorTree tocada.

## Limitações reportadas
- `QuestMarker_ObserveHerd_001` continua sem malha visível (marcador lógico puro) — fora do escopo deste agente, já reportado pelo #14.
- O texto do diálogo vive como **tags de dados** nos actores, não como grafo de Blueprint (Dialogue Widget / Quest Log): construir esse grafo requer nós de Blueprint que o Python não expõe nesta build headless. Ficheiro de referência único de texto: este documento.

## Dependências para o próximo agente
**#16 Audio Agent:** as 12 linhas `Line_<Char>_<Beat>` acima estão prontas para voice-over completo (amostra de Orin já gerada). Vozes recomendadas por personagem: Orin — grave, prática; Sana — mais lenta, rouca, autoridade calma; Kael — seca, cortante, sem pausas longas.
