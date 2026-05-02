#include "NPCMemorySystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPC_MemorySystem::UNPC_MemorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    MaxMemories = 50;
    MemoryDecayRate = 0.1f;
    EmotionalDecayRate = 0.05f;
    
    // Initialize emotional state
    CurrentEmotionalState = FNPC_EmotionalState();
}

void UNPC_MemorySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Add initial memories based on NPC role
    AActor* Owner = GetOwner();
    if (Owner)
    {
        // Check if this is a specific type of NPC based on tags
        if (Owner->Tags.Contains("Hunter"))
        {
            CurrentEmotionalState.Fear = 0.2f;
            CurrentEmotionalState.Aggression = 0.6f;
            CurrentEmotionalState.Curiosity = 0.4f;
        }
        else if (Owner->Tags.Contains("Gatherer"))
        {
            CurrentEmotionalState.Fear = 0.5f;
            CurrentEmotionalState.Curiosity = 0.7f;
            CurrentEmotionalState.Trust = 0.3f;
        }
        else if (Owner->Tags.Contains("Elder"))
        {
            CurrentEmotionalState.Fear = 0.1f;
            CurrentEmotionalState.Trust = 0.8f;
            CurrentEmotionalState.Curiosity = 0.3f;
        }
    }
}

void UNPC_MemorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateEmotionalState(DeltaTime);
    CleanupOldMemories();
}

void UNPC_MemorySystem::AddMemory(ENPC_MemoryType Type, FVector Location, float Importance, const FString& Description)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.MemoryType = Type;
    NewMemory.Location = Location;
    NewMemory.Importance = FMath::Clamp(Importance, 0.0f, 1.0f);
    NewMemory.Description = Description;
    NewMemory.Timestamp = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Calculate emotional impact based on memory type
    switch (Type)
    {
        case ENPC_MemoryType::DinosaurEncounter:
            NewMemory.EmotionalImpact = 0.8f;
            ModifyEmotion(TEXT("Fear"), 0.3f);
            break;
        case ENPC_MemoryType::PlayerSighting:
            NewMemory.EmotionalImpact = 0.5f;
            ModifyEmotion(TEXT("Curiosity"), 0.2f);
            break;
        case ENPC_MemoryType::ResourceLocation:
            NewMemory.EmotionalImpact = 0.3f;
            ModifyEmotion(TEXT("Hunger"), -0.1f);
            break;
        case ENPC_MemoryType::DangerZone:
            NewMemory.EmotionalImpact = 0.7f;
            ModifyEmotion(TEXT("Fear"), 0.2f);
            break;
        case ENPC_MemoryType::SafeArea:
            NewMemory.EmotionalImpact = -0.3f;
            ModifyEmotion(TEXT("Fear"), -0.1f);
            break;
        default:
            NewMemory.EmotionalImpact = 0.1f;
            break;
    }
    
    Memories.Add(NewMemory);
    
    // Remove oldest memories if we exceed the limit
    if (Memories.Num() > MaxMemories)
    {
        Memories.RemoveAt(0);
    }
}

TArray<FNPC_MemoryEntry> UNPC_MemorySystem::GetMemoriesByType(ENPC_MemoryType Type)
{
    TArray<FNPC_MemoryEntry> FilteredMemories;
    
    for (const FNPC_MemoryEntry& Memory : Memories)
    {
        if (Memory.MemoryType == Type)
        {
            FilteredMemories.Add(Memory);
        }
    }
    
    return FilteredMemories;
}

TArray<FNPC_MemoryEntry> UNPC_MemorySystem::GetMemoriesNearLocation(FVector Location, float Radius)
{
    TArray<FNPC_MemoryEntry> NearbyMemories;
    
    for (const FNPC_MemoryEntry& Memory : Memories)
    {
        float Distance = FVector::Dist(Memory.Location, Location);
        if (Distance <= Radius)
        {
            NearbyMemories.Add(Memory);
        }
    }
    
    return NearbyMemories;
}

void UNPC_MemorySystem::ForgetOldMemories(float MaxAge)
{
    if (!GetWorld()) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - Memories[i].Timestamp > MaxAge)
        {
            Memories.RemoveAt(i);
        }
    }
}

bool UNPC_MemorySystem::HasMemoryOfLocation(FVector Location, float Radius, ENPC_MemoryType Type)
{
    TArray<FNPC_MemoryEntry> NearbyMemories = GetMemoriesNearLocation(Location, Radius);
    
    for (const FNPC_MemoryEntry& Memory : NearbyMemories)
    {
        if (Memory.MemoryType == Type)
        {
            return true;
        }
    }
    
    return false;
}

void UNPC_MemorySystem::UpdateEmotionalState(float DeltaTime)
{
    DecayEmotions(DeltaTime);
    
    // Clamp all emotions to valid ranges
    CurrentEmotionalState.Fear = FMath::Clamp(CurrentEmotionalState.Fear, 0.0f, 1.0f);
    CurrentEmotionalState.Curiosity = FMath::Clamp(CurrentEmotionalState.Curiosity, 0.0f, 1.0f);
    CurrentEmotionalState.Aggression = FMath::Clamp(CurrentEmotionalState.Aggression, 0.0f, 1.0f);
    CurrentEmotionalState.Trust = FMath::Clamp(CurrentEmotionalState.Trust, 0.0f, 1.0f);
    CurrentEmotionalState.Hunger = FMath::Clamp(CurrentEmotionalState.Hunger, 0.0f, 1.0f);
    CurrentEmotionalState.Fatigue = FMath::Clamp(CurrentEmotionalState.Fatigue, 0.0f, 1.0f);
}

void UNPC_MemorySystem::ModifyEmotion(const FString& EmotionName, float Delta)
{
    if (EmotionName == TEXT("Fear"))
    {
        CurrentEmotionalState.Fear += Delta;
    }
    else if (EmotionName == TEXT("Curiosity"))
    {
        CurrentEmotionalState.Curiosity += Delta;
    }
    else if (EmotionName == TEXT("Aggression"))
    {
        CurrentEmotionalState.Aggression += Delta;
    }
    else if (EmotionName == TEXT("Trust"))
    {
        CurrentEmotionalState.Trust += Delta;
    }
    else if (EmotionName == TEXT("Hunger"))
    {
        CurrentEmotionalState.Hunger += Delta;
    }
    else if (EmotionName == TEXT("Fatigue"))
    {
        CurrentEmotionalState.Fatigue += Delta;
    }
}

float UNPC_MemorySystem::GetEmotionValue(const FString& EmotionName)
{
    if (EmotionName == TEXT("Fear"))
    {
        return CurrentEmotionalState.Fear;
    }
    else if (EmotionName == TEXT("Curiosity"))
    {
        return CurrentEmotionalState.Curiosity;
    }
    else if (EmotionName == TEXT("Aggression"))
    {
        return CurrentEmotionalState.Aggression;
    }
    else if (EmotionName == TEXT("Trust"))
    {
        return CurrentEmotionalState.Trust;
    }
    else if (EmotionName == TEXT("Hunger"))
    {
        return CurrentEmotionalState.Hunger;
    }
    else if (EmotionName == TEXT("Fatigue"))
    {
        return CurrentEmotionalState.Fatigue;
    }
    
    return 0.0f;
}

bool UNPC_MemorySystem::ShouldFleeFromLocation(FVector Location)
{
    // Check for danger memories near this location
    TArray<FNPC_MemoryEntry> DangerMemories = GetMemoriesNearLocation(Location, 1000.0f);
    
    float DangerScore = 0.0f;
    for (const FNPC_MemoryEntry& Memory : DangerMemories)
    {
        if (Memory.MemoryType == ENPC_MemoryType::DinosaurEncounter || 
            Memory.MemoryType == ENPC_MemoryType::DangerZone)
        {
            DangerScore += Memory.Importance * Memory.EmotionalImpact;
        }
    }
    
    // Factor in current fear level
    float FleeThreshold = 0.3f - (CurrentEmotionalState.Fear * 0.2f);
    
    return DangerScore > FleeThreshold;
}

bool UNPC_MemorySystem::ShouldInvestigateLocation(FVector Location)
{
    // High fear reduces investigation desire
    if (CurrentEmotionalState.Fear > 0.7f)
    {
        return false;
    }
    
    // High curiosity increases investigation desire
    float InvestigateChance = CurrentEmotionalState.Curiosity * 0.8f;
    
    // Check if we have positive memories of this area
    TArray<FNPC_MemoryEntry> NearbyMemories = GetMemoriesNearLocation(Location, 500.0f);
    for (const FNPC_MemoryEntry& Memory : NearbyMemories)
    {
        if (Memory.MemoryType == ENPC_MemoryType::ResourceLocation ||
            Memory.MemoryType == ENPC_MemoryType::SafeArea)
        {
            InvestigateChance += 0.2f;
        }
    }
    
    return InvestigateChance > 0.5f;
}

FVector UNPC_MemorySystem::GetSafestKnownLocation()
{
    FVector SafestLocation = FVector::ZeroVector;
    float HighestSafetyScore = -1.0f;
    
    for (const FNPC_MemoryEntry& Memory : Memories)
    {
        if (Memory.MemoryType == ENPC_MemoryType::SafeArea)
        {
            float SafetyScore = Memory.Importance - Memory.EmotionalImpact;
            if (SafetyScore > HighestSafetyScore)
            {
                HighestSafetyScore = SafetyScore;
                SafestLocation = Memory.Location;
            }
        }
    }
    
    // If no safe location in memory, return owner's current location
    if (HighestSafetyScore < 0.0f && GetOwner())
    {
        SafestLocation = GetOwner()->GetActorLocation();
    }
    
    return SafestLocation;
}

void UNPC_MemorySystem::CleanupOldMemories()
{
    if (!GetWorld()) return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove memories older than 10 minutes (600 seconds)
    for (int32 i = Memories.Num() - 1; i >= 0; i--)
    {
        float MemoryAge = CurrentTime - Memories[i].Timestamp;
        
        // Decay memory importance over time
        float DecayedImportance = Memories[i].Importance * FMath::Exp(-MemoryDecayRate * MemoryAge);
        
        if (DecayedImportance < 0.1f || MemoryAge > 600.0f)
        {
            Memories.RemoveAt(i);
        }
        else
        {
            Memories[i].Importance = DecayedImportance;
        }
    }
}

void UNPC_MemorySystem::DecayEmotions(float DeltaTime)
{
    // Gradually return emotions to baseline
    float DecayAmount = EmotionalDecayRate * DeltaTime;
    
    // Fear decays towards baseline of 0.3
    if (CurrentEmotionalState.Fear > 0.3f)
    {
        CurrentEmotionalState.Fear = FMath::Max(0.3f, CurrentEmotionalState.Fear - DecayAmount);
    }
    else if (CurrentEmotionalState.Fear < 0.3f)
    {
        CurrentEmotionalState.Fear = FMath::Min(0.3f, CurrentEmotionalState.Fear + DecayAmount);
    }
    
    // Curiosity decays towards baseline of 0.5
    if (CurrentEmotionalState.Curiosity > 0.5f)
    {
        CurrentEmotionalState.Curiosity = FMath::Max(0.5f, CurrentEmotionalState.Curiosity - DecayAmount);
    }
    else if (CurrentEmotionalState.Curiosity < 0.5f)
    {
        CurrentEmotionalState.Curiosity = FMath::Min(0.5f, CurrentEmotionalState.Curiosity + DecayAmount);
    }
    
    // Hunger increases over time
    CurrentEmotionalState.Hunger = FMath::Min(1.0f, CurrentEmotionalState.Hunger + (DecayAmount * 0.5f));
    
    // Fatigue increases over time
    CurrentEmotionalState.Fatigue = FMath::Min(1.0f, CurrentEmotionalState.Fatigue + (DecayAmount * 0.3f));
}

float UNPC_MemorySystem::CalculateMemoryImportance(const FNPC_MemoryEntry& Memory)
{
    float BaseImportance = Memory.Importance;
    
    // Recent memories are more important
    if (GetWorld())
    {
        float MemoryAge = GetWorld()->GetTimeSeconds() - Memory.Timestamp;
        float RecencyMultiplier = FMath::Exp(-0.001f * MemoryAge);
        BaseImportance *= RecencyMultiplier;
    }
    
    // High emotional impact memories are more important
    BaseImportance *= (1.0f + FMath::Abs(Memory.EmotionalImpact));
    
    return BaseImportance;
}