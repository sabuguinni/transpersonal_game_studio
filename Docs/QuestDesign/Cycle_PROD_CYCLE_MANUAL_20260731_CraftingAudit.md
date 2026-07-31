# Quest & Mission Designer — Ciclo PROD_CYCLE_MANUAL_20260731

## Contexto
Directiva deste ciclo: "Crafting Programmer" — construir/validar o sistema de crafting (receitas, pickups de recursos, trigger de UI "C").

## Descoberta principal: o sistema já existe e é extenso
Auditoria ao mundo vivo (MinPlayableMap) revelou que ciclos anteriores (#03-#13, incluindo o próprio Quest Designer em ciclos prévios) já implementaram um sistema de crafting completo via tags em actores reais, não em C++ (correto — C++ é inerte neste editor headless):

- **Receitas confirmadas por tag:**
  - `Recipe_StoneAxe_2Rock_1Stick` (StoneAxe = 2 Rochas + 1 Pau)
  - `Recipe_Campfire_3Stick` (Fogueira = 3 Paus)
  - `Recipe_WaterContainer_1Rock_1Leaf` (Recipiente de Água = 1 Rocha + 1 Folha)
- **Trigger de UI:** tags `CraftingUITrigger_PressC`, `Interact_C_OpenCraftMenu`, `PressC_OpenCraftingMenu` já presentes na `CraftingStation_FireLight_001`.
- **NPC mentor de crafting:** `NPC_CraftsmanOrin` / `Dothan` com diálogo ligado a cada receita (`Dialogue_CraftingMentor_Dothan_StoneAxe`, etc.) e VO gravado (`Audio_VO_CampElder_CraftFirstTool_*`).
- **Pickups de recursos no núcleo jogável (x -3000..5000, y -1000..5500):**
  - Rochas (`CraftResource_Rock` / `Rock_Savana_*`): **64** encontradas
  - Paus (`CraftResource_Stick`): **19** encontrados
  - Folhas (`CraftResource_Leaf`): **28** encontradas
  - Quantidades suficientes para múltiplos crafts de cada receita.

## Trabalho real feito neste ciclo (via `ue5_execute`, verificado no mundo vivo)
1. **Auditoria completa** de 3329 actores no nível, filtrando por tags de crafting/quest — confirmou que não é necessário duplicar recursos (regra `hugo_naming_dedup_v3` respeitada).
2. **Correcção de grounding**: tracei cada pickup de recurso no núcleo jogável contra o `Landscape` real (filtro de classe, ignorando folhagem/props conforme `hugo_medir_o_que_o_olho_ve_v1`). De 111 pickups verificados, **6 estavam mal assentes** (a mais de 60uu do chão ou abaixo da superfície):
   - `Rock_Savana_238`, `Rock_Savana_270`, `Resource_Stick_Pile_2`, `Resource_Stick_Savana_005`, `ResourcePickup_Leaf_1`, `ResourcePickup_Leaf_2`
   - Todos corrigidos com `actor.modify(True)` + `set_actor_location` para `ground_z + 10uu`, respeitando a regra de que `set_actor_location` sozinho não suja o package.
3. **Verificação da estação de crafting** (`CraftingStation_FireLight_001`, z=243): dentro da banda de superfície válida (44-302 conforme memória do terreno); trace de chão inconclusivo (só apanhou um StaticMeshActor de copa a z=2306, não o Landscape) — decisão: não mover às cegas, manter a posição actual já validada por ciclos QA anteriores (`QA_Validated_002`).
4. **Persistência**: `EditorLoadingAndSavingUtils.save_dirty_packages(True, True)` chamado no fim — devolveu `True` (nota: não é prova definitiva de bytes em disco, mas é o mecanismo correcto para sublevels, ao contrário de `save_current_level()`).

## Decisões técnicas e justificação
- **Não dupliquei receitas nem pickups.** O sistema já cumpre a directiva (3 receitas: StoneAxe, Campfire, WaterContainer) com recursos suficientes espalhados no mapa. Duplicar teria violado `hugo_naming_dedup_v3` e inflacionado o actor count sem benefício de jogo.
- **Zero ficheiros .cpp/.h criados** — regra absoluta `hugo_absolute_prohibitions_v1`. Todo o "CraftingSystem" existe como configuração de actores/tags no mundo vivo, que é o único mecanismo com efeito real neste editor headless.
- **Correcção de grounding em vez de reconstrução**: apenas os 6 pickups fora da tolerância foram movidos; os restantes 105 já estavam bem assentes.

## Ficheiros criados/modificados no GitHub
- `Docs/QuestDesign/Cycle_PROD_CYCLE_MANUAL_20260731_CraftingAudit.md` (este relatório)

## Mudanças verificadas no mundo UE5 (via ue5_execute)
- 6 actores reposicionados: `Rock_Savana_238`, `Rock_Savana_270`, `Resource_Stick_Pile_2`, `Resource_Stick_Savana_005`, `ResourcePickup_Leaf_1`, `ResourcePickup_Leaf_2` — cada um com `actor.modify(True)` antes do `set_actor_location`, para garantir que o package fica sujo e a mudança persiste.
- `save_dirty_packages(True, True)` executado no fim do ciclo → `True`.

## Para o próximo agente (#15 — Narrative & Dialogue Agent)
- O sistema de crafting está **mecanicamente completo e ligado à narrativa**: as 3 receitas já têm diálogo de NPC mentor (Dothan/Orin) e VO gravado para oferta/progresso/conclusão da primeira ferramenta (`Dlg_CampElder_CraftFirstTool_Offer/InProgress/Complete`).
- Recomenda-se ao #15 **rever a coerência do lore** entre `Lore_ToolmakerRemnant` (tag em `Rock_Savana_005`) e o arco do NPC craftsman, para garantir que a história de "porque há rochas com resquícios de ferramentas antigas" se liga à quest principal.
- Não há bloqueios pendentes para a cadeia — QA (#18) deve confirmar que os 6 pickups corrigidos não colidem com outros actores funcionais na área do hub.
