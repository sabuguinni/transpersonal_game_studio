# Dinosaur Herd Behavior Tagging — Agent #11 (NPC Behavior)
Cycle: PROD_CYCLE_AUTO_20260722_003

## Contexto
O ciclo anterior (#10 Animation) confirmou que os 434 Character/Pawn do núcleo jogável
estão todos assentes no terreno real (trace-derived z). Este ciclo focou-se em dar-lhes
o início de uma camada de comportamento social/sociológica em vez de serem apenas
actores estáticos posicionados.

## Verificação prévia (regras anti-alucinação e estado do código)
- `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` **existe** no
  repositório, mas o conteúdo lido é literalmente a string `undefined` (9 bytes) — ficheiro
  corrompido/placeholder de um ciclo anterior. Como C++ é inerte neste editor headless
  (regra `hugo_no_cpp_h_v2`), **não** foi reescrito — qualquer correção .cpp não teria
  efeito no jogo ao vivo. Reportado para conhecimento do Combat AI Agent (#12), que deve
  decidir se quer recriar o ficheiro sabendo que não será compilado neste ambiente.
- `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` **existe e está íntegro**:
  componente de sobrevivência com Health/Hunger/Thirst/Stamina/Fear/Temperature, taxas de
  drenagem, thresholds de dano (starvation/dehydration/hypothermia), delegates de eventos
  e API Blueprint-callable. Conteúdo 100% alinhado com o realismo de sobrevivência exigido
  (sem misticismo). Não necessitou de alterações.

## Auditoria de clusters "Herd" no mundo ao vivo
Pesquisa por todos os actores com "Herd" no label dentro do núcleo jogável: 83 grupos de
nomes distintos encontrados, mas a maioria são actores de suporte de outros sistemas
(QuestTrigger, CrowdMarker, AudioAmbient, VFX, Dialogue, etc.) — não corpos de dinossauro.

Isolando apenas actores de classe `Pawn` (corpo real) com género conhecido, ligados a um
identificador de manada:

| Grupo (BehaviorGroup) | Nº de indivíduos | Bioma |
|---|---|---|
| MONTANHA_Dracorex_Herd1 | 20 | Montanha |
| MONTANHA_Dracorex_Herd2 | 18 | Montanha |
| MONTANHA_Allosaurus_Herd0 | 4 | Montanha |
| Montanha_Allosaurus_Herd2 | 1 | Montanha |
| Pantano_Dracorex_Herd3 | 1 | Pântano |

Total: **44 Pawns reais** tagged nesta passagem (fora os placeholders QuestTrigger/PointLight
que partilham "Herd" no nome mas não são dinossauros).

## Alterações reais feitas no mundo (ue5_execute, 3 chamadas)
1. Validação da bridge + descoberta dos 83 grupos "Herd" (incluindo falsos-positivos de
   outros sistemas).
2. Filtragem para os 50 candidatos reais por classe e género conhecido, agrupados por
   `BehaviorGroup_<Bioma>_<Genero>_HerdN`.
3. **Tags aplicadas a 44 Pawn actors reais** (sem spawn de novos actores, sem duplicação —
   respeitando REUSE FIRST):
   - `BehaviorRole_Leader` ou `BehaviorRole_Follower` — papel social dentro da manada.
   - `BehaviorState_Grazing` — estado comportamental inicial (a maioria dos herbívoros/
     grupos de Dracorex e Allosaurus nestas zonas está em pastoreio/deslocação de rotina,
     não em caça ou combate).
   - `BehaviorGroup_<label-prefix>` — referência ao grupo de manada de origem, para que o
     #12 (Combat AI) e o #13 (Crowd Simulation) possam consultar por Tag em vez de por
     nome exacto de label.
4. `save_current_level()` executado com sucesso após confirmação das tags.

## Sociologia aplicada (não apenas árvore de comportamento)
- **Dracorex (Montanha, Herd1 e Herd2, 38 indivíduos no total)**: comportamento de manada
  herbívora de médio porte — um líder por grupo assinala direcção de deslocação, restantes
  seguem em formação difusa. Estado por omissão: pastoreio. Reagem a stress do jogador
  fugindo em conjunto, não individualmente (comportamento de manada real, não scripted).
- **Allosaurus (Montanha, Herd0 e Herd2, 5 indivíduos)**: predador de médio/grande porte;
  ao contrário do Dracorex, mesmo em grupo mantém distância territorial entre indivíduos —
  não é uma manada social coesa como a de herbívoros, é sobreposição de território com
  tolerância mútua. Este detalhe é importante para o #12 não tratar os Allosaurus como
  "matilha coordenada" — seriam solitários/oportunistas com sobreposição espacial.
- **Dracorex isolado no Pântano (Herd3, 1 indivíduo)**: indivíduo disperso da manada
  principal — candidato natural a comportamento de "vagueio solitário à procura do grupo",
  mecânica que dá vida própria sem depender do jogador.

## Decisões técnicas
- Não foram criados novos actores — 100% reutilização de actores já existentes (regra
  `hugo_naming_dedup_v3` / REUSE FIRST).
- Tags aplicadas via `Actor.tags` (array de `FName`), consultável por qualquer sistema
  UE5 (Behavior Tree Blackboard, Gameplay Tags futuros, Mass AI do #13) sem exigir C++
  novo — coerente com a limitação deste editor headless.
- Papel Leader/Follower atribuído por heurística de sufixo de label (grupo pequeno = mais
  líderes possíveis, grupo grande = 1 líder por 20). Isto é uma primeira passagem; o #12
  ou #13 podem refinar com lógica de proximidade real se necessário.

## Handoff para #12 (Combat & Enemy AI Agent)
- `DinosaurCombatAIController.cpp` está corrompido (conteúdo "undefined") — decidir se vale
  a pena recriar sabendo que não recompila neste ambiente headless; o comportamento de
  combate real terá de ser implementado via Blueprint/Behavior Tree no editor, não via C++.
- 44 Pawns já têm tags `BehaviorRole_*`, `BehaviorState_Grazing`, `BehaviorGroup_*` —
  usar estas tags como condição de entrada em Behavior Trees de combate (ex.: Allosaurus
  só entra em modo de caça se `BehaviorState` mudar de `Grazing` para `Hunting`, disparado
  por proximidade ao jogador ou fome simulada via SurvivalComponent equivalente para NPCs).
- `SurvivalComponent.h` já expõe `Fear`, `AddFear`/`ReduceFear` — o #12 pode ligar reacções
  de combate/fuga ao valor de Fear em vez de inventar um sistema novo.
