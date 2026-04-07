#include "ConsciousnessSystem.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UConsciousnessComponent::UConsciousnessComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default consciousness metrics
    ConsciousnessData.AwarenessLevel = 25.0f;
    ConsciousnessData.IntegrationLevel = 0.0f;
    ConsciousnessData.TranscendenceProgress = 0.0f;
    ConsciousnessData.UnityConnection = 0.0f;
    ConsciousnessData.CurrentState = EConsciousnessState::Ordinary;
    ConsciousnessData.CurrentAwareness = EAwarenessLevel::Conscious;
}

void UConsciousnessComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Register with the game mode if it's a consciousness-aware game mode
    if (AConsciousnessGameMode* GameMode = Cast<AConsciousnessGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        GameMode->RegisterConsciousEntity(GetOwner());
    }
}

void UConsciousnessComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Gradual awareness growth over time
    float AwarenessGrowth = AwarenessGrowthRate * DeltaTime;
    
    // Apply global consciousness field influence
    if (AConsciousnessGameMode* GameMode = Cast<AConsciousnessGameMode>(UGameplayStatics::GetGameMode(this)))
    {
        float GlobalField = GameMode->GetGlobalConsciousnessLevel();
        AwarenessGrowth *= (1.0f + GlobalField / 100.0f);
    }
    
    ModifyAwareness(AwarenessGrowth);
    
    // Check for state transitions
    EConsciousnessState OldState = ConsciousnessData.CurrentState;
    EAwarenessLevel OldAwareness = ConsciousnessData.CurrentAwareness;
    
    // Update awareness level based on metrics
    if (ConsciousnessData.AwarenessLevel >= 90.0f)
    {
        ConsciousnessData.CurrentAwareness = EAwarenessLevel::Cosmic;
    }
    else if (ConsciousnessData.AwarenessLevel >= 75.0f)
    {
        ConsciousnessData.CurrentAwareness = EAwarenessLevel::Superconscious;
    }
    else if (ConsciousnessData.AwarenessLevel >= 50.0f)
    {
        ConsciousnessData.CurrentAwareness = EAwarenessLevel::Conscious;
    }
    else if (ConsciousnessData.AwarenessLevel >= 25.0f)
    {
        ConsciousnessData.CurrentAwareness = EAwarenessLevel::Subconscious;
    }
    else
    {
        ConsciousnessData.CurrentAwareness = EAwarenessLevel::Unconscious;
    }
    
    // Update consciousness state based on transcendence progress
    if (ConsciousnessData.TranscendenceProgress >= 95.0f && ConsciousnessData.UnityConnection >= 90.0f)
    {
        ConsciousnessData.CurrentState = EConsciousnessState::Unity;
    }
    else if (ConsciousnessData.TranscendenceProgress >= StateTransitionThreshold)
    {
        ConsciousnessData.CurrentState = EConsciousnessState::Transcendent;
    }
    else if (ConsciousnessData.AwarenessLevel >= StateTransitionThreshold)
    {
        ConsciousnessData.CurrentState = EConsciousnessState::Expanded;
    }
    else
    {
        ConsciousnessData.CurrentState = EConsciousnessState::Ordinary;
    }
    
    // Trigger events if states changed
    if (OldState != ConsciousnessData.CurrentState)
    {
        OnConsciousnessStateChanged(OldState, ConsciousnessData.CurrentState);
    }
    
    if (OldAwareness != ConsciousnessData.CurrentAwareness)
    {
        OnAwarenessLevelChanged(OldAwareness, ConsciousnessData.CurrentAwareness);
    }
}

void UConsciousnessComponent::ModifyAwareness(float Delta)
{
    ConsciousnessData.AwarenessLevel = FMath::Clamp(ConsciousnessData.AwarenessLevel + Delta, 0.0f, 100.0f);
    
    // Integration grows slower but is influenced by awareness
    float IntegrationGrowth = Delta * 0.3f * (ConsciousnessData.AwarenessLevel / 100.0f);
    ConsciousnessData.IntegrationLevel = FMath::Clamp(ConsciousnessData.IntegrationLevel + IntegrationGrowth, 0.0f, 100.0f);
    
    // Transcendence requires high integration
    if (ConsciousnessData.IntegrationLevel > 60.0f)
    {
        float TranscendenceGrowth = Delta * 0.2f * (ConsciousnessData.IntegrationLevel / 100.0f);
        ConsciousnessData.TranscendenceProgress = FMath::Clamp(ConsciousnessData.TranscendenceProgress + TranscendenceGrowth, 0.0f, 100.0f);
    }
    
    // Unity connection requires high transcendence
    if (ConsciousnessData.TranscendenceProgress > 75.0f)
    {
        float UnityGrowth = Delta * 0.1f * (ConsciousnessData.TranscendenceProgress / 100.0f);
        ConsciousnessData.UnityConnection = FMath::Clamp(ConsciousnessData.UnityConnection + UnityGrowth, 0.0f, 100.0f);
    }
}

void UConsciousnessComponent::TriggerConsciousnessShift(EConsciousnessState NewState)
{
    EConsciousnessState OldState = ConsciousnessData.CurrentState;
    ConsciousnessData.CurrentState = NewState;
    
    // Adjust metrics to match the new state
    switch (NewState)
    {
        case EConsciousnessState::Expanded:
            ConsciousnessData.AwarenessLevel = FMath::Max(ConsciousnessData.AwarenessLevel, 75.0f);
            break;
        case EConsciousnessState::Transcendent:
            ConsciousnessData.TranscendenceProgress = FMath::Max(ConsciousnessData.TranscendenceProgress, 75.0f);
            break;
        case EConsciousnessState::Unity:
            ConsciousnessData.UnityConnection = FMath::Max(ConsciousnessData.UnityConnection, 90.0f);
            break;
        default:
            break;
    }
    
    OnConsciousnessStateChanged(OldState, NewState);
}

bool UConsciousnessComponent::CanTranscend() const
{
    return ConsciousnessData.AwarenessLevel >= StateTransitionThreshold && 
           ConsciousnessData.IntegrationLevel >= 50.0f;
}

// Game Mode Implementation
AConsciousnessGameMode::AConsciousnessGameMode()
{
    GlobalConsciousnessField = 50.0f;
}

void AConsciousnessGameMode::RegisterConsciousEntity(AActor* Entity)
{
    if (Entity && !ConsciousEntities.Contains(Entity))
    {
        ConsciousEntities.Add(Entity);
        UpdateGlobalField();
    }
}

void AConsciousnessGameMode::UpdateGlobalField()
{
    if (ConsciousEntities.Num() == 0)
    {
        return;
    }
    
    float TotalAwareness = 0.0f;
    int32 ValidEntities = 0;
    
    for (AActor* Entity : ConsciousEntities)
    {
        if (IsValid(Entity))
        {
            if (UConsciousnessComponent* ConsComp = Entity->FindComponentByClass<UConsciousnessComponent>())
            {
                TotalAwareness += ConsComp->GetConsciousnessMetrics().AwarenessLevel;
                ValidEntities++;
            }
        }
    }
    
    if (ValidEntities > 0)
    {
        GlobalConsciousnessField = TotalAwareness / ValidEntities;
        
        // Apply collective consciousness amplification
        float AmplificationFactor = 1.0f + (ValidEntities * 0.1f);
        GlobalConsciousnessField = FMath::Min(GlobalConsciousnessField * AmplificationFactor, 100.0f);
    }
}