#include "NPC_SocialMemorySystem.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

UNPC_SocialMemorySystem::UNPC_SocialMemorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    MemoryDecayRate = 0.1f;
    MaxMemoryDistance = 5000.0f;
    MaxMemoryEntries = 50;
}

void UNPC_SocialMemorySystem::BeginPlay()
{
    Super::BeginPlay();
    SocialMemories.Reserve(MaxMemoryEntries);
}

void UNPC_SocialMemorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    DecayMemories(DeltaTime);
}

void UNPC_SocialMemorySystem::RecordInteraction(APawn* TargetPawn, float RelationshipChange, bool bWasHostile)
{
    if (!TargetPawn || !IsValid(TargetPawn))
    {
        return;
    }

    FNPC_SocialMemory* ExistingMemory = GetMemoryOfPawn(TargetPawn);
    
    if (ExistingMemory)
    {
        // Update existing memory
        ExistingMemory->Relationship = FMath::Clamp(ExistingMemory->Relationship + RelationshipChange, -100.0f, 100.0f);
        ExistingMemory->LastInteractionTime = GetWorld()->GetTimeSeconds();
        ExistingMemory->InteractionCount++;
        ExistingMemory->LastSeenLocation = TargetPawn->GetActorLocation();
        
        if (bWasHostile)
        {
            ExistingMemory->bIsHostile = true;
            ExistingMemory->bIsAlly = false;
        }
        else if (ExistingMemory->Relationship > 50.0f)
        {
            ExistingMemory->bIsAlly = true;
            ExistingMemory->bIsHostile = false;
        }
    }
    else
    {
        // Create new memory entry
        if (SocialMemories.Num() >= MaxMemoryEntries)
        {
            // Remove oldest memory
            int32 OldestIndex = 0;
            float OldestTime = SocialMemories[0].LastInteractionTime;
            
            for (int32 i = 1; i < SocialMemories.Num(); i++)
            {
                if (SocialMemories[i].LastInteractionTime < OldestTime)
                {
                    OldestTime = SocialMemories[i].LastInteractionTime;
                    OldestIndex = i;
                }
            }
            
            SocialMemories.RemoveAt(OldestIndex);
        }
        
        FNPC_SocialMemory NewMemory;
        NewMemory.TargetPawn = TargetPawn;
        NewMemory.Relationship = FMath::Clamp(RelationshipChange, -100.0f, 100.0f);
        NewMemory.LastInteractionTime = GetWorld()->GetTimeSeconds();
        NewMemory.InteractionCount = 1;
        NewMemory.LastSeenLocation = TargetPawn->GetActorLocation();
        NewMemory.bIsHostile = bWasHostile;
        NewMemory.bIsAlly = !bWasHostile && NewMemory.Relationship > 50.0f;
        
        SocialMemories.Add(NewMemory);
    }
}

FNPC_SocialMemory* UNPC_SocialMemorySystem::GetMemoryOfPawn(APawn* TargetPawn)
{
    if (!TargetPawn)
    {
        return nullptr;
    }

    for (FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.TargetPawn.IsValid() && Memory.TargetPawn.Get() == TargetPawn)
        {
            return &Memory;
        }
    }
    
    return nullptr;
}

float UNPC_SocialMemorySystem::GetRelationshipWith(APawn* TargetPawn)
{
    FNPC_SocialMemory* Memory = GetMemoryOfPawn(TargetPawn);
    return Memory ? Memory->Relationship : 0.0f;
}

bool UNPC_SocialMemorySystem::IsHostileTowards(APawn* TargetPawn)
{
    FNPC_SocialMemory* Memory = GetMemoryOfPawn(TargetPawn);
    return Memory ? Memory->bIsHostile : false;
}

bool UNPC_SocialMemorySystem::IsAllyWith(APawn* TargetPawn)
{
    FNPC_SocialMemory* Memory = GetMemoryOfPawn(TargetPawn);
    return Memory ? Memory->bIsAlly : false;
}

void UNPC_SocialMemorySystem::UpdateMemoryLocation(APawn* TargetPawn, FVector NewLocation)
{
    FNPC_SocialMemory* Memory = GetMemoryOfPawn(TargetPawn);
    if (Memory)
    {
        Memory->LastSeenLocation = NewLocation;
        Memory->LastInteractionTime = GetWorld()->GetTimeSeconds();
    }
}

void UNPC_SocialMemorySystem::DecayMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = SocialMemories.Num() - 1; i >= 0; i--)
    {
        FNPC_SocialMemory& Memory = SocialMemories[i];
        
        // Remove invalid references
        if (!Memory.TargetPawn.IsValid())
        {
            SocialMemories.RemoveAt(i);
            continue;
        }
        
        // Decay relationship over time
        float TimeSinceInteraction = CurrentTime - Memory.LastInteractionTime;
        if (TimeSinceInteraction > 60.0f) // Start decaying after 1 minute
        {
            float DecayAmount = MemoryDecayRate * DeltaTime;
            
            if (Memory.Relationship > 0.0f)
            {
                Memory.Relationship = FMath::Max(0.0f, Memory.Relationship - DecayAmount);
            }
            else if (Memory.Relationship < 0.0f)
            {
                Memory.Relationship = FMath::Min(0.0f, Memory.Relationship + DecayAmount);
            }
            
            // Update hostility/ally status based on decayed relationship
            if (FMath::Abs(Memory.Relationship) < 10.0f)
            {
                Memory.bIsHostile = false;
                Memory.bIsAlly = false;
            }
        }
        
        // Remove very old memories
        if (TimeSinceInteraction > 300.0f && FMath::Abs(Memory.Relationship) < 5.0f)
        {
            SocialMemories.RemoveAt(i);
        }
    }
}

TArray<APawn*> UNPC_SocialMemorySystem::GetKnownHostiles()
{
    TArray<APawn*> Hostiles;
    
    for (const FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.bIsHostile && Memory.TargetPawn.IsValid())
        {
            Hostiles.Add(Memory.TargetPawn.Get());
        }
    }
    
    return Hostiles;
}

TArray<APawn*> UNPC_SocialMemorySystem::GetKnownAllies()
{
    TArray<APawn*> Allies;
    
    for (const FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.bIsAlly && Memory.TargetPawn.IsValid())
        {
            Allies.Add(Memory.TargetPawn.Get());
        }
    }
    
    return Allies;
}

void UNPC_SocialMemorySystem::ForgetPawn(APawn* TargetPawn)
{
    for (int32 i = SocialMemories.Num() - 1; i >= 0; i--)
    {
        if (SocialMemories[i].TargetPawn.IsValid() && SocialMemories[i].TargetPawn.Get() == TargetPawn)
        {
            SocialMemories.RemoveAt(i);
            break;
        }
    }
}

void UNPC_SocialMemorySystem::ClearAllMemories()
{
    SocialMemories.Empty();
}