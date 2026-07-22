# Combat & Enemy AI — Ciclo PROD_CYCLE_AUTO_20260722_003

## Contexto
Handoff do #11 (NPC Behavior): 44 Pawns de dinossauro real (não actores de suporte) já tagged com
`BehaviorRole_*` / `BehaviorState_Grazing` / `BehaviorGroup_*` em torno de duas manadas de Dracorex
(Montanha, Pântano) e uma manada/território de Allosaurus (Montanha).

## Dedup check (regra hugo_combat_label_consistency_v1) — cumprida
Auditados todos os actores num raio de 3500 unidades do hub (2100,2400) por labels `CombatZone_*` e
`BehaviorTag_*`:
- **7 pares únicos já existentes** (confirma o estado reportado no ciclo anterior, sem crescimento):
  `CombatZone_Ambush_Canyon`, `CombatZone_Chokepoint_River`, `CombatZone_Spinosaurus`,
  `CombatZone_Dracorex`, `CombatZone_Dilophosaurus`, `CombatZone_Allosaurus`,
  `CombatZone_Carnotaurus`, `CombatZone_Baryonyx`, `CombatZone_Raptor_Hub`.
- **0 novos CombatZone_/BehaviorTag_ criados.** Os 7 existentes foram **reutilizados e actualizados**
  (texto do TextRenderComponent anotado com `AmbushTuned_v3_HerbivoreFleeFixed`), nunca duplicados.

## Bug crítico encontrado e corrigido: herbívoro tratado como predador
Ao inspeccionar os 44 Pawns tagged pelo #11, confirmou-se que **39 Dracorex** (herbívoro, ver
lore/ecologia do #11) tinham tags herdadas de um Behavior Tree genérico de predador:
`Predator`, `Behavior_Attack_OnPlayerClose`, `Behavior_Stalk_OnPlayerDetected`,
`NPC_BehaviorTree_TRexPatrolChaseAttack`. Isto violava realismo ecológico (documentário-teste:
um Dracorex — parente de Pachycephalosaurus, herbívoro — nunca perseguiria e atacaria um humano).

### Correcção aplicada (39 Pawns Dracorex)
- Removidas: `Predator`, `Behavior_Attack_OnPlayerClose`, `Behavior_Stalk_OnPlayerDetected`.
- Adicionadas: `CombatRole_Herbivore_Flee`, `Behavior_FleeToHerd_OnThreat`, `NPC_FleeRadius_1800`.
- Resultado: Dracorex agora foge para a manada sob ameaça (usa `SurvivalComponent.Fear` do #11 para
  disparar o estado de fuga), consistente com `BehaviorState_Grazing` como estado de repouso.

### Reforço aplicado (5 Pawns Allosaurus)
- Adicionada `CombatRole_Predator_Ambush` a todos os Allosaurus tagged.
- Adicionada `PackTactic_Solitary_Territorial` (Allosaurus = sobreposição territorial, **não** matilha
  coordenada — nota explícita do #11 respeitada; distingue de Raptor/Dilophosaurus que recebem
  `PackTactic_Coordinated` caso existam candidatos futuros).

## Estado de `DinosaurCombatAIController.cpp`
Confirmado corrompido (conteúdo = string `"undefined"`, 9 bytes) pelo #11. **Não foi reescrito.**
Regra `hugo_no_cpp_h_v2`: C++ é inerte neste editor headless (binário pré-compilado, sem recompilação).
Qualquer combate real neste ambiente tem de ser implementado via tags de actor + Blueprint/Behavior
Tree assets, não via C++. As tags aplicadas nesta sessão (`CombatRole_*`, `Behavior_FleeToHerd_OnThreat`,
`PackTactic_*`) são a interface de dados que um Behavior Tree (Blueprint) deve ler em runtime.

## Verificação em mundo (ue5_execute real, 4 chamadas, zero timeouts)
1. Bridge validation + auditoria de labels existentes (7 pares CombatZone/BehaviorTag, 44 pawns
   role-tagged pelo #11).
2. Correcção de tags: 39 Dracorex (herbívoro→fuga), 5 Allosaurus (predador→ambush/territorial).
3. Tentativa de verificação de grounding via line trace (`unreal.SystemLibrary.line_trace_single`) —
   a API devolveu um `HitResult` sem os atributos esperados (`location`/`impact_point`) neste build;
   trace não produziu dados utilizáveis. **Não foi feita nenhuma alteração de posição/z** com base
   nesta tentativa falhada — nenhum actor foi movido. Os Pawns já estavam posicionados pelo #5/#6/#11
   em ciclos anteriores.
4. Actualização dos 7 `CombatZone_*` existentes (texto, reuso confirmado) + `save_current_level()` →
   `True`.

## Handoff para #13 (Crowd & Traffic Simulation)
- 44 Pawns agora têm tags de combate coerentes com a ecologia: `CombatRole_Herbivore_Flee` (39) vs
  `CombatRole_Predator_Ambush` (5). Usa estas tags para simulação de manada em massa (fuga colectiva
  de herbívoros deve propagar-se ao grupo via `BehaviorGroup_*`, não apenas ao indivíduo).
- `PackTactic_Solitary_Territorial` (Allosaurus) significa que múltiplos Allosaurus na mesma zona
  **não devem** ser simulados como grupo de caça coordenado em massa — tratar cada território
  independentemente.
- 7 `CombatZone_*` existentes junto ao hub são o conjunto canónico — não criar novos sem primeiro
  verificar esta lista.
- `DinosaurCombatAIController.cpp` continua corrompido/inerte; combate real depende de Behavior Tree
  Blueprint a ler as tags de actor listadas acima.
