#include "NPCBehaviorTree.h"
#include "TranspersonalGame/Characters/NPC/NPC_BaseCharacter.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIController.h"

UNPC_BehaviorTreeManager::UNPC_BehaviorTreeManager()
{
    BehaviorState.CurrentState = ENPC_NPCState::Idle;
    BehaviorState.StateTimer = 0.0f;
    BehaviorState.AlertLevel = 0.0f;
    BehaviorState.bIsInCombat = false;
    BehaviorState.CurrentPatrolIndex = 0;
}

void UNPC_BehaviorTreeManager::InitializeBehaviorTree(ANPC_BaseCharacter* NPCOwner)
{
    OwnerNPC = NPCOwner;
    if (OwnerNPC)
    {
        BehaviorState.CurrentState = ENPC_NPCState::Idle;
        BehaviorState.StateTimer = 0.0f;
        BehaviorState.AlertLevel = 0.0f;
        
        // Initialize default patrol points around spawn location
        if (BehaviorState.PatrolPoints.Num() == 0)
        {
            FVector SpawnLocation = OwnerNPC->GetActorLocation();
            BehaviorState.PatrolPoints.Add(SpawnLocation + FVector(500, 0, 0));
            BehaviorState.PatrolPoints.Add(SpawnLocation + FVector(0, 500, 0));
            BehaviorState.PatrolPoints.Add(SpawnLocation + FVector(-500, 0, 0));
            BehaviorState.PatrolPoints.Add(SpawnLocation + FVector(0, -500, 0));
        }
    }
}

void UNPC_BehaviorTreeManager::UpdateBehaviorState(float DeltaTime)
{
    if (!OwnerNPC) return;

    BehaviorState.StateTimer += DeltaTime;
    UpdateAlertLevel(DeltaTime);

    switch (BehaviorState.CurrentState)
    {
        case ENPC_NPCState::Idle:
            ProcessIdleState(DeltaTime);
            break;
        case ENPC_NPCState::Patrol:
            ProcessPatrolState(DeltaTime);
            break;
        case ENPC_NPCState::Chase:
            ProcessChaseState(DeltaTime);
            break;
        case ENPC_NPCState::Combat:
            ProcessCombatState(DeltaTime);
            break;
        case ENPC_NPCState::Flee:
            ProcessFleeState(DeltaTime);
            break;
        default:
            break;
    }
}

void UNPC_BehaviorTreeManager::SetBehaviorState(ENPC_NPCState NewState)
{
    if (BehaviorState.CurrentState != NewState)
    {
        BehaviorState.CurrentState = NewState;
        BehaviorState.StateTimer = 0.0f;
        
        // State transition logic
        switch (NewState)
        {
            case ENPC_NPCState::Patrol:
                if (AAIController* AIController = Cast<AAIController>(OwnerNPC->GetController()))
                {
                    AIController->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), GetNextPatrolPoint());
                }
                break;
            case ENPC_NPCState::Chase:
                BehaviorState.AlertLevel = FMath::Max(BehaviorState.AlertLevel, 0.7f);
                break;
            case ENPC_NPCState::Combat:
                BehaviorState.bIsInCombat = true;
                BehaviorState.AlertLevel = 1.0f;
                break;
            case ENPC_NPCState::Flee:
                BehaviorState.bIsInCombat = false;
                break;
            default:
                break;
        }
    }
}

void UNPC_BehaviorTreeManager::AddPatrolPoint(const FVector& Point)
{
    BehaviorState.PatrolPoints.Add(Point);
}

void UNPC_BehaviorTreeManager::SetTargetActor(AActor* Target)
{
    BehaviorState.TargetActor = Target;
    if (Target && OwnerNPC)
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerNPC->GetController()))
        {
            AIController->GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), Target);
            AIController->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), Target->GetActorLocation());
        }
    }
}

void UNPC_BehaviorTreeManager::OnPlayerDetected(AActor* Player, float Distance)
{
    SetTargetActor(Player);
    BehaviorState.AlertLevel = FMath::Min(BehaviorState.AlertLevel + 0.3f, 1.0f);
    
    if (Distance < 800.0f) // Close range - start combat
    {
        SetBehaviorState(ENPC_NPCState::Combat);
    }
    else if (Distance < DetectionRange) // Medium range - chase
    {
        SetBehaviorState(ENPC_NPCState::Chase);
    }
}

void UNPC_BehaviorTreeManager::OnPlayerLost()
{
    BehaviorState.TargetActor = nullptr;
    BehaviorState.AlertLevel = FMath::Max(BehaviorState.AlertLevel - 0.2f, 0.0f);
    
    if (BehaviorState.AlertLevel < 0.3f)
    {
        SetBehaviorState(ENPC_NPCState::Patrol);
    }
}

void UNPC_BehaviorTreeManager::OnCombatStart(AActor* Enemy)
{
    SetTargetActor(Enemy);
    SetBehaviorState(ENPC_NPCState::Combat);
}

void UNPC_BehaviorTreeManager::OnCombatEnd()
{
    BehaviorState.bIsInCombat = false;
    BehaviorState.TargetActor = nullptr;
    BehaviorState.AlertLevel = 0.5f;
    SetBehaviorState(ENPC_NPCState::Patrol);
}

void UNPC_BehaviorTreeManager::ProcessIdleState(float DeltaTime)
{
    // After 3 seconds of idle, start patrolling
    if (BehaviorState.StateTimer > 3.0f)
    {
        SetBehaviorState(ENPC_NPCState::Patrol);
    }
    
    // Check for player detection
    if (IsPlayerInRange())
    {
        APawn* Player = UGameplayStatics::GetPlayerPawn(OwnerNPC->GetWorld(), 0);
        if (Player)
        {
            float Distance = FVector::Dist(OwnerNPC->GetActorLocation(), Player->GetActorLocation());
            OnPlayerDetected(Player, Distance);
        }
    }
}

void UNPC_BehaviorTreeManager::ProcessPatrolState(float DeltaTime)
{
    if (!OwnerNPC) return;
    
    // Check if reached patrol point
    FVector CurrentLocation = OwnerNPC->GetActorLocation();
    FVector TargetPatrolPoint = GetNextPatrolPoint();
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetPatrolPoint);
    
    if (DistanceToTarget < 100.0f) // Reached patrol point
    {
        BehaviorState.CurrentPatrolIndex = (BehaviorState.CurrentPatrolIndex + 1) % BehaviorState.PatrolPoints.Num();
        BehaviorState.StateTimer = 0.0f; // Reset timer for next point
        
        // Set new target
        if (AAIController* AIController = Cast<AAIController>(OwnerNPC->GetController()))
        {
            AIController->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), GetNextPatrolPoint());
        }
    }
    
    // Check for player detection
    if (IsPlayerInRange())
    {
        APawn* Player = UGameplayStatics::GetPlayerPawn(OwnerNPC->GetWorld(), 0);
        if (Player)
        {
            float Distance = FVector::Dist(OwnerNPC->GetActorLocation(), Player->GetActorLocation());
            OnPlayerDetected(Player, Distance);
        }
    }
}

void UNPC_BehaviorTreeManager::ProcessChaseState(float DeltaTime)
{
    if (!BehaviorState.TargetActor)
    {
        OnPlayerLost();
        return;
    }
    
    float DistanceToTarget = FVector::Dist(OwnerNPC->GetActorLocation(), BehaviorState.TargetActor->GetActorLocation());
    
    // Too close - start combat
    if (DistanceToTarget < 300.0f)
    {
        SetBehaviorState(ENPC_NPCState::Combat);
    }
    // Too far - lost target
    else if (DistanceToTarget > DetectionRange * 1.5f)
    {
        OnPlayerLost();
    }
    // Update chase target location
    else
    {
        BehaviorState.TargetLocation = BehaviorState.TargetActor->GetActorLocation();
        if (AAIController* AIController = Cast<AAIController>(OwnerNPC->GetController()))
        {
            AIController->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), BehaviorState.TargetLocation);
        }
    }
}

void UNPC_BehaviorTreeManager::ProcessCombatState(float DeltaTime)
{
    if (!BehaviorState.TargetActor)
    {
        OnCombatEnd();
        return;
    }
    
    float DistanceToTarget = FVector::Dist(OwnerNPC->GetActorLocation(), BehaviorState.TargetActor->GetActorLocation());
    
    // Target escaped
    if (DistanceToTarget > 1000.0f)
    {
        OnCombatEnd();
    }
    // Continue combat - update target location
    else
    {
        BehaviorState.TargetLocation = BehaviorState.TargetActor->GetActorLocation();
        if (AAIController* AIController = Cast<AAIController>(OwnerNPC->GetController()))
        {
            AIController->GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), BehaviorState.TargetLocation);
        }
    }
}

void UNPC_BehaviorTreeManager::ProcessFleeState(float DeltaTime)
{
    // Flee for 10 seconds then return to patrol
    if (BehaviorState.StateTimer > 10.0f)
    {
        SetBehaviorState(ENPC_NPCState::Patrol);
    }
}

FVector UNPC_BehaviorTreeManager::GetNextPatrolPoint()
{
    if (BehaviorState.PatrolPoints.Num() == 0)
    {
        return OwnerNPC ? OwnerNPC->GetActorLocation() : FVector::ZeroVector;
    }
    
    return BehaviorState.PatrolPoints[BehaviorState.CurrentPatrolIndex];
}

bool UNPC_BehaviorTreeManager::IsPlayerInRange() const
{
    if (!OwnerNPC) return false;
    
    APawn* Player = UGameplayStatics::GetPlayerPawn(OwnerNPC->GetWorld(), 0);
    if (!Player) return false;
    
    float Distance = FVector::Dist(OwnerNPC->GetActorLocation(), Player->GetActorLocation());
    return Distance <= DetectionRange;
}

void UNPC_BehaviorTreeManager::UpdateAlertLevel(float DeltaTime)
{
    // Gradually decrease alert level over time
    if (BehaviorState.AlertLevel > 0.0f && !BehaviorState.bIsInCombat)
    {
        BehaviorState.AlertLevel = FMath::Max(BehaviorState.AlertLevel - (DeltaTime * 0.1f), 0.0f);
    }
}