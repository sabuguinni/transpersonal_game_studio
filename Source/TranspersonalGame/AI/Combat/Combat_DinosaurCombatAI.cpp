#include "Combat_DinosaurCombatAI.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCombat_DinosaurCombatAI::UCombat_DinosaurCombatAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    CurrentCombatState = ECombat_DinosaurCombatState::Idle;
    DinosaurSpecies = ECombat_DinosaurSpecies::Raptor;
    CurrentTarget = nullptr;
    AttackCooldown = 2.0f;
    LastAttackTime = 0.0f;
    StateChangeInterval = 5.0f;
    LastStateChangeTime = 0.0f;
}

void UCombat_DinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();
    
    // Configure default stats based on species
    ConfigureForSpecies(DinosaurSpecies);
    
    // Start with patrolling behavior
    SetCombatState(ECombat_DinosaurCombatState::Patrolling);
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI initialized for %s"), 
           *GetOwner()->GetName());
}

void UCombat_DinosaurCombatAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateCombatBehavior(DeltaTime);
}

void UCombat_DinosaurCombatAI::SetCombatState(ECombat_DinosaurCombatState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        LastStateChangeTime = GetWorld()->GetTimeSeconds();
        
        UE_LOG(LogTemp, Log, TEXT("%s changed combat state to %d"), 
               *GetOwner()->GetName(), (int32)NewState);
    }
}

void UCombat_DinosaurCombatAI::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (CurrentTarget)
    {
        // Switch to hunting when target acquired
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
        UE_LOG(LogTemp, Log, TEXT("%s acquired target: %s"), 
               *GetOwner()->GetName(), *CurrentTarget->GetName());
    }
    else
    {
        // Return to patrolling when target lost
        SetCombatState(ECombat_DinosaurCombatState::Patrolling);
    }
}

void UCombat_DinosaurCombatAI::InitiateAttack()
{
    if (!CanAttack())
    {
        return;
    }
    
    SetCombatState(ECombat_DinosaurCombatState::Attacking);
    LastAttackTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTarget)
    {
        UE_LOG(LogTemp, Warning, TEXT("%s attacking %s for %.1f damage!"), 
               *GetOwner()->GetName(), *CurrentTarget->GetName(), CombatStats.AttackDamage);
        
        // TODO: Apply damage to target
        // TODO: Play attack animation
        // TODO: Trigger attack sound effects
    }
}

void UCombat_DinosaurCombatAI::ExecuteFleeingBehavior()
{
    SetCombatState(ECombat_DinosaurCombatState::Fleeing);
    
    if (CurrentTarget)
    {
        MoveAwayFromTarget();
        UE_LOG(LogTemp, Log, TEXT("%s fleeing from %s"), 
               *GetOwner()->GetName(), *CurrentTarget->GetName());
    }
}

void UCombat_DinosaurCombatAI::StartPatrolling()
{
    SetCombatState(ECombat_DinosaurCombatState::Patrolling);
    CurrentTarget = nullptr;
    
    UE_LOG(LogTemp, Log, TEXT("%s started patrolling"), *GetOwner()->GetName());
}

bool UCombat_DinosaurCombatAI::DetectNearbyThreats()
{
    AActor* Player = FindClosestPlayer();
    
    if (Player)
    {
        float DistanceToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
        
        if (DistanceToPlayer <= CombatStats.DetectionRadius)
        {
            SetTarget(Player);
            return true;
        }
    }
    
    return false;
}

AActor* UCombat_DinosaurCombatAI::FindClosestPlayer()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    return PlayerPawn;
}

void UCombat_DinosaurCombatAI::ConfigureForSpecies(ECombat_DinosaurSpecies Species)
{
    DinosaurSpecies = Species;
    
    switch (Species)
    {
        case ECombat_DinosaurSpecies::TRex:
            CombatStats.Health = 200.0f;
            CombatStats.AttackDamage = 50.0f;
            CombatStats.AttackRange = 300.0f;
            CombatStats.MovementSpeed = 400.0f;
            CombatStats.DetectionRadius = 1500.0f;
            CombatStats.AggressionLevel = 0.9f;
            AttackCooldown = 3.0f;
            break;
            
        case ECombat_DinosaurSpecies::Raptor:
            CombatStats.Health = 80.0f;
            CombatStats.AttackDamage = 30.0f;
            CombatStats.AttackRange = 150.0f;
            CombatStats.MovementSpeed = 600.0f;
            CombatStats.DetectionRadius = 1200.0f;
            CombatStats.AggressionLevel = 0.8f;
            AttackCooldown = 1.5f;
            break;
            
        case ECombat_DinosaurSpecies::Triceratops:
            CombatStats.Health = 150.0f;
            CombatStats.AttackDamage = 40.0f;
            CombatStats.AttackRange = 250.0f;
            CombatStats.MovementSpeed = 250.0f;
            CombatStats.DetectionRadius = 800.0f;
            CombatStats.AggressionLevel = 0.3f;
            AttackCooldown = 2.5f;
            break;
            
        case ECombat_DinosaurSpecies::Brachiosaurus:
            CombatStats.Health = 300.0f;
            CombatStats.AttackDamage = 20.0f;
            CombatStats.AttackRange = 400.0f;
            CombatStats.MovementSpeed = 150.0f;
            CombatStats.DetectionRadius = 600.0f;
            CombatStats.AggressionLevel = 0.1f;
            AttackCooldown = 4.0f;
            break;
            
        case ECombat_DinosaurSpecies::Ankylosaurus:
            CombatStats.Health = 180.0f;
            CombatStats.AttackDamage = 35.0f;
            CombatStats.AttackRange = 200.0f;
            CombatStats.MovementSpeed = 200.0f;
            CombatStats.DetectionRadius = 700.0f;
            CombatStats.AggressionLevel = 0.4f;
            AttackCooldown = 3.5f;
            break;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Configured %s for species %d"), 
           *GetOwner()->GetName(), (int32)Species);
}

void UCombat_DinosaurCombatAI::UpdateCombatBehavior(float DeltaTime)
{
    switch (CurrentCombatState)
    {
        case ECombat_DinosaurCombatState::Idle:
            ProcessIdleState();
            break;
        case ECombat_DinosaurCombatState::Patrolling:
            ProcessPatrollingState();
            break;
        case ECombat_DinosaurCombatState::Hunting:
            ProcessHuntingState();
            break;
        case ECombat_DinosaurCombatState::Attacking:
            ProcessAttackingState();
            break;
        case ECombat_DinosaurCombatState::Fleeing:
            ProcessFleeingState();
            break;
        case ECombat_DinosaurCombatState::Defending:
            ProcessDefendingState();
            break;
    }
}

void UCombat_DinosaurCombatAI::ProcessIdleState()
{
    // Periodically check for threats
    if (DetectNearbyThreats())
    {
        return; // State changed to hunting
    }
    
    // After some time, start patrolling
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastStateChangeTime > StateChangeInterval)
    {
        StartPatrolling();
    }
}

void UCombat_DinosaurCombatAI::ProcessPatrollingState()
{
    // Check for threats while patrolling
    if (DetectNearbyThreats())
    {
        return; // State changed to hunting
    }
    
    // TODO: Implement actual patrol movement
    // For now, just continue patrolling
}

void UCombat_DinosaurCombatAI::ProcessHuntingState()
{
    if (!CurrentTarget)
    {
        StartPatrolling();
        return;
    }
    
    // Check if target is still in detection range
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget > CombatStats.DetectionRadius * 1.5f) // Give some buffer
    {
        SetTarget(nullptr); // Lose target
        return;
    }
    
    // Move towards target
    MoveTowardsTarget();
    
    // Attack if in range
    if (IsTargetInRange())
    {
        InitiateAttack();
    }
}

void UCombat_DinosaurCombatAI::ProcessAttackingState()
{
    // Return to hunting after attack
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime > 1.0f) // Attack animation duration
    {
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
    }
}

void UCombat_DinosaurCombatAI::ProcessFleeingState()
{
    if (CurrentTarget)
    {
        MoveAwayFromTarget();
        
        // Stop fleeing if far enough away
        float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
        if (DistanceToTarget > CombatStats.DetectionRadius * 2.0f)
        {
            StartPatrolling();
        }
    }
    else
    {
        StartPatrolling();
    }
}

void UCombat_DinosaurCombatAI::ProcessDefendingState()
{
    // Similar to hunting but with defensive positioning
    ProcessHuntingState();
}

bool UCombat_DinosaurCombatAI::IsTargetInRange() const
{
    if (!CurrentTarget)
    {
        return false;
    }
    
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return DistanceToTarget <= CombatStats.AttackRange;
}

bool UCombat_DinosaurCombatAI::CanAttack() const
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void UCombat_DinosaurCombatAI::MoveTowardsTarget()
{
    if (!CurrentTarget)
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // TODO: Use proper movement component
    // For now, just log the intended movement
    UE_LOG(LogTemp, VeryVerbose, TEXT("%s moving towards target"), *GetOwner()->GetName());
}

void UCombat_DinosaurCombatAI::MoveAwayFromTarget()
{
    if (!CurrentTarget)
    {
        return;
    }
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector Direction = (OwnerLocation - TargetLocation).GetSafeNormal();
    
    // TODO: Use proper movement component
    // For now, just log the intended movement
    UE_LOG(LogTemp, VeryVerbose, TEXT("%s fleeing from target"), *GetOwner()->GetName());
}