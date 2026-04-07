#include "ConsciousnessSystem.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UConsciousnessComponent::UConsciousnessComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentState = EConsciousnessState::Ordinary;
    AwarenessLevel = EAwarenessLevel::Conscious;
    
    Metrics.Awareness = 50.0f;
    Metrics.Presence = 50.0f;
    Metrics.Coherence = 50.0f;
    Metrics.Transcendence = 0.0f;
    Metrics.Unity = 0.0f;
    
    StateTransitionTimer = 0.0f;
    MeditationDuration = 0.0f;
    bInTranscendentExperience = false;
}

void UConsciousnessComponent::BeginPlay()
{
    Super::BeginPlay();
    CalculateAwarenessLevel();
}

void UConsciousnessComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateConsciousnessState(DeltaTime);
}

void UConsciousnessComponent::UpdateConsciousnessState(float DeltaTime)
{
    StateTransitionTimer += DeltaTime;
    
    // Natural consciousness fluctuation
    float NoiseValue = FMath::Sin(StateTransitionTimer * 0.1f) * 0.5f + 0.5f;
    
    // Gradually evolve metrics based on current state
    switch (CurrentState)
    {
        case EConsciousnessState::Ordinary:
            Metrics.Awareness = FMath::Clamp(Metrics.Awareness + (NoiseValue - 0.5f) * DeltaTime * 5.0f, 0.0f, 100.0f);
            Metrics.Presence = FMath::Clamp(Metrics.Presence + (NoiseValue - 0.5f) * DeltaTime * 3.0f, 0.0f, 100.0f);
            break;
            
        case EConsciousnessState::Meditative:
            MeditationDuration += DeltaTime;
            Metrics.Awareness = FMath::Clamp(Metrics.Awareness + DeltaTime * 10.0f, 0.0f, 100.0f);
            Metrics.Presence = FMath::Clamp(Metrics.Presence + DeltaTime * 15.0f, 0.0f, 100.0f);
            Metrics.Coherence = FMath::Clamp(Metrics.Coherence + DeltaTime * 8.0f, 0.0f, 100.0f);
            
            // Possibility of transcendent breakthrough
            if (MeditationDuration > 30.0f && Metrics.Awareness > 80.0f && Metrics.Presence > 80.0f)
            {
                if (FMath::RandRange(0.0f, 1.0f) < 0.1f * DeltaTime)
                {
                    TriggerTranscendentExperience();
                }
            }
            break;
            
        case EConsciousnessState::Flow:
            Metrics.Awareness = FMath::Clamp(Metrics.Awareness + DeltaTime * 5.0f, 0.0f, 100.0f);
            Metrics.Coherence = FMath::Clamp(Metrics.Coherence + DeltaTime * 12.0f, 0.0f, 100.0f);
            break;
            
        case EConsciousnessState::Transcendent:
            Metrics.Transcendence = FMath::Clamp(Metrics.Transcendence + DeltaTime * 20.0f, 0.0f, 100.0f);
            Metrics.Unity = FMath::Clamp(Metrics.Unity + DeltaTime * 5.0f, 0.0f, 100.0f);
            
            // Transcendent state naturally fades
            if (bInTranscendentExperience)
            {
                if (StateTransitionTimer > 60.0f) // 1 minute max
                {
                    CurrentState = EConsciousnessState::Meditative;
                    bInTranscendentExperience = false;
                    StateTransitionTimer = 0.0f;
                }
            }
            break;
            
        case EConsciousnessState::Unity:
            Metrics.Unity = FMath::Clamp(Metrics.Unity + DeltaTime * 10.0f, 0.0f, 100.0f);
            Metrics.Transcendence = FMath::Clamp(Metrics.Transcendence + DeltaTime * 5.0f, 0.0f, 100.0f);
            break;
    }
    
    CalculateAwarenessLevel();
    ProcessStateTransitions(DeltaTime);
}

void UConsciousnessComponent::TriggerTranscendentExperience()
{
    CurrentState = EConsciousnessState::Transcendent;
    bInTranscendentExperience = true;
    StateTransitionTimer = 0.0f;
    
    // Immediate boost to transcendence metrics
    Metrics.Transcendence = FMath::Clamp(Metrics.Transcendence + 30.0f, 0.0f, 100.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Transcendent experience triggered!"));
}

void UConsciousnessComponent::EnterMeditativeState()
{
    CurrentState = EConsciousnessState::Meditative;
    MeditationDuration = 0.0f;
    StateTransitionTimer = 0.0f;
}

void UConsciousnessComponent::ExitMeditativeState()
{
    CurrentState = EConsciousnessState::Ordinary;
    MeditationDuration = 0.0f;
    StateTransitionTimer = 0.0f;
}

float UConsciousnessComponent::GetOverallConsciousnessLevel() const
{
    return (Metrics.Awareness + Metrics.Presence + Metrics.Coherence + Metrics.Transcendence + Metrics.Unity) / 5.0f;
}

void UConsciousnessComponent::CalculateAwarenessLevel()
{
    float OverallLevel = GetOverallConsciousnessLevel();
    
    if (OverallLevel < 20.0f)
        AwarenessLevel = EAwarenessLevel::Unconscious;
    else if (OverallLevel < 40.0f)
        AwarenessLevel = EAwarenessLevel::Subconscious;
    else if (OverallLevel < 60.0f)
        AwarenessLevel = EAwarenessLevel::Conscious;
    else if (OverallLevel < 80.0f)
        AwarenessLevel = EAwarenessLevel::Superconscious;
    else
        AwarenessLevel = EAwarenessLevel::Cosmic;
}

void UConsciousnessComponent::ProcessStateTransitions(float DeltaTime)
{
    // Implement natural state transitions based on metrics
    if (CurrentState == EConsciousnessState::Ordinary)
    {
        if (Metrics.Coherence > 70.0f && Metrics.Awareness > 60.0f)
        {
            CurrentState = EConsciousnessState::Flow;
            StateTransitionTimer = 0.0f;
        }
    }
    else if (CurrentState == EConsciousnessState::Flow)
    {
        if (Metrics.Coherence < 50.0f)
        {
            CurrentState = EConsciousnessState::Ordinary;
            StateTransitionTimer = 0.0f;
        }
    }
}

// Game Mode Implementation
AConsciousnessGameMode::AConsciousnessGameMode()
{
    GlobalConsciousnessField = 50.0f;
    MaxPlayers = 8;
}

void AConsciousnessGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize global consciousness field
    GlobalConsciousnessField = 50.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Consciousness Game Mode initialized"));
}

void AConsciousnessGameMode::UpdateGlobalField()
{
    // Calculate global field based on all players' consciousness levels
    float TotalConsciousness = 0.0f;
    int32 PlayerCount = 0;
    
    for (UConsciousnessComponent* Component : PlayerConsciousnessComponents)
    {
        if (Component && IsValid(Component))
        {
            TotalConsciousness += Component->GetOverallConsciousnessLevel();
            PlayerCount++;
        }
    }
    
    if (PlayerCount > 0)
    {
        GlobalConsciousnessField = TotalConsciousness / PlayerCount;
        
        // Amplify collective consciousness effects
        if (PlayerCount > 1)
        {
            float CollectiveBonus = FMath::Sqrt(PlayerCount) * 5.0f;
            GlobalConsciousnessField = FMath::Clamp(GlobalConsciousnessField + CollectiveBonus, 0.0f, 100.0f);
        }
    }
}

void AConsciousnessGameMode::SynchronizePlayerStates()
{
    // Synchronize consciousness states between players for collective experiences
    for (UConsciousnessComponent* Component : PlayerConsciousnessComponents)
    {
        if (Component && IsValid(Component))
        {
            // Apply global field influence
            float FieldInfluence = (GlobalConsciousnessField - Component->GetOverallConsciousnessLevel()) * 0.1f;
            Component->Metrics.Coherence = FMath::Clamp(Component->Metrics.Coherence + FieldInfluence, 0.0f, 100.0f);
        }
    }
}