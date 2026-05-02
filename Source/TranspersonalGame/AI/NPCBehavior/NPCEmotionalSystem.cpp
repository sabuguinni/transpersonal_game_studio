#include "NPCEmotionalSystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

UNPCEmotionalSystem::UNPCEmotionalSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Initialize default values
    CurrentEmotionalState = ENPC_EmotionalState::Calm;
    CurrentIntensity = 0.0f;
    BaselineStress = 0.2f;
    MaxMemories = 20;

    // Initialize personality traits
    Bravery = 0.5f;
    Aggression = 0.3f;
    Curiosity = 0.4f;
    SocialBond = 0.6f;

    EmotionalStateTimer = 0.0f;
    LastMemoryCleanup = 0.0f;

    // Setup default emotional responses
    FNPC_EmotionalResponse PlayerApproachResponse;
    PlayerApproachResponse.TargetState = ENPC_EmotionalState::Cautious;
    PlayerApproachResponse.Intensity = 0.6f;
    PlayerApproachResponse.Duration = 15.0f;
    PlayerApproachResponse.DecayRate = 0.05f;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::PlayerApproach, PlayerApproachResponse);

    FNPC_EmotionalResponse DinosaurResponse;
    DinosaurResponse.TargetState = ENPC_EmotionalState::Fearful;
    DinosaurResponse.Intensity = 0.9f;
    DinosaurResponse.Duration = 30.0f;
    DinosaurResponse.DecayRate = 0.03f;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::DinosaurSighting, DinosaurResponse);

    FNPC_EmotionalResponse NoiseResponse;
    NoiseResponse.TargetState = ENPC_EmotionalState::Cautious;
    NoiseResponse.Intensity = 0.4f;
    NoiseResponse.Duration = 8.0f;
    NoiseResponse.DecayRate = 0.1f;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::LoudNoise, NoiseResponse);

    FNPC_EmotionalResponse ThreatResponse;
    ThreatResponse.TargetState = ENPC_EmotionalState::Panicked;
    ThreatResponse.Intensity = 1.0f;
    ThreatResponse.Duration = 45.0f;
    ThreatResponse.DecayRate = 0.02f;
    EmotionalResponses.Add(ENPC_EmotionalTrigger::GroupThreat, ThreatResponse);
}

void UNPCEmotionalSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize emotional state
    CurrentEmotionalState = ENPC_EmotionalState::Calm;
    CurrentIntensity = BaselineStress;
}

void UNPCEmotionalSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateEmotionalState(DeltaTime);
    ProcessEmotionalDecay(DeltaTime);

    // Clean old memories every 60 seconds
    LastMemoryCleanup += DeltaTime;
    if (LastMemoryCleanup >= 60.0f)
    {
        CleanOldMemories();
        LastMemoryCleanup = 0.0f;
    }
}

void UNPCEmotionalSystem::TriggerEmotionalResponse(ENPC_EmotionalTrigger Trigger, float IntensityMultiplier, FVector TriggerLocation)
{
    if (!EmotionalResponses.Contains(Trigger))
    {
        return;
    }

    FNPC_EmotionalResponse Response = EmotionalResponses[Trigger];
    
    // Apply personality influence
    float PersonalityMultiplier = CalculatePersonalityInfluence(Trigger);
    float FinalIntensity = Response.Intensity * IntensityMultiplier * PersonalityMultiplier;

    // Update current emotional state
    CurrentEmotionalState = Response.TargetState;
    CurrentIntensity = FMath::Clamp(CurrentIntensity + FinalIntensity, 0.0f, 1.0f);
    EmotionalStateTimer = Response.Duration;

    // Add to emotional memory
    AddEmotionalMemory(Trigger, TriggerLocation, FinalIntensity);

    UE_LOG(LogTemp, Log, TEXT("NPC Emotional Response: %s triggered with intensity %.2f"), 
           *UEnum::GetValueAsString(Trigger), FinalIntensity);
}

void UNPCEmotionalSystem::AddEmotionalMemory(ENPC_EmotionalTrigger TriggerType, FVector Location, float EmotionalWeight)
{
    FNPC_EmotionalMemory NewMemory;
    NewMemory.TriggerType = TriggerType;
    NewMemory.Location = Location;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    NewMemory.EmotionalWeight = EmotionalWeight;

    EmotionalMemories.Add(NewMemory);

    // Remove oldest memories if we exceed the limit
    if (EmotionalMemories.Num() > MaxMemories)
    {
        EmotionalMemories.RemoveAt(0);
    }
}

float UNPCEmotionalSystem::GetEmotionalInfluenceAtLocation(FVector Location, ENPC_EmotionalTrigger TriggerType) const
{
    float TotalInfluence = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (const FNPC_EmotionalMemory& Memory : EmotionalMemories)
    {
        if (Memory.TriggerType == TriggerType)
        {
            float Distance = FVector::Dist(Location, Memory.Location);
            float TimeFactor = FMath::Exp(-(CurrentTime - Memory.Timestamp) / 300.0f); // 5 minute decay
            float DistanceFactor = FMath::Exp(-Distance / 1000.0f); // 10 meter influence radius
            
            TotalInfluence += Memory.EmotionalWeight * TimeFactor * DistanceFactor;
        }
    }

    return FMath::Clamp(TotalInfluence, 0.0f, 1.0f);
}

bool UNPCEmotionalSystem::ShouldAvoidLocation(FVector Location) const
{
    float ThreatInfluence = GetEmotionalInfluenceAtLocation(Location, ENPC_EmotionalTrigger::DinosaurSighting);
    float GroupThreatInfluence = GetEmotionalInfluenceAtLocation(Location, ENPC_EmotionalTrigger::GroupThreat);
    
    float TotalThreat = ThreatInfluence + GroupThreatInfluence;
    float AvoidanceThreshold = 1.0f - Bravery; // Braver NPCs have higher threshold
    
    return TotalThreat > AvoidanceThreshold;
}

float UNPCEmotionalSystem::GetCurrentStressLevel() const
{
    float StressFromState = 0.0f;
    
    switch (CurrentEmotionalState)
    {
        case ENPC_EmotionalState::Calm:
            StressFromState = 0.0f;
            break;
        case ENPC_EmotionalState::Cautious:
            StressFromState = 0.3f;
            break;
        case ENPC_EmotionalState::Fearful:
            StressFromState = 0.7f;
            break;
        case ENPC_EmotionalState::Panicked:
            StressFromState = 1.0f;
            break;
        case ENPC_EmotionalState::Aggressive:
            StressFromState = 0.6f;
            break;
        case ENPC_EmotionalState::Territorial:
            StressFromState = 0.5f;
            break;
        case ENPC_EmotionalState::Curious:
            StressFromState = 0.2f;
            break;
        case ENPC_EmotionalState::Protective:
            StressFromState = 0.4f;
            break;
    }
    
    return FMath::Clamp(BaselineStress + (StressFromState * CurrentIntensity), 0.0f, 1.0f);
}

void UNPCEmotionalSystem::ModifyPersonalityTrait(const FString& TraitName, float Value)
{
    if (TraitName == "Bravery")
    {
        Bravery = FMath::Clamp(Value, 0.0f, 1.0f);
    }
    else if (TraitName == "Aggression")
    {
        Aggression = FMath::Clamp(Value, 0.0f, 1.0f);
    }
    else if (TraitName == "Curiosity")
    {
        Curiosity = FMath::Clamp(Value, 0.0f, 1.0f);
    }
    else if (TraitName == "SocialBond")
    {
        SocialBond = FMath::Clamp(Value, 0.0f, 1.0f);
    }
}

FString UNPCEmotionalSystem::GetEmotionalStateDescription() const
{
    FString StateString = UEnum::GetValueAsString(CurrentEmotionalState);
    return FString::Printf(TEXT("%s (Intensity: %.2f, Stress: %.2f)"), 
                          *StateString, CurrentIntensity, GetCurrentStressLevel());
}

void UNPCEmotionalSystem::UpdateEmotionalState(float DeltaTime)
{
    EmotionalStateTimer -= DeltaTime;
    
    // Return to calm state if timer expires
    if (EmotionalStateTimer <= 0.0f && CurrentEmotionalState != ENPC_EmotionalState::Calm)
    {
        CurrentEmotionalState = ENPC_EmotionalState::Calm;
        EmotionalStateTimer = 0.0f;
    }
}

void UNPCEmotionalSystem::ProcessEmotionalDecay(float DeltaTime)
{
    // Gradually reduce emotional intensity
    if (CurrentIntensity > BaselineStress)
    {
        float DecayRate = 0.05f; // Base decay rate
        
        // Faster decay for calm state
        if (CurrentEmotionalState == ENPC_EmotionalState::Calm)
        {
            DecayRate = 0.1f;
        }
        
        CurrentIntensity = FMath::Max(BaselineStress, CurrentIntensity - (DecayRate * DeltaTime));
    }
}

void UNPCEmotionalSystem::CleanOldMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float MemoryLifetime = 600.0f; // 10 minutes
    
    EmotionalMemories.RemoveAll([CurrentTime, MemoryLifetime](const FNPC_EmotionalMemory& Memory)
    {
        return (CurrentTime - Memory.Timestamp) > MemoryLifetime;
    });
}

float UNPCEmotionalSystem::CalculatePersonalityInfluence(ENPC_EmotionalTrigger Trigger) const
{
    float Multiplier = 1.0f;
    
    switch (Trigger)
    {
        case ENPC_EmotionalTrigger::PlayerApproach:
            Multiplier = 1.0f + (Curiosity * 0.5f) - (Bravery * 0.3f);
            break;
        case ENPC_EmotionalTrigger::DinosaurSighting:
            Multiplier = 1.0f + (1.0f - Bravery) * 0.8f;
            break;
        case ENPC_EmotionalTrigger::LoudNoise:
            Multiplier = 1.0f + (1.0f - Bravery) * 0.4f;
            break;
        case ENPC_EmotionalTrigger::GroupThreat:
            Multiplier = 1.0f + (SocialBond * 0.6f) + (1.0f - Bravery) * 0.4f;
            break;
        case ENPC_EmotionalTrigger::ResourceScarcity:
            Multiplier = 1.0f + (Aggression * 0.3f);
            break;
        default:
            Multiplier = 1.0f;
            break;
    }
    
    return FMath::Clamp(Multiplier, 0.1f, 2.0f);
}