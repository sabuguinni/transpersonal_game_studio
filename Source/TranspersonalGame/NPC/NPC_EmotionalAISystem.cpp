#include "NPC_EmotionalAISystem.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

UNPC_EmotionalAISystem::UNPC_EmotionalAISystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize default values
    CurrentEmotionalState = ENPC_EmotionalState::Calm;
    CurrentIntensity = ENPC_EmotionIntensity::Medium;
    EmotionalDecayRate = 1.0f;
    EmotionalSensitivity = 1.0f;
    MaxEmotionalMemories = 50.0f;
    EmotionalUpdateFrequency = 0.5f;
    LastEmotionalUpdate = 0.0f;
    EmotionalStateTimer = 0.0f;

    // Initialize personality traits with random variation
    PersonalityTraits = FNPC_PersonalityTraits();
}

void UNPC_EmotionalAISystem::BeginPlay()
{
    Super::BeginPlay();

    // Initialize emotional state weights
    EmotionalStateWeights.Add(ENPC_EmotionalState::Calm, 1.0f);
    EmotionalStateWeights.Add(ENPC_EmotionalState::Aggressive, 0.0f);
    EmotionalStateWeights.Add(ENPC_EmotionalState::Fearful, 0.0f);
    EmotionalStateWeights.Add(ENPC_EmotionalState::Curious, 0.3f);
    EmotionalStateWeights.Add(ENPC_EmotionalState::Protective, 0.0f);
    EmotionalStateWeights.Add(ENPC_EmotionalState::Hungry, 0.2f);
    EmotionalStateWeights.Add(ENPC_EmotionalState::Territorial, 0.1f);
    EmotionalStateWeights.Add(ENPC_EmotionalState::Social, 0.0f);
    EmotionalStateWeights.Add(ENPC_EmotionalState::Stressed, 0.0f);
    EmotionalStateWeights.Add(ENPC_EmotionalState::Content, 0.5f);

    // Randomize personality slightly for variation
    RandomizePersonality();
}

void UNPC_EmotionalAISystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateEmotionalState(DeltaTime);
    ProcessEmotionalDecay(DeltaTime);
    CleanupOldMemories();
}

void UNPC_EmotionalAISystem::TriggerEmotionalResponse(ENPC_EmotionTrigger Trigger, ENPC_EmotionIntensity Intensity, FVector Location)
{
    CalculateEmotionalResponse(Trigger, Intensity);
    
    // Add to emotional memory
    ENPC_EmotionalState TriggeredState = DetermineNewEmotionalState(Trigger);
    AddEmotionalMemory(TriggeredState, Intensity, Location, false);

    UE_LOG(LogTemp, Log, TEXT("Emotional response triggered: %s with intensity %s"), 
           *UEnum::GetValueAsString(TriggeredState), 
           *UEnum::GetValueAsString(Intensity));
}

void UNPC_EmotionalAISystem::SetEmotionalState(ENPC_EmotionalState NewState, ENPC_EmotionIntensity Intensity)
{
    CurrentEmotionalState = NewState;
    CurrentIntensity = Intensity;
    EmotionalStateTimer = 0.0f;

    // Update emotional weights
    for (auto& Weight : EmotionalStateWeights)
    {
        if (Weight.Key == NewState)
        {
            Weight.Value = FMath::Clamp(Weight.Value + GetEmotionalIntensityValue(), 0.0f, 2.0f);
        }
        else
        {
            Weight.Value = FMath::Max(0.0f, Weight.Value - 0.1f);
        }
    }
}

ENPC_EmotionalState UNPC_EmotionalAISystem::GetDominantEmotionalState() const
{
    ENPC_EmotionalState DominantState = ENPC_EmotionalState::Calm;
    float HighestWeight = 0.0f;

    for (const auto& Weight : EmotionalStateWeights)
    {
        if (Weight.Value > HighestWeight)
        {
            HighestWeight = Weight.Value;
            DominantState = Weight.Key;
        }
    }

    return DominantState;
}

float UNPC_EmotionalAISystem::GetEmotionalIntensityValue() const
{
    switch (CurrentIntensity)
    {
        case ENPC_EmotionIntensity::VeryLow: return 0.1f;
        case ENPC_EmotionIntensity::Low: return 0.3f;
        case ENPC_EmotionIntensity::Medium: return 0.5f;
        case ENPC_EmotionIntensity::High: return 0.7f;
        case ENPC_EmotionIntensity::VeryHigh: return 0.9f;
        case ENPC_EmotionIntensity::Extreme: return 1.0f;
        default: return 0.5f;
    }
}

bool UNPC_EmotionalAISystem::IsInEmotionalState(ENPC_EmotionalState State) const
{
    return CurrentEmotionalState == State || GetDominantEmotionalState() == State;
}

void UNPC_EmotionalAISystem::AddEmotionalMemory(ENPC_EmotionalState State, ENPC_EmotionIntensity Intensity, FVector Location, bool bPersistent)
{
    if (EmotionalMemories.Num() >= MaxEmotionalMemories)
    {
        // Remove oldest non-persistent memory
        for (int32 i = 0; i < EmotionalMemories.Num(); ++i)
        {
            if (!EmotionalMemories[i].bIsPersistent)
            {
                EmotionalMemories.RemoveAt(i);
                break;
            }
        }
    }

    FNPC_EmotionalMemory NewMemory;
    NewMemory.EmotionalState = State;
    NewMemory.Intensity = Intensity;
    NewMemory.Location = Location;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    NewMemory.DecayRate = EmotionalDecayRate;
    NewMemory.bIsPersistent = bPersistent;

    EmotionalMemories.Add(NewMemory);
}

void UNPC_EmotionalAISystem::ClearEmotionalMemories()
{
    EmotionalMemories.Empty();
}

TArray<FNPC_EmotionalMemory> UNPC_EmotionalAISystem::GetEmotionalMemoriesAtLocation(FVector Location, float Radius) const
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

float UNPC_EmotionalAISystem::GetPersonalityInfluence(ENPC_EmotionalState State) const
{
    switch (State)
    {
        case ENPC_EmotionalState::Aggressive: return PersonalityTraits.Aggressiveness;
        case ENPC_EmotionalState::Fearful: return PersonalityTraits.Fearfulness;
        case ENPC_EmotionalState::Curious: return PersonalityTraits.Curiosity;
        case ENPC_EmotionalState::Social: return PersonalityTraits.Sociability;
        case ENPC_EmotionalState::Territorial: return PersonalityTraits.Territoriality;
        case ENPC_EmotionalState::Protective: return PersonalityTraits.Protectiveness;
        default: return 0.5f;
    }
}

void UNPC_EmotionalAISystem::RandomizePersonality()
{
    PersonalityTraits.Aggressiveness = FMath::FRandRange(0.2f, 0.8f);
    PersonalityTraits.Fearfulness = FMath::FRandRange(0.2f, 0.8f);
    PersonalityTraits.Curiosity = FMath::FRandRange(0.3f, 0.9f);
    PersonalityTraits.Sociability = FMath::FRandRange(0.2f, 0.8f);
    PersonalityTraits.Territoriality = FMath::FRandRange(0.1f, 0.7f);
    PersonalityTraits.Protectiveness = FMath::FRandRange(0.3f, 0.9f);
    PersonalityTraits.Adaptability = FMath::FRandRange(0.4f, 0.8f);
    PersonalityTraits.Intelligence = FMath::FRandRange(0.3f, 0.9f);
}

void UNPC_EmotionalAISystem::SetPersonalityPreset(const FString& PresetName)
{
    if (PresetName == "Aggressive")
    {
        PersonalityTraits.Aggressiveness = 0.9f;
        PersonalityTraits.Fearfulness = 0.2f;
        PersonalityTraits.Territoriality = 0.8f;
    }
    else if (PresetName == "Timid")
    {
        PersonalityTraits.Aggressiveness = 0.1f;
        PersonalityTraits.Fearfulness = 0.9f;
        PersonalityTraits.Curiosity = 0.3f;
    }
    else if (PresetName == "Curious")
    {
        PersonalityTraits.Curiosity = 0.9f;
        PersonalityTraits.Intelligence = 0.8f;
        PersonalityTraits.Fearfulness = 0.3f;
    }
    else if (PresetName == "Social")
    {
        PersonalityTraits.Sociability = 0.9f;
        PersonalityTraits.Protectiveness = 0.8f;
        PersonalityTraits.Aggressiveness = 0.3f;
    }
}

float UNPC_EmotionalAISystem::GetMovementSpeedModifier() const
{
    float BaseModifier = 1.0f;
    
    switch (CurrentEmotionalState)
    {
        case ENPC_EmotionalState::Fearful:
            return BaseModifier * (1.5f + GetEmotionalIntensityValue());
        case ENPC_EmotionalState::Aggressive:
            return BaseModifier * (1.3f + GetEmotionalIntensityValue() * 0.5f);
        case ENPC_EmotionalState::Stressed:
            return BaseModifier * (1.2f + GetEmotionalIntensityValue() * 0.3f);
        case ENPC_EmotionalState::Content:
            return BaseModifier * (0.8f - GetEmotionalIntensityValue() * 0.2f);
        case ENPC_EmotionalState::Curious:
            return BaseModifier * (1.1f + GetEmotionalIntensityValue() * 0.2f);
        default:
            return BaseModifier;
    }
}

float UNPC_EmotionalAISystem::GetAggressionModifier() const
{
    float BaseAggression = PersonalityTraits.Aggressiveness;
    
    switch (CurrentEmotionalState)
    {
        case ENPC_EmotionalState::Aggressive:
            return BaseAggression * (2.0f + GetEmotionalIntensityValue());
        case ENPC_EmotionalState::Territorial:
            return BaseAggression * (1.5f + GetEmotionalIntensityValue() * 0.8f);
        case ENPC_EmotionalState::Protective:
            return BaseAggression * (1.8f + GetEmotionalIntensityValue() * 0.6f);
        case ENPC_EmotionalState::Fearful:
            return BaseAggression * (0.3f - GetEmotionalIntensityValue() * 0.2f);
        case ENPC_EmotionalState::Content:
            return BaseAggression * 0.5f;
        default:
            return BaseAggression;
    }
}

float UNPC_EmotionalAISystem::GetDetectionRangeModifier() const
{
    float BaseModifier = 1.0f;
    
    switch (CurrentEmotionalState)
    {
        case ENPC_EmotionalState::Fearful:
            return BaseModifier * (1.8f + GetEmotionalIntensityValue());
        case ENPC_EmotionalState::Curious:
            return BaseModifier * (1.4f + GetEmotionalIntensityValue() * 0.6f);
        case ENPC_EmotionalState::Stressed:
            return BaseModifier * (1.3f + GetEmotionalIntensityValue() * 0.4f);
        case ENPC_EmotionalState::Content:
            return BaseModifier * (0.7f - GetEmotionalIntensityValue() * 0.2f);
        default:
            return BaseModifier;
    }
}

bool UNPC_EmotionalAISystem::ShouldFleeFromThreat() const
{
    float FleeThreshold = PersonalityTraits.Fearfulness * GetEmotionalIntensityValue();
    
    if (CurrentEmotionalState == ENPC_EmotionalState::Fearful)
    {
        return FleeThreshold > 0.4f;
    }
    else if (CurrentEmotionalState == ENPC_EmotionalState::Aggressive)
    {
        return FleeThreshold > 0.8f;
    }
    
    return FleeThreshold > 0.6f;
}

bool UNPC_EmotionalAISystem::ShouldInvestigateSound() const
{
    float InvestigateThreshold = PersonalityTraits.Curiosity * GetEmotionalIntensityValue();
    
    if (CurrentEmotionalState == ENPC_EmotionalState::Curious)
    {
        return InvestigateThreshold > 0.3f;
    }
    else if (CurrentEmotionalState == ENPC_EmotionalState::Fearful)
    {
        return InvestigateThreshold > 0.8f;
    }
    
    return InvestigateThreshold > 0.5f;
}

void UNPC_EmotionalAISystem::UpdateEmotionalState(float DeltaTime)
{
    EmotionalStateTimer += DeltaTime;
    LastEmotionalUpdate += DeltaTime;

    if (LastEmotionalUpdate >= EmotionalUpdateFrequency)
    {
        LastEmotionalUpdate = 0.0f;
        
        // Natural emotional state transitions
        ENPC_EmotionalState NewDominantState = GetDominantEmotionalState();
        if (NewDominantState != CurrentEmotionalState)
        {
            CurrentEmotionalState = NewDominantState;
            EmotionalStateTimer = 0.0f;
        }
    }
}

void UNPC_EmotionalAISystem::ProcessEmotionalDecay(float DeltaTime)
{
    // Decay emotional state weights over time
    for (auto& Weight : EmotionalStateWeights)
    {
        if (Weight.Key != ENPC_EmotionalState::Calm)
        {
            Weight.Value = FMath::Max(0.0f, Weight.Value - EmotionalDecayRate * DeltaTime);
        }
    }

    // Increase calm state as others decay
    if (EmotionalStateWeights.Contains(ENPC_EmotionalState::Calm))
    {
        float* CalmWeight = EmotionalStateWeights.Find(ENPC_EmotionalState::Calm);
        if (CalmWeight)
        {
            *CalmWeight = FMath::Min(2.0f, *CalmWeight + EmotionalDecayRate * DeltaTime * 0.5f);
        }
    }
}

void UNPC_EmotionalAISystem::CalculateEmotionalResponse(ENPC_EmotionTrigger Trigger, ENPC_EmotionIntensity Intensity)
{
    ENPC_EmotionalState TriggeredState = DetermineNewEmotionalState(Trigger);
    float PersonalityModifier = GetPersonalityInfluence(TriggeredState);
    float IntensityValue = GetEmotionalIntensityValue();
    
    // Calculate emotional weight increase
    float WeightIncrease = (IntensityValue * PersonalityModifier * EmotionalSensitivity);
    
    if (EmotionalStateWeights.Contains(TriggeredState))
    {
        float* StateWeight = EmotionalStateWeights.Find(TriggeredState);
        if (StateWeight)
        {
            *StateWeight = FMath::Clamp(*StateWeight + WeightIncrease, 0.0f, 2.0f);
        }
    }
}

ENPC_EmotionalState UNPC_EmotionalAISystem::DetermineNewEmotionalState(ENPC_EmotionTrigger Trigger) const
{
    switch (Trigger)
    {
        case ENPC_EmotionTrigger::PlayerPresence:
            return PersonalityTraits.Fearfulness > 0.6f ? ENPC_EmotionalState::Fearful : ENPC_EmotionalState::Curious;
        case ENPC_EmotionTrigger::LoudNoise:
            return ENPC_EmotionalState::Fearful;
        case ENPC_EmotionTrigger::PredatorSight:
            return ENPC_EmotionalState::Fearful;
        case ENPC_EmotionTrigger::FoodSource:
            return ENPC_EmotionalState::Content;
        case ENPC_EmotionTrigger::PackMemberDanger:
            return ENPC_EmotionalState::Protective;
        case ENPC_EmotionTrigger::TerritoryInvasion:
            return ENPC_EmotionalState::Territorial;
        case ENPC_EmotionTrigger::WeatherChange:
            return ENPC_EmotionalState::Stressed;
        case ENPC_EmotionTrigger::TimeOfDay:
            return ENPC_EmotionalState::Calm;
        case ENPC_EmotionTrigger::HealthStatus:
            return ENPC_EmotionalState::Stressed;
        case ENPC_EmotionTrigger::SocialInteraction:
            return ENPC_EmotionalState::Social;
        default:
            return ENPC_EmotionalState::Calm;
    }
}

void UNPC_EmotionalAISystem::CleanupOldMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    EmotionalMemories.RemoveAll([CurrentTime](const FNPC_EmotionalMemory& Memory)
    {
        if (Memory.bIsPersistent) return false;
        
        float MemoryAge = CurrentTime - Memory.Timestamp;
        float MaxAge = 300.0f / Memory.DecayRate; // 5 minutes base, modified by decay rate
        
        return MemoryAge > MaxAge;
    });
}