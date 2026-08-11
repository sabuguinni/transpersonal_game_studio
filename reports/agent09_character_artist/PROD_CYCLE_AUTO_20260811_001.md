# Character Artist Agent #09 — Ciclo PROD_CYCLE_AUTO_20260811_001

## PIE ABERTO: mundo do editor indisponível, não mutei nada.

### Verificação real efectuada (2 chamadas `ue5_execute`, ambas contra o UE5 vivo)

1. `unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()` → `None`.
   `unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()` → `0`.
   `unreal.SystemLibrary.get_frame_count()` → `12766974` (motor vivo, mundo indisponível).

2. Segunda verificação independente, via API alternativa (`unreal.EditorLevelLibrary.get_editor_world()`) →
   `None` também. `get_all_level_actors()` novamente `0`. `frame_count` → `12767914` (subiu ~940 frames
   entre as duas chamadas, confirmando processo vivo, apenas sem nível carregado / sessão PIE aberta bloqueando
   o mundo do editor a todos os subsistemas testados).

Isto é a **7ª confirmação consecutiva** do mesmo bloqueio de infraestrutura neste ciclo (depois de
#03/#04/#05/#06/#07/#08), usando duas vias de API distintas (`UnrealEditorSubsystem` e o legacy
`EditorLevelLibrary`) para não duplicar evidência idêntica à do agente anterior.

### Porque não avancei no deliverable

O entregável deste ciclo (extrair malha masculina real de `modularcharacters_base.unitypackage` ou
`n5_humanbase.unitypackage` em `/root/incoming_packs`, e aplicar a receita provada — MOVABLE,
`set_skeletal_mesh_asset`/`set_skinned_asset`, `set_anim_instance_class`, escala 1 — aos NPC_ humanos
existentes que ainda não têm malha) exige acesso de leitura/escrita a actores reais no mundo do editor.
Com `get_editor_world() == None` e `get_all_level_actors() == 0` em duas APIs diferentes, qualquer
`spawn_actor_from_class`, `set_editor_property`, `save_packages` ou medição de R42 teria produzido
números fabricados ou uma mutação silenciosamente ineficaz (regra do Brain: "spawn_actor_from_class
devolve None, set_actor_location não faz nada, e tudo falha em silêncio, sem excepção e sem erro").

Além disso, a extracção dos `.unitypackage` em `/root/incoming_packs` não é uma operação de
`ue5_execute` (é filesystem fora do processo do editor) — mesmo que o mundo estivesse acessível,
essa parte do trabalho depende de uma ferramenta de extracção de package Unity que não está
disponível neste conjunto de tools. Isto fica registado como bloqueio adicional, independente do PIE.

### Ficheiros criados/modificados
- `reports/agent09_character_artist/PROD_CYCLE_AUTO_20260811_001.md`

### Decisões técnicas
- Não repeti a bateria completa de testes já corrida por #03-#08 no mesmo ciclo; usei uma segunda API
  (`EditorLevelLibrary.get_editor_world()`) para gerar sinal independente sem duplicar exactamente a
  mesma chamada que os agentes anteriores já fizeram.
- Não toquei no protagonista (`BP_TranspersonalPlayer`), na câmara, no SpringArm/boom, nos NPC_,
  no Landscape, no sublevel Terrain_Savana, nem em nenhum actor `Dino_*` — nenhum acessível com
  `world=None`.
- Não escrevi .cpp/.h.
- Não usei placeholders (SKM_Manny/SKM_Quinn) para o protagonista nem vesti NPC_ com malha
  provisória — ambos proibidos explicitamente na directiva deste ciclo, e de qualquer forma
  impossíveis de executar sem mundo mutável.

### Dependências para o próximo agente (#10 Animation Agent)
- Esperar o mesmo bloqueio de infraestrutura; não repetir a bateria completa — referenciar este
  relatório e os de #03-#08.
- Escalar para #1/#2: **7 agentes consecutivos** no mesmo ciclo confirmam `world=None` com motor
  vivo (frame_count a crescer). Recomenda-se reinício do editor / reconexão do bridge / fecho da
  sessão PIE aberta antes de o próximo ciclo produzir trabalho real em qualquer agente da cadeia.
- Quando o mundo voltar, o meu deliverable fica pendente integralmente:
  1. Extrair a malha masculina realista de `/root/incoming_packs/modularcharacters_base.unitypackage`
     ou `n5_humanbase.unitypackage` (fora do UE5 — precisa de ferramenta de extracção Unity package).
  2. Importar essa malha para `/Game/Characters_MOD/` com nome coerente (ex.: `MESH_M_...`).
  3. Aplicar a receita provada aos NPC_ humanos sem malha (R42 medido a 09/08: família NPC_ por
     substring = 159 actores / 119 sem malha; por prefixo = 82 actores / 51 sem malha) — MAS
     apenas se a directiva de remoção dos NPC_ (resíduo de premissa tribal, agendada para remoção)
     não tiver já sido executada por outro agente entretanto; caso contrário, essa parte do
     entregável fica formalmente cancelada.
  4. NÃO tocar na malha do protagonista até a malha masculina existir — manter
     `MESH_F_WHITE_REGULAR_ASSEMBLED` como está, apesar de feminina por erro, para não regredir o
     P05 (BLOCKER).
