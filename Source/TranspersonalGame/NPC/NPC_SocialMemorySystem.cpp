#include "NPC_SocialMemorySystem.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

UNPC_SocialMemorySystem::UNPC_SocialMemorySystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second

    MemoryDecayRate = 0.1f;
    MaxMemoryDuration = 300.0f; // 5 minutes
    MaxSocialMemories = 20;
    SightRange = 1500.0f;
    HearingRange = 800.0f;
}

void UNPC_SocialMemorySystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize territorial memory with owner's location
    if (AActor* Owner = GetOwner())
    {
        TerritorialMemory.TerritoryCenter = Owner->GetActorLocation();
        TerritorialMemory.TerritoryRadius = 2000.0f;
    }
}

void UNPC_SocialMemorySystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    DecayMemories(DeltaTime);
    CheckTerritorialIntrusion();
    CleanupOldMemories();
}

void UNPC_SocialMemorySystem::RecordInteraction(AActor* TargetActor, ENPC_InteractionType InteractionType, float RelationshipChange)
{
    if (!TargetActor)
    {
        return;
    }

    FNPC_SocialMemory* ExistingMemory = FindSocialMemory(TargetActor);
    
    if (ExistingMemory)
    {
        // Update existing memory
        ExistingMemory->RelationshipValue += RelationshipChange;
        ExistingMemory->LastInteractionTime = GetWorld()->GetTimeSeconds();
        ExistingMemory->LastInteractionType = InteractionType;
        UpdateLastSeenLocation(TargetActor);
        
        // Update hostility and trust based on relationship value
        ExistingMemory->bIsHostile = ExistingMemory->RelationshipValue < -50.0f;
        ExistingMemory->bIsTrusted = ExistingMemory->RelationshipValue > 75.0f;
    }
    else
    {
        // Create new memory if we haven't reached the limit
        if (SocialMemories.Num() < MaxSocialMemories)
        {
            FNPC_SocialMemory NewMemory;
            NewMemory.TargetActor = TargetActor;
            NewMemory.RelationshipValue = RelationshipChange;
            NewMemory.LastInteractionTime = GetWorld()->GetTimeSeconds();
            NewMemory.LastInteractionType = InteractionType;
            NewMemory.LastSeenLocation = TargetActor->GetActorLocation();
            NewMemory.bIsHostile = RelationshipChange < -50.0f;
            NewMemory.bIsTrusted = RelationshipChange > 75.0f;
            
            SocialMemories.Add(NewMemory);
        }
    }
}

FNPC_SocialMemory UNPC_SocialMemorySystem::GetSocialMemory(AActor* TargetActor)
{
    FNPC_SocialMemory* Memory = FindSocialMemory(TargetActor);
    if (Memory)
    {
        return *Memory;
    }
    
    // Return default memory if not found
    return FNPC_SocialMemory();
}

void UNPC_SocialMemorySystem::UpdateRelationship(AActor* TargetActor, float RelationshipChange)
{
    RecordInteraction(TargetActor, ENPC_InteractionType::Neutral, RelationshipChange);
}

bool UNPC_SocialMemorySystem::IsHostile(AActor* TargetActor)
{
    FNPC_SocialMemory* Memory = FindSocialMemory(TargetActor);
    return Memory ? Memory->bIsHostile : false;
}

bool UNPC_SocialMemorySystem::IsTrusted(AActor* TargetActor)
{
    FNPC_SocialMemory* Memory = FindSocialMemory(TargetActor);
    return Memory ? Memory->bIsTrusted : false;
}

TArray<AActor*> UNPC_SocialMemorySystem::GetKnownActors()
{
    TArray<AActor*> KnownActors;
    for (const FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.TargetActor)
        {
            KnownActors.Add(Memory.TargetActor);
        }
    }
    return KnownActors;
}

void UNPC_SocialMemorySystem::SetTerritory(FVector Center, float Radius)
{
    TerritorialMemory.TerritoryCenter = Center;
    TerritorialMemory.TerritoryRadius = Radius;
}

bool UNPC_SocialMemorySystem::IsInTerritory(AActor* Actor)
{
    if (!Actor)
    {
        return false;
    }

    float Distance = FVector::Dist(Actor->GetActorLocation(), TerritorialMemory.TerritoryCenter);
    return Distance <= TerritorialMemory.TerritoryRadius;
}

void UNPC_SocialMemorySystem::CheckTerritorialIntrusion()
{
    if (!GetOwner() || !GetWorld())
    {
        return;
    }

    TerritorialMemory.IntrudersInTerritory.Empty();

    // Find all actors in the world
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != GetOwner() && IsInTerritory(Actor))
        {
            // Check if this is a potential threat (player or other NPCs)
            if (Actor->IsA<APawn>())
            {
                TerritorialMemory.IntrudersInTerritory.Add(Actor);
                
                // Record negative interaction for territorial intrusion
                RecordInteraction(Actor, ENPC_InteractionType::Aggressive, -10.0f);
            }
        }
    }

    // Update intrusion time if there are intruders
    if (TerritorialMemory.IntrudersInTerritory.Num() > 0)
    {
        TerritorialMemory.LastIntrusionTime = GetWorld()->GetTimeSeconds();
    }
}

void UNPC_SocialMemorySystem::DefendTerritory(bool bStartDefending)
{
    TerritorialMemory.bIsDefending = bStartDefending;
}

void UNPC_SocialMemorySystem::DecayMemories(float DeltaTime)
{
    if (!GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (FNPC_SocialMemory& Memory : SocialMemories)
    {
        float TimeSinceInteraction = CurrentTime - Memory.LastInteractionTime;
        
        // Decay relationship values over time
        if (TimeSinceInteraction > 30.0f) // Start decay after 30 seconds
        {
            float DecayAmount = MemoryDecayRate * DeltaTime;
            
            // Decay towards neutral (0)
            if (Memory.RelationshipValue > 0)
            {
                Memory.RelationshipValue = FMath::Max(0.0f, Memory.RelationshipValue - DecayAmount);
            }
            else if (Memory.RelationshipValue < 0)
            {
                Memory.RelationshipValue = FMath::Min(0.0f, Memory.RelationshipValue + DecayAmount);
            }
            
            // Update hostility and trust based on decayed values
            Memory.bIsHostile = Memory.RelationshipValue < -50.0f;
            Memory.bIsTrusted = Memory.RelationshipValue > 75.0f;
        }
    }
}

void UNPC_SocialMemorySystem::ForgetActor(AActor* TargetActor)
{
    SocialMemories.RemoveAll([TargetActor](const FNPC_SocialMemory& Memory)
    {
        return Memory.TargetActor == TargetActor;
    });
}

void UNPC_SocialMemorySystem::ClearAllMemories()
{
    SocialMemories.Empty();
    TerritorialMemory.IntrudersInTerritory.Empty();
    TerritorialMemory.bIsDefending = false;
}

FNPC_SocialMemory* UNPC_SocialMemorySystem::FindSocialMemory(AActor* TargetActor)
{
    for (FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.TargetActor == TargetActor)
        {
            return &Memory;
        }
    }
    return nullptr;
}

void UNPC_SocialMemorySystem::CleanupOldMemories()
{
    if (!GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Remove memories that are too old or have invalid actors
    SocialMemories.RemoveAll([CurrentTime, this](const FNPC_SocialMemory& Memory)
    {
        bool bIsOld = (CurrentTime - Memory.LastInteractionTime) > MaxMemoryDuration;
        bool bIsInvalid = !IsValid(Memory.TargetActor);
        return bIsOld || bIsInvalid;
    });
}

void UNPC_SocialMemorySystem::UpdateLastSeenLocation(AActor* TargetActor)
{
    if (!TargetActor)
    {
        return;
    }

    FNPC_SocialMemory* Memory = FindSocialMemory(TargetActor);
    if (Memory)
    {
        Memory->LastSeenLocation = TargetActor->GetActorLocation();
    }
}