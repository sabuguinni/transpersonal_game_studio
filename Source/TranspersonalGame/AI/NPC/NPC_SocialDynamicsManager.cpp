#include "NPC_SocialDynamicsManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UNPC_SocialDynamicsManager::UNPC_SocialDynamicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    SocialUpdateInterval = 1.0f;
    MaxSocialRange = 2000.0f;
    MaxShortTermMemories = 20;
    MaxLongTermMemories = 100;
    MemoryDecayRate = 0.1f;
    CurrentPatrolIndex = 0;
    PatrolSpeed = 200.0f;
    PatrolWaitTime = 3.0f;
    bIsPatrolling = true;
    CurrentBehaviorState = ENPCBehaviorState::Idle;
    StateChangeTimer = 0.0f;
    CurrentTarget = nullptr;
    LastSocialUpdate = 0.0f;
    PatrolTimer = 0.0f;
    bReachedPatrolPoint = false;
}

void UNPC_SocialDynamicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize with default patrol points if none set
    if (PatrolPoints.Num() == 0)
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        PatrolPoints.Add(OwnerLocation + FVector(1000, 0, 0));
        PatrolPoints.Add(OwnerLocation + FVector(0, 1000, 0));
        PatrolPoints.Add(OwnerLocation + FVector(-1000, 0, 0));
        PatrolPoints.Add(OwnerLocation + FVector(0, -1000, 0));
    }
    
    SetBehaviorState(ENPCBehaviorState::Patrolling);
}

void UNPC_SocialDynamicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!GetOwner())
        return;
    
    // Update social relationships periodically
    LastSocialUpdate += DeltaTime;
    if (LastSocialUpdate >= SocialUpdateInterval)
    {
        UpdateSocialRelationships();
        LastSocialUpdate = 0.0f;
    }
    
    // Update memory decay
    UpdateMemoryDecay(DeltaTime);
    
    // Update behavior based on current state
    StateChangeTimer += DeltaTime;
    
    switch (CurrentBehaviorState)
    {
        case ENPCBehaviorState::Patrolling:
            UpdatePatrolBehavior(DeltaTime);
            break;
            
        case ENPCBehaviorState::Investigating:
            // Check if investigation is complete
            if (StateChangeTimer > 5.0f)
            {
                SetBehaviorState(ENPCBehaviorState::Patrolling);
            }
            break;
            
        case ENPCBehaviorState::Fleeing:
            // Check if safe to return to normal behavior
            if (StateChangeTimer > 10.0f)
            {
                SetBehaviorState(ENPCBehaviorState::Patrolling);
            }
            break;
            
        case ENPCBehaviorState::Socializing:
            // Social interaction timeout
            if (StateChangeTimer > 8.0f)
            {
                SetBehaviorState(ENPCBehaviorState::Patrolling);
            }
            break;
            
        default:
            // Idle timeout
            if (StateChangeTimer > 3.0f)
            {
                SetBehaviorState(ENPCBehaviorState::Patrolling);
            }
            break;
    }
    
    // Check for player interaction
    if (UWorld* World = GetWorld())
    {
        if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0))
        {
            if (ShouldReactToPlayer(PlayerPawn))
            {
                ProcessPlayerInteraction(PlayerPawn);
            }
        }
    }
}

void UNPC_SocialDynamicsManager::UpdateSocialRelationships()
{
    if (!GetOwner())
        return;
    
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Find nearby NPCs
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner() || !Actor)
            continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        if (Distance <= MaxSocialRange)
        {
            // Check if we have a relationship with this actor
            FNPC_SocialRelationship* Relationship = GetRelationshipWith(Actor);
            if (!Relationship)
            {
                // Create new relationship
                FNPC_SocialRelationship NewRelationship;
                NewRelationship.TargetNPC = Actor;
                NewRelationship.TrustLevel = 0.5f;
                NewRelationship.LastInteractionTime = World->GetTimeSeconds();
                SocialRelationships.Add(NewRelationship);
            }
            else
            {
                // Update existing relationship based on proximity
                float TimeSinceLastInteraction = World->GetTimeSeconds() - Relationship->LastInteractionTime;
                if (TimeSinceLastInteraction > 60.0f) // 1 minute
                {
                    // Slowly decay trust over time
                    Relationship->TrustLevel = FMath::Max(0.0f, Relationship->TrustLevel - 0.01f);
                }
                Relationship->LastInteractionTime = World->GetTimeSeconds();
            }
        }
    }
}

FNPC_SocialRelationship* UNPC_SocialDynamicsManager::GetRelationshipWith(AActor* TargetActor)
{
    for (FNPC_SocialRelationship& Relationship : SocialRelationships)
    {
        if (Relationship.TargetNPC == TargetActor)
        {
            return &Relationship;
        }
    }
    return nullptr;
}

void UNPC_SocialDynamicsManager::AddOrUpdateRelationship(AActor* TargetActor, float TrustDelta, float FearDelta, float AggressionDelta)
{
    if (!TargetActor)
        return;
    
    FNPC_SocialRelationship* Relationship = GetRelationshipWith(TargetActor);
    if (!Relationship)
    {
        // Create new relationship
        FNPC_SocialRelationship NewRelationship;
        NewRelationship.TargetNPC = TargetActor;
        NewRelationship.TrustLevel = FMath::Clamp(0.5f + TrustDelta, 0.0f, 1.0f);
        NewRelationship.FearLevel = FMath::Clamp(0.0f + FearDelta, 0.0f, 1.0f);
        NewRelationship.AggressionLevel = FMath::Clamp(0.0f + AggressionDelta, 0.0f, 1.0f);
        NewRelationship.LastInteractionTime = GetWorld()->GetTimeSeconds();
        SocialRelationships.Add(NewRelationship);
    }
    else
    {
        // Update existing relationship
        Relationship->TrustLevel = FMath::Clamp(Relationship->TrustLevel + TrustDelta, 0.0f, 1.0f);
        Relationship->FearLevel = FMath::Clamp(Relationship->FearLevel + FearDelta, 0.0f, 1.0f);
        Relationship->AggressionLevel = FMath::Clamp(Relationship->AggressionLevel + AggressionDelta, 0.0f, 1.0f);
        Relationship->LastInteractionTime = GetWorld()->GetTimeSeconds();
    }
}

void UNPC_SocialDynamicsManager::AddMemory(FVector Location, AActor* Actor, const FString& Description, float Importance)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.AssociatedActor = Actor;
    NewMemory.EventDescription = Description;
    NewMemory.ImportanceLevel = Importance;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    
    ShortTermMemory.Add(NewMemory);
    
    // Clean up if we exceed max memories
    if (ShortTermMemory.Num() > MaxShortTermMemories)
    {
        CleanupOldMemories();
    }
}

void UNPC_SocialDynamicsManager::UpdateMemoryDecay(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay importance of memories over time
    for (FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        float Age = CurrentTime - Memory.Timestamp;
        Memory.ImportanceLevel = FMath::Max(0.0f, Memory.ImportanceLevel - (MemoryDecayRate * DeltaTime * Age));
    }
    
    for (FNPC_MemoryEntry& Memory : LongTermMemory)
    {
        float Age = CurrentTime - Memory.Timestamp;
        Memory.ImportanceLevel = FMath::Max(0.0f, Memory.ImportanceLevel - (MemoryDecayRate * 0.1f * DeltaTime * Age));
    }
    
    // Transfer important short-term memories to long-term
    TransferMemoriesToLongTerm();
}

TArray<FNPC_MemoryEntry> UNPC_SocialDynamicsManager::GetMemoriesNear(FVector Location, float Radius)
{
    TArray<FNPC_MemoryEntry> NearbyMemories;
    
    for (const FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        if (FVector::Dist(Memory.Location, Location) <= Radius)
        {
            NearbyMemories.Add(Memory);
        }
    }
    
    for (const FNPC_MemoryEntry& Memory : LongTermMemory)
    {
        if (FVector::Dist(Memory.Location, Location) <= Radius)
        {
            NearbyMemories.Add(Memory);
        }
    }
    
    return NearbyMemories;
}

void UNPC_SocialDynamicsManager::SetupPatrolRoute(const TArray<FVector>& NewPatrolPoints)
{
    PatrolPoints = NewPatrolPoints;
    CurrentPatrolIndex = 0;
    PatrolTimer = 0.0f;
    bReachedPatrolPoint = false;
}

void UNPC_SocialDynamicsManager::UpdatePatrolBehavior(float DeltaTime)
{
    if (PatrolPoints.Num() == 0 || !GetOwner())
        return;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector TargetPoint = GetNextPatrolPoint();
    
    // Check if we've reached the current patrol point
    if (IsNearPatrolPoint(TargetPoint))
    {
        if (!bReachedPatrolPoint)
        {
            bReachedPatrolPoint = true;
            PatrolTimer = 0.0f;
            
            // Add memory of reaching patrol point
            AddMemory(TargetPoint, nullptr, TEXT("Reached patrol point"), 0.3f);
        }
        
        PatrolTimer += DeltaTime;
        if (PatrolTimer >= PatrolWaitTime)
        {
            // Move to next patrol point
            CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
            bReachedPatrolPoint = false;
            PatrolTimer = 0.0f;
        }
    }
    else
    {
        // Move towards patrol point
        FVector Direction = (TargetPoint - CurrentLocation).GetSafeNormal();
        FVector NewLocation = CurrentLocation + Direction * PatrolSpeed * DeltaTime;
        GetOwner()->SetActorLocation(NewLocation);
    }
}

void UNPC_SocialDynamicsManager::SetBehaviorState(ENPCBehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        StateChangeTimer = 0.0f;
        
        // Log state change
        FString StateName = TEXT("Unknown");
        switch (NewState)
        {
            case ENPCBehaviorState::Idle: StateName = TEXT("Idle"); break;
            case ENPCBehaviorState::Patrolling: StateName = TEXT("Patrolling"); break;
            case ENPCBehaviorState::Investigating: StateName = TEXT("Investigating"); break;
            case ENPCBehaviorState::Fleeing: StateName = TEXT("Fleeing"); break;
            case ENPCBehaviorState::Attacking: StateName = TEXT("Attacking"); break;
            case ENPCBehaviorState::Socializing: StateName = TEXT("Socializing"); break;
        }
        
        AddMemory(GetOwner()->GetActorLocation(), nullptr, FString::Printf(TEXT("Changed state to %s"), *StateName), 0.5f);
    }
}

bool UNPC_SocialDynamicsManager::ShouldReactToPlayer(AActor* PlayerActor)
{
    if (!PlayerActor || !GetOwner())
        return false;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    
    // React if player is within detection range
    if (Distance <= MaxSocialRange * 0.5f) // Half of social range for player detection
    {
        return true;
    }
    
    return false;
}

void UNPC_SocialDynamicsManager::ProcessPlayerInteraction(AActor* PlayerActor)
{
    if (!PlayerActor)
        return;
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PlayerActor->GetActorLocation());
    
    // Add memory of player encounter
    AddMemory(PlayerActor->GetActorLocation(), PlayerActor, TEXT("Encountered player"), 0.8f);
    
    // Update relationship with player
    FNPC_SocialRelationship* PlayerRelationship = GetRelationshipWith(PlayerActor);
    
    if (Distance < 500.0f) // Close encounter
    {
        if (!PlayerRelationship || PlayerRelationship->FearLevel > 0.7f)
        {
            // High fear - flee
            SetBehaviorState(ENPCBehaviorState::Fleeing);
            AddOrUpdateRelationship(PlayerActor, -0.1f, 0.2f, 0.1f);
        }
        else if (PlayerRelationship && PlayerRelationship->TrustLevel > 0.7f)
        {
            // High trust - socialize
            SetBehaviorState(ENPCBehaviorState::Socializing);
            CurrentTarget = PlayerActor;
        }
        else
        {
            // Neutral - investigate
            SetBehaviorState(ENPCBehaviorState::Investigating);
            CurrentTarget = PlayerActor;
        }
    }
    else
    {
        // Distant encounter - just investigate
        SetBehaviorState(ENPCBehaviorState::Investigating);
        CurrentTarget = PlayerActor;
    }
}

void UNPC_SocialDynamicsManager::CleanupOldMemories()
{
    // Remove memories with very low importance
    ShortTermMemory.RemoveAll([](const FNPC_MemoryEntry& Memory)
    {
        return Memory.ImportanceLevel < 0.1f;
    });
    
    // If still too many, remove oldest
    if (ShortTermMemory.Num() > MaxShortTermMemories)
    {
        ShortTermMemory.Sort([](const FNPC_MemoryEntry& A, const FNPC_MemoryEntry& B)
        {
            return A.Timestamp < B.Timestamp;
        });
        
        int32 ToRemove = ShortTermMemory.Num() - MaxShortTermMemories;
        ShortTermMemory.RemoveAt(0, ToRemove);
    }
}

void UNPC_SocialDynamicsManager::TransferMemoriesToLongTerm()
{
    // Transfer important memories to long-term storage
    for (int32 i = ShortTermMemory.Num() - 1; i >= 0; i--)
    {
        const FNPC_MemoryEntry& Memory = ShortTermMemory[i];
        if (Memory.ImportanceLevel > 0.6f && LongTermMemory.Num() < MaxLongTermMemories)
        {
            LongTermMemory.Add(Memory);
            ShortTermMemory.RemoveAt(i);
        }
    }
}

FVector UNPC_SocialDynamicsManager::GetNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
        return GetOwner()->GetActorLocation();
    
    return PatrolPoints[CurrentPatrolIndex];
}

bool UNPC_SocialDynamicsManager::IsNearPatrolPoint(FVector Point, float Tolerance)
{
    if (!GetOwner())
        return false;
    
    return FVector::Dist(GetOwner()->GetActorLocation(), Point) <= Tolerance;
}