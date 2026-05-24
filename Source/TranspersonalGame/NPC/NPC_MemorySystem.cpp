#include "NPC_MemorySystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/Engine.h"

UNPC_MemorySystem::UNPC_MemorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update memory every second
    
    // Initialize memory parameters
    MemoryDecayRate = 0.1f;
    MaxMemoryDistance = 5000.0f;
    MaxStoredMemories = 50;
    LearningRate = 1.0f;
}

void UNPC_MemorySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize social memory
    SocialMemory.OverallTrustLevel = 50.0f;
    SocialMemory.SuccessfulHunts = 0;
    SocialMemory.FailedHunts = 0;
    
    UE_LOG(LogTemp, Log, TEXT("NPC Memory System initialized for %s"), *GetOwner()->GetName());
}

void UNPC_MemorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Process memory decay and cleanup
    ProcessMemoryDecay(DeltaTime);
    
    // Update memory locations based on current observations
    UpdateMemoryLocations();
    
    // Clean up old or irrelevant memories
    if (SocialMemory.KnownIndividuals.Num() > MaxStoredMemories)
    {
        CleanupOldMemories();
    }
}

void UNPC_MemorySystem::AddMemoryEntry(AActor* Subject, const FVector& Location, float EmotionalWeight, ENPC_RelationshipType Relationship)
{
    if (!Subject)
    {
        return;
    }
    
    FNPC_MemoryEntry NewMemory;
    NewMemory.Subject = Subject;
    NewMemory.LastKnownLocation = Location;
    NewMemory.EmotionalWeight = EmotionalWeight;
    NewMemory.TimeSinceLastSeen = 0.0f;
    NewMemory.RelationshipType = Relationship;
    
    // Add contextual event
    FString EventDescription = FString::Printf(TEXT("First encountered at %s"), *Location.ToString());
    NewMemory.AssociatedEvents.Add(EventDescription);
    
    SocialMemory.KnownIndividuals.Add(Subject, NewMemory);
    
    UE_LOG(LogTemp, Log, TEXT("Added memory entry for %s with relationship %d"), 
           *Subject->GetName(), (int32)Relationship);
}

bool UNPC_MemorySystem::HasMemoryOf(AActor* Subject) const
{
    return Subject && SocialMemory.KnownIndividuals.Contains(Subject);
}

FNPC_MemoryEntry UNPC_MemorySystem::GetMemoryOf(AActor* Subject) const
{
    if (HasMemoryOf(Subject))
    {
        return SocialMemory.KnownIndividuals[Subject];
    }
    
    return FNPC_MemoryEntry();
}

void UNPC_MemorySystem::UpdateMemoryEntry(AActor* Subject, const FVector& NewLocation)
{
    if (!HasMemoryOf(Subject))
    {
        return;
    }
    
    FNPC_MemoryEntry& Memory = SocialMemory.KnownIndividuals[Subject];
    Memory.LastKnownLocation = NewLocation;
    Memory.TimeSinceLastSeen = 0.0f;
    
    // Add location update event
    FString EventDescription = FString::Printf(TEXT("Seen at %s"), *NewLocation.ToString());
    Memory.AssociatedEvents.Add(EventDescription);
    
    // Limit event history
    if (Memory.AssociatedEvents.Num() > 10)
    {
        Memory.AssociatedEvents.RemoveAt(0);
    }
}

void UNPC_MemorySystem::ForgetActor(AActor* Subject)
{
    if (HasMemoryOf(Subject))
    {
        SocialMemory.KnownIndividuals.Remove(Subject);
        UE_LOG(LogTemp, Log, TEXT("Forgot actor %s"), *Subject->GetName());
    }
}

TArray<AActor*> UNPC_MemorySystem::GetKnownActorsInRange(float Range) const
{
    TArray<AActor*> ActorsInRange;
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (const auto& MemoryPair : SocialMemory.KnownIndividuals)
    {
        const FNPC_MemoryEntry& Memory = MemoryPair.Value;
        float Distance = FVector::Dist(OwnerLocation, Memory.LastKnownLocation);
        
        if (Distance <= Range)
        {
            ActorsInRange.Add(Memory.Subject);
        }
    }
    
    return ActorsInRange;
}

void UNPC_MemorySystem::LearnBehavior(const FString& BehaviorName)
{
    if (!SocialMemory.LearnedBehaviors.Contains(BehaviorName))
    {
        SocialMemory.LearnedBehaviors.Add(BehaviorName);
        UE_LOG(LogTemp, Log, TEXT("Learned new behavior: %s"), *BehaviorName);
    }
}

bool UNPC_MemorySystem::HasLearnedBehavior(const FString& BehaviorName) const
{
    return SocialMemory.LearnedBehaviors.Contains(BehaviorName);
}

void UNPC_MemorySystem::RecordHuntResult(bool bWasSuccessful)
{
    if (bWasSuccessful)
    {
        SocialMemory.SuccessfulHunts++;
        SocialMemory.OverallTrustLevel = FMath::Min(100.0f, SocialMemory.OverallTrustLevel + 2.0f);
    }
    else
    {
        SocialMemory.FailedHunts++;
        SocialMemory.OverallTrustLevel = FMath::Max(0.0f, SocialMemory.OverallTrustLevel - 1.0f);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Hunt result recorded. Success rate: %.2f%%"), GetHuntSuccessRate());
}

float UNPC_MemorySystem::GetHuntSuccessRate() const
{
    int32 TotalHunts = SocialMemory.SuccessfulHunts + SocialMemory.FailedHunts;
    if (TotalHunts == 0)
    {
        return 0.0f;
    }
    
    return (float)SocialMemory.SuccessfulHunts / (float)TotalHunts * 100.0f;
}

void UNPC_MemorySystem::RememberLocation(const FVector& Location)
{
    // Check if location is already remembered (within tolerance)
    for (const FVector& RememberedLocation : SocialMemory.ImportantLocations)
    {
        if (FVector::Dist(Location, RememberedLocation) < 100.0f)
        {
            return; // Already remembered
        }
    }
    
    SocialMemory.ImportantLocations.Add(Location);
    
    // Limit number of remembered locations
    if (SocialMemory.ImportantLocations.Num() > 20)
    {
        SocialMemory.ImportantLocations.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Remembered new location: %s"), *Location.ToString());
}

FVector UNPC_MemorySystem::GetNearestRememberedLocation(const FVector& CurrentLocation) const
{
    if (SocialMemory.ImportantLocations.Num() == 0)
    {
        return CurrentLocation;
    }
    
    FVector NearestLocation = SocialMemory.ImportantLocations[0];
    float NearestDistance = FVector::Dist(CurrentLocation, NearestLocation);
    
    for (const FVector& Location : SocialMemory.ImportantLocations)
    {
        float Distance = FVector::Dist(CurrentLocation, Location);
        if (Distance < NearestDistance)
        {
            NearestDistance = Distance;
            NearestLocation = Location;
        }
    }
    
    return NearestLocation;
}

bool UNPC_MemorySystem::IsLocationFamiliar(const FVector& Location, float Tolerance) const
{
    for (const FVector& RememberedLocation : SocialMemory.ImportantLocations)
    {
        if (FVector::Dist(Location, RememberedLocation) <= Tolerance)
        {
            return true;
        }
    }
    
    return false;
}

void UNPC_MemorySystem::ModifyRelationship(AActor* Subject, float RelationshipChange)
{
    if (!HasMemoryOf(Subject))
    {
        return;
    }
    
    FNPC_MemoryEntry& Memory = SocialMemory.KnownIndividuals[Subject];
    Memory.EmotionalWeight += RelationshipChange;
    Memory.EmotionalWeight = FMath::Clamp(Memory.EmotionalWeight, -100.0f, 100.0f);
    
    // Update relationship type based on emotional weight
    if (Memory.EmotionalWeight > 50.0f)
    {
        Memory.RelationshipType = ENPC_RelationshipType::Ally;
    }
    else if (Memory.EmotionalWeight < -50.0f)
    {
        Memory.RelationshipType = ENPC_RelationshipType::Enemy;
    }
    else
    {
        Memory.RelationshipType = ENPC_RelationshipType::Neutral;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Modified relationship with %s: %.2f"), 
           *Subject->GetName(), Memory.EmotionalWeight);
}

ENPC_RelationshipType UNPC_MemorySystem::GetRelationshipWith(AActor* Subject) const
{
    if (HasMemoryOf(Subject))
    {
        return SocialMemory.KnownIndividuals[Subject].RelationshipType;
    }
    
    return ENPC_RelationshipType::Neutral;
}

TArray<AActor*> UNPC_MemorySystem::GetAlliesInRange(float Range) const
{
    TArray<AActor*> Allies;
    
    for (const auto& MemoryPair : SocialMemory.KnownIndividuals)
    {
        const FNPC_MemoryEntry& Memory = MemoryPair.Value;
        
        if (Memory.RelationshipType == ENPC_RelationshipType::Ally)
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Memory.LastKnownLocation);
            if (Distance <= Range)
            {
                Allies.Add(Memory.Subject);
            }
        }
    }
    
    return Allies;
}

TArray<AActor*> UNPC_MemorySystem::GetEnemiesInRange(float Range) const
{
    TArray<AActor*> Enemies;
    
    for (const auto& MemoryPair : SocialMemory.KnownIndividuals)
    {
        const FNPC_MemoryEntry& Memory = MemoryPair.Value;
        
        if (Memory.RelationshipType == ENPC_RelationshipType::Enemy)
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Memory.LastKnownLocation);
            if (Distance <= Range)
            {
                Enemies.Add(Memory.Subject);
            }
        }
    }
    
    return Enemies;
}

void UNPC_MemorySystem::ProcessMemoryDecay(float DeltaTime)
{
    TArray<AActor*> ActorsToForget;
    
    for (auto& MemoryPair : SocialMemory.KnownIndividuals)
    {
        FNPC_MemoryEntry& Memory = MemoryPair.Value;
        Memory.TimeSinceLastSeen += DeltaTime;
        
        // Decay emotional weight over time
        if (Memory.EmotionalWeight > 0.0f)
        {
            Memory.EmotionalWeight = FMath::Max(0.0f, Memory.EmotionalWeight - (MemoryDecayRate * DeltaTime));
        }
        else if (Memory.EmotionalWeight < 0.0f)
        {
            Memory.EmotionalWeight = FMath::Min(0.0f, Memory.EmotionalWeight + (MemoryDecayRate * DeltaTime));
        }
        
        // Mark for forgetting if too old or irrelevant
        if (ShouldForgetMemory(Memory))
        {
            ActorsToForget.Add(MemoryPair.Key);
        }
    }
    
    // Remove forgotten actors
    for (AActor* ActorToForget : ActorsToForget)
    {
        ForgetActor(ActorToForget);
    }
}

void UNPC_MemorySystem::CleanupOldMemories()
{
    // Remove oldest memories when over limit
    TArray<AActor*> ActorKeys;
    SocialMemory.KnownIndividuals.GetKeys(ActorKeys);
    
    // Sort by time since last seen (oldest first)
    ActorKeys.Sort([this](const AActor& A, const AActor& B) {
        const FNPC_MemoryEntry& MemoryA = SocialMemory.KnownIndividuals[&A];
        const FNPC_MemoryEntry& MemoryB = SocialMemory.KnownIndividuals[&B];
        return MemoryA.TimeSinceLastSeen > MemoryB.TimeSinceLastSeen;
    });
    
    // Remove excess memories
    int32 MemoriesToRemove = SocialMemory.KnownIndividuals.Num() - MaxStoredMemories;
    for (int32 i = 0; i < MemoriesToRemove; i++)
    {
        ForgetActor(ActorKeys[i]);
    }
}

void UNPC_MemorySystem::UpdateMemoryLocations()
{
    // Update locations of remembered actors that are currently visible
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    for (auto& MemoryPair : SocialMemory.KnownIndividuals)
    {
        AActor* Subject = MemoryPair.Key;
        FNPC_MemoryEntry& Memory = MemoryPair.Value;
        
        if (Subject && IsValid(Subject))
        {
            float Distance = FVector::Dist(OwnerLocation, Subject->GetActorLocation());
            
            // Update if within observation range
            if (Distance <= MaxMemoryDistance)
            {
                UpdateMemoryEntry(Subject, Subject->GetActorLocation());
            }
        }
    }
}

float UNPC_MemorySystem::CalculateEmotionalWeight(AActor* Subject, ENPC_RelationshipType Relationship) const
{
    switch (Relationship)
    {
        case ENPC_RelationshipType::Ally:
            return 75.0f;
        case ENPC_RelationshipType::Enemy:
            return -75.0f;
        case ENPC_RelationshipType::Neutral:
        default:
            return 0.0f;
    }
}

bool UNPC_MemorySystem::ShouldForgetMemory(const FNPC_MemoryEntry& Memory) const
{
    // Forget if actor is null or invalid
    if (!Memory.Subject || !IsValid(Memory.Subject))
    {
        return true;
    }
    
    // Forget if too much time has passed and emotional weight is low
    if (Memory.TimeSinceLastSeen > 300.0f && FMath::Abs(Memory.EmotionalWeight) < 10.0f)
    {
        return true;
    }
    
    // Forget if very old regardless of emotional weight
    if (Memory.TimeSinceLastSeen > 600.0f)
    {
        return true;
    }
    
    return false;
}