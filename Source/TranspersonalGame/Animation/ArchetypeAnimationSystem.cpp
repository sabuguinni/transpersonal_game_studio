#include "ArchetypeAnimationSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/BlendSpace.h"
#include "Animation/ControlRig.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "IKRig/IKRigDefinition.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Characters/CharacterArchetypes.h"
#include "Logging/LogMacros.h"

DEFINE_LOG_CATEGORY_STATIC(LogArchetypeAnimation, Log, All);

UArchetypeAnimationSystem::UArchetypeAnimationSystem()
{
    // Configurar tick
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    // Valores padrão
    CurrentAnimationState = FCharacterAnimationState();
    BlendTimeMultiplier = 1.0f;
    EmotionalTransitionSpeed = 2.0f;
    GestureBlendWeight = 0.8f;
    IKBlendWeight = 1.0f;
    bDebugAnimationStates = false;
    
    // Inicializar timers
    LastGestureTime = 0.0f;
    LastEmotionalUpdate = 0.0f;
    LastSurvivalUpdate = 0.0f;
}

void UArchetypeAnimationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar componentes necessários
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (OwnerCharacter)
    {
        SkeletalMeshComponent = OwnerCharacter->GetMesh();
        MovementComponent = OwnerCharacter->GetCharacterMovement();
        
        // Configurar sistema baseado no arquétipo
        InitializeArchetypeSystem();
        
        // Registrar no subsistema global
        if (UWorld* World = GetWorld())
        {
            if (UArchetypeAnimationSubsystem* AnimSubsystem = World->GetSubsystem<UArchetypeAnimationSubsystem>())
            {
                AnimSubsystem->RegisterCharacterAnimationSystem(this);
            }
        }
    }
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSystem initialized for character: %s"), 
           OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
}

void UArchetypeAnimationSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // Desregistrar do subsistema
    if (UWorld* World = GetWorld())
    {
        if (UArchetypeAnimationSubsystem* AnimSubsystem = World->GetSubsystem<UArchetypeAnimationSubsystem>())
        {
            AnimSubsystem->UnregisterCharacterAnimationSystem(this);
        }
    }
    
    Super::EndPlay(EndPlayReason);
}

void UArchetypeAnimationSystem::TickComponent(float DeltaTime, ELevelTick TickType, 
                                             FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!OwnerCharacter || !SkeletalMeshComponent)
        return;
    
    // Atualizar estado de animação
    UpdateAnimationState(DeltaTime);
    
    // Processar Motion Matching contextual
    UpdateMotionMatching(DeltaTime);
    
    // Atualizar IK adaptativo
    UpdateAdaptiveIK(DeltaTime);
    
    // Processar gestos procedurais
    UpdateProceduralGestures(DeltaTime);
    
    // Atualizar expressões emocionais
    UpdateEmotionalExpressions(DeltaTime);
    
    // Debug visual se ativado
    if (bDebugAnimationStates)
    {
        DebugDrawAnimationState();
    }
}

void UArchetypeAnimationSystem::InitializeArchetypeSystem()
{
    if (!ArchetypeConfig)
    {
        UE_LOG(LogArchetypeAnimation, Warning, TEXT("No ArchetypeConfig set for character: %s"), 
               OwnerCharacter ? *OwnerCharacter->GetName() : TEXT("Unknown"));
        return;
    }
    
    // Configurar Motion Matching baseado no arquétipo
    SetupMotionMatchingForArchetype();
    
    // Configurar IK específico
    SetupArchetypeIK();
    
    // Carregar animações específicas do arquétipo
    LoadArchetypeAnimations();
    
    // Configurar linguagem corporal inicial
    ApplyArchetypeBodyLanguage();
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Archetype system initialized for: %s"), 
           *UEnum::GetValueAsString(ArchetypeConfig->Archetype));
}

void UArchetypeAnimationSystem::UpdateAnimationState(float DeltaTime)
{
    if (!MovementComponent)
        return;
    
    // Atualizar níveis de condição baseados no gameplay
    UpdateConditionLevels(DeltaTime);
    
    // Determinar estado emocional baseado nas condições
    EEmotionalState NewEmotionalState = DetermineEmotionalState();
    
    // Transição suave entre estados emocionais
    if (NewEmotionalState != CurrentAnimationState.CurrentEmotionalState)
    {
        TransitionToEmotionalState(NewEmotionalState, DeltaTime);
    }
    
    // Atualizar estado de sobrevivência
    UpdateSurvivalState(DeltaTime);
    
    // Atualizar timers
    CurrentAnimationState.TimeSinceLastMovement += DeltaTime;
    if (MovementComponent->Velocity.Size() > 10.0f)
    {
        CurrentAnimationState.TimeSinceLastMovement = 0.0f;
    }
}

void UArchetypeAnimationSystem::UpdateConditionLevels(float DeltaTime)
{
    // Simular fadiga baseada em movimento
    if (MovementComponent && MovementComponent->Velocity.Size() > 100.0f)
    {
        CurrentAnimationState.FatigueLevel = FMath::Clamp(
            CurrentAnimationState.FatigueLevel + DeltaTime * 0.1f, 0.0f, 1.0f);
    }
    else
    {
        // Recuperação gradual quando parado
        CurrentAnimationState.FatigueLevel = FMath::Clamp(
            CurrentAnimationState.FatigueLevel - DeltaTime * 0.05f, 0.0f, 1.0f);
    }
    
    // Simular medo baseado em proximidade de perigos (placeholder)
    // TODO: Integrar com sistema de detecção de dinossauros
    if (CurrentAnimationState.bIsInDanger)
    {
        CurrentAnimationState.FearLevel = FMath::Clamp(
            CurrentAnimationState.FearLevel + DeltaTime * 0.5f, 0.0f, 1.0f);
    }
    else
    {
        CurrentAnimationState.FearLevel = FMath::Clamp(
            CurrentAnimationState.FearLevel - DeltaTime * 0.2f, 0.0f, 1.0f);
    }
    
    // Confiança inversamente proporcional ao medo e fadiga
    float ConfidenceTarget = 1.0f - (CurrentAnimationState.FearLevel * 0.6f + 
                                   CurrentAnimationState.FatigueLevel * 0.4f);
    CurrentAnimationState.ConfidenceLevel = FMath::FInterpTo(
        CurrentAnimationState.ConfidenceLevel, ConfidenceTarget, DeltaTime, 2.0f);
}

EEmotionalState UArchetypeAnimationSystem::DetermineEmotionalState()
{
    // Lógica para determinar estado emocional baseado nas condições
    if (CurrentAnimationState.FearLevel > 0.7f)
    {
        return EEmotionalState::Fearful;
    }
    else if (CurrentAnimationState.FatigueLevel > 0.8f)
    {
        return EEmotionalState::Exhausted;
    }
    else if (CurrentAnimationState.bIsObserving)
    {
        return EEmotionalState::Curious;
    }
    else if (CurrentAnimationState.ConfidenceLevel > 0.8f)
    {
        return EEmotionalState::Confident;
    }
    else if (CurrentAnimationState.FearLevel > 0.3f)
    {
        return EEmotionalState::Cautious;
    }
    
    return EEmotionalState::Neutral;
}

void UArchetypeAnimationSystem::TransitionToEmotionalState(EEmotionalState NewState, float DeltaTime)
{
    // Verificar se tempo suficiente passou desde a última transição
    if (GetWorld()->GetTimeSeconds() - LastEmotionalUpdate < 0.5f)
        return;
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Transitioning from %s to %s"), 
           *UEnum::GetValueAsString(CurrentAnimationState.CurrentEmotionalState),
           *UEnum::GetValueAsString(NewState));
    
    CurrentAnimationState.CurrentEmotionalState = NewState;
    LastEmotionalUpdate = GetWorld()->GetTimeSeconds();
    
    // Notificar sistemas dependentes
    OnEmotionalStateChanged.ExecuteIfBound(NewState);
    
    // Atualizar Motion Matching database se necessário
    UpdateMotionMatchingDatabase();
}

void UArchetypeAnimationSystem::UpdateSurvivalState(float DeltaTime)
{
    // Lógica simplificada para estado de sobrevivência
    // TODO: Integrar com sistema de sobrevivência do jogo
    
    ESurvivalState NewSurvivalState = CurrentAnimationState.CurrentSurvivalState;
    
    if (CurrentAnimationState.FatigueLevel > 0.9f || CurrentAnimationState.InjuryLevel > 0.7f)
    {
        NewSurvivalState = ESurvivalState::Desperate;
    }
    else if (CurrentAnimationState.FatigueLevel > 0.6f || CurrentAnimationState.InjuryLevel > 0.4f)
    {
        NewSurvivalState = ESurvivalState::Struggling;
    }
    else if (CurrentAnimationState.FatigueLevel > 0.3f)
    {
        NewSurvivalState = ESurvivalState::Tired;
    }
    else
    {
        NewSurvivalState = ESurvivalState::Fresh;
    }
    
    if (NewSurvivalState != CurrentAnimationState.CurrentSurvivalState)
    {
        CurrentAnimationState.CurrentSurvivalState = NewSurvivalState;
        LastSurvivalUpdate = GetWorld()->GetTimeSeconds();
        
        // Atualizar configurações de animação
        UpdateMotionMatchingDatabase();
    }
}

void UArchetypeAnimationSystem::UpdateMotionMatching(float DeltaTime)
{
    if (!ArchetypeConfig || !ArchetypeConfig->bUseMotionMatching)
        return;
    
    // Atualizar peso do Motion Matching baseado no estado
    float TargetWeight = ArchetypeConfig->MotionMatchingWeight;
    
    // Reduzir peso quando ferido ou muito cansado
    if (CurrentAnimationState.InjuryLevel > 0.5f)
    {
        TargetWeight *= (1.0f - CurrentAnimationState.InjuryLevel * 0.5f);
    }
    
    if (CurrentAnimationState.FatigueLevel > 0.7f)
    {
        TargetWeight *= (1.0f - CurrentAnimationState.FatigueLevel * 0.3f);
    }
    
    // Aplicar peso através do Animation Blueprint
    // TODO: Implementar comunicação com ABP através de variáveis
}

void UArchetypeAnimationSystem::UpdateAdaptiveIK(float DeltaTime)
{
    if (!ArchetypeConfig || !ArchetypeConfig->bUseAdaptiveIK)
        return;
    
    // Atualizar peso do IK baseado no movimento e terreno
    float TargetIKWeight = ArchetypeConfig->IKWeight;
    
    // Reduzir IK quando correndo muito rápido
    if (MovementComponent && MovementComponent->Velocity.Size() > 600.0f)
    {
        TargetIKWeight *= 0.5f;
    }
    
    // Aumentar IK quando em terreno irregular
    // TODO: Implementar detecção de terreno irregular
    
    // Aplicar através do componente IK
    // TODO: Comunicar com AdaptiveIKComponent
}

void UArchetypeAnimationSystem::UpdateProceduralGestures(float DeltaTime)
{
    if (!ArchetypeConfig || !ArchetypeConfig->bUseProceduralGestures)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Verificar se é hora de um novo gesto
    float TimeSinceLastGesture = CurrentTime - LastGestureTime;
    float GestureInterval = 1.0f / ArchetypeConfig->GestureFrequency;
    
    if (TimeSinceLastGesture > GestureInterval)
    {
        // Determinar tipo de gesto baseado no contexto
        EGestureType GestureToPlay = DetermineContextualGesture();
        
        if (GestureToPlay != EGestureType::MAX)
        {
            PlayGesture(GestureToPlay);
            LastGestureTime = CurrentTime;
        }
    }
}

EGestureType UArchetypeAnimationSystem::DetermineContextualGesture()
{
    // Lógica baseada no arquétipo e estado atual
    if (!ArchetypeConfig)
        return EGestureType::MAX;
    
    // Gestos baseados no estado emocional
    switch (CurrentAnimationState.CurrentEmotionalState)
    {
        case EEmotionalState::Curious:
            if (ArchetypeConfig->Archetype == ECharacterArchetype::Protagonist_Paleontologist)
            {
                return EGestureType::ObservationPoint;
            }
            break;
            
        case EEmotionalState::Fearful:
            return EGestureType::CheckSurroundings;
            
        case EEmotionalState::Cautious:
            return EGestureType::ListenCarefully;
            
        case EEmotionalState::Exhausted:
            return EGestureType::DisplayExhaustion;
            
        default:
            break;
    }
    
    // Gestos baseados na atividade atual
    if (CurrentAnimationState.bIsObserving)
    {
        return EGestureType::ExamineObject;
    }
    
    if (CurrentAnimationState.TimeSinceLastMovement > 5.0f)
    {
        return EGestureType::CheckSurroundings;
    }
    
    return EGestureType::MAX;
}

void UArchetypeAnimationSystem::PlayGesture(EGestureType GestureType)
{
    if (!ArchetypeConfig || !SkeletalMeshComponent)
        return;
    
    // Encontrar animação do gesto
    if (TSoftObjectPtr<UAnimMontage>* GestureAnim = ArchetypeConfig->GestureAnimations.Find(GestureType))
    {
        if (UAnimMontage* Montage = GestureAnim->LoadSynchronous())
        {
            // Reproduzir montage com peso apropriado
            SkeletalMeshComponent->GetAnimInstance()->Montage_Play(Montage, 1.0f);
            
            UE_LOG(LogArchetypeAnimation, Log, TEXT("Playing gesture: %s"), 
                   *UEnum::GetValueAsString(GestureType));
        }
    }
}

void UArchetypeAnimationSystem::UpdateEmotionalExpressions(float DeltaTime)
{
    if (!ArchetypeConfig || !ArchetypeConfig->bUseEmotionalOverrides)
        return;
    
    // Atualizar expressões faciais através do Control Rig
    if (ArchetypeConfig->FacialControlRig.IsValid())
    {
        // TODO: Implementar controle de expressões faciais
        // Mapear estados emocionais para parâmetros do Control Rig
    }
    
    // Atualizar postura corporal baseada no estado emocional
    UpdateBodyPosture(DeltaTime);
}

void UArchetypeAnimationSystem::UpdateBodyPosture(float DeltaTime)
{
    // Ajustar postura baseada no estado emocional e físico
    float PostureBlend = 0.0f;
    
    switch (CurrentAnimationState.CurrentEmotionalState)
    {
        case EEmotionalState::Fearful:
            PostureBlend = -0.8f; // Postura mais fechada/defensiva
            break;
            
        case EEmotionalState::Confident:
            PostureBlend = 0.6f; // Postura mais ereta/aberta
            break;
            
        case EEmotionalState::Exhausted:
            PostureBlend = -0.5f; // Postura curvada/cansada
            break;
            
        case EEmotionalState::Curious:
            PostureBlend = 0.3f; // Postura ligeiramente inclinada para frente
            break;
            
        default:
            PostureBlend = 0.0f;
            break;
    }
    
    // Aplicar modificações de postura através do Animation Blueprint
    // TODO: Implementar variáveis de postura no ABP
}

void UArchetypeAnimationSystem::SetupMotionMatchingForArchetype()
{
    if (!ArchetypeConfig)
        return;
    
    // Configurar databases específicos do arquétipo
    // TODO: Implementar carregamento dinâmico de databases
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Motion Matching setup for archetype: %s"), 
           *UEnum::GetValueAsString(ArchetypeConfig->Archetype));
}

void UArchetypeAnimationSystem::SetupArchetypeIK()
{
    if (!ArchetypeConfig || !ArchetypeConfig->IKRigDefinition.IsValid())
        return;
    
    // Configurar IK Rig específico do arquétipo
    // TODO: Implementar configuração do IK Rig
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("IK setup for archetype: %s"), 
           *UEnum::GetValueAsString(ArchetypeConfig->Archetype));
}

void UArchetypeAnimationSystem::LoadArchetypeAnimations()
{
    if (!ArchetypeConfig)
        return;
    
    // Pré-carregar animações críticas
    for (auto& GesturePair : ArchetypeConfig->GestureAnimations)
    {
        if (GesturePair.Value.IsValid())
        {
            GesturePair.Value.LoadSynchronous();
        }
    }
    
    // Carregar variações de locomotion
    for (auto& IdleAnim : ArchetypeConfig->IdleVariations)
    {
        if (IdleAnim.IsValid())
        {
            IdleAnim.LoadSynchronous();
        }
    }
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Archetype animations loaded"));
}

void UArchetypeAnimationSystem::ApplyArchetypeBodyLanguage()
{
    if (!ArchetypeConfig)
        return;
    
    // Aplicar configurações de linguagem corporal específicas
    const FArchetypeBodyLanguage& BodyLang = ArchetypeConfig->BodyLanguage;
    
    // TODO: Aplicar modificadores de postura, velocidade, etc.
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("Body language applied for archetype"));
}

void UArchetypeAnimationSystem::UpdateMotionMatchingDatabase()
{
    if (!ArchetypeConfig)
        return;
    
    // Selecionar database apropriado baseado no estado atual
    TSoftObjectPtr<UPoseSearchDatabase> TargetDatabase;
    
    // Priorizar database emocional se disponível
    if (TSoftObjectPtr<UPoseSearchDatabase>* EmotionalDB = 
        ArchetypeConfig->EmotionalDatabases.Find(CurrentAnimationState.CurrentEmotionalState))
    {
        TargetDatabase = *EmotionalDB;
    }
    // Fallback para database de sobrevivência
    else if (TSoftObjectPtr<UPoseSearchDatabase>* SurvivalDB = 
             ArchetypeConfig->SurvivalDatabases.Find(CurrentAnimationState.CurrentSurvivalState))
    {
        TargetDatabase = *SurvivalDB;
    }
    
    if (TargetDatabase.IsValid())
    {
        // TODO: Comunicar mudança de database para o Motion Matching Component
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Motion Matching database updated"));
    }
}

void UArchetypeAnimationSystem::DebugDrawAnimationState()
{
    if (!GetWorld() || !OwnerCharacter)
        return;
    
    FVector CharacterLocation = OwnerCharacter->GetActorLocation();
    FVector DebugLocation = CharacterLocation + FVector(0, 0, 250);
    
    // Debug info sobre estado atual
    FString DebugString = FString::Printf(TEXT(
        "Emotional: %s\\n"
        "Survival: %s\\n"
        "Fear: %.2f\\n"
        "Fatigue: %.2f\\n"
        "Confidence: %.2f"
    ),
        *UEnum::GetValueAsString(CurrentAnimationState.CurrentEmotionalState),
        *UEnum::GetValueAsString(CurrentAnimationState.CurrentSurvivalState),
        CurrentAnimationState.FearLevel,
        CurrentAnimationState.FatigueLevel,
        CurrentAnimationState.ConfidenceLevel
    );
    
    DrawDebugString(GetWorld(), DebugLocation, DebugString, nullptr, 
                   FColor::Yellow, 0.0f, true);
}

// Funções Blueprint
void UArchetypeAnimationSystem::SetEmotionalState(EEmotionalState NewState)
{
    if (NewState != CurrentAnimationState.CurrentEmotionalState)
    {
        TransitionToEmotionalState(NewState, 0.0f);
    }
}

void UArchetypeAnimationSystem::SetSurvivalCondition(float Fear, float Fatigue, float Injury)
{
    CurrentAnimationState.FearLevel = FMath::Clamp(Fear, 0.0f, 1.0f);
    CurrentAnimationState.FatigueLevel = FMath::Clamp(Fatigue, 0.0f, 1.0f);
    CurrentAnimationState.InjuryLevel = FMath::Clamp(Injury, 0.0f, 1.0f);
}

void UArchetypeAnimationSystem::TriggerGesture(EGestureType GestureType)
{
    PlayGesture(GestureType);
}

void UArchetypeAnimationSystem::SetObservationMode(bool bObserving)
{
    CurrentAnimationState.bIsObserving = bObserving;
}

void UArchetypeAnimationSystem::SetDangerState(bool bInDanger)
{
    CurrentAnimationState.bIsInDanger = bInDanger;
}

// Subsistema Global
void UArchetypeAnimationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogArchetypeAnimation, Log, TEXT("ArchetypeAnimationSubsystem initialized"));
}

void UArchetypeAnimationSubsystem::Deinitialize()
{
    RegisteredSystems.Empty();
    
    Super::Deinitialize();
}

void UArchetypeAnimationSubsystem::RegisterCharacterAnimationSystem(UArchetypeAnimationSystem* System)
{
    if (System && !RegisteredSystems.Contains(System))
    {
        RegisteredSystems.Add(System);
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Animation system registered: %s"), 
               System->GetOwner() ? *System->GetOwner()->GetName() : TEXT("Unknown"));
    }
}

void UArchetypeAnimationSubsystem::UnregisterCharacterAnimationSystem(UArchetypeAnimationSystem* System)
{
    if (System)
    {
        RegisteredSystems.Remove(System);
        UE_LOG(LogArchetypeAnimation, Log, TEXT("Animation system unregistered"));
    }
}

void UArchetypeAnimationSubsystem::UpdateGlobalAnimationState(float DeltaTime)
{
    // Atualizar sistemas globais que afetam todas as animações
    // Por exemplo: clima, hora do dia, eventos globais
    
    for (UArchetypeAnimationSystem* System : RegisteredSystems)
    {
        if (IsValid(System))
        {
            // Aplicar modificadores globais
            // TODO: Implementar efeitos globais
        }
    }
}

TArray<UArchetypeAnimationSystem*> UArchetypeAnimationSubsystem::GetAllAnimationSystems() const
{
    TArray<UArchetypeAnimationSystem*> ValidSystems;
    
    for (UArchetypeAnimationSystem* System : RegisteredSystems)
    {
        if (IsValid(System))
        {
            ValidSystems.Add(System);
        }
    }
    
    return ValidSystems;
}