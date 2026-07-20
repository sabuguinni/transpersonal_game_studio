# Hub Encounter Narrative Beats — Cycle PROD_CYCLE_AUTO_20260720_002

## Contexto
Trabalho realizado sobre actors JÁ EXISTENTES no hub (2100,2400), sem duplicação (regra `hugo_naming_dedup_v2`). Auditoria confirmou 505 actors de dinossauro e 1573 actors totais num raio de 1500 unidades do hub — densidade elevada, redundância de crafting/quest já assinalada em ciclos anteriores.

## Actors Actualizados (tags narrativas adicionadas, sem duplicação)

### `Helper_Actor_Trike_Hub_001` — (2050, 2650, z=80.1)
Herd anchor herbívoro (Herd_ID_0, Herd_Size_7, Quest_TrackHerd_Objective já presente).
Tags narrativas adicionadas:
- `Narr_Beat_FirstHerdSighting` — primeiro beat de descoberta: jogador encontra pegadas/manada perto do hub.
- `Narr_Line_Survivor_TrackTracks` — liga à linha de diálogo gravada (ver áudio abaixo).
- `Narr_Theme_TerritoryReading` — tema: ler o terreno para prever perigo (pragmático, não místico).

### `Helper_Actor_Raptor_Hub_001_Posed` — (2500, 2100, z=100)
### `Helper_Actor_Raptor_Hub_002_Posed` — (2100, 2800, z=100)
Pack hunters (PackID_RaptorSquad_A, CombatTactic_FlankAndSurround já presente).
Tags narrativas adicionadas:
- `Narr_Beat_FirstPredatorWarning` — beat de tensão: jogador percebe que a manada está a ser caçada.
- `Narr_Line_Survivor_PackCount` — liga a contagem de indivíduos da alcateia como informação de sobrevivência.
- `Narr_Theme_FlankAwareness` — tema: consciência táctica de flanqueamento, reforça mecânica de combate do #12.

## Verificação no mundo (real, via ue5_execute)
- Line trace ao Landscape em cada um dos 3 actors confirmou hit válido (struct HitResult não vazio) — os 3 permanecem correctamente assentes no terreno (z 80–100, dentro do intervalo 44–302 do núcleo jogável).
- Nenhum actor foi criado, movido ou apagado — apenas tags adicionadas via `a.tags.append(...)`.

## Áudio associado
Linha de voz gravada para o beat `Narr_Line_Survivor_TrackTracks` — texto de sobrevivência pragmático (sem misticismo):

> "Fresh tracks. Big herd, moving south along the ridge — three-toed, heavy. Triceratops, by the spacing. If they're grazing this close to the hub, water's near. But where there's a herd this size, there's something hunting it. Stay low. Watch the treeline."

Personagem: `Survivor_HerdSighting` (sobrevivente pragmático, não shaman/guia espiritual).

## Próximo agente (#16 Audio Agent)
- Integrar a linha de voz gerada como MetaSound/dialogue cue disparado quando o jogador entra no raio de `AggroRadius_600` / `SightRadius_2500` do `Helper_Actor_Trike_Hub_001`.
- Considerar segunda linha para o beat `Narr_Beat_FirstPredatorWarning` (alerta sobre a alcateia de raptors) — som de rosnado já referenciado em tags existentes (`SFX_Roar_LowGrowl_Radius2000`).
- NÃO duplicar actors — os 3 actors-alvo já têm layout e IA completos; o trabalho de áudio é puramente de trigger/cue.
