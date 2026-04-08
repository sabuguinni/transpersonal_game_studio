#include "NPCMemoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UNPCMemoryComponent::UNPCMemoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
}

void UNPCMemoryComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentGameTime = 0.0f;
}

void UNPCMemoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CurrentGameTime += DeltaTime;
    
    // Decay memories and relationships over time
    DecayMemories(DeltaTime);
    DecayRelationships(DeltaTime);
    
    // Clean up old memories periodically
    if (FMath::RandRange(0.0f, 1.0f) < 0.1f) // 10% chance per tick
    {
        CleanupOldMemories();
    }
}

void UNPCMemoryComponent::AddMemory(const FMemoryEntry& NewMemory)
{
    FMemoryEntry Memory = NewMemory;
    Memory.Timestamp = CurrentGameTime;
    
    // Check if we already have too many memories
    if (Memories.Num() >= MaxMemories)
    {
        // Remove least important memory
        int32 LeastImportantIndex = 0;
        float LowestImportance = CalculateMemoryImportance(Memories[0]);
        
        for (int32 i = 1; i < Memories.Num(); i++)
        {
            float Importance = CalculateMemoryImportance(Memories[i]);
            if (Importance < LowestImportance)
            {
                LowestImportance = Importance;
                LeastImportantIndex = i;
            }
        }
        
        Memories.RemoveAt(LeastImportantIndex);
    }
    
    Memories.Add(Memory);
}

TArray<FMemoryEntry> UNPCMemoryComponent::GetMemoriesAtLocation(FVector Location, float Radius)
{
    TArray<FMemoryEntry> NearbyMemories;
    
    for (const FMemoryEntry& Memory : Memories)
    {
        if (FVector::Dist(Memory.Location, Location) <= Radius)
        {
            NearbyMemories.Add(Memory);
        }
    }
    
    return NearbyMemories;
}

TArray<FMemoryEntry> UNPCMemoryComponent::GetMemoriesByTag(FGameplayTag EventTag)
{
    TArray<FMemoryEntry> TaggedMemories;
    
    for (const FMemoryEntry& Memory : Memories)
    {
        if (Memory.EventType.MatchesTag(EventTag))
        {
            TaggedMemories.Add(Memory);
        }
    }
    
    return TaggedMemories;
}

TArray<FMemoryEntry> UNPCMemoryComponent::GetMemoriesWithActor(AActor* Actor)
{
    TArray<FMemoryEntry> ActorMemories;
    
    if (!Actor)
        return ActorMemories;
    
    for (const FMemoryEntry& Memory : Memories)
    {
        if (Memory.AssociatedActor.IsValid() && Memory.AssociatedActor.Get() == Actor)
        {
            ActorMemories.Add(Memory);
        }
    }
    
    return ActorMemories;
}

void UNPCMemoryComponent::ForgetOldMemories(float MaxAge)
{
    float CutoffTime = CurrentGameTime - MaxAge;
    
    Memories.RemoveAll([CutoffTime, this](const FMemoryEntry& Memory)
    {
        // Keep important memories longer
        if (FMath::Abs(Memory.EmotionalWeight) >= ImportantMemoryThreshold)
        {
            return false;
        }
        
        return Memory.Timestamp < CutoffTime;
    });
}

void UNPCMemoryComponent::UpdateRelationship(AActor* TargetActor, float TrustChange, float FamiliarityChange)
{
    if (!TargetActor)
        return;
    
    FRelationshipData* Relationship = FindOrCreateRelationship(TargetActor);
    
    if (Relationship)
    {
        // Update trust level
        int32 CurrentTrustValue = static_cast<int32>(Relationship->TrustLevel);
        CurrentTrustValue = FMath::Clamp(CurrentTrustValue + FMath::RoundToInt(TrustChange), 
                                       0, static_cast<int32>(ETrustLevel::Bonded));
        Relationship->TrustLevel = static_cast<ETrustLevel>(CurrentTrustValue);
        
        // Update familiarity
        Relationship->Familiarity = FMath::Clamp(Relationship->Familiarity + FamiliarityChange, 0.0f, 1.0f);
        Relationship->LastInteractionTime = CurrentGameTime;
    }
}

FRelationshipData UNPCMemoryComponent::GetRelationship(AActor* TargetActor)
{
    if (!TargetActor)
        return FRelationshipData();
    
    for (const FRelationshipData& Relationship : Relationships)
    {
        if (Relationship.TargetActor.IsValid() && Relationship.TargetActor.Get() == TargetActor)
        {
            return Relationship;
        }
    }
    
    return FRelationshipData();
}

ETrustLevel UNPCMemoryComponent::GetTrustLevel(AActor* TargetActor)
{
    FRelationshipData Relationship = GetRelationship(TargetActor);
    return Relationship.TrustLevel;
}

bool UNPCMemoryComponent::IsKnownActor(AActor* TargetActor)
{
    if (!TargetActor)
        return false;
    
    for (const FRelationshipData& Relationship : Relationships)
    {
        if (Relationship.TargetActor.IsValid() && Relationship.TargetActor.Get() == TargetActor)
        {
            return Relationship.Familiarity > 0.1f;
        }
    }
    
    return false;
}

void UNPCMemoryComponent::LearnFromExperience(FGameplayTag ExperienceType, bool bPositive, float Intensity)
{
    float Weight = bPositive ? Intensity : -Intensity;
    
    if (ExperienceWeights.Contains(ExperienceType))
    {
        ExperienceWeights[ExperienceType] += Weight * 0.1f; // Learning rate
    }
    else
    {
        ExperienceWeights.Add(ExperienceType, Weight * 0.1f);
    }
    
    // Clamp weights
    ExperienceWeights[ExperienceType] = FMath::Clamp(ExperienceWeights[ExperienceType], -1.0f, 1.0f);
}

float UNPCMemoryComponent::GetExperienceWeight(FGameplayTag ExperienceType)
{
    if (ExperienceWeights.Contains(ExperienceType))
    {
        return ExperienceWeights[ExperienceType];
    }
    
    return 0.0f;
}

bool UNPCMemoryComponent::ShouldAvoidLocation(FVector Location, float Radius)
{
    TArray<FMemoryEntry> NearbyMemories = GetMemoriesAtLocation(Location, Radius);
    
    float DangerScore = 0.0f;
    for (const FMemoryEntry& Memory : NearbyMemories)
    {
        if (Memory.EmotionalWeight < -0.3f) // Negative memories
        {
            DangerScore += FMath::Abs(Memory.EmotionalWeight);
        }
    }
    
    return DangerScore > 0.5f;
}

bool UNPCMemoryComponent::ShouldAvoidActor(AActor* Actor)
{
    if (!Actor)
        return false;
    
    FRelationshipData Relationship = GetRelationship(Actor);
    return Relationship.TrustLevel == ETrustLevel::Hostile || Relationship.TrustLevel == ETrustLevel::Fearful;
}

FVector UNPCMemoryComponent::GetLastKnownLocationOf(AActor* Actor)
{
    if (!Actor)
        return FVector::ZeroVector;
    
    TArray<FMemoryEntry> ActorMemories = GetMemoriesWithActor(Actor);
    
    if (ActorMemories.Num() > 0)
    {
        // Sort by timestamp, most recent first
        ActorMemories.Sort([](const FMemoryEntry& A, const FMemoryEntry& B)
        {
            return A.Timestamp > B.Timestamp;
        });
        
        return ActorMemories[0].Location;
    }
    
    return FVector::ZeroVector;
}

float UNPCMemoryComponent::GetTimeSinceLastSeen(AActor* Actor)
{
    if (!Actor)
        return -1.0f;
    
    TArray<FMemoryEntry> ActorMemories = GetMemoriesWithActor(Actor);
    
    if (ActorMemories.Num() > 0)
    {
        float MostRecentTime = 0.0f;
        for (const FMemoryEntry& Memory : ActorMemories)
        {
            if (Memory.Timestamp > MostRecentTime)
            {
                MostRecentTime = Memory.Timestamp;
            }
        }
        
        return CurrentGameTime - MostRecentTime;
    }
    
    return -1.0f;
}

TArray<FVector> UNPCMemoryComponent::GetDangerousLocations()
{
    TArray<FVector> DangerousLocations;
    
    for (const FMemoryEntry& Memory : Memories)
    {
        if (Memory.EmotionalWeight < -0.5f) // Highly negative memories
        {
            DangerousLocations.AddUnique(Memory.Location);
        }
    }
    
    return DangerousLocations;
}

TArray<FVector> UNPCMemoryComponent::GetSafeLocations()
{
    TArray<FVector> SafeLocations;
    
    for (const FMemoryEntry& Memory : Memories)
    {
        if (Memory.EmotionalWeight > 0.3f) // Positive memories
        {
            SafeLocations.AddUnique(Memory.Location);
        }
    }
    
    return SafeLocations;
}

TArray<FVector> UNPCMemoryComponent::GetFoodLocations()
{
    return GetMemoriesByTag(FGameplayTag::RequestGameplayTag(FName("Memory.Food"))).Array();
}

TArray<FVector> UNPCMemoryComponent::GetWaterLocations()
{
    return GetMemoriesByTag(FGameplayTag::RequestGameplayTag(FName("Memory.Water"))).Array();
}

void UNPCMemoryComponent::DecayMemories(float DeltaTime)
{
    for (FMemoryEntry& Memory : Memories)
    {
        // Decay emotional weight over time
        if (FMath::Abs(Memory.EmotionalWeight) > 0.1f)
        {
            float DecayAmount = MemoryDecayRate * DeltaTime;
            if (Memory.EmotionalWeight > 0)
            {
                Memory.EmotionalWeight = FMath::Max(0.0f, Memory.EmotionalWeight - DecayAmount);
            }
            else
            {
                Memory.EmotionalWeight = FMath::Min(0.0f, Memory.EmotionalWeight + DecayAmount);
            }
        }
    }
}

void UNPCMemoryComponent::DecayRelationships(float DeltaTime)
{
    for (FRelationshipData& Relationship : Relationships)
    {
        float TimeSinceInteraction = CurrentGameTime - Relationship.LastInteractionTime;
        
        if (TimeSinceInteraction > 3600.0f) // 1 hour
        {
            // Decay familiarity over time
            Relationship.Familiarity = FMath::Max(0.0f, Relationship.Familiarity - RelationshipDecayRate * DeltaTime);
            
            // Reset trust towards neutral if no recent interactions
            if (TimeSinceInteraction > 86400.0f) // 24 hours
            {
                int32 CurrentTrust = static_cast<int32>(Relationship.TrustLevel);
                int32 NeutralTrust = static_cast<int32>(ETrustLevel::Neutral);
                
                if (CurrentTrust != NeutralTrust)
                {
                    int32 Direction = (CurrentTrust > NeutralTrust) ? -1 : 1;
                    CurrentTrust += Direction;
                    Relationship.TrustLevel = static_cast<ETrustLevel>(CurrentTrust);
                }
            }
        }
    }
    
    // Remove very old relationships
    Relationships.RemoveAll([this](const FRelationshipData& Relationship)
    {
        return (CurrentGameTime - Relationship.LastInteractionTime) > MaxRelationshipAge;
    });
}

FRelationshipData* UNPCMemoryComponent::FindOrCreateRelationship(AActor* TargetActor)
{
    if (!TargetActor)
        return nullptr;
    
    // Find existing relationship
    for (FRelationshipData& Relationship : Relationships)
    {
        if (Relationship.TargetActor.IsValid() && Relationship.TargetActor.Get() == TargetActor)
        {
            return &Relationship;
        }
    }
    
    // Create new relationship
    FRelationshipData NewRelationship;
    NewRelationship.TargetActor = TargetActor;
    NewRelationship.LastInteractionTime = CurrentGameTime;
    
    Relationships.Add(NewRelationship);
    return &Relationships.Last();
}

void UNPCMemoryComponent::CleanupOldMemories()
{
    // Remove memories with invalid actors
    Memories.RemoveAll([](const FMemoryEntry& Memory)
    {
        return Memory.AssociatedActor.IsValid() && !Memory.AssociatedActor.Get();
    });
    
    // Remove relationships with invalid actors
    Relationships.RemoveAll([](const FRelationshipData& Relationship)
    {
        return Relationship.TargetActor.IsValid() && !Relationship.TargetActor.Get();
    });
}

float UNPCMemoryComponent::CalculateMemoryImportance(const FMemoryEntry& Memory)
{
    float Importance = FMath::Abs(Memory.EmotionalWeight);
    
    // Recent memories are more important
    float Age = CurrentGameTime - Memory.Timestamp;
    float RecencyFactor = FMath::Exp(-Age / 3600.0f); // Exponential decay over hours
    
    return Importance * RecencyFactor;
}