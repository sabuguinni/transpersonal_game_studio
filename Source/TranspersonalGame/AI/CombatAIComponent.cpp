// CombatAIComponent.cpp
// Combat & Enemy AI Agent #12 — Transpersonal Game Studio
// Full implementation of tactical dinosaur combat AI

#include "CombatAIComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UCombatAIComponent::UCombatAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick for AI — performance-friendly
    CurrentHealth = MaxHealth;
}

void UCombatAIComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
    CurrentState = ECombat_AIState::Patrolling;

    // Species-specific defaults
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        MaxHealth = 500.0f;
        CurrentHealth = 500.0f;
        AttackDamage = 80.0f;
        AttackRange = 350.0f;
        DetectionRadius = 2000.0f;
        ChaseSpeed = 500.0f;
        PatrolSpeed = 150.0f;
        bIsPackHunter = false;
        bCanAmbush = false;
        bFleesWhenLowHealth = false; // TRex never flees
        break;

    case ECombat_DinoSpecies::Velociraptor:
        MaxHealth = 80.0f;
        CurrentHealth = 80.0f;
        AttackDamage = 25.0f;
        AttackRange = 180.0f;
        DetectionRadius = 1200.0f;
        ChaseSpeed = 700.0f;
        PatrolSpeed = 300.0f;
        bIsPackHunter = true;
        bCanAmbush = true;
        AmbushTriggerDistance = 250.0f;
        FlankingAngle = 120.0f;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        MaxHealth = 800.0f;
        CurrentHealth = 800.0f;
        AttackDamage = 40.0f; // Stomp damage
        AttackRange = 400.0f;
        DetectionRadius = 800.0f;
        ChaseSpeed = 300.0f;
        PatrolSpeed = 100.0f;
        bIsPackHunter = false;
        bFleesWhenLowHealth = false;
        break;

    case ECombat_DinoSpecies::Triceratops:
        MaxHealth = 350.0f;
        CurrentHealth = 350.0f;
        AttackDamage = 55.0f;
        AttackRange = 280.0f;
        DetectionRadius = 900.0f;
        ChaseSpeed = 450.0f;
        PatrolSpeed = 180.0f;
        bIsPackHunter = false;
        bCanAmbush = false;
        break;

    case ECombat_DinoSpecies::Pterodactyl:
        MaxHealth = 60.0f;
        CurrentHealth = 60.0f;
        AttackDamage = 15.0f;
        AttackRange = 150.0f;
        DetectionRadius = 1500.0f;
        ChaseSpeed = 900.0f;
        PatrolSpeed = 400.0f;
        bIsPackHunter = true;
        bFleesWhenLowHealth = true;
        FleeHealthThreshold = 30.0f;
        break;

    default:
        break;
    }
}

void UCombatAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bIsDead) return;

    TimeSinceLastAttack += DeltaTime;

    UpdateCombatState(DeltaTime);

    if (CurrentState == ECombat_AIState::Attacking)
    {
        ProcessAttack(DeltaTime);
    }

    // Periodic threat scan when not already engaged
    if (CurrentState == ECombat_AIState::Idle || CurrentState == ECombat_AIState::Patrolling)
    {
        ScanForThreats();
    }
}

void UCombatAIComponent::UpdateCombatState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        if (CurrentState == ECombat_AIState::Hunting ||
            CurrentState == ECombat_AIState::Attacking ||
            CurrentState == ECombat_AIState::PackHunt)
        {
            SetState(ECombat_AIState::Patrolling);
        }
        return;
    }

    // Check if target is still alive/valid
    if (!IsValid(CurrentTarget))
    {
        ClearTarget();
        return;
    }

    // Flee if low health
    if (bFleesWhenLowHealth && CurrentHealth <= FleeHealthThreshold)
    {
        SetState(ECombat_AIState::Fleeing);
        return;
    }

    // Check distance to target
    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= AttackRange)
    {
        SetState(ECombat_AIState::Attacking);
    }
    else if (DistToTarget <= DetectionRadius)
    {
        if (bIsPackHunter && PackData.PackMembers.Num() > 1)
        {
            SetState(ECombat_AIState::PackHunt);
        }
        else
        {
            SetState(ECombat_AIState::Hunting);
        }
    }
    else
    {
        // Target out of detection range — lose interest
        ClearTarget();
    }
}

void UCombatAIComponent::ProcessAttack(float DeltaTime)
{
    if (!CurrentTarget || !IsValid(CurrentTarget)) return;
    if (TimeSinceLastAttack < AttackCooldown) return;
    if (!IsTargetInAttackRange()) return;

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        AttackDamage,
        GetOwner()->GetInstigatorController(),
        GetOwner(),
        nullptr
    );

    TimeSinceLastAttack = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: Attacked %s for %.1f damage"),
        *GetOwner()->GetActorLabel(),
        *CurrentTarget->GetActorLabel(),
        AttackDamage);
}

void UCombatAIComponent::ScanForThreats()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find player character as primary threat
    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(World, 0);
    if (!PlayerChar) return;

    float DistToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerChar->GetActorLocation());

    if (DistToPlayer <= DetectionRadius)
    {
        ECombat_ThreatLevel Threat = EvaluateThreat(PlayerChar);
        if (Threat != ECombat_ThreatLevel::None)
        {
            SetTarget(PlayerChar);
            SetState(ECombat_AIState::Alerted);

            // Alert pack members if pack hunter
            if (bIsPackHunter)
            {
                AlertPackMembers(PlayerChar);
            }
        }
    }
}

void UCombatAIComponent::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    ThreatLevel = (NewTarget != nullptr) ? ECombat_ThreatLevel::High : ECombat_ThreatLevel::None;
}

void UCombatAIComponent::ClearTarget()
{
    CurrentTarget = nullptr;
    ThreatLevel = ECombat_ThreatLevel::None;
    SetState(ECombat_AIState::Patrolling);
}

void UCombatAIComponent::TakeDamage_Combat(float DamageAmount, AActor* DamageInstigator)
{
    if (bIsDead) return;

    CurrentHealth = FMath::Max(0.0f, CurrentHealth - DamageAmount);

    UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: Took %.1f damage. HP: %.1f/%.1f"),
        *GetOwner()->GetActorLabel(),
        DamageAmount,
        CurrentHealth,
        MaxHealth);

    // Retaliate — set attacker as target
    if (DamageInstigator && IsValid(DamageInstigator))
    {
        SetTarget(DamageInstigator);
        SetState(ECombat_AIState::Attacking);
    }

    if (CurrentHealth <= 0.0f)
    {
        bIsDead = true;
        SetState(ECombat_AIState::Idle);
        UE_LOG(LogTemp, Log, TEXT("CombatAI [%s]: DEAD"), *GetOwner()->GetActorLabel());
    }
    else if (bFleesWhenLowHealth && CurrentHealth <= FleeHealthThreshold)
    {
        SetState(ECombat_AIState::Fleeing);
    }
}

void UCombatAIComponent::AlertPackMembers(AActor* Threat)
{
    if (!PackData.bIsCoordinating) return;

    for (AActor* Member : PackData.PackMembers)
    {
        if (!IsValid(Member) || Member == GetOwner()) continue;

        UCombatAIComponent* MemberAI = Member->FindComponentByClass<UCombatAIComponent>();
        if (MemberAI && !MemberAI->bIsDead)
        {
            MemberAI->SetTarget(Threat);
            MemberAI->SetState(ECombat_AIState::PackHunt);
        }
    }
}

void UCombatAIComponent::SetState(ECombat_AIState NewState)
{
    if (CurrentState == NewState) return;
    CurrentState = NewState;
}

ECombat_ThreatLevel UCombatAIComponent::EvaluateThreat(AActor* PotentialThreat) const
{
    if (!PotentialThreat || !IsValid(PotentialThreat)) return ECombat_ThreatLevel::None;

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PotentialThreat->GetActorLocation());

    if (Dist > DetectionRadius) return ECombat_ThreatLevel::None;
    if (Dist > DetectionRadius * 0.75f) return ECombat_ThreatLevel::Low;
    if (Dist > DetectionRadius * 0.5f) return ECombat_ThreatLevel::Medium;
    if (Dist > AttackRange * 2.0f) return ECombat_ThreatLevel::High;
    return ECombat_ThreatLevel::Lethal;
}

FVector UCombatAIComponent::GetFlankingPosition(int32 FlankIndex, int32 TotalFlankers) const
{
    if (!CurrentTarget || TotalFlankers <= 0) return FVector::ZeroVector;

    FVector TargetLoc = CurrentTarget->GetActorLocation();
    float AngleStep = 360.0f / TotalFlankers;
    float Angle = FMath::DegreesToRadians(FlankIndex * AngleStep + FlankingAngle);
    float FlankRadius = AttackRange * 1.5f;

    return TargetLoc + FVector(
        FMath::Cos(Angle) * FlankRadius,
        FMath::Sin(Angle) * FlankRadius,
        0.0f
    );
}

bool UCombatAIComponent::IsTargetInAttackRange() const
{
    if (!CurrentTarget || !IsValid(CurrentTarget)) return false;
    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return Dist <= AttackRange;
}

bool UCombatAIComponent::IsAlive() const
{
    return !bIsDead && CurrentHealth > 0.0f;
}

float UCombatAIComponent::GetHealthPercent() const
{
    if (MaxHealth <= 0.0f) return 0.0f;
    return CurrentHealth / MaxHealth;
}
