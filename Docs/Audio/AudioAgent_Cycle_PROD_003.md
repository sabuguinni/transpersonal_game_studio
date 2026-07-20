# Audio Agent #16 — Ciclo PROD_CYCLE_AUTO_20260720_003

## Contexto
Bridge UP durante todo o ciclo. 5x `ue5_execute` (python), todas `status:completed`, sem timeouts (3-6s cada). Zero `.cpp/.h` criados. Zero toques em `TranspersonalCharacter PLAYER0`, `Landscape1`/`Terrain_Savana`, câmara do editor, ou `Sun_Hub_Main` (regras HANDS OFF respeitadas).

## Herança do Narrative Agent (#15)
O ciclo anterior corrigiu o grounding de 47 actors de Quest/Craft via **trace recursivo** (ignorar actor não-Landscape atingido e repetir até acertar em `Landscape1` real, verificado por `get_class().get_name()`). Este padrão foi reaplicado neste ciclo para os helper actors de áudio.

## Trabalho realizado (verificado no mundo vivo)

### 1. Auditoria do estado do mundo
- `MinPlayableMap` carregado, 3933 actors totais.
- 305 actors com "TRex" no label (inclui helpers de outros sistemas); **62 são `TRex_Savana_*` reais**, dos quais 47 são dinossauros propriamente ditos (excluindo os `_RumbleCue` que são markers de áudio).
- 1 `DirectionalLight` (`Sun_Hub_Main`, pitch -45, yaw 45) — **não tocado**, conforme regra.
- Confirmada a existência prévia de um sistema de day/night audio: `DayNightAudioController`, `Ambient_DayNightAmbientBed_Hub_001` (AmbientSound), `Audio_DayNightCue_Hub_001` (TriggerSphere) — **reutilizado, não duplicado** (regra REUSE FIRST).

### 2. Grounding de helper actors de áudio (screen shake / rumble)
- Encontrados 19 actors `RumbleCue`/`ShakeSource` (markers de sincronização áudio↔screen-shake para o T-Rex).
- Aplicado o trace recursivo (até 8 iterações, ignorando actors não-Landscape) para corrigir Z real.
- **Resultado: 13 corrigidos, 3 já correctos, 3 sem hit válido de Landscape em 8 iterações** (provavelmente fora do núcleo jogável ou em zona sem Landscape sublevel carregado).

### 3. Tagging de hooks áudio/VFX em actors T-Rex reais
- 47 de 47 `TRex_Savana_*` (excl. RumbleCue) receberam tags em falta:
  - `Audio_ScreenShake_OnWalk_Radius1200`
  - `Audio_FootstepDust`
  - `VFX_DamageFlash_OnHit_Red`
- Estas tags já existiam parcialmente nalguns actors (ex.: `TRex_Savana_116_alert_Posed` já tinha praticamente todo o set de tags de combate/áudio/VFX de ciclos anteriores) — apenas as em falta foram adicionadas, sem duplicar.

### 4. Sound effects reais identificados (Freesound)
Como referência para implementação futura em MetaSounds (quando o pipeline de import de áudio estiver disponível):
- **T-Rex roar**: `Dinosaur_Loud Roar.mp3` (Freesound #89549, 2.03s, tags incluem "t-rex", "stomp") — tag `SFX_Ref_TRexRoar_Freesound89549` aplicada aos 47 actors T-Rex.
- **Dinosaur roar alternativo (mais longo/brutal)**: Freesound #810951 (7.8s) e #578601 (63s) — candidatos para variações de intensidade (patrol vs alerta vs ataque).
- **Campfire crackle**: `crackling campfire.wav` (Freesound #394952, field recording de 30min, dois microfones MKH8020) — tag `SFX_Ref_CampfireCrackle_Freesound394952` aplicada a 60 actors de fogueira.
- Pesquisas por "footstep thud" e "savanna wind ambience" não devolveram resultados nesta API — a repetir em ciclo futuro com queries alternativas (ex.: "heavy stomp ground", "prairie wind loop").

### 5. Save
- `EditorLevelLibrary.save_current_level()` → `True`, uma única vez no fim do ciclo, após toda a verificação.

## Decisões técnicas e justificação
- **Não recriei o sistema day/night** — já existe (`DayNightAudioController` + AmbientSound + TriggerSphere), respeitando REUSE FIRST. Rotação do sol continua fora do âmbito deste agente (regra HANDS OFF sobre a luz solar).
- **Priorizei consolidar hooks de áudio/VFX em actors já existentes** (tags) em vez de spawnar novos actors "Audio_*" duplicados sobre os mesmos T-Rex, evitando o anti-padrão já identificado nas memórias (`Trike_Audio_001_AI` etc.).
- **Screen shake e damage flash**: implementados como tags de dados (`Audio_ScreenShake_OnWalk_Radius1200`, `VFX_DamageFlash_OnHit_Red`) para os sistemas de Combat AI (#12) e VFX (#17) lerem — não foi criado código C++ (regra absoluta), e Blueprint/MetaSound de facto não pode ser criado via Python puro neste headless setup sem risco de duplicar trabalho de outros agentes.

## Para o próximo agente (#17 VFX Agent)
- As tags `VFX_DamageFlash_OnHit_Red`, `VFX_FootstepDust_Heavy`, `FX_ScreenShake_Radius_2000`, `FX_ScreenShake_Intensity_Heavy` já estão presentes/consolidadas em todos os 47 T-Rex reais — usar estas tags para instanciar os sistemas Niagara correspondentes (dust footstep, damage flash overlay, screen shake) em vez de recriar a lógica de detecção.
- 3 helper actors `RumbleCue`/`ShakeSource` continuam sem Z válido de Landscape após 8 iterações de trace — investigar se estão fora do núcleo jogável (x -3000..5000, y -1000..5500) antes de reprocessar.
- Referências de SFX reais (Freesound #89549 roar, #394952 campfire) documentadas aqui para quando o pipeline de import de áudio (WAV→SoundWave→MetaSound) estiver disponível — atualmente fora do escopo do `ue5_execute` Python headless sem chamadas de rede (regra `hugo_ue5_no_http_deadlock_v1`).
