#include "Combat_AIBehaviorComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombat_AIBehaviorComponent::UCombat_AIBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    AIState.CurrentState = ECombat_AIBehaviorState::Idle;
    AIState.AggressionLevel = 0.5f;
    AIState.DetectionRadius = 2000.0f;
    AIState.AttackRange = 500.0f;
    AIState.CurrentTarget = nullptr;
    
    StateUpdateInterval = 0.5f;
    bEnableDebugLogging = false;
    LastStateUpdateTime = 0.0f;
}

void UCombat_AIBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat AI Behavior Component initialized for %s"), 
               *GetOwner()->GetName());
    }
}

void UCombat_AIBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateAIBehavior(DeltaTime);
}

void UCombat_AIBehaviorComponent::SetBehaviorState(ECombat_AIBehaviorState NewState)
{
    if (AIState.CurrentState != NewState)
    {
        ECombat_AIBehaviorState OldState = AIState.CurrentState;
        AIState.CurrentState = NewState;
        
        if (bEnableDebugLogging)
        {
            UE_LOG(LogTemp, Warning, TEXT("AI State changed from %d to %d for %s"), 
                   (int32)OldState, (int32)NewState, *GetOwner()->GetName());
        }
    }
}

ECombat_AIBehaviorState UCombat_AIBehaviorComponent::GetCurrentBehaviorState() const
{
    return AIState.CurrentState;
}

AActor* UCombat_AIBehaviorComponent::DetectNearestPlayer()
{
    if (!GetOwner() || !GetWorld())
    {
        return nullptr;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    AActor* NearestPlayer = nullptr;
    float NearestDistance = AIState.DetectionRadius;
    
    // Find all player controllers
    for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            APawn* PlayerPawn = PC->GetPawn();
            float Distance = FVector::Dist(OwnerLocation, PlayerPawn->GetActorLocation());
            
            if (Distance < NearestDistance)
            {
                NearestDistance = Distance;
                NearestPlayer = PlayerPawn;
            }
        }
    }
    
    // Debug visualization
    if (bEnableDebugLogging && GetWorld())
    {
        DrawDebugSphere(GetWorld(), OwnerLocation, AIState.DetectionRadius, 12, 
                       NearestPlayer ? FColor::Red : FColor::Green, false, 0.1f);
    }
    
    return NearestPlayer;
}

bool UCombat_AIBehaviorComponent::IsTargetInAttackRange() const
{
    if (!AIState.CurrentTarget || !GetOwner())
    {
        return false;
    }
    
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                  AIState.CurrentTarget->GetActorLocation());
    return Distance <= AIState.AttackRange;
}

void UCombat_AIBehaviorComponent::UpdateAIBehavior(float DeltaTime)
{
    LastStateUpdateTime += DeltaTime;
    
    if (LastStateUpdateTime < StateUpdateInterval)
    {
        return;
    }
    
    LastStateUpdateTime = 0.0f;
    
    switch (AIState.CurrentState)
    {
        case ECombat_AIBehaviorState::Idle:
            ProcessIdleState();
            break;
            
        case ECombat_AIBehaviorState::Patrol:
            ProcessPatrolState();
            break;
            
        case ECombat_AIBehaviorState::Chase:
            ProcessChaseState();
            break;
            
        case ECombat_AIBehaviorState::Attack:
            ProcessAttackState();
            break;
            
        case ECombat_AIBehaviorState::Flee:
            ProcessFleeState();
            break;
    }
}

void UCombat_AIBehaviorComponent::OnTargetDetected(AActor* Target)
{
    AIState.CurrentTarget = Target;
    
    if (AIState.AggressionLevel > 0.3f)
    {
        SetBehaviorState(ECombat_AIBehaviorState::Chase);
    }
    else
    {
        SetBehaviorState(ECombat_AIBehaviorState::Flee);
    }
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target detected: %s"), 
               Target ? *Target->GetName() : TEXT("NULL"));
    }
}

void UCombat_AIBehaviorComponent::OnTargetLost()
{
    AIState.CurrentTarget = nullptr;
    SetBehaviorState(ECombat_AIBehaviorState::Patrol);
    
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Target lost, returning to patrol"));
    }
}

void UCombat_AIBehaviorComponent::ProcessIdleState()
{
    AActor* DetectedPlayer = DetectNearestPlayer();
    
    if (DetectedPlayer)
    {
        OnTargetDetected(DetectedPlayer);
    }
    else
    {
        // Randomly switch to patrol after some time
        if (FMath::RandRange(0.0f, 1.0f) < 0.1f)
        {
            SetBehaviorState(ECombat_AIBehaviorState::Patrol);
        }
    }
}

void UCombat_AIBehaviorComponent::ProcessPatrolState()
{
    AActor* DetectedPlayer = DetectNearestPlayer();
    
    if (DetectedPlayer)
    {
        OnTargetDetected(DetectedPlayer);
    }
    else
    {
        // Continue patrolling or return to idle
        if (FMath::RandRange(0.0f, 1.0f) < 0.05f)
        {
            SetBehaviorState(ECombat_AIBehaviorState::Idle);
        }
    }
}

void UCombat_AIBehaviorComponent::ProcessChaseState()
{
    if (!AIState.CurrentTarget)
    {
        OnTargetLost();
        return;
    }
    
    // Check if target is still in detection range
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                  AIState.CurrentTarget->GetActorLocation());
    
    if (Distance > AIState.DetectionRadius * 1.5f)
    {
        OnTargetLost();
        return;
    }
    
    // Switch to attack if in range
    if (IsTargetInAttackRange())
    {
        SetBehaviorState(ECombat_AIBehaviorState::Attack);
    }
}

void UCombat_AIBehaviorComponent::ProcessAttackState()
{
    if (!AIState.CurrentTarget)
    {
        OnTargetLost();
        return;
    }
    
    // If target moves out of attack range, chase again
    if (!IsTargetInAttackRange())
    {
        SetBehaviorState(ECombat_AIBehaviorState::Chase);
    }
    
    // Perform attack logic here
    if (bEnableDebugLogging)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attacking target: %s"), 
               *AIState.CurrentTarget->GetName());
    }
}

void UCombat_AIBehaviorComponent::ProcessFleeState()
{
    if (!AIState.CurrentTarget)
    {
        SetBehaviorState(ECombat_AIBehaviorState::Idle);
        return;
    }
    
    // Check if target is far enough to stop fleeing
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), 
                                  AIState.CurrentTarget->GetActorLocation());
    
    if (Distance > AIState.DetectionRadius * 2.0f)
    {
        OnTargetLost();
    }
}