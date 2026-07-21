# Combat & Enemy AI — Ciclo PROD_CYCLE_AUTO_20260721_001 (Agente #12)

## Contexto herdado do Agente #11 (NPC Behavior)

O Agente #11 descobriu que os 58 actors "TRex" no `MinPlayableMap` são 100% VFX/Audio
(NiagaraActor, AmbientSound, Emitter, StaticMeshActor decorativo) — **não existe nenhum
corpo real de T-Rex** (SkeletalMeshActor ou Character) em todo o nível. Confirmei esse
achado neste ciclo por auditoria independente.

## Auditoria de duplicação (regra `hugo_combat_label_consistency_v1`)

Antes de criar qualquer `CombatZone_`/`BehaviorTag_`, procurei labels existentes a
menos de 3500 unidades do hub (2100, 2400):
- **CombatZone_\* encontrados: 0**
- **BehaviorTag_\* encontrados: 0**

Não havia nada para reutilizar — o histórico de ~1680 pares acumulados em ciclos
anteriores parece ter sido limpo ou nunca existiu neste snapshot do nível. Prossegui
para criar tags novas, mas apenas em corpos reais (ver abaixo), não em VFX fantasma.

## Descoberta chave deste ciclo: corpos de dinossauro REAIS existem — mas não são T-Rex

Auditando todos os `SkeletalMeshActor` com label contendo "TRex/Trike/Triceratops/Raptor/Brachio",
encontrei **11 corpos reais e legítimos** dentro do núcleo jogável (X -3000..5000, Y -1000..5500):

| Espécie | Contagem | Actors |
|---|---|---|
| Triceratops (Trike) | 6 | Trike_Savana_004, 005 (x2 — provável duplicado de posição diferente), 006, 007_grazing, 008 |
| Raptor | 5 | Raptor_Floresta_180_Posed, 181_Posed, 182_alert_Posed, Raptor_Savana_Hub_001, Raptor_Savana_Hub_002 |
| T-Rex | **0** | nenhum corpo real — apenas VFX/áudio (confirmado por #11 e por mim) |

Todos os 11 estão corretamente assentes no terreno (z entre 91.4 e 141.0, dentro do
intervalo de superfície válido 44–302) e nenhum teve a `mobility` alterada.

## Desenho de combate tático aplicado (tags reais, sem .cpp/.h)

Seguindo a regra `hugo_no_cpp_h_v2` (C++ é inerte neste editor headless), implementei
o comportamento de combate como **tags reais nos 11 actors vivos**, não como ficheiros
C++ mortos. Filosofia: "combate é uma conversa, não uma equação" — cada espécie tem um
arco de tensão distinto, não apenas um número de dano.

### Raptor — Caçador em Alcateia (Combat_PackHunter)
Tags aplicadas (22 por actor, incluindo as de #11 mais estas):
- `CombatRole_PackHunter`
- `Combat_AmbushTrigger_1500` — inicia perseguição furtiva a 1500u
- `Combat_FlankDistance_600` — tenta cercar o jogador a partir de 600u
- `Combat_AttackRange_200` — ataque de mordida a curto alcance
- `Combat_FleeHP_0.25_IfIsolated` — foge a 25% HP **se isolado** (não em alcateia)
- `Combat_PackBonus_DamagePlus15pct_Per_Ally_Within_800` — dano bónus por aliado próximo
- `BehaviorTag_CombatState_Stalk`, `BehaviorTag_GroupCoordination_Pack`, `BehaviorTag_RetreatIfAlone`

**Design intent:** um Raptor sozinho é vencível; uma alcateia de 3+ é uma emboscada real.
O jogador sente que perdeu por entrar no território errado sozinho, não porque o jogo
"roubou" — coerente com a convicção central do agente.

### Triceratops — Defensor Territorial (Combat_TerritorialDefender)
- `CombatRole_TerritorialDefender`
- `Combat_WarningDisplay_Range_1000` — exibição de aviso (cabeça baixa, bufar) a 1000u antes de atacar
- `Combat_ChargeTrigger_500` — carga real a 500u se o aviso for ignorado
- `Combat_ChargeDamage_60`
- `Combat_FrontalArmor_ReduceDamage_50pct` — couraça frontal reduz dano recebido de frente
- `Combat_FleeHP_0.15`
- `Combat_ChargeCooldown_4s` — janela de contra-ataque para o jogador após a carga
- `BehaviorTag_CombatState_Graze`, `BehaviorTag_ThreatEscalation_Display_Then_Charge`, `BehaviorTag_HoldGround`

**Design intent:** o Triceratops nunca ataca sem aviso — dá ao jogador uma janela clara
para recuar (display) antes da carga. A couraça frontal recompensa flanquear em vez de
enfrentar de frente — decisão tática, não sorte.

## Verificação em UE5 (3 chamadas ue5_execute reais)

1. Auditoria de labels `CombatZone_`/`BehaviorTag_` existentes perto do hub (0 encontrados)
   + inventário de classes dos 58 actors "TRex" (confirma 100% VFX/Audio).
2. Aplicação das tags de combate acima aos 11 `SkeletalMeshActor` reais (Trike + Raptor),
   sem criar novos actors. `total_actors` antes/depois: 3332 → 3332 (nenhuma duplicação).
3. Verificação final: todos os 11 actors com z entre 91.4–141.0 (dentro de 44–302, grounded),
   todos com ≥12 tags de combate aplicadas, nenhuma mobility alterada.

Nível gravado uma vez, no fim, após toda a verificação (regra "one save at the end").

## Pendência crítica para próximos ciclos (não resolvida por mim, fora do meu escopo)

Confirmando o que #11 já sinalizou: **não há malha de T-Rex em todo o nível**. Já
existem ~50+ actors VFX/Audio rotulados "TRex" simulando presença sem corpo. Recomendo
fortemente que #06 (Environment Artist) ou #09 (Character Artist) importem uma
SkeletalMesh real de Tyrannosaurus antes de qualquer agente futuro voltar a empilhar
tags de combate sobre os proxies VFX — isso seria o anti-padrão já sinalizado em
`hugo_naming_dedup_v2` (metadata sem geometria).

## Ficheiros

- **Criado:** `Docs/AI/Combat_AI_Tactical_Design_PROD_CYCLE_AUTO_20260721_001.md` (este ficheiro)
- **Nenhum .cpp/.h criado** — regra `hugo_no_cpp_h_v2` respeitada integralmente
- **Mundo live:** 11 actors reais (6 Trike + 5 Raptor) tagged com IA de combate tático,
  nível gravado

## Handoff para #13 (Crowd & Traffic Simulation)

Os 11 corpos reais com tags de combate estão prontos para qualquer camada de simulação
de massa que precises de aplicar. Não crie novos T-Rex — não há malha. Se precisares de
mais Raptors/Trikes para densidade de alcateia, reutiliza labels existentes
(`Type_Bioma_NNN`) antes de spawnar novos, conforme `hugo_naming_dedup_v2`.
