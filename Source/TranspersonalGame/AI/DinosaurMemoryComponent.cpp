#include "DinosaurMemoryComponent.h"
#include "DinosaurAIController.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"

UDinosaurMemoryComponent::UDinosaurMemoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    // Initialize default values
    MaxLocationMemories = 50;
    MaxActorMemories = 20;
    MaxEventMemories = 30;
    MemoryDecayRate = 0.1f;
    ForgetThreshold = 0.1f;
    ShortTermMemoryDuration = 300.0f;
    LongTermMemoryThreshold = 0.8f;
}

void UDinosaurMemoryComponent::BeginPlay()
{
    Super::BeginPlay();
    
    CachedWorld = GetWorld();
    OwnerController = Cast<ADinosaurAIController>(GetOwner());
    
    if (!OwnerController)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurMemoryComponent requires ADinosaurAIController owner"));
    }
}

void UDinosaurMemoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    MemoryUpdateTimer += DeltaTime;
    ConsolidationTimer += DeltaTime;
    
    // Decay memories periodically
    if (MemoryUpdateTimer >= 5.0f) // Every 5 seconds
    {
        DecayMemories(MemoryUpdateTimer);
        MemoryUpdateTimer = 0.0f;
    }
    
    // Consolidate memories periodically
    if (ConsolidationTimer >= 60.0f) // Every minute
    {
        ConsolidateMemories();
        ConsolidationTimer = 0.0f;
    }
}

void UDinosaurMemoryComponent::RecordLocation(FVector Location, const FString& LocationType, float Importance)
{
    float CurrentTime = GetCurrentTimestamp();
    
    // Check if we already have a memory of this location type nearby
    for (FMemoryLocation& Memory : LocationMemories)
    {
        if (Memory.LocationType == LocationType && 
            FVector::Dist(Memory.Location, Location) < 100.0f) // Within 100 units
        {
            // Update existing memory
            Memory.Location = (Memory.Location + Location) * 0.5f; // Average position
            Memory.Timestamp = CurrentTime;
            Memory.Importance = FMath::Max(Memory.Importance, Importance);
            Memory.VisitCount++;
            UpdateMemoryImportance(Memory);
            return;
        }
    }
    
    // Create new memory
    FMemoryLocation NewMemory(Location, LocationType, CurrentTime, Importance);
    LocationMemories.Add(NewMemory);
    
    // Prune if we have too many memories
    if (LocationMemories.Num() > MaxLocationMemories)
    {
        PruneMemoriesByImportance();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Recorded location memory: %s at %s"), *LocationType, *Location.ToString());
}

FVector UDinosaurMemoryComponent::RecallLocationOfType(const FString& LocationType) const
{
    FMemoryLocation BestMemory;
    float BestScore = -1.0f;
    
    for (const FMemoryLocation& Memory : LocationMemories)
    {
        if (Memory.LocationType == LocationType)
        {
            // Score based on importance and recency
            float TimeSinceLastVisit = GetCurrentTimestamp() - Memory.Timestamp;
            float RecencyScore = FMath::Exp(-TimeSinceLastVisit / 3600.0f); // Decay over 1 hour
            float Score = Memory.Importance * RecencyScore * Memory.VisitCount;
            
            if (Score > BestScore)
            {
                BestScore = Score;
                BestMemory = Memory;
            }
        }
    }
    
    return BestMemory.Location;
}

TArray<FMemoryLocation> UDinosaurMemoryComponent::GetLocationMemoriesOfType(const FString& LocationType) const
{
    TArray<FMemoryLocation> FilteredMemories;
    
    for (const FMemoryLocation& Memory : LocationMemories)
    {
        if (Memory.LocationType == LocationType)
        {
            FilteredMemories.Add(Memory);
        }
    }
    
    // Sort by importance
    FilteredMemories.Sort([](const FMemoryLocation& A, const FMemoryLocation& B) {
        return A.Importance > B.Importance;
    });
    
    return FilteredMemories;
}

FMemoryLocation UDinosaurMemoryComponent::GetNearestLocationOfType(const FString& LocationType, FVector CurrentLocation) const
{
    FMemoryLocation NearestMemory;
    float NearestDistance = FLT_MAX;
    
    for (const FMemoryLocation& Memory : LocationMemories)
    {
        if (Memory.LocationType == LocationType)
        {
            float Distance = FVector::Dist(CurrentLocation, Memory.Location);
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestMemory = Memory;
            }
        }
    }
    
    return NearestMemory;
}

void UDinosaurMemoryComponent::ForgetLocation(FVector Location, float Radius)
{
    LocationMemories.RemoveAll([Location, Radius](const FMemoryLocation& Memory) {
        return FVector::Dist(Memory.Location, Location) <= Radius;
    });
}

void UDinosaurMemoryComponent::RecordSighting(AActor* Actor, FVector Location)
{
    if (!Actor)
        return;
    
    float CurrentTime = GetCurrentTimestamp();
    
    // Find existing memory or create new one
    FMemoryActor* ExistingMemory = ActorMemories.FindByPredicate([Actor](const FMemoryActor& Memory) {
        return Memory.Actor.Get() == Actor;
    });
    
    if (ExistingMemory)
    {
        // Update existing memory
        ExistingMemory->LastKnownLocation = Location;
        ExistingMemory->LastSeenTimestamp = CurrentTime;
        ExistingMemory->EncounterCount++;
        ExistingMemory->Familiarity = FMath::Min(1.0f, ExistingMemory->Familiarity + 0.1f);
        UpdateMemoryImportance(*ExistingMemory);
    }
    else
    {
        // Create new memory
        FMemoryActor NewMemory;
        NewMemory.Actor = Actor;
        NewMemory.LastKnownLocation = Location;
        NewMemory.LastSeenTimestamp = CurrentTime;
        NewMemory.Familiarity = 0.1f;
        NewMemory.ThreatLevel = 0.5f; // Neutral until proven otherwise
        NewMemory.EncounterCount = 1;
        NewMemory.bIsFriendly = false; // Assume neutral
        
        ActorMemories.Add(NewMemory);
        
        // Prune if too many memories
        if (ActorMemories.Num() > MaxActorMemories)
        {
            // Remove oldest, least familiar actors
            ActorMemories.Sort([](const FMemoryActor& A, const FMemoryActor& B) {
                return A.Familiarity * A.EncounterCount > B.Familiarity * B.EncounterCount;
            });
            ActorMemories.RemoveAt(ActorMemories.Num() - 1);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Recorded sighting of %s at %s"), 
           Actor ? *Actor->GetName() : TEXT("Unknown"), *Location.ToString());
}

bool UDinosaurMemoryComponent::HasMemoryOf(AActor* Actor) const
{
    return ActorMemories.ContainsByPredicate([Actor](const FMemoryActor& Memory) {
        return Memory.Actor.Get() == Actor;
    });
}

FMemoryActor UDinosaurMemoryComponent::GetMemoryOf(AActor* Actor) const
{
    const FMemoryActor* Memory = ActorMemories.FindByPredicate([Actor](const FMemoryActor& Memory) {
        return Memory.Actor.Get() == Actor;
    });
    
    return Memory ? *Memory : FMemoryActor();
}

void UDinosaurMemoryComponent::UpdateActorRelationship(AActor* Actor, float FamiliarityChange, float ThreatChange)
{
    FMemoryActor* Memory = ActorMemories.FindByPredicate([Actor](const FMemoryActor& Memory) {
        return Memory.Actor.Get() == Actor;
    });
    
    if (Memory)
    {
        Memory->Familiarity = FMath::Clamp(Memory->Familiarity + FamiliarityChange, 0.0f, 1.0f);
        Memory->ThreatLevel = FMath::Clamp(Memory->ThreatLevel + ThreatChange, 0.0f, 1.0f);
        Memory->bIsFriendly = (Memory->Familiarity > 0.6f && Memory->ThreatLevel < 0.3f);
        UpdateMemoryImportance(*Memory);
    }
}

FVector UDinosaurMemoryComponent::GetLastKnownLocationOf(AActor* Actor) const
{
    const FMemoryActor* Memory = ActorMemories.FindByPredicate([Actor](const FMemoryActor& Memory) {
        return Memory.Actor.Get() == Actor;
    });
    
    return Memory ? Memory->LastKnownLocation : FVector::ZeroVector;
}

void UDinosaurMemoryComponent::RecordEvent(const FString& EventType, FVector Location, float EmotionalImpact, AActor* RelatedActor, const FString& Description)
{
    FMemoryEvent NewEvent;
    NewEvent.EventType = EventType;
    NewEvent.Location = Location;
    NewEvent.Timestamp = GetCurrentTimestamp();
    NewEvent.EmotionalImpact = EmotionalImpact;
    NewEvent.RelatedActor = RelatedActor;
    NewEvent.Description = Description;
    
    EventMemories.Add(NewEvent);
    
    // Prune old events if necessary
    if (EventMemories.Num() > MaxEventMemories)
    {
        // Sort by emotional impact and recency
        EventMemories.Sort([this](const FMemoryEvent& A, const FMemoryEvent& B) {
            float TimeA = GetCurrentTimestamp() - A.Timestamp;
            float TimeB = GetCurrentTimestamp() - B.Timestamp;
            float ScoreA = FMath::Abs(A.EmotionalImpact) * FMath::Exp(-TimeA / 1800.0f); // 30 min decay
            float ScoreB = FMath::Abs(B.EmotionalImpact) * FMath::Exp(-TimeB / 1800.0f);
            return ScoreA > ScoreB;
        });
        
        // Remove least significant events
        while (EventMemories.Num() > MaxEventMemories)
        {
            EventMemories.RemoveAt(EventMemories.Num() - 1);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Recorded event: %s at %s with impact %.2f"), 
           *EventType, *Location.ToString(), EmotionalImpact);
}

void UDinosaurMemoryComponent::RecordBehaviorChange(EDinosaurBehaviorState PreviousState, EDinosaurBehaviorState NewState)
{
    FString EventDescription = FString::Printf(TEXT("Behavior changed from %d to %d"), 
                                               static_cast<int32>(PreviousState), 
                                               static_cast<int32>(NewState));
    
    float EmotionalImpact = 0.0f;
    
    // Assign emotional impact based on state changes
    if (NewState == EDinosaurBehaviorState::Fleeing)
        EmotionalImpact = -0.8f; // Very negative
    else if (NewState == EDinosaurBehaviorState::Socializing)
        EmotionalImpact = 0.6f; // Positive
    else if (NewState == EDinosaurBehaviorState::Resting)
        EmotionalImpact = 0.3f; // Mildly positive
    
    FVector CurrentLocation = OwnerController && OwnerController->GetPawn() ? 
                             OwnerController->GetPawn()->GetActorLocation() : FVector::ZeroVector;
    
    RecordEvent(TEXT("BehaviorChange"), CurrentLocation, EmotionalImpact, nullptr, EventDescription);
}

TArray<FMemoryEvent> UDinosaurMemoryComponent::GetEventsOfType(const FString& EventType) const
{
    TArray<FMemoryEvent> FilteredEvents;
    
    for (const FMemoryEvent& Event : EventMemories)
    {
        if (Event.EventType == EventType)
        {
            FilteredEvents.Add(Event);
        }
    }
    
    return FilteredEvents;
}

FMemoryEvent UDinosaurMemoryComponent::GetMostRecentEvent() const
{
    if (EventMemories.Num() == 0)
        return FMemoryEvent();
    
    const FMemoryEvent* MostRecent = &EventMemories[0];
    for (const FMemoryEvent& Event : EventMemories)
    {
        if (Event.Timestamp > MostRecent->Timestamp)
        {
            MostRecent = &Event;
        }
    }
    
    return *MostRecent;
}

float UDinosaurMemoryComponent::GetEmotionalStateFromMemories() const
{
    float TotalEmotionalImpact = 0.0f;
    float TotalWeight = 0.0f;
    float CurrentTime = GetCurrentTimestamp();
    
    for (const FMemoryEvent& Event : EventMemories)
    {
        float TimeSinceEvent = CurrentTime - Event.Timestamp;
        float Weight = FMath::Exp(-TimeSinceEvent / 1800.0f); // 30 minute decay
        TotalEmotionalImpact += Event.EmotionalImpact * Weight;
        TotalWeight += Weight;
    }
    
    return TotalWeight > 0.0f ? TotalEmotionalImpact / TotalWeight : 0.0f;
}

bool UDinosaurMemoryComponent::IsLocationFamiliar(FVector Location, float Radius) const
{
    for (const FMemoryLocation& Memory : LocationMemories)
    {
        if (FVector::Dist(Memory.Location, Location) <= Radius && Memory.VisitCount > 2)
        {
            return true;
        }
    }
    return false;
}

float UDinosaurMemoryComponent::GetLocationSafety(FVector Location, float Radius) const
{
    float SafetyScore = 0.5f; // Neutral starting point
    int32 EventCount = 0;
    
    for (const FMemoryEvent& Event : EventMemories)
    {
        if (FVector::Dist(Event.Location, Location) <= Radius)
        {
            float TimeSinceEvent = GetCurrentTimestamp() - Event.Timestamp;
            float Weight = FMath::Exp(-TimeSinceEvent / 3600.0f); // 1 hour decay
            SafetyScore += Event.EmotionalImpact * Weight * 0.1f;
            EventCount++;
        }
    }
    
    return FMath::Clamp(SafetyScore, 0.0f, 1.0f);
}

TArray<FVector> UDinosaurMemoryComponent::GetDangerousAreas() const
{
    TArray<FVector> DangerousAreas;
    
    for (const FMemoryEvent& Event : EventMemories)
    {
        if (Event.EmotionalImpact < -0.5f) // Significantly negative events
        {
            DangerousAreas.Add(Event.Location);
        }
    }
    
    return DangerousAreas;
}

TArray<FVector> UDinosaurMemoryComponent::GetSafeAreas() const
{
    TArray<FVector> SafeAreas;
    
    for (const FMemoryLocation& Memory : LocationMemories)
    {
        if (Memory.LocationType == TEXT("RestingSpot") || Memory.LocationType == TEXT("SafeZone"))
        {
            SafeAreas.Add(Memory.Location);
        }
    }
    
    return SafeAreas;
}

void UDinosaurMemoryComponent::ConsolidateMemories()
{
    // Merge similar location memories
    for (int32 i = LocationMemories.Num() - 1; i >= 0; i--)
    {
        for (int32 j = i - 1; j >= 0; j--)
        {
            if (LocationMemories[i].LocationType == LocationMemories[j].LocationType &&
                FVector::Dist(LocationMemories[i].Location, LocationMemories[j].Location) < 50.0f)
            {
                // Merge memories
                FMemoryLocation& Keeper = LocationMemories[j];
                FMemoryLocation& ToMerge = LocationMemories[i];
                
                Keeper.Location = (Keeper.Location + ToMerge.Location) * 0.5f;
                Keeper.Importance = FMath::Max(Keeper.Importance, ToMerge.Importance);
                Keeper.VisitCount += ToMerge.VisitCount;
                Keeper.Timestamp = FMath::Max(Keeper.Timestamp, ToMerge.Timestamp);
                
                LocationMemories.RemoveAt(i);
                break;
            }
        }
    }
    
    // Clean up invalid actor references
    ActorMemories.RemoveAll([](const FMemoryActor& Memory) {
        return !Memory.Actor.IsValid();
    });
    
    UE_LOG(LogTemp, Log, TEXT("Memory consolidation complete. Locations: %d, Actors: %d, Events: %d"),
           LocationMemories.Num(), ActorMemories.Num(), EventMemories.Num());
}

void UDinosaurMemoryComponent::DebugPrintMemories() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== DINOSAUR MEMORY DEBUG ==="));
    UE_LOG(LogTemp, Warning, TEXT("Location Memories: %d"), LocationMemories.Num());
    for (const FMemoryLocation& Memory : LocationMemories)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s at %s (Importance: %.2f, Visits: %d)"),
               *Memory.LocationType, *Memory.Location.ToString(), Memory.Importance, Memory.VisitCount);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Actor Memories: %d"), ActorMemories.Num());
    for (const FMemoryActor& Memory : ActorMemories)
    {
        if (Memory.Actor.IsValid())
        {
            UE_LOG(LogTemp, Warning, TEXT("  %s (Familiarity: %.2f, Threat: %.2f, Encounters: %d)"),
                   *Memory.Actor->GetName(), Memory.Familiarity, Memory.ThreatLevel, Memory.EncounterCount);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Event Memories: %d"), EventMemories.Num());
    for (const FMemoryEvent& Memory : EventMemories)
    {
        UE_LOG(LogTemp, Warning, TEXT("  %s: %s (Impact: %.2f)"),
               *Memory.EventType, *Memory.Description, Memory.EmotionalImpact);
    }
}

void UDinosaurMemoryComponent::ClearAllMemories()
{
    LocationMemories.Empty();
    ActorMemories.Empty();
    EventMemories.Empty();
    UE_LOG(LogTemp, Warning, TEXT("All memories cleared"));
}

float UDinosaurMemoryComponent::GetCurrentTimestamp() const
{
    return CachedWorld ? CachedWorld->GetTimeSeconds() : 0.0f;
}

void UDinosaurMemoryComponent::DecayMemories(float DeltaTime)
{
    float CurrentTime = GetCurrentTimestamp();
    
    // Decay location memories
    for (FMemoryLocation& Memory : LocationMemories)
    {
        float TimeSinceLastAccess = CurrentTime - Memory.Timestamp;
        if (TimeSinceLastAccess > ShortTermMemoryDuration)
        {
            Memory.Importance *= (1.0f - MemoryDecayRate * DeltaTime / 60.0f); // Per minute decay
        }
    }
    
    // Decay actor memories
    for (FMemoryActor& Memory : ActorMemories)
    {
        float TimeSinceLastSeen = CurrentTime - Memory.LastSeenTimestamp;
        if (TimeSinceLastSeen > ShortTermMemoryDuration)
        {
            Memory.Familiarity *= (1.0f - MemoryDecayRate * DeltaTime / 60.0f);
        }
    }
    
    // Clean up forgotten memories
    CleanupOldMemories();
}

void UDinosaurMemoryComponent::CleanupOldMemories()
{
    // Remove forgotten location memories
    LocationMemories.RemoveAll([this](const FMemoryLocation& Memory) {
        return ShouldForgetMemory(Memory.Timestamp, Memory.Importance);
    });
    
    // Remove forgotten actor memories
    ActorMemories.RemoveAll([this](const FMemoryActor& Memory) {
        return ShouldForgetMemory(Memory.LastSeenTimestamp, Memory.Familiarity) || !Memory.Actor.IsValid();
    });
    
    // Remove old events (keep only recent significant ones)
    float CurrentTime = GetCurrentTimestamp();
    EventMemories.RemoveAll([CurrentTime](const FMemoryEvent& Memory) {
        float TimeSinceEvent = CurrentTime - Memory.Timestamp;
        return TimeSinceEvent > 3600.0f && FMath::Abs(Memory.EmotionalImpact) < 0.3f; // 1 hour for minor events
    });
}

void UDinosaurMemoryComponent::PruneMemoriesByImportance()
{
    if (LocationMemories.Num() <= MaxLocationMemories)
        return;
    
    // Sort by importance (descending)
    LocationMemories.Sort([](const FMemoryLocation& A, const FMemoryLocation& B) {
        return A.Importance > B.Importance;
    });
    
    // Remove least important memories
    while (LocationMemories.Num() > MaxLocationMemories)
    {
        LocationMemories.RemoveAt(LocationMemories.Num() - 1);
    }
}

bool UDinosaurMemoryComponent::ShouldForgetMemory(float LastAccessed, float Importance) const
{
    float CurrentTime = GetCurrentTimestamp();
    float TimeSinceAccess = CurrentTime - LastAccessed;
    
    // Don't forget important memories too quickly
    if (Importance > LongTermMemoryThreshold)
        return false;
    
    // Forget based on importance and time
    float ForgetProbability = (TimeSinceAccess / 3600.0f) * (1.0f - Importance); // Hours * inverse importance
    return ForgetProbability > 2.0f; // Forget after 2+ hours for unimportant memories
}

void UDinosaurMemoryComponent::UpdateMemoryImportance(FMemoryLocation& Memory)
{
    // Increase importance based on visit count and recency
    float RecencyBonus = FMath::Exp(-(GetCurrentTimestamp() - Memory.Timestamp) / 1800.0f); // 30 min decay
    float VisitBonus = FMath::Min(1.0f, Memory.VisitCount * 0.1f);
    Memory.Importance = FMath::Min(1.0f, Memory.Importance + RecencyBonus * 0.1f + VisitBonus * 0.05f);
}

void UDinosaurMemoryComponent::UpdateMemoryImportance(FMemoryActor& Memory)
{
    // Increase importance based on familiarity and encounter count
    float FamiliarityBonus = Memory.Familiarity * 0.2f;
    float EncounterBonus = FMath::Min(1.0f, Memory.EncounterCount * 0.1f);
    // Threat level also increases importance
    float ThreatBonus = Memory.ThreatLevel * 0.3f;
    
    float NewImportance = FamiliarityBonus + EncounterBonus + ThreatBonus;
    // Store importance in familiarity field for now (could add separate importance field)
    Memory.Familiarity = FMath::Max(Memory.Familiarity, NewImportance);
}

void UDinosaurMemoryComponent::UpdateMemoryImportance(FMemoryEvent& Memory)
{
    // Events maintain their emotional impact as importance
    // Could add decay over time here if needed
}