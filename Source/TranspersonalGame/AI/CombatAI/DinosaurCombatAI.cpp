#include "DinosaurCombatAI.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ADinosaurCombatAI::ADinosaurCombatAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Default T-Rex stats
    Stats.Health = 500.0f;
    Stats.MaxHealth = 500.0f;
    Stats.AggroRadius = 1200.0f;
    Stats.ChaseSpeed = 850.0f;
    Stats.AttackRange = 180.0f;
    Stats.DamagePerHit = 75.0f;
    Stats.AttackCooldown = 1.8f;
    Stats.RetreatHealthThreshold = 0.15f;

    CurrentState = ECombat_DinoState::Patrol;
    CurrentWaypointIndex = 0;
    StateTimer = 0.0f;
    TimeSinceLastAttack = 0.0f;
}

void ADinosaurCombatAI::BeginPlay()
{
    Super::BeginPlay();

    // Adjust stats by species
    switch (Species)
    {
    case ECombat_DinoSpecies::Velociraptor:
        Stats.Health = 120.0f;
        Stats.MaxHealth = 120.0f;
        Stats.AggroRadius = 800.0f;
        Stats.ChaseSpeed = 1100.0f;
        Stats.AttackRange = 90.0f;
        Stats.DamagePerHit = 25.0f;
        Stats.AttackCooldown = 0.8f;
        PackData.bIsPackHunter = true;
        PackData.PackSize = 3;
        break;

    case ECombat_DinoSpecies::Triceratops:
        Stats.Health = 800.0f;
        Stats.MaxHealth = 800.0f;
        Stats.AggroRadius = 600.0f;
        Stats.ChaseSpeed = 700.0f;
        Stats.AttackRange = 200.0f;
        Stats.DamagePerHit = 90.0f;
        Stats.AttackCooldown = 2.5f;
        Stats.RetreatHealthThreshold = 0.05f;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        Stats.Health = 2000.0f;
        Stats.MaxHealth = 2000.0f;
        Stats.AggroRadius = 400.0f;
        Stats.ChaseSpeed = 500.0f;
        Stats.AttackRange = 300.0f;
        Stats.DamagePerHit = 120.0f;
        Stats.AttackCooldown = 3.0f;
        Stats.RetreatHealthThreshold = 0.0f; // Never retreats
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        Stats.Health = 80.0f;
        Stats.MaxHealth = 80.0f;
        Stats.AggroRadius = 1500.0f;
        Stats.ChaseSpeed = 1400.0f;
        Stats.AttackRange = 120.0f;
        Stats.DamagePerHit = 20.0f;
        Stats.AttackCooldown = 1.2f;
        break;

    default: // TRex — already set in constructor
        break;
    }
}

void ADinosaurCombatAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == ECombat_DinoState::Dead)
    {
        return;
    }

    TimeSinceLastAttack += DeltaTime;
    StateTimer += DeltaTime;

    // Check if player entered aggro range
    if (CurrentState == ECombat_DinoState::Patrol || CurrentState == ECombat_DinoState::Alert)
    {
        if (IsPlayerInAggroRange())
        {
            SetCombatState(ECombat_DinoState::Chase);
        }
    }

    switch (CurrentState)
    {
    case ECombat_DinoState::Patrol:
        UpdatePatrolState(DeltaTime);
        break;
    case ECombat_DinoState::Alert:
        UpdateAlertState(DeltaTime);
        break;
    case ECombat_DinoState::Chase:
        UpdateChaseState(DeltaTime);
        break;
    case ECombat_DinoState::Attack:
        UpdateAttackState(DeltaTime);
        break;
    case ECombat_DinoState::Retreat:
        UpdateRetreatState(DeltaTime);
        break;
    default:
        break;
    }
}

void ADinosaurCombatAI::SetCombatState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState)
    {
        return;
    }

    CurrentState = NewState;
    StateTimer = 0.0f;
}

void ADinosaurCombatAI::TakeCombatDamage(float DamageAmount)
{
    if (CurrentState == ECombat_DinoState::Dead)
    {
        return;
    }

    Stats.Health = FMath::Max(0.0f, Stats.Health - DamageAmount);

    // Transition to alert/chase if hit while patrolling
    if (CurrentState == ECombat_DinoState::Patrol)
    {
        SetCombatState(ECombat_DinoState::Alert);
    }

    // Check retreat threshold
    if (Stats.Health > 0.0f && GetHealthPercent() <= Stats.RetreatHealthThreshold)
    {
        SetCombatState(ECombat_DinoState::Retreat);
        return;
    }

    // Death
    if (Stats.Health <= 0.0f)
    {
        SetCombatState(ECombat_DinoState::Dead);
    }
}

bool ADinosaurCombatAI::IsPlayerInAggroRange() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        return false;
    }

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    return DistToPlayer <= Stats.AggroRadius;
}

float ADinosaurCombatAI::GetHealthPercent() const
{
    if (Stats.MaxHealth <= 0.0f)
    {
        return 0.0f;
    }
    return Stats.Health / Stats.MaxHealth;
}

void ADinosaurCombatAI::InitiatePackFlank(TArray<ADinosaurCombatAI*> PackMembers)
{
    if (!PackData.bIsPackHunter || PackMembers.Num() == 0)
    {
        return;
    }

    // Alpha (this actor) charges directly; others flank at PackData.FlankingAngle
    SetCombatState(ECombat_DinoState::Chase);

    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i] && PackMembers[i] != this)
        {
            PackMembers[i]->SetCombatState(ECombat_DinoState::Flank);
        }
    }
}

void ADinosaurCombatAI::UpdatePatrolState(float DeltaTime)
{
    if (PatrolWaypoints.Num() == 0)
    {
        return;
    }

    AActor* TargetWaypoint = PatrolWaypoints[CurrentWaypointIndex];
    if (!TargetWaypoint)
    {
        return;
    }

    FVector ToWaypoint = TargetWaypoint->GetActorLocation() - GetActorLocation();
    float DistToWaypoint = ToWaypoint.Size();

    if (DistToWaypoint < 100.0f)
    {
        // Advance to next waypoint
        CurrentWaypointIndex = (CurrentWaypointIndex + 1) % PatrolWaypoints.Num();
    }
}

void ADinosaurCombatAI::UpdateAlertState(float DeltaTime)
{
    // Stay alert for 5 seconds, then return to patrol
    if (StateTimer > 5.0f && !IsPlayerInAggroRange())
    {
        SetCombatState(ECombat_DinoState::Patrol);
    }
}

void ADinosaurCombatAI::UpdateChaseState(float DeltaTime)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn)
    {
        SetCombatState(ECombat_DinoState::Patrol);
        return;
    }

    CurrentTarget = PlayerPawn;
    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    // Close enough to attack
    if (DistToPlayer <= Stats.AttackRange)
    {
        SetCombatState(ECombat_DinoState::Attack);
        return;
    }

    // Lost the player (too far)
    if (DistToPlayer > Stats.AggroRadius * 2.0f)
    {
        CurrentTarget = nullptr;
        SetCombatState(ECombat_DinoState::Alert);
    }
}

void ADinosaurCombatAI::UpdateAttackState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetCombatState(ECombat_DinoState::Patrol);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    // Target moved out of attack range — resume chase
    if (DistToTarget > Stats.AttackRange * 1.5f)
    {
        SetCombatState(ECombat_DinoState::Chase);
        return;
    }

    // Perform attack if cooldown elapsed
    if (TimeSinceLastAttack >= Stats.AttackCooldown)
    {
        TimeSinceLastAttack = 0.0f;
        // Damage is applied via Blueprint or GameMode — this is the trigger point
        UGameplayStatics::ApplyDamage(CurrentTarget, Stats.DamagePerHit, nullptr, this, nullptr);
    }
}

void ADinosaurCombatAI::UpdateRetreatState(float DeltaTime)
{
    // Move away from player — handled by movement component in Blueprint
    // After 10 seconds of retreat, transition to dead-stop (exhausted)
    if (StateTimer > 10.0f)
    {
        SetCombatState(ECombat_DinoState::Patrol);
    }
}
