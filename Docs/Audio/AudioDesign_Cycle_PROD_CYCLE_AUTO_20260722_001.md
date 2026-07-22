# Audio Design — Ciclo PROD_CYCLE_AUTO_20260722_001 (Agent #16)

## Directiva do ciclo
Polish & Effects: screen shake perto do T-Rex, damage flash (overlay vermelho), footstep dust (jogador + dinossauros), day/night cycle.

## Bridge: UP
3x `ue5_execute` (`python`), todos `status:completed`, ~3s cada, zero timeouts.

## Ações reais executadas no mundo vivo (verificadas)

### 1. Footstep dust + footstep audio — consistência em 107 Character-class actors
Auditoria encontrou 107 actores de classe `Character` no mapa (NPCs, tribais, crowd actors). **Nenhum** dos actores placed é `PLAYER0` — confirma a memória `hugo_player_is_class_not_actor_v1` (o jogador nasce via GameMode, não é actor colocado), portanto a regra HANDS OFF ao `PLAYER0` foi respeitada por inexistência de alvo, não por omissão.

- **0** já tinham tag de footstep dust.
- **107** actualizados com: `VFX_FootstepDust_Enabled`, `Audio_FootstepSFX_Linked`, `FootstepSound_LightPad` (som mais leve que o `FootstepSound_Heavy_Thud` já usado pelos T-Rex).
- Exemplos: `AnimatedCharacter_130`, `AnimatedCharacter_162`, `Player_Character_Visual`, `TribalHunter_Montanha_01`, `ElderTracker_Savana`, `CrowdActor_Pantano_0`.

### 2. Damage flash — consistência
- **65 de 107** Character actors não tinham tag de damage flash; adicionadas `VFX_DamageFlash_RedOverlay_0.3s` + `FX_DamageFlashOverlay` (mesmo padrão já usado por `AnimatedCharacter_162` e `Player_Character_Visual` em ciclos anteriores).
- 42 já estavam correctamente tagueados (nenhuma duplicação).

### 3. Screen shake por proximidade do T-Rex
- **112** actores com "TRex"/"Trex" no label auditados.
- **58** ainda não tinham tag de screen shake; adicionadas `ScreenShake_Proximity`, `ShakeRadius_1500`, `ScreenShake_Radius_1500_Intensity_0.6` (consistente com o padrão já estabelecido em `Audio_TRexProximity_001` e `Audio_TRexRoarProximity_001` de ciclos anteriores).
- **54** já estavam tagueados corretamente (sem duplicação de tags).

### 4. Day/night cycle — BLOQUEADO por regra HANDS OFF
A directiva pedia "rotating directional light" para ciclo dia/noite. **Não implementado** — a regra `PLAYABLE-FIRST v4` (HANDS OFF) proíbe explicitamente modificar o sol (`Sun_Main_Directional`, único DirectionalLight no mapa, pitch=-45°, yaw=0°). Confirmado no mundo que o actor não foi tocado (leitura pós-ação idêntica à leitura pré-ação). Esta funcionalidade fica bloqueada até o Hugo dar instrução explícita para alterar o sol, ou até se criar um sistema separado (ex: Blueprint de Sequencer controlando exposição/cor sem rodar o actor Sun em si) — decisão que não me compete tomar autonomamente por implicar mudança de arquitectura de iluminação (regra "CONFLITO CRÍTICO" da autonomia).

### 5. Nota de duplicação observada (não corrigida neste ciclo)
Durante a auditoria encontrei actores com labels "matryoshka" ainda presentes (ex: `FX_FootstepDust_VFX_DustBurst_TRex_Savana_001_alert_Posed`), o padrão descrito na memória `hugo_cleanup_cycle17_v1` como tarefa de limpeza *one-time* de um ciclo passado. Não fiz limpeza/eliminação neste ciclo (fora do escopo da minha directiva actual e a memória indica que é tarefa pontual já executada antes) — reporto para o Integration/QA Agent avaliar se há recorrência do padrão em ciclos de áudio/VFX subsequentes.

## Save
1x `save_current_level()` no final do turno → `True` (aviso de depreciação do plugin Editor Scripting Utilities, sem erro funcional).

## Pesquisa de sons (Freesound)
- Query "heavy dinosaur footstep thud dust" e "tribal wood stone crafting impact hit" → 0 resultados (queries compostas demasiado específicas).
- Query simplificada "footstep dust" → 5 resultados (foley de crackers usável como base de footstep leve/dust, ex: `Graham Cracker Steps` #414310, `Cracker Foley 6 Far.wav` #415639).
- Segunda query "large creature roar low growl" tentada para roar de T-Rex (ver resultado na chamada da tool).

## Voice sample (ElevenLabs)
Gerada a linha do Beat 3 da Game Bible ("Dois. Nunca há só um...") pedida pelo Narrative Agent #15. **Upload para storage falhou novamente**: `403 Unauthorized / Invalid Compact JWS`. Confirma que é falha de infraestrutura (credenciais/token do Supabase Storage expiradas ou mal configuradas), não reprodutível por acção do agente — o áudio foi sintetizado com sucesso do lado do ElevenLabs, só o upload falha. Recomendo ao Studio Director (#01) escalar para verificação das credenciais de storage (JWS/JWT do serviço).

## Ficheiros criados
- `Docs/Audio/AudioDesign_Cycle_PROD_CYCLE_AUTO_20260722_001.md` (este documento)

## Dependências / próximos passos
- **#17 VFX Agent**: usar as tags `VFX_FootstepDust_Enabled` / `VFX_DamageFlash_RedOverlay_0.3s` / `ScreenShake_Proximity` já consolidadas em 100% dos Character e TRex actors para implementar os Niagara systems e Blueprint de camera shake correspondentes.
- **#01/#02**: decidir arquitectura de day/night cycle sem tocar directamente no actor Sun (ex: Sequencer track, ou parâmetro de tempo dirigindo intensidade/cor via material, ou criar um segundo light "moon" complementar) — está bloqueado por regra HANDS OFF.
- **Infra**: falha 403 no upload de TTS para storage persiste há 2 ciclos consecutivos (#15 e #16) — precisa de atenção de infraestrutura fora do âmbito dos agentes de conteúdo.
