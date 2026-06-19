#include "NPCMemoryComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

UNPCMemoryComponent::UNPCMemoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.5f; // tick every 0.5s — memory decay is not frame-critical
}

void UNPCMemoryComponent::BeginPlay()
{
    Super::BeginPlay();
    Memories.Reserve(MaxMemories);
}

void UNPCMemoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    DecayMemories(DeltaTime * 2.f); // 0.5s tick * 2 = 1s equivalent decay
    PruneExpiredMemories();

    // Recompute overall threat level as weighted average
    if (Memories.IsEmpty())
    {
        CurrentThreatLevel = 0.f;
        return;
    }

    float TotalWeight = 0.f;
    for (const FNPC_MemoryRecord& Mem : Memories)
    {
        TotalWeight += Mem.ThreatWeight;
    }
    CurrentThreatLevel = FMath::Clamp(TotalWeight / static_cast<float>(Memories.Num()), 0.f, 1.f);
}

void UNPCMemoryComponent::RecordEvent(ENPC_MemoryEventType EventType, FVector Location, FName SourceTag, float ThreatWeight)
{
    // Try to merge with an existing nearby memory of the same type
    for (FNPC_MemoryRecord& Existing : Memories)
    {
        if (Existing.EventType == EventType &&
            FVector::Dist(Existing.EventLocation, Location) < MergeRadius)
        {
            // Reinforce — boost weight, update location toward new event
            Existing.ThreatWeight  = FMath::Min(1.f, Existing.ThreatWeight + ThreatWeight * 0.5f);
            Existing.EventLocation = FMath::Lerp(Existing.EventLocation, Location, 0.3f);
            Existing.RecordedAtTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
            Existing.SourceTag     = SourceTag;
            return;
        }
    }

    // Evict the weakest memory if at capacity
    if (Memories.Num() >= MaxMemories)
    {
        int32 WeakestIdx = 0;
        float WeakestWeight = Memories[0].ThreatWeight;
        for (int32 i = 1; i < Memories.Num(); ++i)
        {
            if (Memories[i].ThreatWeight < WeakestWeight)
            {
                WeakestWeight = Memories[i].ThreatWeight;
                WeakestIdx    = i;
            }
        }
        Memories.RemoveAt(WeakestIdx);
    }

    // Add new memory
    FNPC_MemoryRecord NewRecord;
    NewRecord.EventType      = EventType;
    NewRecord.EventLocation  = Location;
    NewRecord.RecordedAtTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    NewRecord.ThreatWeight   = FMath::Clamp(ThreatWeight, 0.f, 1.f);
    NewRecord.SourceTag      = SourceTag;
    Memories.Add(NewRecord);
}

FVector UNPCMemoryComponent::GetMostThreateningLocation() const
{
    if (Memories.IsEmpty()) return FVector::ZeroVector;

    const FNPC_MemoryRecord* Best = nullptr;
    for (const FNPC_MemoryRecord& Mem : Memories)
    {
        if (!Best || Mem.ThreatWeight > Best->ThreatWeight)
        {
            Best = &Mem;
        }
    }
    return Best ? Best->EventLocation : FVector::ZeroVector;
}

FVector UNPCMemoryComponent::GetSafestKnownLocation() const
{
    // Safe zone memories are explicitly tagged; otherwise return lowest-threat location
    for (const FNPC_MemoryRecord& Mem : Memories)
    {
        if (Mem.EventType == ENPC_MemoryEventType::SafeZoneFound)
        {
            return Mem.EventLocation;
        }
    }

    if (Memories.IsEmpty()) return FVector::ZeroVector;

    const FNPC_MemoryRecord* Safest = nullptr;
    for (const FNPC_MemoryRecord& Mem : Memories)
    {
        if (!Safest || Mem.ThreatWeight < Safest->ThreatWeight)
        {
            Safest = &Mem;
        }
    }
    return Safest ? Safest->EventLocation : FVector::ZeroVector;
}

bool UNPCMemoryComponent::HasActiveMemoryOf(ENPC_MemoryEventType EventType, float Threshold) const
{
    for (const FNPC_MemoryRecord& Mem : Memories)
    {
        if (Mem.EventType == EventType && Mem.ThreatWeight >= Threshold)
        {
            return true;
        }
    }
    return false;
}

void UNPCMemoryComponent::ClearAllMemories()
{
    Memories.Empty();
    CurrentThreatLevel = 0.f;
}

// ── Private helpers ────────────────────────────────────────────────────────────

void UNPCMemoryComponent::DecayMemories(float DeltaTime)
{
    if (MemoryDecayDuration <= 0.f) return;

    const float DecayRate = 1.f / MemoryDecayDuration;
    for (FNPC_MemoryRecord& Mem : Memories)
    {
        Mem.ThreatWeight = FMath::Max(0.f, Mem.ThreatWeight - DecayRate * DeltaTime);
    }
}

void UNPCMemoryComponent::PruneExpiredMemories()
{
    // Remove memories whose weight has fully decayed
    Memories.RemoveAll([](const FNPC_MemoryRecord& Mem)
    {
        return Mem.ThreatWeight <= 0.f;
    });
}
