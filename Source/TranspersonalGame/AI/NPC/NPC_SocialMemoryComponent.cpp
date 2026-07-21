#include "NPC_SocialMemoryComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"

UNPC_SocialMemoryComponent::UNPC_SocialMemoryComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    MemoryDecayRate = 0.1f;
    MaxMemoryDuration = 300.0f; // 5 minutes
    MaxMemoryEntries = 50;
}

void UNPC_SocialMemoryComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UNPC_SocialMemoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    DecayMemories(DeltaTime);
}

void UNPC_SocialMemoryComponent::RecordInteraction(AActor* OtherActor, float InteractionValue, ENPCBehaviorState ObservedBehavior)
{
    if (!OtherActor)
    {
        return;
    }

    // Find existing memory or create new one
    FNPC_SocialMemory* ExistingMemory = GetMemoryOf(OtherActor);
    
    if (ExistingMemory)
    {
        // Update existing memory
        ExistingMemory->RelationshipValue += InteractionValue;
        ExistingMemory->RelationshipValue = FMath::Clamp(ExistingMemory->RelationshipValue, -100.0f, 100.0f);
        ExistingMemory->LastInteractionTime = GetWorld()->GetTimeSeconds();
        ExistingMemory->LastKnownLocation = OtherActor->GetActorLocation();
        ExistingMemory->LastObservedBehavior = ObservedBehavior;
    }
    else
    {
        // Create new memory
        if (SocialMemories.Num() >= MaxMemoryEntries)
        {
            // Remove oldest memory
            float OldestTime = GetWorld()->GetTimeSeconds();
            int32 OldestIndex = 0;
            
            for (int32 i = 0; i < SocialMemories.Num(); i++)
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
        NewMemory.RememberedActor = OtherActor;
        NewMemory.RelationshipValue = InteractionValue;
        NewMemory.LastInteractionTime = GetWorld()->GetTimeSeconds();
        NewMemory.LastKnownLocation = OtherActor->GetActorLocation();
        NewMemory.LastObservedBehavior = ObservedBehavior;
        
        SocialMemories.Add(NewMemory);
    }
}

FNPC_SocialMemory* UNPC_SocialMemoryComponent::GetMemoryOf(AActor* Actor)
{
    if (!Actor)
    {
        return nullptr;
    }

    for (FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.RememberedActor == Actor)
        {
            return &Memory;
        }
    }
    
    return nullptr;
}

float UNPC_SocialMemoryComponent::GetRelationshipValue(AActor* Actor)
{
    FNPC_SocialMemory* Memory = GetMemoryOf(Actor);
    return Memory ? Memory->RelationshipValue : 0.0f;
}

TArray<AActor*> UNPC_SocialMemoryComponent::GetKnownActors()
{
    TArray<AActor*> KnownActors;
    
    for (const FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (Memory.RememberedActor && IsValid(Memory.RememberedActor))
        {
            KnownActors.Add(Memory.RememberedActor);
        }
    }
    
    return KnownActors;
}

void UNPC_SocialMemoryComponent::ForgetActor(AActor* Actor)
{
    if (!Actor)
    {
        return;
    }

    for (int32 i = SocialMemories.Num() - 1; i >= 0; i--)
    {
        if (SocialMemories[i].RememberedActor == Actor)
        {
            SocialMemories.RemoveAt(i);
            break;
        }
    }
}

void UNPC_SocialMemoryComponent::DecayMemories(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    for (int32 i = SocialMemories.Num() - 1; i >= 0; i--)
    {
        FNPC_SocialMemory& Memory = SocialMemories[i];
        
        // Remove memories of destroyed actors
        if (!IsValid(Memory.RememberedActor))
        {
            SocialMemories.RemoveAt(i);
            continue;
        }
        
        // Remove very old memories
        float MemoryAge = CurrentTime - Memory.LastInteractionTime;
        if (MemoryAge > MaxMemoryDuration)
        {
            SocialMemories.RemoveAt(i);
            continue;
        }
        
        // Decay relationship values towards neutral over time
        if (Memory.RelationshipValue > 0.0f)
        {
            Memory.RelationshipValue = FMath::Max(0.0f, Memory.RelationshipValue - MemoryDecayRate * DeltaTime);
        }
        else if (Memory.RelationshipValue < 0.0f)
        {
            Memory.RelationshipValue = FMath::Min(0.0f, Memory.RelationshipValue + MemoryDecayRate * DeltaTime);
        }
    }
}

AActor* UNPC_SocialMemoryComponent::GetClosestRememberedActor(const FVector& Location, float MaxDistance)
{
    AActor* ClosestActor = nullptr;
    float ClosestDistance = MaxDistance;
    
    for (const FNPC_SocialMemory& Memory : SocialMemories)
    {
        if (IsValid(Memory.RememberedActor))
        {
            float Distance = FVector::Dist(Location, Memory.RememberedActor->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                ClosestActor = Memory.RememberedActor;
            }
        }
    }
    
    return ClosestActor;
}