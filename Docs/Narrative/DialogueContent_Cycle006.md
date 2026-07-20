# Narrative & Dialogue — Cycle PROD_CYCLE_AUTO_20260720_006

## Contexto
O #14 (Quest Designer) consolidou o sistema de crafting em `CraftingStation_Hub_001` (2100,2450,100)
e deixou tags de diálogo prontas (`Dialogue_TribeElder`, `NPC_Role_CraftingMentor`,
`Audio_PendingUpload`). O NPC `NPC_CraftsmanNPC_HubDialogue` (2550,2200,100) já existia com a linha
`Recipe_WaterContainer` associada mas sem nome nem texto escrito.

## O que foi feito (real, verificado no mundo vivo)

### 1. Personagem: Taru, o Artesão da Tribo
Batizei o NPC `NPC_CraftsmanNPC_HubDialogue` como **Taru** (tag `NPC_Name_Taru`). É o mentor prático
de crafting no hub — não um "sábio espiritual", mas um artesão pragmático que ensina técnicas de
sobrevivência por necessidade, não por revelação.

### 2. Linha de diálogo 1 — Machado de Pedra (Recipe_StoneAxe_2Rock_1Stick)
Falada por Taru na `CraftingStation_Hub_001`:

> "Watch closely. You bind the stone to the stick with wet sinew, then let it dry over the fire.
> Pull too tight and the wood splits. Pull too loose and the head flies off the first time you
> swing it into a hide. This axe took me three winters to get right. Yours will take one, if you
> listen."

Gerada via `text_to_speech` (voice: TribeElder). **Nota de infraestrutura:** o upload para Supabase
Storage falhou (`403 Invalid Compact JWS` — problema de assinatura de token, não de conteúdo). O
áudio foi sintetizado com sucesso mas não ficou persistido num URL público; a tag
`Audio_PendingUpload` foi mantida como falsa neste caso específico — reverti a remoção e recomendo
ao #16 (Audio Agent) tentar novamente o upload ou investigar a JWS do bucket.

Tags adicionadas a `CraftingStation_Hub_001`:
- `DialogueText_AxeBinding_Written`
- `VoiceLine_Delivered_TribeElder_AxeBinding` (áudio gerado, upload pendente)
- `DialogueLine_02_Campfire` + `DialogueText_Campfire_Written` (linha curta de fogueira, ver abaixo)
- `Narrative_Cycle006`

### 3. Linha de diálogo 2 — Fogueira (Recipe_Campfire_3Stick)
Texto curto associado (não vocalizado neste ciclo, reservado para #16):

> "Three sticks, dry ones, stacked like a tripod. Strike the stone low, catch the spark in the
> moss. Don't build it under branches — you'll bring down more than warmth."

### 4. Linha de diálogo 3 — Recipiente de Água (Recipe_WaterContainer_1Rock_1Leaf) — Taru
Texto associado ao NPC via tag `DialogueLine_WaterContainer_Full`:

> "Big leaf, folded twice, weighted with a flat stone at the bottom so it holds its shape. It won't
> last more than a day before it dries out and cracks — refold it every morning at the river."

## Tom e alinhamento com a regra anti-alucinação
Todas as linhas são pragmáticas, técnicas, focadas em sobrevivência real (ligação de ferramentas,
construção de fogo, gestão de recursos perecíveis). Nenhuma referência a espiritualidade, sabedoria
mística ou comunicação sobrenatural — Taru é um artesão, não um "wisdom keeper".

## Verificação no mundo (ue5_execute)
1. Bridge validation + localização de `CraftingStation_Hub_001` e `NPC_CraftsmanNPC_HubDialogue` —
   confirmado, tags e coordenadas lidas.
2. Tags de diálogo escritas em ambos os actores (2 chamadas de `ue5_execute`).
3. Save do mapa: primeira tentativa (`save_current_level` via `EditorLoadingAndSavingUtils`) devolveu
   `False`; retry com `EditorLevelLibrary.save_current_level()` devolveu `True` — persistido.

## Ficheiros
- `Docs/Narrative/DialogueContent_Cycle006.md` (este ficheiro)
- **Nenhum .cpp/.h criado** (regra `hugo_no_cpp_h_v2`).

## Dependências para o próximo agente (#16 Audio Agent)
- Áudio da linha de Taru sobre o machado já foi sintetizado via ElevenLabs mas **não** persistiu no
  Supabase Storage (erro 403 Invalid Compact JWS no upload). Recomenda-se re-tentar o upload ou
  verificar a assinatura do token/bucket antes de gerar mais vozes.
- Tags `DialogueText_*_Written` em `CraftingStation_Hub_001` e `NPC_CraftsmanNPC_HubDialogue` marcam
  que texto está pronto para 3 linhas: Axe Binding, Campfire, Water Container — todas prontas para
  voice-over.
- Usar lookup pelo label `CraftingStation_Hub_001` / `NPC_CraftsmanNPC_HubDialogue` (não criar novos
  duplicados — regra `hugo_naming_dedup_v2`).
