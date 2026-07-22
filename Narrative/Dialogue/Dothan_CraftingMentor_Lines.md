# Dothan — Crafting Mentor (Hub NPC)

**Actor real no mundo:** `NPC_CraftsmanNPC_Hub_001` @ (2120, 2280, 240) — bioma Cretaceous Forest / Hub Content Zone.
**Actor secundário relacionado (mesma função narrativa, zona diferente):** `NPC_Anchor_CraftsmanOrin_001` @ (1650, 2700, 240) — variante "Orin", mantido separado por já ter tags próprias de ciclos anteriores (não fundido para evitar apagar trabalho de outro agente).
**Trigger de interação:** `CraftingStation_Hub_001` @ (2100, 2450, 243) perto do PlayerStart (1200,1200,301) — tag `NarrativeRef_CraftsmanDothan` liga-o em lore a este NPC, sem duplicar actor.

Personagem: sobrevivente pragmático, sem misticismo — ensina crafting por necessidade de sobrevivência, não por "sabedoria espiritual". Fala de forma directa, com frases curtas, focadas em recursos e perigo.

## Tag: `Dialogue_CraftingMentor_Dothan_StoneAxe` (já existente, ciclos anteriores)
> "Precisas de duas pedras boas e um pau direito. Sem machado, não abres um crânio de presa nem cortas lenha. Traz-me isso e mostro-te como amarrar a corda."

## Tag: `Dialogue_CraftingMentor_Dothan_Campfire` (novo, este ciclo)
> "Três paus secos, bem juntos, não molhados pelo orvalho. Uma fogueira mal feita apaga-se a meio da noite — e à noite é quando os predadores caçam. Não te enganes nisto."

## Tag: `Dialogue_CraftingMentor_Dothan_WaterContainer` (novo, este ciclo)
> "Uma pedra oca e uma folha grande para vedar. Sem água contigo, não aguentas um dia de caça com este calor. Enche-o sempre antes de te afastares do rio."

## Requisitos de receita (referência técnica — já implementados como tags de actor pelo Agent #14)
- Stone Axe: `RecipeReq_2Rock_1Stick_StoneAxe`
- Campfire: `RecipeReq_3Stick_Campfire`
- Water Container: `RecipeReq_1Rock_1Leaf_WaterContainer`

## Estado de missões associadas
- `Quest_GatherCraftMaterials_001` — reúne materiais (Rock/Stick/Leaf), diálogo de progresso: "Ainda faltam paus. Vai à zona a norte do teu abrigo, há mais lá."
- `Quest_CraftContainer_001` — fabricar o recipiente de água, linha de conclusão: "Boa. Agora não morres de sede à primeira caçada longa."

## Notas para o próximo agente (#16 Audio Agent)
- 3 novas linhas de diálogo acima precisam de VO. NPC já tem tag `HasDialogue_VO` — mas nenhuma linha de Campfire/WaterContainer tem ficheiro de voz ainda (só StoneAxe estava referenciado em ciclos anteriores via `VO_Korg_DangerWarning_Assigned` noutro NPC, não este).
- Tom de voz sugerido: adulto, rouco, prático — nada de tom "sábio" ou contemplativo. É um sobrevivente a ensinar outro sobrevivente.
- Gap conhecido (herdado do #14): confirmar se `NPC_Anchor_CraftsmanOrin_001` deve ser fundido com `NPC_CraftsmanNPC_Hub_001` num ciclo futuro de limpeza (não é decisão de narrativa isolada — envolve o QA/#18 para aprovar remoção de duplicado).
