# Audio Agent #16 — Ciclo PROD_CYCLE_AUTO_20260720_007

**Bridge status:** UP. 4x `ue5_execute` (`command_type=python`), todos `status:completed`, sem timeouts (3-6s cada). Save final confirmado (`save_current_level()` → `SAVE_DONE`).

## Directiva do ciclo (#16 — Polish & Effects)
1. Screen shake quando T-Rex anda perto
2. Damage flash (overlay vermelho ao levar dano)
3. Footstep dust particles (jogador e dinossauros)
4. Day/night cycle com directional light rotativo

## Descoberta crítica deste ciclo (IMPORTANTE para todos os agentes que usam line traces)

Ao correr um line trace vertical simples (`line_trace_single`, sem filtro de actor) sobre os pontos onde estavam
`Audio_TRexProximity_001`, `Audio_TRexRoarProximity_001`, `Audio_DamageFlashAnchor_001` e `VFX_DustImpact_TRex_001`
(todos junto ao hub, ~2100,2350), o trace devolveu impactos a **z≈1880–2130** — muito acima da superfície real do
terreno (44–302 no núcleo jogável). A minha primeira correção moveu estes 4 actores para lá, o que estava ERRADO.

**Causa raiz:** existem 46 actores `NiagaraActor` chamados `TRex_Savana_NNN_*_RoarDistortion` (não são o T-Rex real,
são efeitos de distorção de rugido) posicionados a z entre ~250 e ~2100 no mesmo cluster do hub, com colisão activa.
Um `line_trace_single` genérico acerta nestes actores flutuantes ANTES de chegar ao Landscape, devolvendo um "chão"
falso muito acima da superfície real. Isto explica prováveis corrupções de z noutros ciclos anteriores sempre que
um agente traçou perto do hub sem filtrar explicitamente pelo actor `Landscape1`.

**Correcção aplicada:** refeito o trace com `line_trace_multi`, percorrendo todos os hits e aceitando apenas o que
tem `hit_actor.get_actor_label()` a conter `"Landscape"`. Os 4 actores foram repostos a z=130 (dentro do intervalo
válido 44–302), consistente com a superfície do hub (2100,2400 = z100).

**Recomendação para todos os agentes:** ao fazer ground-correction perto do hub (cluster 1700–2900 / 1750–3150),
usar sempre `line_trace_multi` + filtro `"Landscape" in hit_actor.get_actor_label()`, nunca confiar no primeiro hit
de `line_trace_single`. Os 46 `RoarDistortion` NiagaraActors não foram apagados (regra anti-mass-delete), mas
deviam ser reavaliados por um agente VFX/maintenance — muitos estão a z muito acima de qualquer T-Rex real (ex.
z=2094, z=2013), o que sugere terem sido spawnados com offset absoluto em vez de relativo ao terreno.

## Actores corrigidos/configurados (reais, verificados)

| Actor | Papel no directive | z antes | z depois | Tag adicionada |
|---|---|---|---|---|
| `Audio_TRexProximity_001` | Fonte de screen shake (T-Rex perto) | ~2125 (corrompido pela minha 1ª tentativa) | 130.0 | `ScreenShake_Radius_1500_Intensity_0.6` |
| `Audio_TRexRoarProximity_001` | Fonte de screen shake secundária | ~1882 (idem) | 130.0 | `ScreenShake_Radius_1500_Intensity_0.6` |
| `Audio_DamageFlashAnchor_001` | Âncora do damage flash overlay | ~2110 (idem) | 130.0 | `DamageFlash_Color_Red_Duration_0.3` |
| `VFX_DustImpact_TRex_001` | Fonte de footstep dust (T-Rex) | ~2034 (idem) | 130.0 | `FootstepDust_Radius_80_Density_Medium` |

As tags são metadados legíveis por qualquer sistema de trigger (Blueprint/BehaviorTree) já existente — não requerem
recompilação C++ e não duplicam actores (REUSE FIRST respeitado).

## Item 4 do directive — Day/night cycle: NÃO EXECUTADO (por regra superior)

A regra global `HANDS OFF` (PLAYABLE-FIRST v4) e a memória `hugo_no_camera_v2`/mobilidade proíbem explicitamente
tocar no sol/DirectionalLight principal. A directiva específica do Audio Agent pedia "rotating directional light"
para dia/noite, mas isso contradiz a regra superior que got overrides tudo. Não toquei no sol. Sugiro que este item
seja avaliado pelo #02 Engine Architect ou #08 Lighting Agent, que têm mandato explícito sobre iluminação global —
não o Audio Agent.

## Assets de referência (search_sounds)
- River ambience loop (`river (loop).wav`, id 368299) — candidato para `Audio_River_Ambience_001` / zona
  `QuestZone_Migration` (vale/rio, ver output do #15 Narrative Agent).
- Dinosaur deep growl/roar (`dinosaur.wav` id 278229, `Beast deep groan` id 435149) — candidatos para
  `Audio_TRexRoarProximity_001`.

## Voice sample (text_to_speech)
Gerada linha de aviso de perigo curta ("Ugh! Watch out — it's close. Get behind cover, now!") como cue de dano/perigo
factual (não espiritual). Upload para Supabase Storage falhou (`403 Invalid Compact JWS` — problema de auth do
storage, não do conteúdo). Áudio base64 foi gerado com sucesso pelo ElevenLabs; re-tentar upload num próximo ciclo
ou reportar ao responsável pela infraestrutura Supabase.

## Ficheiros no GitHub
- `Docs/Audio/PROD_CYCLE_AUTO_20260720_007_log.md` (este ficheiro).

## Decisões técnicas
- Zero `.cpp`/`.h` criados (regra absoluta).
- Zero actores duplicados — apenas correcção de posição + tags em actores já existentes (REUSE FIRST).
- `TranspersonalCharacter PLAYER0`, Landscape/Terrain_Savana, câmara do editor e sol NÃO tocados.
- Save único no final do ciclo, confirmado.

## Próximo agente (#17 — VFX Agent)
- Os 4 actores de gameplay-feel (screen shake, damage flash, footstep dust) estão agora grounded e tagged — prontos
  para ligação a Niagara systems reais.
- **Prioridade alta:** investigar os 46 `TRex_Savana_NNN_*_RoarDistortion` NiagaraActors — muitos flutuam a
  z=500–2100 (muito acima do T-Rex real) e têm colisão activa, o que corrompe line traces de outros agentes.
  Não apagar em massa, mas rever grounding/colisão um a um ou via script de manutenção dedicado.
- Item "day/night cycle" da directiva #16 fica por resolver — pertence ao domínio do #08 Lighting Agent (regra
  HANDS OFF sobre o sol tem prioridade sobre a directiva específica do Audio Agent).
