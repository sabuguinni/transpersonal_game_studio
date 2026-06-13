#include "NPCMemorySystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UNPCMemorySystem::UNPCMemorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MemoryDecayRate = 1.0f;
    MaxMemoryAge = 600.0f;
    MaxMemoryEntries = 20;
    MemoryUpdateInterval = 1.0f;
    LastMemoryUpdate = 0.0f;
}

void UNPCMemorySystem::BeginPlay()
{
    Super::BeginPlay();
    LastMemoryUpdate = GetWorld()->GetTimeSeconds();
}

void UNPCMemorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastMemoryUpdate >= MemoryUpdateInterval)
    {
        UpdateMemoryDecay(DeltaTime);
        CleanupOldMemories();
        LastMemoryUpdate = CurrentTime;
    }
}

void UNPCMemorySystem::AddMemoryEntry(AActor* Target, const FVector& Location, float ThreatLevel, bool bIsHostile)
{
    if (!Target)
    {
        return;
    }

    int32 ExistingIndex = FindMemoryIndex(Target);
    float CurrentTime = GetWorld()->GetTimeSeconds();

    if (ExistingIndex != INDEX_NONE)
    {
        // Update existing memory
        MemoryEntries[ExistingIndex].LastKnownLocation = Location;
        MemoryEntries[ExistingIndex].Timestamp = CurrentTime;
        MemoryEntries[ExistingIndex].ThreatLevel = FMath::Max(MemoryEntries[ExistingIndex].ThreatLevel, ThreatLevel);
        MemoryEntries[ExistingIndex].bIsHostile = MemoryEntries[ExistingIndex].bIsHostile || bIsHostile;
    }
    else
    {
        // Add new memory entry
        if (MemoryEntries.Num() >= MaxMemoryEntries)
        {
            // Remove oldest memory to make space
            int32 OldestIndex = 0;
            float OldestTime = MemoryEntries[0].Timestamp;
            for (int32 i = 1; i < MemoryEntries.Num(); i++)
            {
                if (MemoryEntries[i].Timestamp < OldestTime)
                {
                    OldestTime = MemoryEntries[i].Timestamp;
                    OldestIndex = i;
                }
            }
            MemoryEntries.RemoveAt(OldestIndex);
        }

        FNPC_MemoryEntry NewEntry;
        NewEntry.Target = Target;
        NewEntry.LastKnownLocation = Location;
        NewEntry.Timestamp = CurrentTime;
        NewEntry.ThreatLevel = ThreatLevel;
        NewEntry.bIsHostile = bIsHostile;
        MemoryEntries.Add(NewEntry);
    }
}

void UNPCMemorySystem::UpdateMemoryEntry(AActor* Target, const FVector& NewLocation)
{
    int32 Index = FindMemoryIndex(Target);
    if (Index != INDEX_NONE)
    {
        MemoryEntries[Index].LastKnownLocation = NewLocation;
        MemoryEntries[Index].Timestamp = GetWorld()->GetTimeSeconds();
    }
}

void UNPCMemorySystem::RemoveMemoryEntry(AActor* Target)
{
    int32 Index = FindMemoryIndex(Target);
    if (Index != INDEX_NONE)
    {
        MemoryEntries.RemoveAt(Index);
    }
}

FNPC_MemoryEntry UNPCMemorySystem::GetMemoryEntry(AActor* Target)
{
    int32 Index = FindMemoryIndex(Target);
    if (Index != INDEX_NONE)
    {
        return MemoryEntries[Index];
    }
    return FNPC_MemoryEntry();
}

bool UNPCMemorySystem::HasMemoryOf(AActor* Target)
{
    return FindMemoryIndex(Target) != INDEX_NONE;
}

TArray<FNPC_MemoryEntry> UNPCMemorySystem::GetAllMemories()
{
    return MemoryEntries;
}

TArray<FNPC_MemoryEntry> UNPCMemorySystem::GetHostileMemories()
{
    TArray<FNPC_MemoryEntry> HostileMemories;
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.bIsHostile && Entry.ThreatLevel > 0.0f)
        {
            HostileMemories.Add(Entry);
        }
    }
    return HostileMemories;
}

void UNPCMemorySystem::ClearOldMemories(float MaxAge)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        if (CurrentTime - MemoryEntries[i].Timestamp > MaxAge)
        {
            MemoryEntries.RemoveAt(i);
        }
    }
}

float UNPCMemorySystem::CalculateThreatLevel(AActor* Target)
{
    if (!Target)
    {
        return 0.0f;
    }

    float ThreatLevel = 0.0f;
    
    // Base threat calculation
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    float ProximityThreat = FMath::Max(0.0f, 1000.0f - Distance) / 1000.0f;
    
    // Check if target is moving towards us
    FVector TargetVelocity = Target->GetVelocity();
    FVector DirectionToUs = (GetOwner()->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
    float ApproachThreat = FMath::Max(0.0f, FVector::DotProduct(TargetVelocity.GetSafeNormal(), DirectionToUs));
    
    ThreatLevel = (ProximityThreat * 0.7f) + (ApproachThreat * 0.3f);
    
    // Amplify threat if target is already marked as hostile
    if (HasMemoryOf(Target))
    {
        FNPC_MemoryEntry Memory = GetMemoryEntry(Target);
        if (Memory.bIsHostile)
        {
            ThreatLevel *= 1.5f;
        }
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

AActor* UNPCMemorySystem::GetHighestThreatTarget()
{
    AActor* HighestThreatTarget = nullptr;
    float HighestThreat = 0.0f;
    
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.Target && Entry.ThreatLevel > HighestThreat)
        {
            HighestThreat = Entry.ThreatLevel;
            HighestThreatTarget = Entry.Target;
        }
    }
    
    return HighestThreatTarget;
}

FVector UNPCMemorySystem::GetLastKnownLocation(AActor* Target)
{
    int32 Index = FindMemoryIndex(Target);
    if (Index != INDEX_NONE)
    {
        return MemoryEntries[Index].LastKnownLocation;
    }
    return FVector::ZeroVector;
}

void UNPCMemorySystem::ShareMemoryWith(UNPCMemorySystem* OtherNPC, float ShareRadius)
{
    if (!OtherNPC || !GetOwner())
    {
        return;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), OtherNPC->GetOwner()->GetActorLocation());
    if (Distance > ShareRadius)
    {
        return;
    }
    
    // Share hostile memories
    TArray<FNPC_MemoryEntry> HostileMemories = GetHostileMemories();
    for (const FNPC_MemoryEntry& Memory : HostileMemories)
    {
        if (Memory.Target && !OtherNPC->HasMemoryOf(Memory.Target))
        {
            OtherNPC->ReceiveSharedMemory(Memory);
        }
    }
}

void UNPCMemorySystem::ReceiveSharedMemory(const FNPC_MemoryEntry& SharedMemory)
{
    if (!SharedMemory.Target)
    {
        return;
    }
    
    // Reduce threat level for shared memories (second-hand information)
    float ReducedThreat = SharedMemory.ThreatLevel * 0.7f;
    AddMemoryEntry(SharedMemory.Target, SharedMemory.LastKnownLocation, ReducedThreat, SharedMemory.bIsHostile);
}

void UNPCMemorySystem::UpdateMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        float Age = CurrentTime - Entry.Timestamp;
        float DecayFactor = FMath::Max(0.0f, 1.0f - (Age * MemoryDecayRate / MaxMemoryAge));
        Entry.ThreatLevel *= DecayFactor;
    }
}

void UNPCMemorySystem::CleanupOldMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = MemoryEntries.Num() - 1; i >= 0; i--)
    {
        float Age = CurrentTime - MemoryEntries[i].Timestamp;
        if (Age > MaxMemoryAge || MemoryEntries[i].ThreatLevel <= 0.01f)
        {
            MemoryEntries.RemoveAt(i);
        }
    }
}

int32 UNPCMemorySystem::FindMemoryIndex(AActor* Target)
{
    for (int32 i = 0; i < MemoryEntries.Num(); i++)
    {
        if (MemoryEntries[i].Target == Target)
        {
            return i;
        }
    }
    return INDEX_NONE;
}