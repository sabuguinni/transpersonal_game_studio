#include "Combat_AttackSystem.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "CollisionQueryParams.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

UCombat_AttackSystem::UCombat_AttackSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance

    // Default attack configuration
    FCombat_AttackData DefaultBite;
    DefaultBite.Damage = 35.0f;
    DefaultBite.Range = 150.0f;
    DefaultBite.Cooldown = 2.5f;
    DefaultBite.WindupTime = 0.8f;
    DefaultBite.AttackType = ECombat_AttackType::Bite;
    DefaultBite.bRequiresLineOfSight = true;

    FCombat_AttackData DefaultClaw;
    DefaultClaw.Damage = 20.0f;
    DefaultClaw.Range = 120.0f;
    DefaultClaw.Cooldown = 1.5f;
    DefaultClaw.WindupTime = 0.4f;
    DefaultClaw.AttackType = ECombat_AttackType::Claw;
    DefaultClaw.bRequiresLineOfSight = true;

    AvailableAttacks.Add(DefaultBite);
    AvailableAttacks.Add(DefaultClaw);

    DetectionRange = 800.0f;
    AggroRange = 600.0f;
    bIsAttacking = false;
    bCanAttack = true;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    CurrentAttackIndex = 0;
}

void UCombat_AttackSystem::BeginPlay()
{
    Super::BeginPlay();
    
    LastAttackTime = GetWorld()->GetTimeSeconds();
}

void UCombat_AttackSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!GetOwner())
    {
        return;
    }

    UpdateTargetTracking(DeltaTime);

    // Auto-attack if we have a valid target and can attack
    if (CurrentTarget && bCanAttack && !bIsAttacking)
    {
        if (CanAttackTarget(CurrentTarget))
        {
            TryAttack(CurrentTarget);
        }
    }
}

bool UCombat_AttackSystem::TryAttack(AActor* Target)
{
    if (!Target || !bCanAttack || bIsAttacking)
    {
        return false;
    }

    if (!CanAttackTarget(Target))
    {
        return false;
    }

    FCombat_AttackData BestAttack = GetBestAttackForTarget(Target);
    
    bIsAttacking = true;
    bCanAttack = false;
    SetCurrentTarget(Target);

    // Start windup timer
    if (BestAttack.WindupTime > 0.0f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            AttackWindupTimer,
            [this, BestAttack, Target]()
            {
                ExecuteAttack(BestAttack, Target);
            },
            BestAttack.WindupTime,
            false
        );
    }
    else
    {
        ExecuteAttack(BestAttack, Target);
    }

    OnAttackStarted(BestAttack, Target);
    return true;
}

bool UCombat_AttackSystem::CanAttackTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return false;
    }

    float Distance = GetDistanceToTarget(Target);
    
    // Check if any attack can reach the target
    bool bInRange = false;
    for (const FCombat_AttackData& Attack : AvailableAttacks)
    {
        if (Distance <= Attack.Range)
        {
            bInRange = true;
            break;
        }
    }

    if (!bInRange)
    {
        return false;
    }

    // Check line of sight if required
    FCombat_AttackData BestAttack = GetBestAttackForTarget(Target);
    if (BestAttack.bRequiresLineOfSight && !HasLineOfSight(Target))
    {
        return false;
    }

    return true;
}

void UCombat_AttackSystem::ExecuteAttack(const FCombat_AttackData& AttackData, AActor* Target)
{
    if (!Target || !GetOwner())
    {
        bIsAttacking = false;
        return;
    }

    // Final validation before hit
    if (!CanAttackTarget(Target))
    {
        OnAttackMissed(AttackData, Target);
        bIsAttacking = false;
        StartAttackCooldown(AttackData.Cooldown);
        return;
    }

    ProcessAttackHit(AttackData, Target);
    OnAttackHit(AttackData, Target, AttackData.Damage);

    bIsAttacking = false;
    StartAttackCooldown(AttackData.Cooldown);
    LastAttackTime = GetWorld()->GetTimeSeconds();
}

void UCombat_AttackSystem::StartAttackCooldown(float CooldownDuration)
{
    bCanAttack = false;
    
    GetWorld()->GetTimerManager().SetTimer(
        AttackCooldownTimer,
        this,
        &UCombat_AttackSystem::OnAttackCooldownComplete,
        CooldownDuration,
        false
    );
}

AActor* UCombat_AttackSystem::FindNearestTarget()
{
    if (!GetOwner())
    {
        return nullptr;
    }

    AActor* NearestTarget = nullptr;
    float NearestDistance = DetectionRange;

    UWorld* World = GetWorld();
    if (!World)
    {
        return nullptr;
    }

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    // Find player characters within detection range
    for (FConstPlayerControllerIterator Iterator = World->GetPlayerControllerIterator(); Iterator; ++Iterator)
    {
        APlayerController* PC = Iterator->Get();
        if (PC && PC->GetPawn())
        {
            APawn* PlayerPawn = PC->GetPawn();
            float Distance = FVector::Dist(OwnerLocation, PlayerPawn->GetActorLocation());
            
            if (Distance <= DetectionRange && Distance < NearestDistance)
            {
                if (HasLineOfSight(PlayerPawn))
                {
                    NearestTarget = PlayerPawn;
                    NearestDistance = Distance;
                }
            }
        }
    }

    return NearestTarget;
}

float UCombat_AttackSystem::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return FLT_MAX;
    }

    return FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
}

bool UCombat_AttackSystem::HasLineOfSight(AActor* Target) const
{
    if (!Target || !GetOwner())
    {
        return false;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }

    FVector StartLocation = GetOwner()->GetActorLocation();
    FVector EndLocation = Target->GetActorLocation();
    
    // Adjust for eye height
    StartLocation.Z += 100.0f;
    EndLocation.Z += 100.0f;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());
    QueryParams.AddIgnoredActor(Target);
    QueryParams.bTraceComplex = false;

    FHitResult HitResult;
    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return !bHit;
}

void UCombat_AttackSystem::SetCurrentTarget(AActor* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        AActor* OldTarget = CurrentTarget;
        CurrentTarget = NewTarget;

        if (OldTarget)
        {
            OnTargetLost(OldTarget);
        }

        if (NewTarget)
        {
            OnTargetAcquired(NewTarget);
        }
    }
}

FCombat_AttackData UCombat_AttackSystem::GetBestAttackForTarget(AActor* Target) const
{
    if (!Target || AvailableAttacks.Num() == 0)
    {
        return FCombat_AttackData();
    }

    float Distance = GetDistanceToTarget(Target);
    
    // Find the most damaging attack that can reach the target
    FCombat_AttackData BestAttack;
    float BestDamage = 0.0f;
    bool bFoundValidAttack = false;

    for (const FCombat_AttackData& Attack : AvailableAttacks)
    {
        if (Distance <= Attack.Range)
        {
            if (!bFoundValidAttack || Attack.Damage > BestDamage)
            {
                BestAttack = Attack;
                BestDamage = Attack.Damage;
                bFoundValidAttack = true;
            }
        }
    }

    // If no attack can reach, return the longest range attack
    if (!bFoundValidAttack && AvailableAttacks.Num() > 0)
    {
        BestAttack = AvailableAttacks[0];
        for (const FCombat_AttackData& Attack : AvailableAttacks)
        {
            if (Attack.Range > BestAttack.Range)
            {
                BestAttack = Attack;
            }
        }
    }

    return BestAttack;
}

void UCombat_AttackSystem::OnAttackCooldownComplete()
{
    bCanAttack = true;
}

void UCombat_AttackSystem::OnAttackWindupComplete()
{
    // This is handled by lambda in TryAttack
}

void UCombat_AttackSystem::ProcessAttackHit(const FCombat_AttackData& AttackData, AActor* Target)
{
    if (!Target)
    {
        return;
    }

    // Apply damage if target has health component
    // For now, just log the hit
    UE_LOG(LogTemp, Warning, TEXT("Combat Attack Hit: %s dealt %.1f %s damage to %s"), 
        *GetOwner()->GetName(),
        AttackData.Damage,
        *UEnum::GetValueAsString(AttackData.AttackType),
        *Target->GetName()
    );

    // TODO: Integrate with health/damage system when available
}

bool UCombat_AttackSystem::IsValidTarget(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }

    // Check if target is a player pawn
    APawn* TargetPawn = Cast<APawn>(Target);
    if (TargetPawn && TargetPawn->IsPlayerControlled())
    {
        return true;
    }

    return false;
}

void UCombat_AttackSystem::UpdateTargetTracking(float DeltaTime)
{
    if (!GetOwner())
    {
        return;
    }

    // If we don't have a target, try to find one
    if (!CurrentTarget)
    {
        AActor* NewTarget = FindNearestTarget();
        if (NewTarget)
        {
            SetCurrentTarget(NewTarget);
        }
        return;
    }

    // Check if current target is still valid
    if (!IsValidTarget(CurrentTarget))
    {
        SetCurrentTarget(nullptr);
        return;
    }

    float Distance = GetDistanceToTarget(CurrentTarget);
    
    // Lose target if they're too far away
    if (Distance > DetectionRange * 1.2f) // 20% hysteresis
    {
        SetCurrentTarget(nullptr);
        return;
    }

    // Lose target if we can't see them for too long
    if (!HasLineOfSight(CurrentTarget))
    {
        // TODO: Add timer for losing sight
        // For now, immediately lose target
        SetCurrentTarget(nullptr);
        return;
    }
}