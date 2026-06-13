#include "NPCMemoryComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

UNPCMemoryComponent::UNPCMemoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default memory settings
    MaxMemoryDuration = 300.0f; // 5 minutes
    MaxMemoryEntries = 50;
    MemoryDecayRate = 0.1f;
}

void UNPCMemoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNPCMemoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Decay memories over time
    DecayMemories(DeltaTime);
    
    // Clean up old memories periodically
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 10.0f) // Clean up every 10 seconds
    {
        ClearOldMemories();
        CleanupTimer = 0.0f;
    }
}

void UNPCMemoryComponent::AddMemoryEntry(AActor* Actor, FVector Location, float ThreatLevel, bool bIsFriendly)
{
    if (!Actor)
    {
        return;
    }

    // Check if we already have a memory of this actor
    FNPC_MemoryEntry* ExistingEntry = GetMemoryEntry(Actor);
    if (ExistingEntry)
    {
        // Update existing entry
        ExistingEntry->LastKnownLocation = Location;
        ExistingEntry->ThreatLevel = ThreatLevel;
        ExistingEntry->LastSeenTime = GetWorld()->GetTimeSeconds();
        ExistingEntry->bIsFriendly = bIsFriendly;
        return;
    }

    // Create new memory entry
    FNPC_MemoryEntry NewEntry;
    NewEntry.Actor = Actor;
    NewEntry.LastKnownLocation = Location;
    NewEntry.ThreatLevel = ThreatLevel;
    NewEntry.LastSeenTime = GetWorld()->GetTimeSeconds();
    NewEntry.bIsFriendly = bIsFriendly;

    MemoryEntries.Add(NewEntry);

    // Remove oldest entries if we exceed max capacity
    while (MemoryEntries.Num() > MaxMemoryEntries)
    {
        // Find oldest entry
        int32 OldestIndex = 0;
        float OldestTime = MemoryEntries[0].LastSeenTime;
        
        for (int32 i = 1; i < MemoryEntries.Num(); i++)
        {
            if (MemoryEntries[i].LastSeenTime < OldestTime)
            {
                OldestTime = MemoryEntries[i].LastSeenTime;
                OldestIndex = i;
            }
        }
        
        MemoryEntries.RemoveAt(OldestIndex);
    }
}

void UNPCMemoryComponent::UpdateMemoryEntry(AActor* Actor, FVector NewLocation, float NewThreatLevel)
{
    FNPC_MemoryEntry* Entry = GetMemoryEntry(Actor);
    if (Entry)
    {
        Entry->LastKnownLocation = NewLocation;
        Entry->ThreatLevel = NewThreatLevel;
        Entry->LastSeenTime = GetWorld()->GetTimeSeconds();
    }
}

void UNPCMemoryComponent::RemoveMemoryEntry(AActor* Actor)
{
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        if (MemoryEntries[i].Actor == Actor)
        {
            MemoryEntries.RemoveAt(i);
            break;
        }
    }
}

FNPC_MemoryEntry* UNPCMemoryComponent::GetMemoryEntry(AActor* Actor)
{
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.Actor == Actor)
        {
            return &Entry;
        }
    }
    return nullptr;
}

TArray<FNPC_MemoryEntry> UNPCMemoryComponent::GetMemoriesByType(ENPC_MemoryType MemoryType)
{
    TArray<FNPC_MemoryEntry> FilteredMemories;
    
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        switch (MemoryType)
        {
        case ENPC_MemoryType::Threat:
            if (Entry.ThreatLevel > 0.5f && !Entry.bIsFriendly)
            {
                FilteredMemories.Add(Entry);
            }
            break;
        case ENPC_MemoryType::Ally:
            if (Entry.bIsFriendly)
            {
                FilteredMemories.Add(Entry);
            }
            break;
        default:
            FilteredMemories.Add(Entry);
            break;
        }
    }
    
    return FilteredMemories;
}

TArray<FNPC_MemoryEntry> UNPCMemoryComponent::GetNearbyMemories(FVector Location, float Radius)
{
    TArray<FNPC_MemoryEntry> NearbyMemories;
    
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        float Distance = FVector::Dist(Entry.LastKnownLocation, Location);
        if (Distance <= Radius)
        {
            NearbyMemories.Add(Entry);
        }
    }
    
    return NearbyMemories;
}

AActor* UNPCMemoryComponent::GetHighestThreatInMemory()
{
    AActor* HighestThreat = nullptr;
    float HighestThreatLevel = 0.0f;
    
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.ThreatLevel > HighestThreatLevel && !Entry.bIsFriendly)
        {
            HighestThreatLevel = Entry.ThreatLevel;
            HighestThreat = Entry.Actor;
        }
    }
    
    return HighestThreat;
}

void UNPCMemoryComponent::ClearOldMemories()
{
    if (!GetWorld())
    {
        return;
    }
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        float MemoryAge = CurrentTime - MemoryEntries[i].LastSeenTime;
        if (MemoryAge > MaxMemoryDuration)
        {
            MemoryEntries.RemoveAt(i);
        }
    }
}

void UNPCMemoryComponent::DecayMemories(float DeltaTime)
{
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        // Decay threat level over time
        if (Entry.ThreatLevel > 0.0f)
        {
            Entry.ThreatLevel = FMath::Max(0.0f, Entry.ThreatLevel - (MemoryDecayRate * DeltaTime));
        }
    }
}

bool UNPCMemoryComponent::HasMemoryOf(AActor* Actor)
{
    return GetMemoryEntry(Actor) != nullptr;
}

float UNPCMemoryComponent::GetThreatLevelOf(AActor* Actor)
{
    FNPC_MemoryEntry* Entry = GetMemoryEntry(Actor);
    return Entry ? Entry->ThreatLevel : 0.0f;
}