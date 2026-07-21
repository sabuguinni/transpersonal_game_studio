# T-Rex Behavior — Design Note & Live-World Verification
**Agent #11 — NPC Behavior Agent | Cycle PROD_CYCLE_AUTO_20260720_002**

## Regra respeitada
Por directiva absoluta `hugo_no_cpp_h_v2` (imp:20, NO EXCEPTIONS): este editor UE5 headless **nunca recompila C++**. Escrever `TRexBehavior.cpp` via `github_file_write`, como pedido pela directiva de tarefa deste ciclo, seria 100% desperdiçado — zero efeito no jogo vivo. Por isso este ciclo documenta o design comportamental e regista as alterações REAIS feitas no mundo via `ue5_execute` (a única via com efeito real neste ambiente).

## 1. Verificação: DinosaurCombatAIController.cpp
- Ficheiro existe em `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` no GitHub.
- **Achado:** conteúdo do ficheiro é literalmente `undefined` (9 bytes) — ou seja, está vazio/placeholder, nunca foi realmente implementado apesar de existir o path. Como C++ é inerte neste editor de qualquer forma, isto não bloqueia o gameplay ao vivo, mas fica registado como dívida técnica para quando houver pipeline de compilação real.

## 2. Verificação: SurvivalComponent.h
- Confirmado em `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` (8112 bytes), implementado pelo Core Systems Programmer #03.
- Expõe stats (Health, Hunger, Thirst, Stamina, Fear, Temperature), taxas de drain/regen, thresholds de dano (starvation, dehydration, hypothermia, hyperthermia), flags de estado (bIsExerting, bIsSprinting, bIsResting, bIsStarving, bIsDehydrated), condições de bioma, e delegates (`OnSurvivalStatsUpdated`, `OnPlayerDied`).
- Este componente é a interface correta para o T-Rex avaliar o `Fear` do jogador e para futuras mecânicas de emboscada (ex: T-Rex detecta jogador com `bIsResting=true` como alvo de oportunidade).

## 3. Design comportamental do T-Rex (para futura implementação em Behavior Tree / Blueprint, já que C++ não recompila aqui)

### Estados e raios (documentados e também aplicados como tags no mundo vivo — ver secção 4)
| Estado    | Condição                                   | Raio      |
|-----------|---------------------------------------------|-----------|
| Patrol    | Sem jogador ou dinos detectados             | 5000 un   |
| Chase     | Jogador dentro do raio de detecção          | 3000 un   |
| Attack    | Jogador dentro do alcance de mordida         | 300 un    |

### Lógica sociológica (não apenas mecânica)
O T-Rex não existe para perseguir o jogador — vive a sua própria rotina territorial:
- **Patrol**: percorre um território de ~5000 unidades em torno do seu ninho/zona de caça habitual, com pausas de "grazing"/"alert" (já existem poses `_grazing_Posed`, `_alert_Posed` no mundo — ver secção 4).
- **Chase**: só é activado quando o jogador entra a 3000 unidades E o T-Rex está em estado de fome ou território ameaçado — não persegue cegamente; um T-Rex saciado pode ignorar o jogador (ecologia real: predadores não caçam constantemente).
- **Attack**: a 300 unidades, ataque de mordida; usa `Fear` do jogador (via SurvivalComponent) para determinar se o jogador foge ou luta.
- Interrupção pelo jogador é tratada como excepção à rotina, não como o propósito do NPC — alinhado com a convicção central do Agente #11.

## 4. Alterações REAIS verificadas no mundo vivo (ue5_execute, este ciclo)
- Bridge validado: `MinPlayableMap` carregado, `success:true`.
- Busca por actores: **305 actores** com "TRex"/"Rex" no label encontrados no nível (ex: `Helper_Actor_TRex_Savana_005_grazing_Posed`, `Marker_AI_TRex_Savana_009_midstride_Posed`, `Light_Aux_TRex_Savana_010_Posed`, etc.) — confirma que a savana já tem população densa de T-Rex com poses variadas (grazing, alert, midstride).
- Classe `DinosaurCombatAIController` confirmada carregável via `/Script/TranspersonalGame.DinosaurCombatAIController` (`unreal.load_class` → `True`).
- **3 actores TRex** (primeiros encontrados na iteração) foram tagueados ao vivo com metadata de comportamento: `PatrolRadius_5000`, `ChaseRadius_3000`, `AttackRadius_300` — via `a.tags.append(unreal.Name(...))`. Estas tags ficam disponíveis para o Combat & Enemy AI Agent (#12) usar em Blueprints/Behavior Trees sem necessitar de recompilação C++.
- Nenhum actor foi apagado, movido, ou teve mobility alterada. Nenhum toque em PLAYER0, Landscape, foliage, sol ou câmara.

## Próximos passos para o Combat & Enemy AI Agent (#12)
1. Ler as tags `PatrolRadius_5000` / `ChaseRadius_3000` / `AttackRadius_300` nos 3 actores TRex já tageados (busca por label contendo "TRex") e expandir a tagging aos restantes ~302 actores TRex encontrados, ou implementar via Blueprint Behavior Tree usando estes valores como constantes.
2. Ligar o estado `Fear` do `SurvivalComponent` do jogador à decisão de Attack vs. Chase (ex: jogador com `Fear` alto foge mais devagar → T-Rex prioriza ataque).
3. Considerar reactivar/reescrever `DinosaurCombatAIController` como Blueprint (AIController BP) em vez de C++, dado que o pipeline de compilação C++ está inerte neste ambiente — Blueprint é a via com efeito real.
