#include "ArchetypeAnimationSystem.h"
#include "Engine/World.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "Animation/AnimSequence.h"
#include "Animation/BlendSpace.h"
#include "Animation/AnimMontage.h"
#include "IKRigDefinition.h"
#include "ControlRig.h"
#include "Characters/CharacterArchetypes.h"
#include "MotionMatching/MotionMatchingController.h"
#include "IK/AdaptiveIKComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchetypeAnimation, Log, All);

UArchetypeAnimationSystem::UArchetypeAnimationSystem()
{
    // Configuração padrão do sistema
    bTickEvenWhenPaused = false;
    
    // Inicializar configurações padrão para cada arquétipo
    InitializeDefaultConfigurations();
}

void UArchetypeAnimationSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Archetype Animation System initialized"));
    
    // Carregar configurações de arquétipos
    LoadArchetypeConfigurations();
    
    // Configurar callbacks de eventos
    SetupEventCallbacks();
}

void UArchetypeAnimationSystem::Deinitialize()
{
    // Limpar todas as configurações ativas
    ActiveCharacterStates.Empty();
    ArchetypeConfigurations.Empty();
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Archetype Animation System deinitialized"));
    
    Super::Deinitialize();
}

void UArchetypeAnimationSystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Atualizar estados de animação de todos os personagens ativos
    UpdateActiveCharacterAnimations(DeltaTime);
    
    // Processar mudanças de estado emocional
    ProcessEmotionalStateChanges(DeltaTime);
    
    // Atualizar gestos procedurais
    UpdateProceduralGestures(DeltaTime);
}

TStatId UArchetypeAnimationSystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UArchetypeAnimationSystem, STATGROUP_Tickables);
}

void UArchetypeAnimationSystem::RegisterCharacter(ACharacter* Character, ECharacterArchetype Archetype)
{
    if (!Character)
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("Attempted to register null character"));
        return;
    }
    
    // Criar estado de animação para o personagem
    FCharacterAnimationState NewState;
    NewState.CurrentEmotionalState = EEmotionalState::Neutral;
    NewState.CurrentSurvivalState = ESurvivalState::Fresh;
    NewState.CurrentStressLevel = EStressLevel::Calm;
    
    ActiveCharacterStates.Add(Character, NewState);
    
    // Aplicar configuração do arquétipo
    ApplyArchetypeConfiguration(Character, Archetype);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Registered character %s with archetype %s"), 
           *Character->GetName(), 
           *UEnum::GetValueAsString(Archetype));
}

void UArchetypeAnimationSystem::UnregisterCharacter(ACharacter* Character)
{
    if (!Character)
    {
        return;
    }
    
    ActiveCharacterStates.Remove(Character);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Unregistered character %s"), *Character->GetName());
}

void UArchetypeAnimationSystem::UpdateCharacterEmotionalState(ACharacter* Character, EEmotionalState NewState, float Intensity)
{
    if (!Character || !ActiveCharacterStates.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& AnimState = ActiveCharacterStates[Character];
    
    // Transição suave entre estados emocionais
    if (AnimState.CurrentEmotionalState != NewState)
    {
        AnimState.CurrentEmotionalState = NewState;
        AnimState.EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
        
        // Aplicar mudanças de Motion Matching baseadas no estado emocional
        ApplyEmotionalMotionMatching(Character, NewState, Intensity);
        
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Character %s emotional state changed to %s (intensity: %.2f)"), 
               *Character->GetName(), 
               *UEnum::GetValueAsString(NewState),
               Intensity);
    }
}

void UArchetypeAnimationSystem::UpdateCharacterSurvivalState(ACharacter* Character, ESurvivalState NewState)
{
    if (!Character || !ActiveCharacterStates.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& AnimState = ActiveCharacterStates[Character];
    
    if (AnimState.CurrentSurvivalState != NewState)
    {
        AnimState.CurrentSurvivalState = NewState;
        
        // Aplicar mudanças de postura baseadas no estado de sobrevivência
        ApplySurvivalPostureChanges(Character, NewState);
        
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Character %s survival state changed to %s"), 
               *Character->GetName(), 
               *UEnum::GetValueAsString(NewState));
    }
}

void UArchetypeAnimationSystem::TriggerGesture(ACharacter* Character, EGestureType GestureType, float Intensity)
{
    if (!Character || !ActiveCharacterStates.Contains(Character))
    {
        return;
    }
    
    // Encontrar configuração do arquétipo
    ECharacterArchetype Archetype = GetCharacterArchetype(Character);
    if (!ArchetypeConfigurations.Contains(Archetype))
    {
        return;
    }
    
    const FArchetypeAnimationConfig& Config = ArchetypeConfigurations[Archetype];
    
    // Verificar se existe animação para este gesto
    if (Config.GestureAnimations.Contains(GestureType))
    {
        TSoftObjectPtr<UAnimMontage> GestureMontage = Config.GestureAnimations[GestureType];
        if (GestureMontage.IsValid())
        {
            // Reproduzir gesto
            PlayGestureMontage(Character, GestureMontage.Get(), Intensity);
            
            UE_LOG(LogArchetypeAnimation, Log, TEXT("Triggered gesture %s for character %s"), 
                   *UEnum::GetValueAsString(GestureType),
                   *Character->GetName());
        }
    }
}

void UArchetypeAnimationSystem::InitializeDefaultConfigurations()
{
    // Configuração para Paleontologista (Protagonista)
    FArchetypeAnimationConfig PaleontologistConfig;
    PaleontologistConfig.Archetype = ECharacterArchetype::Protagonist_Paleontologist;
    PaleontologistConfig.BodyLanguage.Archetype = EBodyLanguageArchetype::ScientificCuriosity;
    PaleontologistConfig.BodyLanguage.MovementPersonality = EMovementPersonality::Cautious;
    PaleontologistConfig.BodyLanguage.GestureFrequency = 0.7f; // Gestos científicos frequentes
    PaleontologistConfig.BodyLanguage.EyeContactLevel = 0.8f; // Observador atento
    PaleontologistConfig.BodyLanguage.PersonalSpaceRadius = 120.0f;
    PaleontologistConfig.BodyLanguage.HeadTiltAngle = 5.0f; // Ligeira inclinação curiosa
    PaleontologistConfig.BlendTime = 0.15f; // Transições rápidas para observação
    PaleontologistConfig.GestureFrequency = 0.8f;
    
    ArchetypeConfigurations.Add(ECharacterArchetype::Protagonist_Paleontologist, PaleontologistConfig);
    
    // Configuração para Sobrevivente
    FArchetypeAnimationConfig SurvivorConfig;
    SurvivorConfig.Archetype = ECharacterArchetype::NPC_Survivor;
    SurvivorConfig.BodyLanguage.Archetype = EBodyLanguageArchetype::ConstantVigilance;
    SurvivorConfig.BodyLanguage.MovementPersonality = EMovementPersonality::Hyperalert;
    SurvivorConfig.BodyLanguage.GestureFrequency = 0.3f; // Gestos mínimos para não chamar atenção
    SurvivorConfig.BodyLanguage.EyeContactLevel = 0.9f; // Sempre alerta
    SurvivorConfig.BodyLanguage.PersonalSpaceRadius = 200.0f; // Maior espaço pessoal
    SurvivorConfig.BodyLanguage.TensionLevel = 0.7f; // Alta tensão
    SurvivorConfig.BodyLanguage.ShoulderTension = 0.8f;
    SurvivorConfig.BlendTime = 0.1f; // Reações rápidas
    
    ArchetypeConfigurations.Add(ECharacterArchetype::NPC_Survivor, SurvivorConfig);
    
    // Configuração para Explorador
    FArchetypeAnimationConfig ExplorerConfig;
    ExplorerConfig.Archetype = ECharacterArchetype::NPC_Explorer;
    ExplorerConfig.BodyLanguage.Archetype = EBodyLanguageArchetype::TerritorialConfidence;
    ExplorerConfig.BodyLanguage.MovementPersonality = EMovementPersonality::Confident;
    ExplorerConfig.BodyLanguage.GestureFrequency = 0.6f;
    ExplorerConfig.BodyLanguage.EyeContactLevel = 0.7f;
    ExplorerConfig.BodyLanguage.PersonalSpaceRadius = 100.0f; // Confortável em proximidade
    ExplorerConfig.BodyLanguage.MovementSpeed = 1.2f; // Movimento mais rápido
    ExplorerConfig.BodyLanguage.PostureWeight = 0.9f; // Postura ereta
    
    ArchetypeConfigurations.Add(ECharacterArchetype::NPC_Explorer, ExplorerConfig);
}

void UArchetypeAnimationSystem::LoadArchetypeConfigurations()
{
    // TODO: Carregar configurações de arquivos de dados
    // Por agora usar configurações padrão
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Loaded %d archetype configurations"), ArchetypeConfigurations.Num());
}

void UArchetypeAnimationSystem::SetupEventCallbacks()
{
    // TODO: Configurar callbacks para eventos do jogo
    // Exemplo: OnCharacterDamaged, OnCharacterStressed, etc.
}

void UArchetypeAnimationSystem::UpdateActiveCharacterAnimations(float DeltaTime)
{
    for (auto& CharacterStatePair : ActiveCharacterStates)
    {
        ACharacter* Character = CharacterStatePair.Key;
        FCharacterAnimationState& AnimState = CharacterStatePair.Value;
        
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Atualizar timers
        AnimState.LastGestureTime += DeltaTime;
        
        // Verificar se personagem está parado
        if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
        {
            if (MovementComp->Velocity.SizeSquared() < 10.0f) // Praticamente parado
            {
                AnimState.TimeSinceLastMovement += DeltaTime;
            }
            else
            {
                AnimState.TimeSinceLastMovement = 0.0f;
            }
        }
        
        // Atualizar fadiga baseada em movimento
        UpdateFatigueLevel(Character, AnimState, DeltaTime);
        
        // Atualizar IK adaptativo
        UpdateAdaptiveIK(Character, AnimState, DeltaTime);
    }
}

void UArchetypeAnimationSystem::ProcessEmotionalStateChanges(float DeltaTime)
{
    for (auto& CharacterStatePair : ActiveCharacterStates)
    {
        ACharacter* Character = CharacterStatePair.Key;
        FCharacterAnimationState& AnimState = CharacterStatePair.Value;
        
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Processar decaimento natural do stress
        if (AnimState.CurrentStressLevel > EStressLevel::Calm)
        {
            // Reduzir stress gradualmente se não houver ameaças
            if (!AnimState.bIsInDanger)
            {
                AnimState.FearLevel = FMath::Max(0.0f, AnimState.FearLevel - DeltaTime * 0.1f);
                
                // Atualizar nível de stress baseado no medo
                UpdateStressLevel(Character, AnimState);
            }
        }
        
        // Processar recuperação de confiança
        if (AnimState.ConfidenceLevel < 1.0f && AnimState.CurrentStressLevel <= EStressLevel::Aware)
        {
            AnimState.ConfidenceLevel = FMath::Min(1.0f, AnimState.ConfidenceLevel + DeltaTime * 0.05f);
        }
    }
}

void UArchetypeAnimationSystem::UpdateProceduralGestures(float DeltaTime)
{
    for (auto& CharacterStatePair : ActiveCharacterStates)
    {
        ACharacter* Character = CharacterStatePair.Key;
        FCharacterAnimationState& AnimState = CharacterStatePair.Value;
        
        if (!IsValid(Character))
        {
            continue;
        }
        
        // Verificar se deve executar gesto procedural
        ECharacterArchetype Archetype = GetCharacterArchetype(Character);
        if (ArchetypeConfigurations.Contains(Archetype))
        {
            const FArchetypeAnimationConfig& Config = ArchetypeConfigurations[Archetype];
            
            // Calcular probabilidade de gesto baseada na frequência configurada
            float GestureChance = Config.GestureFrequency * DeltaTime;
            
            // Aumentar chance se personagem está parado há tempo
            if (AnimState.TimeSinceLastMovement > 3.0f)
            {
                GestureChance *= 2.0f;
            }
            
            // Executar gesto se passou tempo suficiente desde o último
            if (AnimState.LastGestureTime > 2.0f && FMath::RandRange(0.0f, 1.0f) < GestureChance)
            {
                EGestureType RandomGesture = SelectRandomGestureForArchetype(Archetype, AnimState);
                if (RandomGesture != EGestureType::MAX)
                {
                    TriggerGesture(Character, RandomGesture, 0.7f);
                    AnimState.LastGestureTime = 0.0f;
                }
            }
        }
    }
}

void UArchetypeAnimationSystem::ApplyArchetypeConfiguration(ACharacter* Character, ECharacterArchetype Archetype)
{
    if (!ArchetypeConfigurations.Contains(Archetype))
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("No configuration found for archetype %s"), 
               *UEnum::GetValueAsString(Archetype));
        return;
    }
    
    const FArchetypeAnimationConfig& Config = ArchetypeConfigurations[Archetype];
    
    // Aplicar configuração de Motion Matching
    if (UMotionMatchingController* MMController = Character->FindComponentByClass<UMotionMatchingController>())
    {
        MMController->SetArchetypeConfiguration(Config);
    }
    
    // Aplicar configuração de IK adaptativo
    if (UAdaptiveIKComponent* IKComponent = Character->FindComponentByClass<UAdaptiveIKComponent>())
    {
        IKComponent->SetArchetypeConfiguration(Config);
    }
    
    // Configurar parâmetros de movimento
    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        MovementComp->MaxWalkSpeed *= Config.BodyLanguage.MovementSpeed;
    }
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Applied archetype configuration for %s to character %s"), 
           *UEnum::GetValueAsString(Archetype),
           *Character->GetName());
}

void UArchetypeAnimationSystem::ApplyEmotionalMotionMatching(ACharacter* Character, EEmotionalState EmotionalState, float Intensity)
{
    ECharacterArchetype Archetype = GetCharacterArchetype(Character);
    if (!ArchetypeConfigurations.Contains(Archetype))
    {
        return;
    }
    
    const FArchetypeAnimationConfig& Config = ArchetypeConfigurations[Archetype];
    
    // Verificar se existe database específico para este estado emocional
    if (Config.EmotionalDatabases.Contains(EmotionalState))
    {
        TSoftObjectPtr<UPoseSearchDatabase> EmotionalDatabase = Config.EmotionalDatabases[EmotionalState];
        
        if (UMotionMatchingController* MMController = Character->FindComponentByClass<UMotionMatchingController>())
        {
            MMController->SetEmotionalDatabase(EmotionalDatabase.Get(), Intensity);
        }
    }
}

void UArchetypeAnimationSystem::ApplySurvivalPostureChanges(ACharacter* Character, ESurvivalState SurvivalState)
{
    if (!ActiveCharacterStates.Contains(Character))
    {
        return;
    }
    
    FCharacterAnimationState& AnimState = ActiveCharacterStates[Character];
    
    // Ajustar parâmetros de postura baseados no estado de sobrevivência
    switch (SurvivalState)
    {
        case ESurvivalState::Fresh:
            AnimState.ConfidenceLevel = 0.8f;
            AnimState.FatigueLevel = 0.0f;
            break;
            
        case ESurvivalState::Adapting:
            AnimState.ConfidenceLevel = 0.6f;
            AnimState.FatigueLevel = 0.2f;
            break;
            
        case ESurvivalState::Experienced:
            AnimState.ConfidenceLevel = 0.9f;
            AnimState.FatigueLevel = 0.3f;
            break;
            
        case ESurvivalState::Hardened:
            AnimState.ConfidenceLevel = 1.0f;
            AnimState.FatigueLevel = 0.4f;
            break;
            
        case ESurvivalState::Desperate:
            AnimState.ConfidenceLevel = 0.3f;
            AnimState.FatigueLevel = 0.8f;
            AnimState.FearLevel = 0.7f;
            break;
            
        case ESurvivalState::Broken:
            AnimState.ConfidenceLevel = 0.1f;
            AnimState.FatigueLevel = 1.0f;
            AnimState.FearLevel = 0.9f;
            break;
    }
    
    // Aplicar mudanças visuais de postura
    ApplyPostureChanges(Character, AnimState);
}

void UArchetypeAnimationSystem::PlayGestureMontage(ACharacter* Character, UAnimMontage* Montage, float Intensity)
{
    if (!Character || !Montage)
    {
        return;
    }
    
    if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
    {
        if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
        {
            // Reproduzir montage com intensidade ajustada
            float PlayRate = FMath::Lerp(0.8f, 1.2f, Intensity);
            AnimInstance->Montage_Play(Montage, PlayRate);
        }
    }
}

ECharacterArchetype UArchetypeAnimationSystem::GetCharacterArchetype(ACharacter* Character) const
{
    // TODO: Implementar sistema de identificação de arquétipo
    // Por agora retornar paleontologista para o player
    if (Character && Character->IsPlayerControlled())
    {
        return ECharacterArchetype::Protagonist_Paleontologist;
    }
    
    return ECharacterArchetype::NPC_Survivor; // Padrão para NPCs
}

void UArchetypeAnimationSystem::UpdateFatigueLevel(ACharacter* Character, FCharacterAnimationState& AnimState, float DeltaTime)
{
    if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
    {
        float VelocityMagnitude = MovementComp->Velocity.Size();
        
        // Aumentar fadiga baseada na velocidade de movimento
        if (VelocityMagnitude > 300.0f) // Correndo
        {
            AnimState.FatigueLevel += DeltaTime * 0.1f;
        }
        else if (VelocityMagnitude > 100.0f) // Andando
        {
            AnimState.FatigueLevel += DeltaTime * 0.02f;
        }
        else // Parado - recuperar lentamente
        {
            AnimState.FatigueLevel -= DeltaTime * 0.05f;
        }
        
        AnimState.FatigueLevel = FMath::Clamp(AnimState.FatigueLevel, 0.0f, 1.0f);
    }
}

void UArchetypeAnimationSystem::UpdateAdaptiveIK(ACharacter* Character, FCharacterAnimationState& AnimState, float DeltaTime)
{
    if (UAdaptiveIKComponent* IKComponent = Character->FindComponentByClass<UAdaptiveIKComponent>())
    {
        // Atualizar IK baseado no estado atual do personagem
        IKComponent->UpdateIKWeights(AnimState.FatigueLevel, AnimState.InjuryLevel, AnimState.FearLevel);
    }
}

void UArchetypeAnimationSystem::UpdateStressLevel(ACharacter* Character, FCharacterAnimationState& AnimState)
{
    EStressLevel NewStressLevel = EStressLevel::Calm;
    
    if (AnimState.FearLevel > 0.8f)
    {
        NewStressLevel = EStressLevel::Panicked;
    }
    else if (AnimState.FearLevel > 0.6f)
    {
        NewStressLevel = EStressLevel::Tense;
    }
    else if (AnimState.FearLevel > 0.3f)
    {
        NewStressLevel = EStressLevel::Alert;
    }
    else if (AnimState.FearLevel > 0.1f)
    {
        NewStressLevel = EStressLevel::Aware;
    }
    
    if (AnimState.CurrentStressLevel != NewStressLevel)
    {
        AnimState.CurrentStressLevel = NewStressLevel;
        
        // Aplicar mudanças de animação baseadas no stress
        ApplyStressAnimationChanges(Character, NewStressLevel);
    }
}

void UArchetypeAnimationSystem::ApplyStressAnimationChanges(ACharacter* Character, EStressLevel StressLevel)
{
    if (UMotionMatchingController* MMController = Character->FindComponentByClass<UMotionMatchingController>())
    {
        MMController->SetStressLevel(StressLevel);
    }
}

void UArchetypeAnimationSystem::ApplyPostureChanges(ACharacter* Character, const FCharacterAnimationState& AnimState)
{
    // TODO: Implementar mudanças visuais de postura
    // Aplicar curvaturas, tensão muscular, etc.
}

EGestureType UArchetypeAnimationSystem::SelectRandomGestureForArchetype(ECharacterArchetype Archetype, const FCharacterAnimationState& AnimState) const
{
    // Selecionar gestos apropriados baseados no arquétipo e estado
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist_Paleontologist:
            if (AnimState.CurrentEmotionalState == EEmotionalState::Curious)
            {
                return FMath::RandBool() ? EGestureType::ObservationPoint : EGestureType::ExamineObject;
            }
            else if (AnimState.bIsObserving)
            {
                return EGestureType::TakeNotes;
            }
            return EGestureType::CheckSurroundings;
            
        case ECharacterArchetype::NPC_Survivor:
            if (AnimState.CurrentStressLevel >= EStressLevel::Alert)
            {
                return EGestureType::SignalDanger;
            }
            return EGestureType::CheckSurroundings;
            
        case ECharacterArchetype::NPC_Explorer:
            return EGestureType::ShowDetermination;
            
        default:
            return EGestureType::CheckSurroundings;
    }
}