#include "NPC_EmotionalStateComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UNPC_EmotionalStateComponent::UNPC_EmotionalStateComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default emotional state
    CurrentState = ENPC_EmotionalState::Calm;
    EmotionalIntensity = 0.0f;
    EmotionalStability = 1.0f;
    StateTransitionTimer = 0.0f;
    LastPlayerDistance = 0.0f;
    PreviousState = ENPC_EmotionalState::Calm;

    // Setup default emotional triggers
    FNPC_EmotionalTrigger PlayerProximityTrigger;
    PlayerProximityTrigger.TargetState = ENPC_EmotionalState::Curious;
    PlayerProximityTrigger.TriggerDistance = 2000.0f;
    PlayerProximityTrigger.IntensityMultiplier = 0.5f;
    PlayerProximityTrigger.DecayRate = 0.2f;
    EmotionalTriggers.Add(PlayerProximityTrigger);

    FNPC_EmotionalTrigger ThreatTrigger;
    ThreatTrigger.TargetState = ENPC_EmotionalState::Fearful;
    ThreatTrigger.TriggerDistance = 1000.0f;
    ThreatTrigger.IntensityMultiplier = 1.5f;
    ThreatTrigger.DecayRate = 0.1f;
    EmotionalTriggers.Add(ThreatTrigger);
}

void UNPC_EmotionalStateComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize emotional state
    CurrentState = ENPC_EmotionalState::Calm;
    EmotionalIntensity = 0.0f;
    StateTransitionTimer = 0.0f;
}

void UNPC_EmotionalStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update emotional state over time
    UpdateEmotionalDecay(DeltaTime);
    ProcessEmotionalTriggers();

    // Update state transition timer
    StateTransitionTimer += DeltaTime;

    // Check for player proximity and update emotional response
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        UpdateEmotionalResponseToPlayer(PlayerPawn);
    }
}

void UNPC_EmotionalStateComponent::TriggerEmotionalState(ENPC_EmotionalState NewState, float Intensity)
{
    if (!ShouldTransitionToState(NewState))
    {
        return;
    }

    PreviousState = CurrentState;
    CurrentState = NewState;
    EmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    StateTransitionTimer = 0.0f;

    // Log state change for debugging
    UE_LOG(LogTemp, Log, TEXT("NPC Emotional State changed to: %s with intensity: %f"), 
           *GetEmotionalStateString(), EmotionalIntensity);
}

void UNPC_EmotionalStateComponent::ModifyEmotionalIntensity(float DeltaIntensity)
{
    EmotionalIntensity = FMath::Clamp(EmotionalIntensity + DeltaIntensity, 0.0f, 1.0f);
    
    // If intensity drops to zero, return to calm state
    if (EmotionalIntensity <= 0.0f && CurrentState != ENPC_EmotionalState::Calm)
    {
        TriggerEmotionalState(ENPC_EmotionalState::Calm, 0.0f);
    }
}

bool UNPC_EmotionalStateComponent::IsInEmotionalState(ENPC_EmotionalState StateToCheck) const
{
    return CurrentState == StateToCheck;
}

float UNPC_EmotionalStateComponent::GetEmotionalIntensityForState(ENPC_EmotionalState State) const
{
    if (CurrentState == State)
    {
        return EmotionalIntensity;
    }
    return 0.0f;
}

void UNPC_EmotionalStateComponent::UpdateEmotionalResponseToPlayer(AActor* PlayerActor)
{
    if (!PlayerActor || !GetOwner())
    {
        return;
    }

    float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    
    // Check if player distance has changed significantly
    float DistanceDelta = FMath::Abs(DistanceToPlayer - LastPlayerDistance);
    if (DistanceDelta > 100.0f) // Only update if player moved significantly
    {
        LastPlayerDistance = DistanceToPlayer;

        // Determine emotional response based on distance
        if (DistanceToPlayer < 500.0f)
        {
            // Very close - trigger fear or aggression depending on NPC type
            if (CurrentState == ENPC_EmotionalState::Calm)
            {
                TriggerEmotionalState(ENPC_EmotionalState::Fearful, 0.8f);
            }
            else if (CurrentState == ENPC_EmotionalState::Territorial)
            {
                TriggerEmotionalState(ENPC_EmotionalState::Aggressive, 1.0f);
            }
        }
        else if (DistanceToPlayer < 1500.0f)
        {
            // Medium distance - curiosity or caution
            if (CurrentState == ENPC_EmotionalState::Calm)
            {
                TriggerEmotionalState(ENPC_EmotionalState::Curious, 0.4f);
            }
        }
        else if (DistanceToPlayer > 3000.0f)
        {
            // Far away - return to calm if not in territorial state
            if (CurrentState != ENPC_EmotionalState::Territorial)
            {
                DecayEmotionalState(0.1f);
            }
        }
    }
}

void UNPC_EmotionalStateComponent::AssessThreatLevel(AActor* ThreatActor, float ThreatLevel)
{
    if (!ThreatActor || ThreatLevel <= 0.0f)
    {
        return;
    }

    // High threat level triggers fear or aggression
    if (ThreatLevel > 0.7f)
    {
        if (EmotionalStability > 1.2f) // High stability NPCs become aggressive
        {
            TriggerEmotionalState(ENPC_EmotionalState::Aggressive, ThreatLevel);
        }
        else // Low stability NPCs become fearful
        {
            TriggerEmotionalState(ENPC_EmotionalState::Fearful, ThreatLevel);
        }
    }
    else if (ThreatLevel > 0.3f)
    {
        // Medium threat triggers agitation
        TriggerEmotionalState(ENPC_EmotionalState::Agitated, ThreatLevel * 0.8f);
    }
}

void UNPC_EmotionalStateComponent::DecayEmotionalState(float DeltaTime)
{
    if (CurrentState == ENPC_EmotionalState::Calm)
    {
        return; // Already calm
    }

    // Apply emotional decay based on stability
    float DecayRate = 0.1f / EmotionalStability;
    ModifyEmotionalIntensity(-DecayRate * DeltaTime);
}

FString UNPC_EmotionalStateComponent::GetEmotionalStateString() const
{
    switch (CurrentState)
    {
        case ENPC_EmotionalState::Calm: return TEXT("Calm");
        case ENPC_EmotionalState::Agitated: return TEXT("Agitated");
        case ENPC_EmotionalState::Fearful: return TEXT("Fearful");
        case ENPC_EmotionalState::Aggressive: return TEXT("Aggressive");
        case ENPC_EmotionalState::Curious: return TEXT("Curious");
        case ENPC_EmotionalState::Territorial: return TEXT("Territorial");
        case ENPC_EmotionalState::Protective: return TEXT("Protective");
        case ENPC_EmotionalState::Hunting: return TEXT("Hunting");
        default: return TEXT("Unknown");
    }
}

void UNPC_EmotionalStateComponent::UpdateEmotionalDecay(float DeltaTime)
{
    // Natural emotional decay over time
    if (CurrentState != ENPC_EmotionalState::Calm && StateTransitionTimer > 5.0f)
    {
        DecayEmotionalState(DeltaTime);
    }
}

void UNPC_EmotionalStateComponent::ProcessEmotionalTriggers()
{
    // Process environmental and situational triggers
    for (const FNPC_EmotionalTrigger& Trigger : EmotionalTriggers)
    {
        // Check if trigger conditions are met
        // This would be expanded based on specific trigger types
        if (CurrentState == ENPC_EmotionalState::Calm && 
            FMath::RandRange(0.0f, 1.0f) < 0.01f) // 1% chance per update
        {
            TriggerEmotionalState(Trigger.TargetState, Trigger.IntensityMultiplier * 0.3f);
            break; // Only process one trigger per update
        }
    }
}

bool UNPC_EmotionalStateComponent::ShouldTransitionToState(ENPC_EmotionalState NewState) const
{
    // Prevent rapid state changes
    if (StateTransitionTimer < 1.0f && NewState != CurrentState)
    {
        return false;
    }

    // Some states have priority over others
    if (CurrentState == ENPC_EmotionalState::Aggressive && NewState == ENPC_EmotionalState::Curious)
    {
        return false; // Don't become curious while aggressive
    }

    if (CurrentState == ENPC_EmotionalState::Fearful && NewState == ENPC_EmotionalState::Curious)
    {
        return false; // Don't become curious while fearful
    }

    return true;
}