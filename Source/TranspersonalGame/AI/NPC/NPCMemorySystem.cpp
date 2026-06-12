#include "NPCMemorySystem.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPC_MemorySystem::UNPC_MemorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f;
    
    MaxMemoryEntries = 50;
    MemoryDecayRate = 0.1f;
    ForgetThreshold = 0.1f;
}

void UNPC_MemorySystem::BeginPlay()
{
    Super::BeginPlay();
    
    MemoryEntries.Reserve(MaxMemoryEntries);
}

void UNPC_MemorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Decay memory importance over time
    float CurrentTime = GetWorld()->GetTimeSeconds();
    for (FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        float TimeSinceMemory = CurrentTime - Entry.Timestamp;
        Entry.Importance = FMath::Max(0.0f, Entry.Importance - (MemoryDecayRate * TimeSinceMemory * DeltaTime));
    }
    
    // Remove forgotten memories
    ForgetOldMemories();
}

void UNPC_MemorySystem::AddMemory(FVector Location, ENPC_MemoryType Type, float Importance, AActor* Actor)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    NewMemory.MemoryType = Type;
    NewMemory.Importance = FMath::Clamp(Importance, 0.0f, 10.0f);
    NewMemory.RelatedActor = Actor;
    
    // If we're at max capacity, remove least important memory
    if (MemoryEntries.Num() >= MaxMemoryEntries)
    {
        int32 LeastImportantIndex = 0;
        float LowestImportance = MemoryEntries[0].Importance;
        
        for (int32 i = 1; i < MemoryEntries.Num(); i++)
        {
            if (MemoryEntries[i].Importance < LowestImportance)
            {
                LowestImportance = MemoryEntries[i].Importance;
                LeastImportantIndex = i;
            }
        }
        
        MemoryEntries.RemoveAt(LeastImportantIndex);
    }
    
    MemoryEntries.Add(NewMemory);
}

TArray<FNPC_MemoryEntry> UNPC_MemorySystem::GetMemoriesOfType(ENPC_MemoryType Type)
{
    TArray<FNPC_MemoryEntry> FilteredMemories;
    
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.MemoryType == Type)
        {
            FilteredMemories.Add(Entry);
        }
    }
    
    return FilteredMemories;
}

FNPC_MemoryEntry UNPC_MemorySystem::GetMostImportantMemory()
{
    if (MemoryEntries.Num() == 0)
    {
        return FNPC_MemoryEntry();
    }
    
    FNPC_MemoryEntry MostImportant = MemoryEntries[0];
    
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.Importance > MostImportant.Importance)
        {
            MostImportant = Entry;
        }
    }
    
    return MostImportant;
}

bool UNPC_MemorySystem::HasMemoryOfActor(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }
    
    for (const FNPC_MemoryEntry& Entry : MemoryEntries)
    {
        if (Entry.RelatedActor == Actor)
        {
            return true;
        }
    }
    
    return false;
}

void UNPC_MemorySystem::ForgetOldMemories()
{
    MemoryEntries.RemoveAll([this](const FNPC_MemoryEntry& Entry)
    {
        return Entry.Importance <= ForgetThreshold;
    });
}

void UNPC_MemorySystem::ClearAllMemories()
{
    MemoryEntries.Empty();
}