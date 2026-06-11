#include "NPCBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

UNPC_BehaviorComponent::UNPC_BehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Default behavior settings
    SightRange = 1500.0f;
    HearingRange = 800.0f;
    MovementSpeed = 300.0f;
    AggressionLevel = 0.5f;
    
    // Initialize behavior state
    BehaviorState.CurrentMode = ENPC_BehaviorMode::Idle;
    BehaviorState.StateTimer = 0.0f;
    BehaviorState.TargetLocation = FVector::ZeroVector;
    BehaviorState.TargetActor = nullptr;
    
    // Initialize memory
    NPCMemory.LastSeenPlayerLocation = FVector::ZeroVector;
    NPCMemory.TimeSincePlayerSeen = 999.0f;
    NPCMemory.CurrentPatrolIndex = 0;
}

void UNPC_BehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial behavior mode
    SetBehaviorMode(ENPC_BehaviorMode::Patrol);
    
    // Add default patrol points if none exist
    if (NPCMemory.PatrolPoints.Num() == 0)
    {
        FVector OwnerLocation = GetOwner()->GetActorLocation();
        AddPatrolPoint(OwnerLocation + FVector(500, 0, 0));
        AddPatrolPoint(OwnerLocation + FVector(0, 500, 0));
        AddPatrolPoint(OwnerLocation + FVector(-500, 0, 0));
        AddPatrolPoint(OwnerLocation + FVector(0, -500, 0));
    }
}

void UNPC_BehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update behavior state timer
    BehaviorState.StateTimer += DeltaTime;
    
    // Process sensory input
    ProcessSensoryInput(DeltaTime);
    
    // Update memory
    UpdateMemory(DeltaTime);
    
    // Execute current behavior
    ExecuteBehavior(DeltaTime);
}

void UNPC_BehaviorComponent::SetBehaviorMode(ENPC_BehaviorMode NewMode)
{
    if (BehaviorState.CurrentMode != NewMode)
    {
        BehaviorState.CurrentMode = NewMode;
        BehaviorState.StateTimer = 0.0f;
        
        // Reset target when changing modes
        BehaviorState.TargetActor = nullptr;
        BehaviorState.TargetLocation = FVector::ZeroVector;
        
        UE_LOG(LogTemp, Log, TEXT("NPC %s changed behavior to mode %d"), 
               *GetOwner()->GetName(), (int32)NewMode);
    }
}

void UNPC_BehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    if (NPCMemory.PatrolPoints.Num() == 0) return;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    FVector TargetPatrolPoint = NPCMemory.PatrolPoints[NPCMemory.CurrentPatrolIndex];
    
    // Check if we've reached the current patrol point
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetPatrolPoint);
    if (DistanceToTarget < 100.0f)
    {
        // Move to next patrol point
        NPCMemory.CurrentPatrolIndex = (NPCMemory.CurrentPatrolIndex + 1) % NPCMemory.PatrolPoints.Num();
        BehaviorState.StateTimer = 0.0f;
    }
    
    // Set movement target
    BehaviorState.TargetLocation = TargetPatrolPoint;
    
    // Move towards target (simplified movement)
    FVector Direction = (TargetPatrolPoint - CurrentLocation).GetSafeNormal();
    FVector NewLocation = CurrentLocation + Direction * MovementSpeed * DeltaTime;
    GetOwner()->SetActorLocation(NewLocation);
}

void UNPC_BehaviorComponent::UpdateChaseBehavior(float DeltaTime)
{
    APawn* Player = GetPlayerPawn();
    if (!Player)
    {
        SetBehaviorMode(ENPC_BehaviorMode::Patrol);
        return;
    }
    
    FVector PlayerLocation = Player->GetActorLocation();
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    // Update target location to player position
    BehaviorState.TargetLocation = PlayerLocation;
    BehaviorState.TargetActor = Player;
    
    // Check if player is still in range
    float DistanceToPlayer = FVector::Dist(CurrentLocation, PlayerLocation);
    if (DistanceToPlayer > SightRange * 1.5f)
    {
        // Lost player, return to patrol
        SetBehaviorMode(ENPC_BehaviorMode::Patrol);
        return;
    }
    
    // Move towards player (faster than patrol)
    FVector Direction = (PlayerLocation - CurrentLocation).GetSafeNormal();
    FVector NewLocation = CurrentLocation + Direction * MovementSpeed * 1.5f * DeltaTime;
    GetOwner()->SetActorLocation(NewLocation);
    
    // Update memory
    NPCMemory.LastSeenPlayerLocation = PlayerLocation;
    NPCMemory.TimeSincePlayerSeen = 0.0f;
}

void UNPC_BehaviorComponent::UpdateFlockingBehavior(float DeltaTime)
{
    // Find nearby NPCs with behavior components
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), FoundActors);
    
    FVector Separation = FVector::ZeroVector;
    FVector Alignment = FVector::ZeroVector;
    FVector Cohesion = FVector::ZeroVector;
    int32 NeighborCount = 0;
    
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    for (AActor* Actor : FoundActors)
    {
        if (Actor == GetOwner()) continue;
        
        UNPC_BehaviorComponent* OtherBehavior = Actor->FindComponentByClass<UNPC_BehaviorComponent>();
        if (!OtherBehavior) continue;
        
        FVector OtherLocation = Actor->GetActorLocation();
        float Distance = FVector::Dist(CurrentLocation, OtherLocation);
        
        if (Distance < 500.0f) // Flocking range
        {
            // Separation: steer away from nearby neighbors
            if (Distance < 200.0f)
            {
                FVector Diff = CurrentLocation - OtherLocation;
                Diff.Normalize();
                Separation += Diff / Distance; // Weight by distance
            }
            
            // Alignment: steer towards average heading of neighbors
            FVector OtherVelocity = (OtherBehavior->BehaviorState.TargetLocation - OtherLocation).GetSafeNormal();
            Alignment += OtherVelocity;
            
            // Cohesion: steer towards average position of neighbors
            Cohesion += OtherLocation;
            
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        // Average the forces
        Alignment /= NeighborCount;
        Cohesion /= NeighborCount;
        Cohesion = (Cohesion - CurrentLocation).GetSafeNormal();
        
        // Combine forces
        FVector FlockingForce = Separation * 2.0f + Alignment * 1.0f + Cohesion * 1.0f;
        FlockingForce.Normalize();
        
        // Apply flocking movement
        FVector NewLocation = CurrentLocation + FlockingForce * MovementSpeed * 0.8f * DeltaTime;
        GetOwner()->SetActorLocation(NewLocation);
        
        BehaviorState.TargetLocation = CurrentLocation + FlockingForce * 100.0f;
    }
    else
    {
        // No neighbors, return to patrol
        SetBehaviorMode(ENPC_BehaviorMode::Patrol);
    }
}

bool UNPC_BehaviorComponent::CanSeePlayer()
{
    APawn* Player = GetPlayerPawn();
    if (!Player) return false;
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = Player->GetActorLocation();
    
    // Simple line trace for line of sight
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Player);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );
    
    return !bHit; // Can see if no obstruction
}

void UNPC_BehaviorComponent::AddPatrolPoint(FVector NewPoint)
{
    NPCMemory.PatrolPoints.Add(NewPoint);
}

void UNPC_BehaviorComponent::ClearPatrolPoints()
{
    NPCMemory.PatrolPoints.Empty();
    NPCMemory.CurrentPatrolIndex = 0;
}

void UNPC_BehaviorComponent::ProcessSensoryInput(float DeltaTime)
{
    // Check for player in sight range
    if (IsPlayerInSightRange() && CanSeePlayer())
    {
        APawn* Player = GetPlayerPawn();
        if (Player)
        {
            NPCMemory.LastSeenPlayerLocation = Player->GetActorLocation();
            NPCMemory.TimeSincePlayerSeen = 0.0f;
            
            // Switch to chase if aggressive enough
            if (AggressionLevel > 0.3f && BehaviorState.CurrentMode != ENPC_BehaviorMode::Chase)
            {
                SetBehaviorMode(ENPC_BehaviorMode::Chase);
            }
        }
    }
    
    // Check for player in hearing range
    if (IsPlayerInHearingRange())
    {
        // Increase alertness but don't immediately chase
        if (BehaviorState.CurrentMode == ENPC_BehaviorMode::Idle)
        {
            SetBehaviorMode(ENPC_BehaviorMode::Patrol);
        }
    }
}

void UNPC_BehaviorComponent::UpdateMemory(float DeltaTime)
{
    NPCMemory.TimeSincePlayerSeen += DeltaTime;
    
    // Forget player location after some time
    if (NPCMemory.TimeSincePlayerSeen > 10.0f)
    {
        NPCMemory.LastSeenPlayerLocation = FVector::ZeroVector;
    }
}

void UNPC_BehaviorComponent::ExecuteBehavior(float DeltaTime)
{
    switch (BehaviorState.CurrentMode)
    {
        case ENPC_BehaviorMode::Idle:
            // Do nothing, just wait
            if (BehaviorState.StateTimer > 3.0f)
            {
                SetBehaviorMode(ENPC_BehaviorMode::Patrol);
            }
            break;
            
        case ENPC_BehaviorMode::Patrol:
            UpdatePatrolBehavior(DeltaTime);
            break;
            
        case ENPC_BehaviorMode::Chase:
            UpdateChaseBehavior(DeltaTime);
            break;
            
        case ENPC_BehaviorMode::Flocking:
            UpdateFlockingBehavior(DeltaTime);
            break;
            
        case ENPC_BehaviorMode::Fleeing:
            // Simple flee behavior - move away from player
            {
                APawn* Player = GetPlayerPawn();
                if (Player)
                {
                    FVector PlayerLocation = Player->GetActorLocation();
                    FVector CurrentLocation = GetOwner()->GetActorLocation();
                    FVector FleeDirection = (CurrentLocation - PlayerLocation).GetSafeNormal();
                    
                    FVector NewLocation = CurrentLocation + FleeDirection * MovementSpeed * 1.2f * DeltaTime;
                    GetOwner()->SetActorLocation(NewLocation);
                    
                    // Stop fleeing after some time
                    if (BehaviorState.StateTimer > 5.0f)
                    {
                        SetBehaviorMode(ENPC_BehaviorMode::Patrol);
                    }
                }
            }
            break;
    }
}

APawn* UNPC_BehaviorComponent::GetPlayerPawn()
{
    if (GetWorld())
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC)
        {
            return PC->GetPawn();
        }
    }
    return nullptr;
}

float UNPC_BehaviorComponent::GetDistanceToPlayer()
{
    APawn* Player = GetPlayerPawn();
    if (Player)
    {
        return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    }
    return 999999.0f;
}

bool UNPC_BehaviorComponent::IsPlayerInSightRange()
{
    return GetDistanceToPlayer() <= SightRange;
}

bool UNPC_BehaviorComponent::IsPlayerInHearingRange()
{
    return GetDistanceToPlayer() <= HearingRange;
}