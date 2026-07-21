// RaptorPackAI.cpp
// Agent #12 — Combat & Enemy AI Agent
// Raptor pack coordination — flanking AI, role-based attack, pack flee logic
// CYCLE: PROD_CYCLE_AUTO_20260629_007

#include "RaptorPackAI.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

URaptorPackAI::URaptorPackAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz tick — sufficient for pack AI
    AliveCount = 0;
}

void URaptorPackAI::BeginPlay()
{
    Super::BeginPlay();

    // Start periodic scan for player target
    GetWorld()->GetTimerManager().SetTimer(
        ScanTimerHandle,
        this,
        &URaptorPackAI::ScanForTarget,
        0.5f,   // Scan every 0.5s
        true
    );

    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Initialized. Pack size: %d"), PackMembers.Num());
}

void URaptorPackAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    TimeSinceLastAttack += DeltaTime;

    // State machine update
    switch (PackState)
    {
    case ECombat_PackState::Stalking:
        // Move pack toward target quietly
        if (TargetPawn && IsValid(TargetPawn))
        {
            float Dist = FVector::Dist(GetOwner()->GetActorLocation(), TargetPawn->GetActorLocation());
            if (Dist < DetectionRange * 0.4f)
            {
                // Close enough — coordinate attack
                SetPackState(ECombat_PackState::Coordinating);
                BuildAttackPlan();
            }
        }
        else
        {
            SetPackState(ECombat_PackState::Idle);
        }
        break;

    case ECombat_PackState::Attacking:
        if (!TargetPawn || !IsValid(TargetPawn))
        {
            SetPackState(ECombat_PackState::Idle);
        }
        else if (ShouldPackFlee())
        {
            ForcePackFlee();
        }
        break;

    case ECombat_PackState::Regrouping:
        // After 3 seconds regrouping, resume stalking if target still exists
        break;

    default:
        break;
    }

#if WITH_EDITOR
    // Debug visualization in editor
    if (TargetPawn && IsValid(TargetPawn) && PackState != ECombat_PackState::Idle)
    {
        DrawDebugSphere(
            GetWorld(),
            TargetPawn->GetActorLocation(),
            80.0f,
            12,
            FColor::Red,
            false,
            0.15f
        );
        for (const FCombat_RaptorMember& Member : PackMembers)
        {
            if (Member.bIsAlive && Member.RaptorActor && IsValid(Member.RaptorActor))
            {
                FColor RoleColor = (Member.Role == ECombat_RaptorRole::Alpha) ? FColor::Orange :
                                   (Member.Role == ECombat_RaptorRole::Flanker) ? FColor::Cyan :
                                   FColor::Yellow;
                DrawDebugLine(
                    GetWorld(),
                    Member.RaptorActor->GetActorLocation(),
                    TargetPawn->GetActorLocation(),
                    RoleColor,
                    false,
                    0.15f,
                    0,
                    2.0f
                );
            }
        }
    }
#endif
}

void URaptorPackAI::RegisterPackMember(AActor* RaptorActor, ECombat_RaptorRole Role)
{
    if (!RaptorActor || !IsValid(RaptorActor))
    {
        UE_LOG(LogTemp, Warning, TEXT("[RaptorPackAI] RegisterPackMember: invalid actor"));
        return;
    }

    if (PackMembers.Num() >= MaxPackSize)
    {
        UE_LOG(LogTemp, Warning, TEXT("[RaptorPackAI] Pack full (%d/%d)"), PackMembers.Num(), MaxPackSize);
        return;
    }

    FCombat_RaptorMember NewMember;
    NewMember.RaptorActor = RaptorActor;
    NewMember.Role = Role;
    NewMember.Health = 100.0f;
    NewMember.bIsAlive = true;
    NewMember.bIsAttacking = false;

    PackMembers.Add(NewMember);
    UpdateAliveCount();

    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Registered raptor '%s' as %s. Pack: %d/%d"),
        *RaptorActor->GetName(),
        *UEnum::GetValueAsString(Role),
        PackMembers.Num(),
        MaxPackSize);
}

void URaptorPackAI::SetTarget(AActor* NewTarget)
{
    if (NewTarget == TargetPawn) return;

    TargetPawn = NewTarget;

    if (TargetPawn && IsValid(TargetPawn))
    {
        UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Target acquired: %s"), *TargetPawn->GetName());
        SetPackState(ECombat_PackState::Stalking);
    }
    else
    {
        SetPackState(ECombat_PackState::Idle);
    }
}

void URaptorPackAI::NotifyRaptorDamaged(AActor* RaptorActor, float DamageAmount)
{
    if (!RaptorActor) return;

    for (FCombat_RaptorMember& Member : PackMembers)
    {
        if (Member.RaptorActor == RaptorActor && Member.bIsAlive)
        {
            Member.Health = FMath::Max(0.0f, Member.Health - DamageAmount);

            UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Raptor '%s' damaged: %.1f HP remaining"),
                *RaptorActor->GetName(), Member.Health);

            if (Member.Health <= 0.0f)
            {
                NotifyRaptorDead(RaptorActor);
            }
            else if (Member.Health < RetreatHealthThreshold)
            {
                // Injured raptor retreats
                Member.Role = ECombat_RaptorRole::Retreating;
                Member.bIsAttacking = false;
                UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Raptor '%s' retreating (HP: %.1f)"),
                    *RaptorActor->GetName(), Member.Health);
            }

            // Pack regroups if attacked
            if (PackState == ECombat_PackState::Attacking && ShouldPackFlee())
            {
                ForcePackFlee();
            }
            else if (PackState == ECombat_PackState::Attacking)
            {
                SetPackState(ECombat_PackState::Regrouping);
                // Resume attack after brief regroup
                FTimerHandle RegroupTimer;
                GetWorld()->GetTimerManager().SetTimer(
                    RegroupTimer,
                    [this]() {
                        if (TargetPawn && IsValid(TargetPawn) && !ShouldPackFlee())
                        {
                            SetPackState(ECombat_PackState::Attacking);
                            ExecuteAttackPlan();
                        }
                    },
                    2.0f,
                    false
                );
            }
            break;
        }
    }
}

void URaptorPackAI::NotifyRaptorDead(AActor* RaptorActor)
{
    if (!RaptorActor) return;

    for (FCombat_RaptorMember& Member : PackMembers)
    {
        if (Member.RaptorActor == RaptorActor && Member.bIsAlive)
        {
            Member.bIsAlive = false;
            Member.bIsAttacking = false;
            UpdateAliveCount();

            OnRaptorKilled.Broadcast(RaptorActor);

            UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Raptor '%s' killed. Alive: %d"),
                *RaptorActor->GetName(), AliveCount);

            if (ShouldPackFlee())
            {
                ForcePackFlee();
            }
            break;
        }
    }
}

void URaptorPackAI::ForcePackFlee()
{
    SetPackState(ECombat_PackState::Fleeing);
    OnPackFled.Broadcast(AliveCount);

    // Mark all living members as retreating
    for (FCombat_RaptorMember& Member : PackMembers)
    {
        if (Member.bIsAlive)
        {
            Member.Role = ECombat_RaptorRole::Retreating;
            Member.bIsAttacking = false;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Pack fleeing! Survivors: %d"), AliveCount);
}

void URaptorPackAI::SetPackState(ECombat_PackState NewState)
{
    if (NewState == PackState) return;

    ECombat_PackState OldState = PackState;
    PackState = NewState;
    OnPackStateChanged.Broadcast(OldState, NewState);

    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] State: %s -> %s"),
        *UEnum::GetValueAsString(OldState),
        *UEnum::GetValueAsString(NewState));
}

void URaptorPackAI::BuildAttackPlan()
{
    if (!TargetPawn || !IsValid(TargetPawn)) return;

    FVector TargetLoc = TargetPawn->GetActorLocation();
    FVector OwnerLoc = GetOwner()->GetActorLocation();
    FVector ToTarget = (TargetLoc - OwnerLoc).GetSafeNormal();
    FVector Right = FVector::CrossProduct(ToTarget, FVector::UpVector);

    CurrentAttackPlan.AlphaChargeDirection = ToTarget;
    CurrentAttackPlan.LeftFlankPosition = TargetLoc + Right * FlankRadius;
    CurrentAttackPlan.RightFlankPosition = TargetLoc - Right * FlankRadius;
    CurrentAttackPlan.CoordinationDelay = CoordinationDelay;
    CurrentAttackPlan.bPlanReady = true;

    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Attack plan built. Alpha charges, flankers at +/-%.0fu"),
        FlankRadius);

    // Assign flanker positions to pack members
    int32 FlankIndex = 0;
    for (FCombat_RaptorMember& Member : PackMembers)
    {
        if (!Member.bIsAlive) continue;

        if (Member.Role == ECombat_RaptorRole::Flanker)
        {
            Member.AssignedFlankPosition = (FlankIndex % 2 == 0)
                ? CurrentAttackPlan.LeftFlankPosition
                : CurrentAttackPlan.RightFlankPosition;
            FlankIndex++;
        }
        else if (Member.Role == ECombat_RaptorRole::Alpha)
        {
            Member.AssignedFlankPosition = TargetLoc; // Alpha goes direct
        }
    }

    // Execute after coordination delay
    GetWorld()->GetTimerManager().SetTimer(
        CoordinationTimerHandle,
        this,
        &URaptorPackAI::ExecuteAttackPlan,
        CoordinationDelay,
        false
    );

    SetPackState(ECombat_PackState::Attacking);
}

void URaptorPackAI::ExecuteAttackPlan()
{
    if (!CurrentAttackPlan.bPlanReady) return;
    if (!TargetPawn || !IsValid(TargetPawn)) return;

    UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] Executing attack plan — %d raptors attacking"), AliveCount);

    for (FCombat_RaptorMember& Member : PackMembers)
    {
        if (!Member.bIsAlive || Member.Role == ECombat_RaptorRole::Retreating) continue;

        Member.bIsAttacking = true;

        // Apply damage if raptor is in attack range
        if (Member.RaptorActor && IsValid(Member.RaptorActor))
        {
            float DistToTarget = FVector::Dist(
                Member.RaptorActor->GetActorLocation(),
                TargetPawn->GetActorLocation()
            );

            if (DistToTarget <= AttackRange)
            {
                float Damage = (Member.Role == ECombat_RaptorRole::Alpha)
                    ? AlphaAttackDamage
                    : FlankAttackDamage;

                UGameplayStatics::ApplyDamage(
                    TargetPawn,
                    Damage,
                    nullptr,
                    Member.RaptorActor,
                    nullptr
                );

                UE_LOG(LogTemp, Log, TEXT("[RaptorPackAI] %s raptor dealt %.1f damage to %s"),
                    *UEnum::GetValueAsString(Member.Role),
                    Damage,
                    *TargetPawn->GetName());
            }
        }
    }
}

void URaptorPackAI::ScanForTarget()
{
    if (PackState != ECombat_PackState::Idle) return;
    if (AliveCount == 0) return;

    // Find player pawn
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    float Dist = FVector::Dist(GetOwner()->GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist <= DetectionRange)
    {
        SetTarget(PlayerPawn);
    }
}

void URaptorPackAI::UpdateAliveCount()
{
    AliveCount = 0;
    for (const FCombat_RaptorMember& Member : PackMembers)
    {
        if (Member.bIsAlive) AliveCount++;
    }
}

bool URaptorPackAI::ShouldPackFlee() const
{
    if (PackMembers.Num() == 0) return false;
    float DeadRatio = 1.0f - (float(AliveCount) / float(PackMembers.Num()));
    return DeadRatio >= PackFleeThreshold;
}
