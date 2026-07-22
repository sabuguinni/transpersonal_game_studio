# Diálogo — Quest_CraftFirstTool_001 (Ciclo PROD_CYCLE_AUTO_20260722_004)

Agente: #15 Narrative & Dialogue Agent
Fonte de trabalho: #14 Quest & Mission Designer (`Quest_CraftFirstTool_001`, `Recipe_StoneAxe_001`, `Recipe_Campfire_001`, `Recipe_WaterContainer_001`)

## Reuse-first (regra `hugo_naming_dedup_v3`)
Auditoria confirmou: **nenhum NPC novo foi criado.** O quest giver do tutorial de crafting é o
`QuestGiver_HubCamp_001` já existente (2050,2350,100), que já carregava diálogo de outras quests
(`TrackHerd`, `GatherMaterials`) e infraestrutura de áudio (`Audio_MetaSound_DialogueBus_CampElder`,
`Audio_Trigger_ProximityRadius_500`). Apenas foram adicionadas tags de diálogo novas ao actor existente.

## Personagem: QuestGiver_HubCamp_001 ("o Ancião do Acampamento")
Tom pragmático, directo, sem misticismo — um sobrevivente experiente que ensina competência, não fé.
Consistente com `NarrativeRole_Pragmatic` já presente noutros givers (Korg, Thok, Uga).

### Linha 1 — Oferta (`Dlg_CampElder_CraftFirstTool_Offer`)
> "Mãos vazias não param um raptor. Traz-me duas pedras e um ramo — vou mostrar-te como talhar
> um machado. Não é magia. É pressão certa, no sítio certo."

### Linha 2 — Em progresso (`Dlg_CampElder_CraftFirstTool_InProgress`)
> "Ainda sem machado? Procura pedras junto ao rio, ramos caídos perto das árvores. Não voltes
> de mãos a abanar."

### Linha 3 — Conclusão (`Dlg_CampElder_CraftFirstTool_Complete`)
> "Isso. Sente o peso? Essa pedra já não é uma pedra — é uma extensão do teu braço. Agora
> aprende a fazer fogo, e talvez sobrevivas à primeira noite."

## Texto de lore/flavor dos Recipes (tags aplicadas, sem áudio ainda)
- `Recipe_StoneAxe_001` → `Lore_StoneAxe_TwoRocksOneStick_ChopsWoodFaster`: "Uma pedra lascada presa
  a um ramo com fibra vegetal. Corta madeira em metade do tempo e serve como arma de emergência
  contra pequenos predadores."
- `Recipe_Campfire_001` → `Lore_Campfire_ThreeSticks_KeepsPredatorsBackAtNight`: "Três ramos secos,
  atrito e paciência. O fogo é a única coisa que mantém os grandes carnívoros longe durante a noite."
- `Recipe_WaterContainer_001` → `Lore_WaterContainer_RockAndLeaf_CarryWaterFromRiver`: "Uma pedra
  côncava forrada com folhas largas. Permite transportar água do rio para longe da fonte — reduz
  viagens arriscadas em território de predadores."

## Verificação real (ue5_execute)
- Tags de diálogo adicionadas ao actor **existente** `QuestGiver_HubCamp_001` (nenhum actor novo
  criado) — confirmado por leitura de `giver_final_tags` no resultado do comando.
- Tags de lore adicionadas aos 3 `Recipe_*` já criados pelo #14 — confirmado por `added_recipe_tags`.
- Nenhuma alteração de posição, mobility, ou geometria — apenas metadata (Tags).

## Decisões técnicas
- Diálogo implementado como **Tags** no actor (não como asset C++/UDataAsset) — única forma
  persistente e verificável neste editor headless sem recompilação (`hugo_no_cpp_h_v2`).
- Texto mantém tom de sobrevivência pragmática: sem referências espirituais, sem "sabedoria",
  sem comunicação mística — apenas ensino prático de competência (regra anti-alucinação do jogo).
- Não foi criado áudio TTS neste ciclo porque o ciclo anterior (`PROD_CYCLE_AUTO_20260722_003`)
  reportou falha de upload para o Supabase Storage (JWT inválido) — gerar áudio sem conseguir
  persistir o URL seria trabalho perdido. Recomenda-se ao #16 (Audio Agent) confirmar se o
  problema de storage foi resolvido antes de gerar VO para estas 3 linhas.

## Para o próximo agente (#16 Audio Agent)
- 3 linhas de diálogo prontas para TTS em `QuestGiver_HubCamp_001` (tags `Dlg_CampElder_CraftFirstTool_*`).
- Voice profile sugerido: masculino, grave, pragmático — consistente com o padrão já usado para
  `VO_Korg_DangerWarning` e `VO_Thok_TrikeTracks` (tag `Audio_VO_Format_Male_Gruff_Pragmatic` seria
  o formato certo a reutilizar, se aplicável a este NPC também).
- Infraestrutura de áudio já existe no actor (`Audio_MetaSound_DialogueBus_CampElder`,
  `Audio_Trigger_ProximityRadius_500`) — reutilizar, não duplicar.
