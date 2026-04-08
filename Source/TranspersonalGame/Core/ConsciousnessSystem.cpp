#include "ConsciousnessSystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Sound/SoundBase.h"
#include "Materials/MaterialInterface.h"
#include "Kismet/GameplayStatics.h"

UConsciousnessSystem::UConsciousnessSystem()
{
    CurrentState = EConsciousnessState::Waking;
    PreviousState = EConsciousnessState::Waking;
    bIsTransitioning = false;
    TransitionProgress = 0.0f;
    
    // Initialize with basic waking consciousness metrics
    CurrentMetrics.Awareness = 0.5f;
    CurrentMetrics.Clarity = 0.5f;
    CurrentMetrics.Equanimity = 0.5f;
    CurrentMetrics.Integration = 0.5f;
    CurrentMetrics.Transcendence = 0.0f;
    
    // Start with physical perception layer active
    ActivePerceptionLayers.Add(EPerceptionLayer::Physical);
}

void UConsciousnessSystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness System Initialized"));
    
    // Load consciousness states data table
    if (!ConsciousnessStatesTable)
    {
        ConsciousnessStatesTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Data/ConsciousnessStatesTable"));
        if (!ConsciousnessStatesTable)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load ConsciousnessStatesTable"));
        }
    }
}

void UConsciousnessSystem::Deinitialize()
{
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(TransitionTimerHandle);
    }
    
    Super::Deinitialize();
}

bool UConsciousnessSystem::TransitionToState(EConsciousnessState NewState, bool bForceTransition)
{
    if (NewState == CurrentState)
    {
        return true;
    }
    
    if (bIsTransitioning && !bForceTransition)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot transition to new state while already transitioning"));
        return false;
    }
    
    if (!bForceTransition && !ValidateStateTransition(CurrentState, NewState))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid state transition from %d to %d"), (int32)CurrentState, (int32)NewState);
        return false;
    }
    
    // Begin transition
    PreviousState = CurrentState;
    CurrentState = NewState;
    bIsTransitioning = true;
    TransitionProgress = 0.0f;
    
    // Get transition duration from data table
    float TransitionDuration = 3.0f;
    if (FConsciousnessStateData* StateData = GetStateData(NewState))
    {
        TransitionDuration = StateData->TransitionDuration;
        
        // Play transition sound
        if (StateData->TransitionSound && GetWorld())
        {
            UGameplayStatics::PlaySound2D(GetWorld(), StateData->TransitionSound);
        }
    }
    
    // Start transition timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            TransitionTimerHandle,
            this,
            &UConsciousnessSystem::UpdateTransitionProgress,
            0.1f,
            true
        );
        
        // Set completion timer
        FTimerHandle CompletionTimer;
        GetWorld()->GetTimerManager().SetTimer(
            CompletionTimer,
            this,
            &UConsciousnessSystem::CompleteTransition,
            TransitionDuration,
            false
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Starting transition from %d to %d"), (int32)PreviousState, (int32)CurrentState);
    
    return true;
}

void UConsciousnessSystem::UpdateMetrics(const FConsciousnessMetrics& NewMetrics)
{
    CurrentMetrics = NewMetrics;
    
    // Clamp values to valid range
    CurrentMetrics.Awareness = FMath::Clamp(CurrentMetrics.Awareness, 0.0f, 1.0f);
    CurrentMetrics.Clarity = FMath::Clamp(CurrentMetrics.Clarity, 0.0f, 1.0f);
    CurrentMetrics.Equanimity = FMath::Clamp(CurrentMetrics.Equanimity, 0.0f, 1.0f);
    CurrentMetrics.Integration = FMath::Clamp(CurrentMetrics.Integration, 0.0f, 1.0f);
    CurrentMetrics.Transcendence = FMath::Clamp(CurrentMetrics.Transcendence, 0.0f, 1.0f);
    
    OnMetricsUpdated.Broadcast(CurrentMetrics);
    
    UE_LOG(LogTemp, Log, TEXT("Consciousness metrics updated: A=%.2f, C=%.2f, E=%.2f, I=%.2f, T=%.2f"),
        CurrentMetrics.Awareness, CurrentMetrics.Clarity, CurrentMetrics.Equanimity,
        CurrentMetrics.Integration, CurrentMetrics.Transcendence);
}

void UConsciousnessSystem::ModifyMetric(const FString& MetricName, float Delta)
{
    FConsciousnessMetrics NewMetrics = CurrentMetrics;
    
    if (MetricName == "Awareness")
    {
        NewMetrics.Awareness += Delta;
    }
    else if (MetricName == "Clarity")
    {
        NewMetrics.Clarity += Delta;
    }
    else if (MetricName == "Equanimity")
    {
        NewMetrics.Equanimity += Delta;
    }
    else if (MetricName == "Integration")
    {
        NewMetrics.Integration += Delta;
    }
    else if (MetricName == "Transcendence")
    {
        NewMetrics.Transcendence += Delta;
    }
    
    UpdateMetrics(NewMetrics);
}

bool UConsciousnessSystem::CanAccessPerceptionLayer(EPerceptionLayer Layer) const
{
    switch (CurrentState)
    {
        case EConsciousnessState::Waking:
            return Layer == EPerceptionLayer::Physical || Layer == EPerceptionLayer::Emotional;
            
        case EConsciousnessState::Dreaming:
            return Layer != EPerceptionLayer::Physical;
            
        case EConsciousnessState::DeepMeditation:
            return Layer == EPerceptionLayer::Mental || Layer == EPerceptionLayer::Intuitive;
            
        case EConsciousnessState::Transpersonal:
            return Layer == EPerceptionLayer::Archetypal || Layer == EPerceptionLayer::Cosmic;
            
        case EConsciousnessState::Unity:
            return true; // Can access all layers
            
        case EConsciousnessState::Void:
            return false; // No perception layers in void state
            
        default:
            return false;
    }
}

void UConsciousnessSystem::ActivatePerceptionLayer(EPerceptionLayer Layer)
{
    if (!CanAccessPerceptionLayer(Layer))
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot activate perception layer %d in current state %d"), (int32)Layer, (int32)CurrentState);
        return;
    }
    
    if (!ActivePerceptionLayers.Contains(Layer))
    {
        ActivePerceptionLayers.Add(Layer);
        OnPerceptionLayerActivated.Broadcast(Layer);
        
        UE_LOG(LogTemp, Log, TEXT("Activated perception layer: %d"), (int32)Layer);
    }
}

void UConsciousnessSystem::DeactivatePerceptionLayer(EPerceptionLayer Layer)
{
    if (ActivePerceptionLayers.Contains(Layer))
    {
        ActivePerceptionLayers.Remove(Layer);
        UE_LOG(LogTemp, Log, TEXT("Deactivated perception layer: %d"), (int32)Layer);
    }
}

float UConsciousnessSystem::GetTranscendenceLevel() const
{
    // Calculate transcendence level based on all metrics
    float BaseTranscendence = CurrentMetrics.Transcendence;
    float MetricAverage = (CurrentMetrics.Awareness + CurrentMetrics.Clarity + 
                          CurrentMetrics.Equanimity + CurrentMetrics.Integration) / 4.0f;
    
    return FMath::Min(BaseTranscendence + (MetricAverage * 0.3f), 1.0f);
}

FConsciousnessStateData* UConsciousnessSystem::GetStateData(EConsciousnessState State) const
{
    if (!ConsciousnessStatesTable)
    {
        return nullptr;
    }
    
    FString StateName = UEnum::GetValueAsString(State);
    return ConsciousnessStatesTable->FindRow<FConsciousnessStateData>(FName(*StateName), TEXT(""));
}

void UConsciousnessSystem::CompleteTransition()
{
    bIsTransitioning = false;
    TransitionProgress = 1.0f;
    
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(TransitionTimerHandle);
    }
    
    // Apply state effects
    ApplyStateEffects(CurrentState);
    
    // Broadcast state change
    OnStateChanged.Broadcast(PreviousState, CurrentState);
    
    UE_LOG(LogTemp, Log, TEXT("Completed transition to state: %d"), (int32)CurrentState);
}

void UConsciousnessSystem::UpdateTransitionProgress()
{
    TransitionProgress += 0.1f;
    TransitionProgress = FMath::Clamp(TransitionProgress, 0.0f, 1.0f);
}

bool UConsciousnessSystem::ValidateStateTransition(EConsciousnessState FromState, EConsciousnessState ToState) const
{
    // Check if player has sufficient metrics for the target state
    if (FConsciousnessStateData* StateData = GetStateData(ToState))
    {
        const FConsciousnessMetrics& Required = StateData->RequiredMetrics;
        
        return CurrentMetrics.Awareness >= Required.Awareness &&
               CurrentMetrics.Clarity >= Required.Clarity &&
               CurrentMetrics.Equanimity >= Required.Equanimity &&
               CurrentMetrics.Integration >= Required.Integration &&
               CurrentMetrics.Transcendence >= Required.Transcendence;
    }
    
    return true; // Allow transition if no data found
}

void UConsciousnessSystem::ApplyStateEffects(EConsciousnessState State)
{
    // Clear current perception layers
    ActivePerceptionLayers.Empty();
    
    // Activate appropriate perception layers for the new state
    switch (State)
    {
        case EConsciousnessState::Waking:
            ActivePerceptionLayers.Add(EPerceptionLayer::Physical);
            ActivePerceptionLayers.Add(EPerceptionLayer::Emotional);
            break;
            
        case EConsciousnessState::Dreaming:
            ActivePerceptionLayers.Add(EPerceptionLayer::Emotional);
            ActivePerceptionLayers.Add(EPerceptionLayer::Mental);
            break;
            
        case EConsciousnessState::DeepMeditation:
            ActivePerceptionLayers.Add(EPerceptionLayer::Mental);
            ActivePerceptionLayers.Add(EPerceptionLayer::Intuitive);
            break;
            
        case EConsciousnessState::Transpersonal:
            ActivePerceptionLayers.Add(EPerceptionLayer::Intuitive);
            ActivePerceptionLayers.Add(EPerceptionLayer::Archetypal);
            break;
            
        case EConsciousnessState::Unity:
            // Add all perception layers
            ActivePerceptionLayers.Add(EPerceptionLayer::Physical);
            ActivePerceptionLayers.Add(EPerceptionLayer::Emotional);
            ActivePerceptionLayers.Add(EPerceptionLayer::Mental);
            ActivePerceptionLayers.Add(EPerceptionLayer::Intuitive);
            ActivePerceptionLayers.Add(EPerceptionLayer::Archetypal);
            ActivePerceptionLayers.Add(EPerceptionLayer::Cosmic);
            break;
            
        case EConsciousnessState::Void:
            // No perception layers in void state
            break;
    }
}