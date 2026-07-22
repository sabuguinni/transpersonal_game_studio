# Crafting System — Recipe & Resource Design (Agent #14)

> NOTA IMPORTANTE: por regra global do estúdio (`hugo_no_cpp_h_v2`), este agente NÃO escreve `.cpp`/`.h` — o editor headless não recompila C++ novo e qualquer ficheiro desse tipo teria efeito zero no jogo ao vivo. A implementação real dos dados de crafting deve ser feita via **DataTable/Blueprint** dentro do UE5 (o JSON abaixo serve de fonte para importar essa DataTable), e a lógica de UI/trigger via Blueprint ou pelo `CraftingMenuTrigger_Hub_001` já existente na cena (ver secção "Estado na cena").

## Receitas base (3)

| Receita | Ingredientes | Output | Tempo de crafting |
|---|---|---|---|
| Machado de Pedra (Stone Axe) | 2x Rock + 1x Stick | Stone Axe (ferramenta de corte/combate ligeiro) | 3s |
| Fogueira (Campfire) | 3x Stick | Campfire (fonte de calor/luz, cozinhar carne) | 2s |
| Recipiente de Água (Water Container) | 1x Rock + 1x Leaf | Water Container (transporta água, mitiga sede) | 2s |

Dados em formato de import (DataTable `CraftingRecipe`):

```json
[
  {
    "RecipeID": "Recipe_StoneAxe",
    "DisplayName": "Machado de Pedra",
    "Ingredients": [ {"Resource": "Rock", "Quantity": 2}, {"Resource": "Stick", "Quantity": 1} ],
    "OutputItem": "Item_StoneAxe",
    "OutputQuantity": 1,
    "CraftTimeSeconds": 3.0,
    "Category": "Tool"
  },
  {
    "RecipeID": "Recipe_Campfire",
    "DisplayName": "Fogueira",
    "Ingredients": [ {"Resource": "Stick", "Quantity": 3} ],
    "OutputItem": "Item_Campfire",
    "OutputQuantity": 1,
    "CraftTimeSeconds": 2.0,
    "Category": "Survival"
  },
  {
    "RecipeID": "Recipe_WaterContainer",
    "DisplayName": "Recipiente de Água",
    "Ingredients": [ {"Resource": "Rock", "Quantity": 1}, {"Resource": "Leaf", "Quantity": 1} ],
    "OutputItem": "Item_WaterContainer",
    "OutputQuantity": 1,
    "CraftTimeSeconds": 2.0,
    "Category": "Survival"
  }
]
```

## Recursos (pickups) — auditoria da cena (MinPlayableMap, verificado via `ue5_execute`)

| Recurso | Contagem antes deste ciclo | Acção tomada | Contagem depois |
|---|---|---|---|
| Rock_Savana_* | 69 | nenhuma (suficiente para as 3 receitas) | 69 |
| Stick_Savana_* | 12 | nenhuma (suficiente para Stone Axe + Campfire) | 12 |
| Leaf_* (Quest_/Savana_) | 4 | **+8 spawned** (`Leaf_Savana_013`–`020`, grounded a z=105 junto ao hub, mesma cota verificada dos `Stick_Savana_100/101/102`) | 12 |

Todos os novos `Leaf_Savana_0XX` têm tags `CraftingResource` + `Leaf` para pickup lógico e seguem a convenção `Type_Bioma_NNN`.

## Estado do trigger de menu de crafting ("press C")

Auditoria encontrou **10 triggers/actors de crafting já existentes** na cena (acumulados de ciclos anteriores), incluindo `CraftingMenuTrigger_Hub_001`, `CraftingUITrigger_Hub_001`, `CraftingUI_Trigger_Hub`, `Trigger_Quest_CraftStoneAxe_001`, etc. Por regra de **reuse-first / anti-duplicação** (`hugo_naming_dedup_v3`), **não foi criado nenhum trigger novo** — o já existente `CraftingMenuTrigger_Hub_001` deve ser o canónico.

**Recomendação para QA (#18) / Integration (#19):** os restantes 9 triggers de crafting parecem duplicados semânticos do mesmo conceito (abrir menu de crafting) criados por agentes/ciclos diferentes sob nomes distintos. Não foram apagados por não ser tarefa de limpeza atribuída a este agente neste ciclo — recomenda-se consolidação numa passada de manutenção dedicada, mantendo apenas `CraftingMenuTrigger_Hub_001` como o trigger funcional ligado ao input "C".

## Próximos passos sugeridos (para #15 Narrative / #19 Integration)

1. Ligar o `CraftingMenuTrigger_Hub_001` a um Widget Blueprint de menu de crafting (input "C") — trabalho de Blueprint, fora do âmbito deste agente (sem C++).
2. Importar o JSON de receitas acima como `DataTable` (`Recipe` struct) em `/Game/Data/DT_CraftingRecipes`.
3. Consolidar os 9 triggers de crafting duplicados numa manutenção futura (não neste ciclo).
