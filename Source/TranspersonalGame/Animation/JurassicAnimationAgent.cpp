#include "JurassicAnimationAgent.h"
#include "Animation/AnimInstance.h"
#include "Animation/PoseSearchDatabase.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "IKRigDefinition.h"
#include "ControlRig.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Core/TranspersonalGameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogJurassicAnimation, Log, All);

UJurassicAnimationAgent::UJurassicAnimationAgent()
{
    // Configuração padrão
    bIsInitialized = false;
    bUseMotionMatching = true;
    bUseAdaptiveIK = true;
    bUseProceduralGestures = true;
    bUseEmotionalAnimations = true;
    bUseSurvivalStateAnimations = true;
    
    GlobalAnimationSpeed = 1.0f;
    StressInfluenceWeight = 0.7f;
    SurvivalInfluenceWeight = 0.8f;
    PersonalityInfluenceWeight = 0.9f;
    
    // Configurar arquétipos de linguagem corporal
    InitializeArchetypeBodyLanguage();
    
    // Configurar dados de motion matching
    InitializeArchetypeMotionData();
}

void UJurassicAnimationAgent::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Initializing Jurassic Animation Agent..."));
    
    // Carregar configurações de animação
    LoadAnimationConfiguration();
    
    // Configurar sistema de Motion Matching
    SetupMotionMatchingSystem();
    
    // Configurar sistema de IK adaptativo
    SetupAdaptiveIKSystem();
    
    // Configurar sistema de gestos procedurais
    SetupProceduralGestureSystem();
    
    bIsInitialized = true;
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Jurassic Animation Agent initialized successfully"));
}

void UJurassicAnimationAgent::Deinitialize()
{
    // Cleanup
    ArchetypeBodyLanguageMap.Empty();
    ArchetypeMotionDataMap.Empty();
    ActiveCharacterAnimations.Empty();
    
    Super::Deinitialize();
}

void UJurassicAnimationAgent::InitializeArchetypeBodyLanguage()
{
    // Paleontologista - Curiosidade Científica
    FArchetypeBodyLanguage PaleontologistBodyLanguage;
    PaleontologistBodyLanguage.Archetype = EBodyLanguageArchetype::ScientificCuriosity;
    PaleontologistBodyLanguage.MovementPersonality = EMovementPersonality::Contemplative;
    PaleontologistBodyLanguage.PostureWeight = 0.8f; // Postura ereta, confiante
    PaleontologistBodyLanguage.GestureFrequency = 0.7f; // Gestos frequentes ao explicar
    PaleontologistBodyLanguage.EyeContactLevel = 0.9f; // Olhar direto, investigativo
    PaleontologistBodyLanguage.PersonalSpaceRadius = 120.0f; // Confortável com proximidade
    PaleontologistBodyLanguage.MovementSpeed = 0.9f; // Ligeiramente mais lento, observador
    PaleontologistBodyLanguage.TensionLevel = 0.3f; // Relaxado mas alerta
    PaleontologistBodyLanguage.HeadTiltAngle = 5.0f; // Ligeira inclinação curiosa
    PaleontologistBodyLanguage.ShoulderTension = 0.2f; // Ombros relaxados
    PaleontologistBodyLanguage.HandGestureStyle = 0.8f; // Gestos expressivos
    PaleontologistBodyLanguage.BlinkRate = 0.8f; // Piscar menos frequente (concentração)\n    PaleontologistBodyLanguage.BreathingDepth = 0.8f; // Respiração profunda, controlada
    
    ArchetypeBodyLanguageMap.Add(ECharacterArchetype::Protagonist_Paleontologist, PaleontologistBodyLanguage);
    
    // Sobrevivente Pesquisador - Vigilância Constante
    FArchetypeBodyLanguage SurvivorResearcherBodyLanguage;
    SurvivorResearcherBodyLanguage.Archetype = EBodyLanguageArchetype::ConstantVigilance;
    SurvivorResearcherBodyLanguage.MovementPersonality = EMovementPersonality::Cautious;
    SurvivorResearcherBodyLanguage.PostureWeight = 0.6f; // Postura mais baixa, defensiva
    SurvivorResearcherBodyLanguage.GestureFrequency = 0.4f; // Gestos contidos
    SurvivorResearcherBodyLanguage.EyeContactLevel = 0.6f; // Olhar esquivo, verificando arredores
    SurvivorResearcherBodyLanguage.PersonalSpaceRadius = 200.0f; // Maior distância pessoal
    SurvivorResearcherBodyLanguage.MovementSpeed = 1.1f; // Movimentos mais rápidos
    SurvivorResearcherBodyLanguage.TensionLevel = 0.7f; // Tenso, sempre alerta
    SurvivorResearcherBodyLanguage.HeadTiltAngle = -2.0f; // Cabeça ligeiramente baixa
    SurvivorResearcherBodyLanguage.ShoulderTension = 0.8f; // Ombros tensos
    SurvivorResearcherBodyLanguage.HandGestureStyle = 0.3f; // Gestos contidos, defensivos
    SurvivorResearcherBodyLanguage.BlinkRate = 1.3f; // Piscar mais frequente (stress)
    SurvivorResearcherBodyLanguage.BreathingDepth = 0.6f; // Respiração mais superficial
    
    ArchetypeBodyLanguageMap.Add(ECharacterArchetype::Survivor_Researcher, SurvivorResearcherBodyLanguage);
    
    // Explorador - Confiança Territorial
    FArchetypeBodyLanguage ExplorerBodyLanguage;
    ExplorerBodyLanguage.Archetype = EBodyLanguageArchetype::TerritorialConfidence;
    ExplorerBodyLanguage.MovementPersonality = EMovementPersonality::Confident;
    ExplorerBodyLanguage.PostureWeight = 0.9f; // Postura muito ereta, dominante
    ExplorerBodyLanguage.GestureFrequency = 0.6f; // Gestos decididos
    ExplorerBodyLanguage.EyeContactLevel = 0.8f; // Olhar direto, assertivo
    ExplorerBodyLanguage.PersonalSpaceRadius = 100.0f; // Confortável em espaços pequenos
    ExplorerBodyLanguage.MovementSpeed = 1.2f; // Movimentos decididos
    ExplorerBodyLanguage.TensionLevel = 0.4f; // Controlado mas pronto
    ExplorerBodyLanguage.HeadTiltAngle = 3.0f; // Queixo ligeiramente levantado
    ExplorerBodyLanguage.ShoulderTension = 0.3f; // Ombros para trás, confiantes
    ExplorerBodyLanguage.HandGestureStyle = 0.7f; // Gestos amplos, territoriais
    ExplorerBodyLanguage.BlinkRate = 0.9f; // Piscar normal
    ExplorerBodyLanguage.BreathingDepth = 0.9f; // Respiração profunda, controlada
    
    ArchetypeBodyLanguageMap.Add(ECharacterArchetype::Survivor_Explorer, ExplorerBodyLanguage);
    
    // Militar - Disciplina Militar
    FArchetypeBodyLanguage MilitaryBodyLanguage;
    MilitaryBodyLanguage.Archetype = EBodyLanguageArchetype::MilitaryDiscipline;
    MilitaryBodyLanguage.MovementPersonality = EMovementPersonality::Hyperalert;
    MilitaryBodyLanguage.PostureWeight = 1.0f; // Postura militar perfeita
    MilitaryBodyLanguage.GestureFrequency = 0.3f; // Gestos mínimos, controlados
    MilitaryBodyLanguage.EyeContactLevel = 0.7f; // Olhar direto mas profissional
    MilitaryBodyLanguage.PersonalSpaceRadius = 150.0f; // Distância respeitosa
    MilitaryBodyLanguage.MovementSpeed = 1.0f; // Movimentos precisos
    MilitaryBodyLanguage.TensionLevel = 0.5f; // Controlado, disciplinado
    MilitaryBodyLanguage.HeadTiltAngle = 0.0f; // Cabeça perfeitamente alinhada
    MilitaryBodyLanguage.ShoulderTension = 0.4f; // Ombros quadrados
    MilitaryBodyLanguage.HandGestureStyle = 0.2f; // Gestos mínimos, precisos
    MilitaryBodyLanguage.BlinkRate = 0.7f; // Piscar controlado
    MilitaryBodyLanguage.BreathingDepth = 0.8f; // Respiração disciplinada
    
    ArchetypeBodyLanguageMap.Add(ECharacterArchetype::Survivor_Military, MilitaryBodyLanguage);
    
    // Adicionar outros arquétipos conforme necessário...
}

void UJurassicAnimationAgent::InitializeArchetypeMotionData()
{
    // Configurar dados de motion matching para cada arquétipo
    // Estes serão preenchidos com os assets reais quando estiverem disponíveis
    
    for (auto& ArchetypePair : ArchetypeBodyLanguageMap)
    {
        FArchetypeMotionData MotionData;
        MotionData.bUseMotionMatching = true;
        MotionData.bUseAdaptiveIK = true;
        MotionData.bUseProceduralGestures = true;
        MotionData.BlendTime = 0.2f;
        MotionData.MotionMatchingWeight = 1.0f;
        MotionData.IKWeight = 1.0f;
        
        ArchetypeMotionDataMap.Add(ArchetypePair.Key, MotionData);
    }
}

void UJurassicAnimationAgent::LoadAnimationConfiguration()
{
    // Carregar configurações de animação do Data Asset
    // Esta função será expandida quando os assets estiverem disponíveis
    
    UE_LOG(LogJurassicAnimation, Log, TEXT("Loading animation configuration..."));
    
    // Por agora, usar configurações padrão
    // TODO: Carregar de Data Asset quando disponível
}

void UJurassicAnimationAgent::SetupMotionMatchingSystem()
{
    UE_LOG(LogJurassicAnimation, Log, TEXT("Setting up Motion Matching system..."));
    
    // Configurar o sistema de Motion Matching
    // Integração com o UMotionMatchingSubsystem
    
    if (UWorld* World = GetWorld())
    {
        if (UMotionMatchingSubsystem* MMSubsystem = World->GetSubsystem<UMotionMatchingSubsystem>())
        {
            // Registrar este agente com o subsistema de Motion Matching
            MMSubsystem->RegisterAnimationAgent(this);
        }
    }
}

void UJurassicAnimationAgent::SetupAdaptiveIKSystem()
{
    UE_LOG(LogJurassicAnimation, Log, TEXT("Setting up Adaptive IK system..."));
    
    // Configurar parâmetros padrão para IK adaptativo
    DefaultIKSettings.bEnableFootIK = true;
    DefaultIKSettings.bEnableHandIK = false;
    DefaultIKSettings.FootIKRange = 50.0f;
    DefaultIKSettings.IKBlendSpeed = 5.0f;
    DefaultIKSettings.TerrainTraceDistance = 100.0f;
    DefaultIKSettings.FootOffsetInterpSpeed = 15.0f;
}

void UJurassicAnimationAgent::SetupProceduralGestureSystem()
{
    UE_LOG(LogJurassicAnimation, Log, TEXT("Setting up Procedural Gesture system..."));
    
    // Configurar sistema de gestos procedurais
    // Estes gestos serão baseados no estado emocional e arquétipo do personagem
}

UAnimInstance* UJurassicAnimationAgent::CreateAnimationInstanceForCharacter(
    USkeletalMeshComponent* SkeletalMeshComponent, 
    ECharacterArchetype Archetype)
{
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogJurassicAnimation, Warning, TEXT("Invalid SkeletalMeshComponent provided"));
        return nullptr;
    }
    
    // Obter dados do arquétipo
    const FArchetypeBodyLanguage* BodyLanguage = ArchetypeBodyLanguageMap.Find(Archetype);
    const FArchetypeMotionData* MotionData = ArchetypeMotionDataMap.Find(Archetype);
    
    if (!BodyLanguage || !MotionData)
    {
        UE_LOG(LogJurassicAnimation, Warning, TEXT("No animation data found for archetype: %d"), (int32)Archetype);
        return nullptr;
    }
    
    // Criar instância de animação personalizada
    UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    
    if (AnimInstance)
    {
        // Configurar a instância de animação com os dados do arquétipo
        ConfigureAnimationInstance(AnimInstance, *BodyLanguage, *MotionData);
        
        // Registrar para atualizações
        FCharacterAnimationState AnimState;
        AnimState.SkeletalMeshComponent = SkeletalMeshComponent;
        AnimState.AnimInstance = AnimInstance;
        AnimState.Archetype = Archetype;
        AnimState.CurrentBodyLanguage = *BodyLanguage;
        AnimState.CurrentMotionData = *MotionData;
        AnimState.StressLevel = EStressLevel::Calm;
        AnimState.SurvivalState = ESurvivalState::Fresh;
        
        ActiveCharacterAnimations.Add(SkeletalMeshComponent, AnimState);
    }
    
    return AnimInstance;
}

void UJurassicAnimationAgent::ConfigureAnimationInstance(
    UAnimInstance* AnimInstance,
    const FArchetypeBodyLanguage& BodyLanguage,
    const FArchetypeMotionData& MotionData)
{
    if (!AnimInstance)
    {
        return;
    }
    
    // Configurar parâmetros da instância de animação baseados no arquétipo
    // Estes parâmetros serão lidos pelo Animation Blueprint
    
    // Configurar linguagem corporal
    AnimInstance->SetRootMotionMode(ERootMotionMode::RootMotionFromEverything);
    
    // TODO: Configurar variáveis específicas do Animation Blueprint quando disponível
    // Por exemplo:
    // AnimInstance->SetVariableFloat(\"PostureWeight\", BodyLanguage.PostureWeight);
    // AnimInstance->SetVariableFloat(\"GestureFrequency\", BodyLanguage.GestureFrequency);
    // AnimInstance->SetVariableFloat(\"MovementSpeed\", BodyLanguage.MovementSpeed);
}

void UJurassicAnimationAgent::UpdateCharacterAnimation(
    USkeletalMeshComponent* SkeletalMeshComponent,
    float DeltaTime)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    FCharacterAnimationState* AnimState = ActiveCharacterAnimations.Find(SkeletalMeshComponent);
    if (!AnimState)
    {
        return;
    }
    
    // Atualizar estado de stress baseado no ambiente
    UpdateStressLevel(*AnimState, DeltaTime);
    
    // Atualizar estado de sobrevivência
    UpdateSurvivalState(*AnimState, DeltaTime);
    
    // Aplicar influências ao movimento
    ApplyStressInfluence(*AnimState);
    ApplySurvivalInfluence(*AnimState);
    ApplyPersonalityInfluence(*AnimState);
    
    // Atualizar gestos procedurais
    UpdateProceduralGestures(*AnimState, DeltaTime);
    
    // Atualizar IK adaptativo
    UpdateAdaptiveIK(*AnimState, DeltaTime);
}

void UJurassicAnimationAgent::UpdateStressLevel(FCharacterAnimationState& AnimState, float DeltaTime)
{
    // Lógica para determinar o nível de stress baseado em:
    // - Proximidade de dinossauros
    // - Nível de saúde
    // - Recursos disponíveis
    // - Tempo desde último descanso
    
    // Por agora, usar lógica simples
    // TODO: Integrar com sistemas de IA e ambiente
}

void UJurassicAnimationAgent::UpdateSurvivalState(FCharacterAnimationState& AnimState, float DeltaTime)
{
    // Lógica para determinar o estado de sobrevivência baseado em:
    // - Tempo no mundo Jurássico
    // - Sucessos/falhas na sobrevivência
    // - Recursos coletados
    // - Experiência adquirida
    
    // Por agora, usar lógica simples
    // TODO: Integrar com sistema de progressão do jogador
}

void UJurassicAnimationAgent::ApplyStressInfluence(FCharacterAnimationState& AnimState)
{
    float StressMultiplier = 1.0f;
    
    switch (AnimState.StressLevel)
    {
        case EStressLevel::Calm:
            StressMultiplier = 1.0f;
            break;
        case EStressLevel::Aware:
            StressMultiplier = 1.1f;
            break;
        case EStressLevel::Alert:
            StressMultiplier = 1.2f;
            break;
        case EStressLevel::Tense:
            StressMultiplier = 1.4f;
            break;
        case EStressLevel::Panicked:
            StressMultiplier = 1.8f;
            break;
    }
    
    // Aplicar multiplicador aos parâmetros de animação
    AnimState.CurrentBodyLanguage.MovementSpeed *= StressMultiplier;
    AnimState.CurrentBodyLanguage.TensionLevel = FMath::Clamp(
        AnimState.CurrentBodyLanguage.TensionLevel * StressMultiplier, 0.0f, 1.0f);
    AnimState.CurrentBodyLanguage.BlinkRate *= StressMultiplier;
}

void UJurassicAnimationAgent::ApplySurvivalInfluence(FCharacterAnimationState& AnimState)
{
    float SurvivalMultiplier = 1.0f;
    
    switch (AnimState.SurvivalState)
    {
        case ESurvivalState::Fresh:
            SurvivalMultiplier = 1.0f;
            break;
        case ESurvivalState::Adapting:
            SurvivalMultiplier = 0.95f;
            break;
        case ESurvivalState::Experienced:
            SurvivalMultiplier = 1.1f;
            break;
        case ESurvivalState::Hardened:
            SurvivalMultiplier = 1.2f;
            break;
        case ESurvivalState::Desperate:
            SurvivalMultiplier = 0.8f;
            break;
        case ESurvivalState::Broken:
            SurvivalMultiplier = 0.6f;
            break;
    }
    
    // Aplicar influência da sobrevivência
    AnimState.CurrentBodyLanguage.PostureWeight *= SurvivalMultiplier;
    AnimState.CurrentBodyLanguage.MovementSpeed *= SurvivalMultiplier;
}

void UJurassicAnimationAgent::ApplyPersonalityInfluence(FCharacterAnimationState& AnimState)
{
    // Aplicar influência da personalidade baseada no arquétipo
    // Esta influência é constante e define o \"baseline\" do personagem
    
    const FArchetypeBodyLanguage* BaseBodyLanguage = ArchetypeBodyLanguageMap.Find(AnimState.Archetype);
    if (BaseBodyLanguage)
    {
        // Interpolar de volta para os valores base da personalidade
        float InterpolationSpeed = PersonalityInfluenceWeight * GetWorld()->GetDeltaSeconds();
        
        AnimState.CurrentBodyLanguage.PostureWeight = FMath::FInterpTo(
            AnimState.CurrentBodyLanguage.PostureWeight,
            BaseBodyLanguage->PostureWeight,
            GetWorld()->GetDeltaSeconds(),
            InterpolationSpeed);
            
        AnimState.CurrentBodyLanguage.GestureFrequency = FMath::FInterpTo(
            AnimState.CurrentBodyLanguage.GestureFrequency,
            BaseBodyLanguage->GestureFrequency,
            GetWorld()->GetDeltaSeconds(),
            InterpolationSpeed);
    }
}

void UJurassicAnimationAgent::UpdateProceduralGestures(FCharacterAnimationState& AnimState, float DeltaTime)
{
    if (!bUseProceduralGestures)
    {
        return;
    }
    
    // Atualizar timer de gestos
    AnimState.GestureTimer += DeltaTime;
    
    // Verificar se é hora de um novo gesto baseado na frequência do arquétipo
    float GestureInterval = 1.0f / FMath::Max(AnimState.CurrentBodyLanguage.GestureFrequency, 0.1f);
    
    if (AnimState.GestureTimer >= GestureInterval)
    {
        // Trigger um gesto procedural
        TriggerProceduralGesture(AnimState);
        AnimState.GestureTimer = 0.0f;
    }
}

void UJurassicAnimationAgent::UpdateAdaptiveIK(FCharacterAnimationState& AnimState, float DeltaTime)
{
    if (!bUseAdaptiveIK)
    {
        return;
    }
    
    // Atualizar IK adaptativo para terreno irregular
    // TODO: Integrar com componente de IK adaptativo
}

void UJurassicAnimationAgent::TriggerProceduralGesture(FCharacterAnimationState& AnimState)
{
    // Selecionar um gesto apropriado baseado no arquétipo e estado atual
    // TODO: Implementar seleção de gestos procedurais
    
    UE_LOG(LogJurassicAnimation, Verbose, TEXT("Triggering procedural gesture for archetype: %d"), (int32)AnimState.Archetype);
}

void UJurassicAnimationAgent::SetCharacterStressLevel(
    USkeletalMeshComponent* SkeletalMeshComponent,
    EStressLevel NewStressLevel)
{
    if (FCharacterAnimationState* AnimState = ActiveCharacterAnimations.Find(SkeletalMeshComponent))
    {
        AnimState->StressLevel = NewStressLevel;
        UE_LOG(LogJurassicAnimation, Log, TEXT("Character stress level changed to: %d"), (int32)NewStressLevel);
    }
}

void UJurassicAnimationAgent::SetCharacterSurvivalState(
    USkeletalMeshComponent* SkeletalMeshComponent,
    ESurvivalState NewSurvivalState)
{
    if (FCharacterAnimationState* AnimState = ActiveCharacterAnimations.Find(SkeletalMeshComponent))
    {
        AnimState->SurvivalState = NewSurvivalState;
        UE_LOG(LogJurassicAnimation, Log, TEXT("Character survival state changed to: %d"), (int32)NewSurvivalState);
    }
}

FArchetypeBodyLanguage UJurassicAnimationAgent::GetBodyLanguageForArchetype(ECharacterArchetype Archetype) const
{
    if (const FArchetypeBodyLanguage* BodyLanguage = ArchetypeBodyLanguageMap.Find(Archetype))
    {
        return *BodyLanguage;
    }
    
    // Retornar configuração padrão se não encontrar
    return FArchetypeBodyLanguage();
}

FArchetypeMotionData UJurassicAnimationAgent::GetMotionDataForArchetype(ECharacterArchetype Archetype) const
{
    if (const FArchetypeMotionData* MotionData = ArchetypeMotionDataMap.Find(Archetype))
    {
        return *MotionData;
    }
    
    // Retornar configuração padrão se não encontrar
    return FArchetypeMotionData();
}

void UJurassicAnimationAgent::PlayEmotionalReaction(
    USkeletalMeshComponent* SkeletalMeshComponent,
    EEmotionalReaction Reaction)
{
    if (!SkeletalMeshComponent)
    {
        return;
    }
    
    FCharacterAnimationState* AnimState = ActiveCharacterAnimations.Find(SkeletalMeshComponent);
    if (!AnimState)
    {
        return;
    }
    
    // Selecionar animação de reação emocional apropriada
    const FArchetypeMotionData* MotionData = ArchetypeMotionDataMap.Find(AnimState->Archetype);
    if (MotionData && MotionData->EmotionalReactions.Num() > 0)
    {
        // Selecionar uma reação aleatória apropriada
        int32 ReactionIndex = FMath::RandRange(0, MotionData->EmotionalReactions.Num() - 1);
        TSoftObjectPtr<UAnimMontage> ReactionMontage = MotionData->EmotionalReactions[ReactionIndex];
        
        if (UAnimMontage* Montage = ReactionMontage.LoadSynchronous())
        {
            if (UAnimInstance* AnimInstance = SkeletalMeshComponent->GetAnimInstance())
            {
                AnimInstance->Montage_Play(Montage);
                UE_LOG(LogJurassicAnimation, Log, TEXT("Playing emotional reaction: %d"), (int32)Reaction);
            }
        }
    }
}

bool UJurassicAnimationAgent::IsCharacterRegistered(USkeletalMeshComponent* SkeletalMeshComponent) const
{
    return ActiveCharacterAnimations.Contains(SkeletalMeshComponent);
}

void UJurassicAnimationAgent::UnregisterCharacter(USkeletalMeshComponent* SkeletalMeshComponent)
{
    if (ActiveCharacterAnimations.Contains(SkeletalMeshComponent))
    {
        ActiveCharacterAnimations.Remove(SkeletalMeshComponent);
        UE_LOG(LogJurassicAnimation, Log, TEXT("Character unregistered from animation system"));
    }
}