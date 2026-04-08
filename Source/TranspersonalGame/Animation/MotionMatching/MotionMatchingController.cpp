#include "MotionMatchingController.h"
#include "Engine/World.h"
#include "PoseSearch/PoseSearchDatabase.h"
#include "PoseSearch/PoseSearchSchema.h"

UMotionMatchingController::UMotionMatchingController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    DatabaseUpdateTimer = 0.0f;
    ModifierUpdateTimer = 0.0f;
    CurrentDatabase = nullptr;
    PreviousDatabase = nullptr;
}

void UMotionMatchingController::BeginPlay()
{
    Super::BeginPlay();
    
    // Encontrar o Animation System Manager no mesmo actor
    AnimationManager = GetOwner()->FindComponentByClass<UAnimationSystemManager>();
    
    if (AnimationManager)
    {
        // Conectar ao evento de mudança de estado emocional
        AnimationManager->OnEmotionalStateChanged.AddDynamic(this, &UMotionMatchingController::UpdateDatabaseSelection);
        
        // Configuração inicial
        UpdateDatabaseSelection();
        ApplyPersonalityModifiers();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("MotionMatchingController: Animation System Manager not found!"));
    }
}

void UMotionMatchingController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    DatabaseUpdateTimer += DeltaTime;
    ModifierUpdateTimer += DeltaTime;
    
    // Atualizar seleção de database a cada 0.2 segundos
    if (DatabaseUpdateTimer >= 0.2f)
    {
        DatabaseUpdateTimer = 0.0f;
        UpdateDatabaseSelection();
    }
    
    // Atualizar modificadores a cada 0.1 segundos
    if (ModifierUpdateTimer >= 0.1f)
    {
        ModifierUpdateTimer = 0.0f;
        ApplyPersonalityModifiers();
    }
    
    SmoothDatabaseTransition();
}

void UMotionMatchingController::UpdateDatabaseSelection()
{
    if (!AnimationManager)
        return;
        
    PreviousDatabase = CurrentDatabase;
    CurrentDatabase = GetOptimalDatabase();
    
    if (CurrentDatabase != PreviousDatabase && CurrentDatabase != nullptr)
    {
        OnDatabaseChanged.Broadcast(CurrentDatabase);
    }
}

void UMotionMatchingController::ApplyPersonalityModifiers()
{
    if (!AnimationManager)
        return;
        
    CalculatePersonalityModifiers();
}

UPoseSearchDatabase* UMotionMatchingController::GetOptimalDatabase() const
{
    if (!AnimationManager)
        return nullptr;
        
    FMotionMatchingDatabase* DatabaseSet = GetDatabaseSetForCharacterType(AnimationManager->CharacterType);
    if (!DatabaseSet)
        return nullptr;
        
    return SelectDatabaseForEmotionalState(*DatabaseSet, AnimationManager->CurrentEmotionalState);
}

float UMotionMatchingController::GetBlendTime() const
{
    if (!AnimationManager)
        return 0.2f; // Tempo padrão
        
    // Tempo de blend baseado na personalidade
    float baseBlendTime = 0.2f;
    
    // Personagens mais nervosos fazem transições mais rápidas
    float nervousnessModifier = 1.0f - (AnimationManager->Personality.Nervousness * 0.3f);
    
    // Personagens mais confiantes fazem transições mais suaves
    float confidenceModifier = 1.0f + (AnimationManager->Personality.Confidence * 0.2f);
    
    return baseBlendTime * nervousnessModifier * confidenceModifier;
}

FMotionMatchingDatabase* UMotionMatchingController::GetDatabaseSetForCharacterType(ECharacterAnimationType CharacterType)
{
    switch (CharacterType)
    {
        case ECharacterAnimationType::Player:
            return &DatabasesPlayer;
        case ECharacterAnimationType::SmallHerbivore:
            return &DatabasesSmallHerbivore;
        case ECharacterAnimationType::LargeHerbivore:
            return &DatabasesLargeHerbivore;
        case ECharacterAnimationType::SmallCarnivore:
            return &DatabasesSmallCarnivore;
        case ECharacterAnimationType::LargeCarnivore:
            return &DatabasesLargeCarnivore;
        case ECharacterAnimationType::Apex:
            return &DatabasesApex;
        default:
            return nullptr;
    }
}

UPoseSearchDatabase* UMotionMatchingController::SelectDatabaseForEmotionalState(const FMotionMatchingDatabase& DatabaseSet, EEmotionalState EmotionalState) const
{
    switch (EmotionalState)
    {
        case EEmotionalState::Calm:
            return DatabaseSet.IdleDatabase;
        case EEmotionalState::Alert:
            return DatabaseSet.AlertDatabase;
        case EEmotionalState::Nervous:
            return DatabaseSet.AlertDatabase; // Usar alert como fallback
        case EEmotionalState::Fearful:
            return DatabaseSet.FearDatabase;
        case EEmotionalState::Aggressive:
            return DatabaseSet.AggressiveDatabase;
        case EEmotionalState::Hunting:
            return DatabaseSet.AggressiveDatabase; // Usar aggressive como fallback
        case EEmotionalState::Feeding:
            return DatabaseSet.FeedingDatabase;
        case EEmotionalState::Resting:
            return DatabaseSet.RestingDatabase;
        default:
            return DatabaseSet.LocomotionDatabase; // Fallback padrão
    }
}

void UMotionMatchingController::CalculatePersonalityModifiers()
{
    if (!AnimationManager)
        return;
        
    const FCharacterPersonality& Personality = AnimationManager->Personality;
    EEmotionalState CurrentState = AnimationManager->CurrentEmotionalState;
    
    // Calcular modificadores baseados na personalidade e estado emocional
    CurrentModifiers.SpeedMultiplier = Personality.MovementSpeed;
    
    // Ajustar velocidade baseado no estado emocional
    switch (CurrentState)
    {
        case EEmotionalState::Fearful:
            CurrentModifiers.SpeedMultiplier *= 1.4f;
            break;
        case EEmotionalState::Nervous:
            CurrentModifiers.SpeedMultiplier *= 1.1f;
            break;
        case EEmotionalState::Resting:
            CurrentModifiers.SpeedMultiplier *= 0.5f;
            break;
        case EEmotionalState::Hunting:
            CurrentModifiers.SpeedMultiplier *= 1.2f;
            break;
    }
    
    // Frequência de passos baseada na personalidade
    CurrentModifiers.StepFrequency = Personality.StepLength;
    
    // Chance de movimentos nervosos
    CurrentModifiers.NervousTwitchChance = Personality.Nervousness;
    if (CurrentState == EEmotionalState::Nervous || CurrentState == EEmotionalState::Fearful)
    {
        CurrentModifiers.NervousTwitchChance *= 2.0f;
    }
    
    // Chance de olhar em volta (curiosidade)
    CurrentModifiers.HeadLookAroundChance = Personality.Curiosity;
    if (CurrentState == EEmotionalState::Alert)
    {
        CurrentModifiers.HeadLookAroundChance *= 1.5f;
    }
    
    // Nível de tensão corporal
    CurrentModifiers.BodyTensionLevel = 1.0f;
    switch (CurrentState)
    {
        case EEmotionalState::Alert:
            CurrentModifiers.BodyTensionLevel = 1.2f;
            break;
        case EEmotionalState::Nervous:
            CurrentModifiers.BodyTensionLevel = 1.3f;
            break;
        case EEmotionalState::Fearful:
            CurrentModifiers.BodyTensionLevel = 1.5f;
            break;
        case EEmotionalState::Resting:
            CurrentModifiers.BodyTensionLevel = 0.6f;
            break;
        case EEmotionalState::Aggressive:
            CurrentModifiers.BodyTensionLevel = 1.4f;
            break;
    }
    
    // Aplicar modificadores da personalidade
    CurrentModifiers.BodyTensionLevel *= (1.0f + (Personality.Nervousness * 0.3f));
    CurrentModifiers.BodyTensionLevel *= (1.0f + (Personality.Aggression * 0.2f));
}

void UMotionMatchingController::SmoothDatabaseTransition()
{
    // Esta função seria expandida para implementar transições suaves entre databases
    // Por agora, apenas registra mudanças para debug
    
    if (CurrentDatabase != PreviousDatabase && CurrentDatabase != nullptr)
    {
        UE_LOG(LogTemp, Log, TEXT("Motion Matching: Database changed to %s"), 
               CurrentDatabase ? *CurrentDatabase->GetName() : TEXT("None"));
    }
}