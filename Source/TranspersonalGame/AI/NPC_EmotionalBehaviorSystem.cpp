#include "NPC_EmotionalBehaviorSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNPC_EmotionalBehaviorSystem::UNPC_EmotionalBehaviorSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default values
    CurrentEmotionalState = ENPC_EmotionalState::Calm;
    CurrentEmotionalIntensity = 0.0f;
    EmotionalDecayRate = 0.5f;

    // Emotional thresholds
    FearThreshold = 0.7f;
    AngerThreshold = 0.6f;
    HappinessThreshold = 0.5f;

    // Memory settings
    MaxEmotionalMemories = 20;
    MemoryDecayTime = 300.0f; // 5 minutes

    // Default personality (neutral)
    Neuroticism = 0.5f;
    Extraversion = 0.5f;
    Agreeableness = 0.5f;
    Conscientiousness = 0.5f;
    Openness = 0.5f;

    // Timers
    EmotionalStateTimer = 0.0f;
    MemoryCleanupTimer = 0.0f;
}

void UNPC_EmotionalBehaviorSystem::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultEmotionalResponses();
    
    UE_LOG(LogTemp, Log, TEXT("NPC Emotional Behavior System initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UNPC_EmotionalBehaviorSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateEmotionalState(DeltaTime);
    
    // Cleanup old memories periodically
    MemoryCleanupTimer += DeltaTime;
    if (MemoryCleanupTimer >= 30.0f) // Every 30 seconds
    {
        CleanupOldMemories();
        MemoryCleanupTimer = 0.0f;
    }
}

void UNPC_EmotionalBehaviorSystem::TriggerEmotionalResponse(ENPC_EmotionalTrigger Trigger, float Intensity, const FVector& Location)
{
    if (EmotionalResponses.Contains(Trigger))
    {
        FNPC_EmotionalResponse Response = EmotionalResponses[Trigger];
        
        // Apply personality modifier
        float PersonalityModifier = CalculatePersonalityModifier(Trigger);
        float ModifiedIntensity = FMath::Clamp(Intensity * PersonalityModifier, 0.0f, 1.0f);
        
        // Set new emotional state if override is enabled or intensity is higher
        if (Response.bOverrideCurrentEmotion || ModifiedIntensity > CurrentEmotionalIntensity)
        {
            SetEmotionalState(Response.NewState, ModifiedIntensity, Response.Duration);
        }
        
        // Add to emotional memory
        AddEmotionalMemory(Trigger, Response.NewState, Location, ModifiedIntensity);
        
        UE_LOG(LogTemp, Log, TEXT("NPC %s triggered emotional response: %s (Intensity: %.2f)"), 
               GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
               *UEnum::GetValueAsString(Response.NewState),
               ModifiedIntensity);
    }
}

void UNPC_EmotionalBehaviorSystem::SetEmotionalState(ENPC_EmotionalState NewState, float Intensity, float Duration)
{
    CurrentEmotionalState = NewState;
    CurrentEmotionalIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    if (Duration > 0.0f)
    {
        EmotionalStateTimer = Duration;
    }
    else
    {
        EmotionalStateTimer = 0.0f;
    }
}

ENPC_EmotionalState UNPC_EmotionalBehaviorSystem::GetCurrentEmotionalState() const
{
    return CurrentEmotionalState;
}

float UNPC_EmotionalBehaviorSystem::GetCurrentEmotionalIntensity() const
{
    return CurrentEmotionalIntensity;
}

bool UNPC_EmotionalBehaviorSystem::IsInEmotionalState(ENPC_EmotionalState State) const
{
    return CurrentEmotionalState == State && CurrentEmotionalIntensity > 0.1f;
}

void UNPC_EmotionalBehaviorSystem::AddEmotionalMemory(ENPC_EmotionalTrigger Trigger, ENPC_EmotionalState Response, const FVector& Location, float Weight)
{
    FNPC_EmotionalMemory NewMemory;
    NewMemory.Trigger = Trigger;
    NewMemory.EmotionalResponse = Response;
    NewMemory.Location = Location;
    NewMemory.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    NewMemory.EmotionalWeight = Weight;
    
    EmotionalMemories.Add(NewMemory);
    
    // Remove oldest memories if we exceed the limit
    while (EmotionalMemories.Num() > MaxEmotionalMemories)
    {
        EmotionalMemories.RemoveAt(0);
    }
}

TArray<FNPC_EmotionalMemory> UNPC_EmotionalBehaviorSystem::GetEmotionalMemoriesAtLocation(const FVector& Location, float Radius) const
{
    TArray<FNPC_EmotionalMemory> NearbyMemories;
    
    for (const FNPC_EmotionalMemory& Memory : EmotionalMemories)
    {
        if (FVector::Dist(Memory.Location, Location) <= Radius)
        {
            NearbyMemories.Add(Memory);
        }
    }
    
    return NearbyMemories;
}

FNPC_EmotionalMemory UNPC_EmotionalBehaviorSystem::GetStrongestEmotionalMemory() const
{
    FNPC_EmotionalMemory StrongestMemory;
    float HighestWeight = 0.0f;
    
    for (const FNPC_EmotionalMemory& Memory : EmotionalMemories)
    {
        if (Memory.EmotionalWeight > HighestWeight)
        {
            HighestWeight = Memory.EmotionalWeight;
            StrongestMemory = Memory;
        }
    }
    
    return StrongestMemory;
}

void UNPC_EmotionalBehaviorSystem::ClearEmotionalMemories()
{
    EmotionalMemories.Empty();
}

void UNPC_EmotionalBehaviorSystem::SetPersonalityTraits(float InNeuroticism, float InExtraversion, float InAgreeableness, float InConscientiousness, float InOpenness)
{
    Neuroticism = FMath::Clamp(InNeuroticism, 0.0f, 1.0f);
    Extraversion = FMath::Clamp(InExtraversion, 0.0f, 1.0f);
    Agreeableness = FMath::Clamp(InAgreeableness, 0.0f, 1.0f);
    Conscientiousness = FMath::Clamp(InConscientiousness, 0.0f, 1.0f);
    Openness = FMath::Clamp(InOpenness, 0.0f, 1.0f);
}

float UNPC_EmotionalBehaviorSystem::GetEmotionalModifier(ENPC_EmotionalTrigger Trigger) const
{
    return CalculatePersonalityModifier(Trigger);
}

bool UNPC_EmotionalBehaviorSystem::ShouldFleeFromThreat() const
{
    return (IsInEmotionalState(ENPC_EmotionalState::Fearful) && CurrentEmotionalIntensity > FearThreshold) ||
           (IsInEmotionalState(ENPC_EmotionalState::Suspicious) && CurrentEmotionalIntensity > 0.8f);
}

bool UNPC_EmotionalBehaviorSystem::ShouldApproachPlayer() const
{
    return (IsInEmotionalState(ENPC_EmotionalState::Happy) && CurrentEmotionalIntensity > 0.4f) ||
           (IsInEmotionalState(ENPC_EmotionalState::Curious) && CurrentEmotionalIntensity > 0.5f) ||
           (IsInEmotionalState(ENPC_EmotionalState::Excited) && CurrentEmotionalIntensity > 0.3f);
}

bool UNPC_EmotionalBehaviorSystem::ShouldBeAggressive() const
{
    return (IsInEmotionalState(ENPC_EmotionalState::Angry) && CurrentEmotionalIntensity > AngerThreshold) ||
           (IsInEmotionalState(ENPC_EmotionalState::Aggressive) && CurrentEmotionalIntensity > 0.5f);
}

float UNPC_EmotionalBehaviorSystem::GetMovementSpeedModifier() const
{
    switch (CurrentEmotionalState)
    {
        case ENPC_EmotionalState::Fearful:
            return 1.0f + (CurrentEmotionalIntensity * 0.5f); // Up to 50% faster when fearful
        case ENPC_EmotionalState::Angry:
        case ENPC_EmotionalState::Aggressive:
            return 1.0f + (CurrentEmotionalIntensity * 0.3f); // Up to 30% faster when angry
        case ENPC_EmotionalState::Excited:
            return 1.0f + (CurrentEmotionalIntensity * 0.2f); // Up to 20% faster when excited
        case ENPC_EmotionalState::Sad:
        case ENPC_EmotionalState::Depressed:
            return 1.0f - (CurrentEmotionalIntensity * 0.3f); // Up to 30% slower when sad
        default:
            return 1.0f;
    }
}

float UNPC_EmotionalBehaviorSystem::GetInteractionWillingness() const
{
    switch (CurrentEmotionalState)
    {
        case ENPC_EmotionalState::Happy:
        case ENPC_EmotionalState::Excited:
        case ENPC_EmotionalState::Curious:
            return 0.5f + (CurrentEmotionalIntensity * 0.5f); // More willing when positive
        case ENPC_EmotionalState::Fearful:
        case ENPC_EmotionalState::Angry:
        case ENPC_EmotionalState::Suspicious:
        case ENPC_EmotionalState::Aggressive:
            return 0.5f - (CurrentEmotionalIntensity * 0.4f); // Less willing when negative
        case ENPC_EmotionalState::Sad:
        case ENPC_EmotionalState::Depressed:
            return 0.3f - (CurrentEmotionalIntensity * 0.2f); // Much less willing when depressed
        default:
            return 0.5f; // Neutral willingness when calm
    }
}

void UNPC_EmotionalBehaviorSystem::UpdateEmotionalState(float DeltaTime)
{
    // Decay emotional intensity over time
    if (CurrentEmotionalIntensity > 0.0f)
    {
        CurrentEmotionalIntensity = FMath::Max(0.0f, CurrentEmotionalIntensity - (EmotionalDecayRate * DeltaTime));
        
        // Return to calm state when intensity reaches zero
        if (CurrentEmotionalIntensity <= 0.01f)
        {
            CurrentEmotionalState = ENPC_EmotionalState::Calm;
            CurrentEmotionalIntensity = 0.0f;
        }
    }
    
    // Handle timed emotional states
    if (EmotionalStateTimer > 0.0f)
    {
        EmotionalStateTimer -= DeltaTime;
        if (EmotionalStateTimer <= 0.0f)
        {
            // Transition to calm state when timer expires
            SetEmotionalState(ENPC_EmotionalState::Calm, 0.0f);
        }
    }
}

void UNPC_EmotionalBehaviorSystem::CleanupOldMemories()
{
    if (!GetWorld()) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    EmotionalMemories.RemoveAll([CurrentTime, this](const FNPC_EmotionalMemory& Memory)
    {
        return (CurrentTime - Memory.Timestamp) > MemoryDecayTime;
    });
}

float UNPC_EmotionalBehaviorSystem::CalculatePersonalityModifier(ENPC_EmotionalTrigger Trigger) const
{
    float Modifier = 1.0f;
    
    switch (Trigger)
    {
        case ENPC_EmotionalTrigger::ThreatDetected:
        case ENPC_EmotionalTrigger::DinosaurSighting:
            Modifier *= (1.0f + Neuroticism * 0.5f); // Neurotic NPCs react more strongly to threats
            break;
        case ENPC_EmotionalTrigger::SocialInteraction:
        case ENPC_EmotionalTrigger::PlayerApproach:
            Modifier *= (0.5f + Extraversion * 0.5f); // Extraverted NPCs react more to social situations
            break;
        case ENPC_EmotionalTrigger::TerritoryViolation:
            Modifier *= (0.5f + (1.0f - Agreeableness) * 0.8f); // Less agreeable NPCs react more to violations
            break;
        case ENPC_EmotionalTrigger::WeatherChange:
        case ENPC_EmotionalTrigger::TimeOfDay:
            Modifier *= (0.7f + Openness * 0.6f); // Open NPCs react more to environmental changes
            break;
        default:
            break;
    }
    
    return FMath::Clamp(Modifier, 0.1f, 2.0f);
}

void UNPC_EmotionalBehaviorSystem::InitializeDefaultEmotionalResponses()
{
    // Player approach
    FNPC_EmotionalResponse PlayerApproachResponse;
    PlayerApproachResponse.NewState = ENPC_EmotionalState::Curious;
    PlayerApproachResponse.Intensity = 0.6f;
    PlayerApproachResponse.Duration = 15.0f;
    PlayerApproachResponse.bOverrideCurrentEmotion = false;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::PlayerApproach, PlayerApproachResponse);
    
    // Dinosaur sighting
    FNPC_EmotionalResponse DinosaurResponse;
    DinosaurResponse.NewState = ENPC_EmotionalState::Fearful;
    DinosaurResponse.Intensity = 0.8f;
    DinosaurResponse.Duration = 30.0f;
    DinosaurResponse.bOverrideCurrentEmotion = true;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::DinosaurSighting, DinosaurResponse);
    
    // Food found
    FNPC_EmotionalResponse FoodResponse;
    FoodResponse.NewState = ENPC_EmotionalState::Happy;
    FoodResponse.Intensity = 0.7f;
    FoodResponse.Duration = 20.0f;
    FoodResponse.bOverrideCurrentEmotion = false;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::FoodFound, FoodResponse);
    
    // Threat detected
    FNPC_EmotionalResponse ThreatResponse;
    ThreatResponse.NewState = ENPC_EmotionalState::Fearful;
    ThreatResponse.Intensity = 0.9f;
    ThreatResponse.Duration = 45.0f;
    ThreatResponse.bOverrideCurrentEmotion = true;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::ThreatDetected, ThreatResponse);
    
    // Social interaction
    FNPC_EmotionalResponse SocialResponse;
    SocialResponse.NewState = ENPC_EmotionalState::Happy;
    SocialResponse.Intensity = 0.5f;
    SocialResponse.Duration = 10.0f;
    SocialResponse.bOverrideCurrentEmotion = false;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::SocialInteraction, SocialResponse);
    
    // Territory violation
    FNPC_EmotionalResponse TerritoryResponse;
    TerritoryResponse.NewState = ENPC_EmotionalState::Angry;
    TerritoryResponse.Intensity = 0.7f;
    TerritoryResponse.Duration = 25.0f;
    TerritoryResponse.bOverrideCurrentEmotion = true;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::TerritoryViolation, TerritoryResponse);
    
    // Injury received
    FNPC_EmotionalResponse InjuryResponse;
    InjuryResponse.NewState = ENPC_EmotionalState::Angry;
    InjuryResponse.Intensity = 0.8f;
    InjuryResponse.Duration = 60.0f;
    InjuryResponse.bOverrideCurrentEmotion = true;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::InjuryReceived, InjuryResponse);
}