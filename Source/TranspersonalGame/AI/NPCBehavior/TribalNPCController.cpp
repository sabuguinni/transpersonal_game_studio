#include "TribalNPCController.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIModule/Classes/BehaviorTree/BlackboardComponent.h"

ATribalNPCController::ATribalNPCController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default values
    TribalRole = ENPC_TribalRole::Hunter;
    CurrentBehaviorState = ENPC_BehaviorState::Idle;
    PatrolRadius = 1500.0f;
    WorkDuration = 30.0f;
    SocialDistance = 500.0f;
    
    // Memory system defaults
    MaxShortTermMemories = 10;
    MaxLongTermMemories = 50;
    
    // Behavior timing
    StateChangeTimer = 0.0f;
    NextStateChangeTime = 15.0f;
    
    HomeLocation = FVector::ZeroVector;
    CurrentTarget = FVector::ZeroVector;
}

void ATribalNPCController::BeginPlay()
{
    Super::BeginPlay();
    
    // Set home location to current spawn location
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
        CurrentTarget = HomeLocation;
    }
    
    // Start with role-appropriate behavior
    HandleRoleSpecificBehavior();
}

void ATribalNPCController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateBehaviorState(DeltaTime);
    ExecuteCurrentBehavior();
    ProcessMemories();
}

void ATribalNPCController::SetTribalRole(ENPC_TribalRole NewRole)
{
    TribalRole = NewRole;
    HandleRoleSpecificBehavior();
}

void ATribalNPCController::ChangeBehaviorState(ENPC_BehaviorState NewState)
{
    if (CurrentBehaviorState != NewState)
    {
        CurrentBehaviorState = NewState;
        StateChangeTimer = 0.0f;
        
        // Add memory of state change
        if (GetPawn())
        {
            AddMemoryEntry(GetPawn()->GetActorLocation(), 
                          FString::Printf(TEXT("StateChange_%s"), 
                          *UEnum::GetValueAsString(NewState)), 2.0f);
        }
    }
}

void ATribalNPCController::StartPatrolling()
{
    ChangeBehaviorState(ENPC_BehaviorState::Patrolling);
    CurrentTarget = GetRandomPatrolPoint();
}

void ATribalNPCController::StartWorking()
{
    ChangeBehaviorState(ENPC_BehaviorState::Working);
    NextStateChangeTime = WorkDuration;
}

void ATribalNPCController::StartSocializing()
{
    ChangeBehaviorState(ENPC_BehaviorState::Socializing);
    NextStateChangeTime = FMath::RandRange(10.0f, 25.0f);
}

void ATribalNPCController::AddMemoryEntry(FVector Location, FString EventType, float Importance)
{
    FNPC_MemoryEntry NewMemory;
    NewMemory.Location = Location;
    NewMemory.EventType = EventType;
    NewMemory.Importance = Importance;
    NewMemory.Timestamp = GetWorld()->GetTimeSeconds();
    
    // Add to short-term memory
    ShortTermMemory.Add(NewMemory);
    
    // Limit short-term memory size
    if (ShortTermMemory.Num() > MaxShortTermMemories)
    {
        // Move oldest important memories to long-term
        for (int32 i = 0; i < ShortTermMemory.Num() - MaxShortTermMemories; i++)
        {
            if (ShortTermMemory[i].Importance >= 3.0f)
            {
                LongTermMemory.Add(ShortTermMemory[i]);
            }
        }
        
        // Remove excess short-term memories
        ShortTermMemory.RemoveAt(0, ShortTermMemory.Num() - MaxShortTermMemories);
    }
    
    // Limit long-term memory size
    if (LongTermMemory.Num() > MaxLongTermMemories)
    {
        LongTermMemory.RemoveAt(0, LongTermMemory.Num() - MaxLongTermMemories);
    }
}

void ATribalNPCController::ProcessMemories()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Decay memory importance over time
    for (FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        float TimeDiff = CurrentTime - Memory.Timestamp;
        if (TimeDiff > 60.0f) // 1 minute decay
        {
            Memory.Importance *= 0.9f;
        }
    }
    
    // Remove very low importance memories
    ShortTermMemory.RemoveAll([](const FNPC_MemoryEntry& Memory)
    {
        return Memory.Importance < 0.1f;
    });
}

bool ATribalNPCController::HasMemoryOfLocation(FVector Location, float Radius)
{
    for (const FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        if (FVector::Dist(Memory.Location, Location) <= Radius)
        {
            return true;
        }
    }
    
    for (const FNPC_MemoryEntry& Memory : LongTermMemory)
    {
        if (FVector::Dist(Memory.Location, Location) <= Radius)
        {
            return true;
        }
    }
    
    return false;
}

FNPC_MemoryEntry ATribalNPCController::GetMostImportantMemory()
{
    FNPC_MemoryEntry MostImportant;
    float HighestImportance = 0.0f;
    
    for (const FNPC_MemoryEntry& Memory : ShortTermMemory)
    {
        if (Memory.Importance > HighestImportance)
        {
            HighestImportance = Memory.Importance;
            MostImportant = Memory;
        }
    }
    
    return MostImportant;
}

float ATribalNPCController::GetDistanceToPlayer()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn && GetPawn())
    {
        return FVector::Dist(GetPawn()->GetActorLocation(), PlayerPawn->GetActorLocation());
    }
    return -1.0f;
}

bool ATribalNPCController::IsPlayerNearby(float Radius)
{
    float Distance = GetDistanceToPlayer();
    return Distance >= 0.0f && Distance <= Radius;
}

FVector ATribalNPCController::GetRandomPatrolPoint()
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    FVector PatrolPoint = HomeLocation + (RandomDirection * RandomDistance);
    
    // Try to find valid navigation point
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(PatrolPoint, NavLocation, FVector(500.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return PatrolPoint;
}

void ATribalNPCController::UpdateBehaviorState(float DeltaTime)
{
    StateChangeTimer += DeltaTime;
    
    // Check for player proximity - affects behavior
    if (IsPlayerNearby(800.0f))
    {
        if (CurrentBehaviorState != ENPC_BehaviorState::Investigating && 
            CurrentBehaviorState != ENPC_BehaviorState::Fleeing)
        {
            // React to player based on role
            switch (TribalRole)
            {
                case ENPC_TribalRole::Scout:
                    ChangeBehaviorState(ENPC_BehaviorState::Investigating);
                    break;
                case ENPC_TribalRole::Elder:
                    ChangeBehaviorState(ENPC_BehaviorState::Socializing);
                    break;
                default:
                    if (FMath::RandRange(0.0f, 1.0f) < 0.3f)
                    {
                        ChangeBehaviorState(ENPC_BehaviorState::Fleeing);
                    }
                    break;
            }
        }
        
        // Add player sighting to memory
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            AddMemoryEntry(PlayerPawn->GetActorLocation(), TEXT("PlayerSighting"), 4.0f);
        }
    }
    
    // State change timing
    if (StateChangeTimer >= NextStateChangeTime)
    {
        HandleRoleSpecificBehavior();
        NextStateChangeTime = FMath::RandRange(15.0f, 45.0f);
        StateChangeTimer = 0.0f;
    }
}

void ATribalNPCController::ExecuteCurrentBehavior()
{
    if (!GetPawn()) return;
    
    switch (CurrentBehaviorState)
    {
        case ENPC_BehaviorState::Patrolling:
            // Move towards current target
            if (FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget) < 200.0f)
            {
                CurrentTarget = GetRandomPatrolPoint();
            }
            MoveToLocation(CurrentTarget);
            break;
            
        case ENPC_BehaviorState::Working:
            // Stay in place and "work"
            StopMovement();
            break;
            
        case ENPC_BehaviorState::Socializing:
            // Move slowly around home area
            if (FVector::Dist(GetPawn()->GetActorLocation(), HomeLocation) > SocialDistance)
            {
                MoveToLocation(HomeLocation);
            }
            break;
            
        case ENPC_BehaviorState::Fleeing:
            // Move away from player
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (PlayerPawn)
            {
                FVector FleeDirection = GetPawn()->GetActorLocation() - PlayerPawn->GetActorLocation();
                FleeDirection.Normalize();
                FVector FleeTarget = GetPawn()->GetActorLocation() + (FleeDirection * 1000.0f);
                MoveToLocation(FleeTarget);
            }
            break;
            
        case ENPC_BehaviorState::Investigating:
            // Move towards player slowly
            APawn* PlayerPawn2 = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
            if (PlayerPawn2)
            {
                FVector PlayerLocation = PlayerPawn2->GetActorLocation();
                FVector SafeDistance = PlayerLocation + ((GetPawn()->GetActorLocation() - PlayerLocation).GetSafeNormal() * 400.0f);
                MoveToLocation(SafeDistance);
            }
            break;
            
        default:
            // Idle - stay near home
            if (FVector::Dist(GetPawn()->GetActorLocation(), HomeLocation) > 300.0f)
            {
                MoveToLocation(HomeLocation);
            }
            break;
    }
}

void ATribalNPCController::HandleRoleSpecificBehavior()
{
    switch (TribalRole)
    {
        case ENPC_TribalRole::Hunter:
            if (FMath::RandRange(0.0f, 1.0f) < 0.6f)
            {
                StartPatrolling();
            }
            else
            {
                StartWorking(); // Preparing hunting tools
            }
            break;
            
        case ENPC_TribalRole::Gatherer:
            if (FMath::RandRange(0.0f, 1.0f) < 0.7f)
            {
                StartWorking(); // Gathering resources
            }
            else
            {
                StartPatrolling(); // Looking for new gathering spots
            }
            break;
            
        case ENPC_TribalRole::Scout:
            StartPatrolling(); // Scouts always patrol
            PatrolRadius *= 1.5f; // Scouts have larger patrol radius
            break;
            
        case ENPC_TribalRole::Elder:
            if (FMath::RandRange(0.0f, 1.0f) < 0.8f)
            {
                StartSocializing(); // Elders mostly socialize
            }
            else
            {
                ChangeBehaviorState(ENPC_BehaviorState::Idle); // Rest
            }
            break;
    }
}