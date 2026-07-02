// RaptorPackBehavior.cpp
// Agent #12 — Combat & Enemy AI Agent
// Full raptor pack coordination implementation

#include "RaptorPackBehavior.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

URaptorPackBehaviorComponent::URaptorPackBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz — sufficient for AI
}

void URaptorPackBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // PackLeader auto-discovers pack members with same tag
    if (Role == ECombat_RaptorRole::PackLeader)
    {
        TArray<AActor*> TaggedActors;
        UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("RaptorPackAI"), TaggedActors);
        for (AActor* A : TaggedActors)
        {
            if (A == GetOwner()) continue;
            URaptorPackBehaviorComponent* Comp = A->FindComponentByClass<URaptorPackBehaviorComponent>();
            if (Comp)
            {
                PackMembers.Add(Comp);
            }
        }
        UE_LOG(LogTemp, Log, TEXT("[RaptorPack] PackLeader found %d pack members"), PackMembers.Num());
    }
}

void URaptorPackBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Cooldown timers
    if (bAttackCooldownActive)
    {
        AttackCooldownTimer -= DeltaTime;
        if (AttackCooldownTimer <= 0.f)
        {
            bAttackCooldownActive = false;
        }
    }

    switch (PackState)
    {
        case ECombat_PackState::Idle:       TickIdle(DeltaTime);       break;
        case ECombat_PackState::Stalking:   TickStalking(DeltaTime);   break;
        case ECombat_PackState::Flanking:   TickFlanking(DeltaTime);   break;
        case ECombat_PackState::Attacking:  TickAttacking(DeltaTime);  break;
        case ECombat_PackState::Retreating: TickRetreating(DeltaTime); break;
    }
}

// ─── State: Idle ─────────────────────────────────────────────────────────────
void URaptorPackBehaviorComponent::TickIdle(float DeltaTime)
{
    // Only PackLeader or solo raptors scan for targets
    if (Role == ECombat_RaptorRole::PackLeader || PackMembers.Num() == 0)
    {
        ScanForPlayer();
    }
}

// ─── State: Stalking ─────────────────────────────────────────────────────────
void URaptorPackBehaviorComponent::TickStalking(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_PackState::Idle);
        return;
    }

    LastKnownTargetLocation = CurrentTarget->GetActorLocation();
    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), LastKnownTargetLocation);

    // PackLeader: when close enough, initiate flanking formation
    if (Role == ECombat_RaptorRole::PackLeader && PackMembers.Num() > 0)
    {
        if (DistToTarget < Config.DetectionRange * 0.6f)
        {
            // Signal pack to begin flanking
            for (URaptorPackBehaviorComponent* Member : PackMembers)
            {
                if (Member)
                {
                    Member->SetPackTarget(CurrentTarget);
                }
            }
            TransitionToState(ECombat_PackState::Flanking);
            return;
        }
    }

    // Move toward target at stalk speed
    MoveTowardsLocation(LastKnownTargetLocation, Config.StalkSpeed, DeltaTime);

    // Lost sight — return to idle
    if (!IsTargetInSight(CurrentTarget) && DistToTarget > Config.DetectionRange * 1.2f)
    {
        CurrentTarget = nullptr;
        TransitionToState(ECombat_PackState::Idle);
    }
}

// ─── State: Flanking ─────────────────────────────────────────────────────────
void URaptorPackBehaviorComponent::TickFlanking(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_PackState::Retreating);
        return;
    }

    LastKnownTargetLocation = CurrentTarget->GetActorLocation();

    // Calculate this raptor's flank position based on role
    FVector FlankPos = CalculateFlankPosition(CurrentTarget);
    float DistToFlank = FVector::Dist(GetOwner()->GetActorLocation(), FlankPos);

    if (DistToFlank > Config.FlankPositionTolerance)
    {
        // Move to flank position at chase speed
        MoveTowardsLocation(FlankPos, Config.ChaseSpeed, DeltaTime);
        bFlankPositionReached = false;
    }
    else
    {
        bFlankPositionReached = true;
    }

    // PackLeader: check if all flankers are in position, then sync attack
    if (Role == ECombat_RaptorRole::PackLeader)
    {
        bool bAllInPosition = bFlankPositionReached;
        for (URaptorPackBehaviorComponent* Member : PackMembers)
        {
            if (Member && !Member->bFlankPositionReached)
            {
                bAllInPosition = false;
                break;
            }
        }

        if (bAllInPosition)
        {
            SyncAttackTimer += DeltaTime;
            if (SyncAttackTimer >= Config.SyncAttackDelay)
            {
                SyncAttackTimer = 0.f;
                BroadcastSyncAttack();
            }
        }
    }

    // Individual raptor: if sync signal received, attack
    if (bSyncAttackSignalReceived)
    {
        bSyncAttackSignalReceived = false;
        TransitionToState(ECombat_PackState::Attacking);
    }
}

// ─── State: Attacking ────────────────────────────────────────────────────────
void URaptorPackBehaviorComponent::TickAttacking(float DeltaTime)
{
    if (!CurrentTarget)
    {
        TransitionToState(ECombat_PackState::Retreating);
        return;
    }

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= Config.AttackRange)
    {
        // In range — perform attack if cooldown expired
        if (!bAttackCooldownActive)
        {
            PerformAttack();
        }
    }
    else
    {
        // Chase at full speed
        MoveTowardsLocation(CurrentTarget->GetActorLocation(), Config.ChaseSpeed, DeltaTime);

        // If too far, retreat
        if (DistToTarget > Config.DetectionRange * 1.5f)
        {
            TransitionToState(ECombat_PackState::Retreating);
        }
    }
}

// ─── State: Retreating ───────────────────────────────────────────────────────
void URaptorPackBehaviorComponent::TickRetreating(float DeltaTime)
{
    // Move away from last known target location
    if (!LastKnownTargetLocation.IsZero())
    {
        FVector AwayDir = (GetOwner()->GetActorLocation() - LastKnownTargetLocation).GetSafeNormal();
        FVector RetreatTarget = GetOwner()->GetActorLocation() + AwayDir * 1500.f;
        MoveTowardsLocation(RetreatTarget, Config.StalkSpeed, DeltaTime);
    }

    // After retreating far enough, go idle
    if (!CurrentTarget || FVector::Dist(GetOwner()->GetActorLocation(), LastKnownTargetLocation) > Config.DetectionRange * 1.8f)
    {
        CurrentTarget = nullptr;
        TransitionToState(ECombat_PackState::Idle);
    }
}

// ─── Public API ──────────────────────────────────────────────────────────────
void URaptorPackBehaviorComponent::SetPackTarget(AActor* Target)
{
    CurrentTarget = Target;
    if (Target)
    {
        LastKnownTargetLocation = Target->GetActorLocation();
        TransitionToState(ECombat_PackState::Stalking);
    }
}

void URaptorPackBehaviorComponent::ReceiveSyncAttackSignal()
{
    bSyncAttackSignalReceived = true;
    UE_LOG(LogTemp, Log, TEXT("[RaptorPack] %s received sync attack signal"), *GetOwner()->GetName());
}

void URaptorPackBehaviorComponent::BroadcastSyncAttack()
{
    // Signal all pack members to attack simultaneously
    for (URaptorPackBehaviorComponent* Member : PackMembers)
    {
        if (Member)
        {
            Member->ReceiveSyncAttackSignal();
        }
    }
    // Leader also attacks
    ReceiveSyncAttackSignal();
    UE_LOG(LogTemp, Log, TEXT("[RaptorPack] PackLeader broadcast sync attack to %d members"), PackMembers.Num());
}

FVector URaptorPackBehaviorComponent::CalculateFlankPosition(AActor* Target) const
{
    if (!Target) return GetOwner()->GetActorLocation();

    FVector TargetLoc = Target->GetActorLocation();
    FVector TargetForward = Target->GetActorForwardVector();
    FVector TargetRight = Target->GetActorRightVector();

    switch (Role)
    {
        case ECombat_RaptorRole::Distractor:
            // Distractor goes directly in front of target
            return TargetLoc + TargetForward * Config.FlankRadius;

        case ECombat_RaptorRole::FlankerLeft:
            // Left flanker goes to the left-rear
            return TargetLoc - TargetForward * (Config.FlankRadius * 0.5f) - TargetRight * Config.FlankRadius;

        case ECombat_RaptorRole::FlankerRight:
            // Right flanker goes to the right-rear
            return TargetLoc - TargetForward * (Config.FlankRadius * 0.5f) + TargetRight * Config.FlankRadius;

        case ECombat_RaptorRole::PackLeader:
            // Leader stays at detection distance, coordinates
            return TargetLoc + TargetForward * (Config.FlankRadius * 1.5f);

        default:
            return TargetLoc;
    }
}

void URaptorPackBehaviorComponent::PerformAttack()
{
    if (!CurrentTarget) return;

    float DistToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget > Config.AttackRange) return;

    // Apply damage via UE5 damage system
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        Config.AttackDamage,
        nullptr,
        GetOwner(),
        nullptr
    );

    // Start cooldown
    bAttackCooldownActive = true;
    AttackCooldownTimer = Config.AttackCooldown;

    UE_LOG(LogTemp, Log, TEXT("[RaptorPack] %s attacked %s for %.0f damage"),
        *GetOwner()->GetName(), *CurrentTarget->GetName(), Config.AttackDamage);
}

bool URaptorPackBehaviorComponent::IsTargetInSight(AActor* Target) const
{
    if (!Target || !GetWorld()) return false;

    FVector Start = GetOwner()->GetActorLocation() + FVector(0, 0, 50.f);
    FVector End = Target->GetActorLocation() + FVector(0, 0, 50.f);

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, Params);

    // If we hit something that isn't the target, line of sight is blocked
    if (bHit && HitResult.GetActor() != Target)
    {
        return false;
    }
    return true;
}

// ─── Private Helpers ─────────────────────────────────────────────────────────
void URaptorPackBehaviorComponent::TransitionToState(ECombat_PackState NewState)
{
    if (PackState == NewState) return;
    UE_LOG(LogTemp, Verbose, TEXT("[RaptorPack] %s: %d -> %d"), *GetOwner()->GetName(), (int)PackState, (int)NewState);
    PackState = NewState;
    bFlankPositionReached = false;
    SyncAttackTimer = 0.f;
}

void URaptorPackBehaviorComponent::ScanForPlayer()
{
    if (!GetWorld()) return;

    ACharacter* PlayerChar = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerChar) return;

    float DistToPlayer = FVector::Dist(GetOwner()->GetActorLocation(), PlayerChar->GetActorLocation());
    if (DistToPlayer <= Config.DetectionRange && IsTargetInSight(PlayerChar))
    {
        SetPackTarget(PlayerChar);
    }
}

void URaptorPackBehaviorComponent::MoveTowardsLocation(const FVector& TargetLocation, float Speed, float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner) return;

    FVector CurrentLoc = Owner->GetActorLocation();
    FVector Direction = (TargetLocation - CurrentLoc).GetSafeNormal();
    FVector NewLoc = CurrentLoc + Direction * Speed * DeltaTime;

    Owner->SetActorLocation(NewLoc, true);

    // Face movement direction
    if (!Direction.IsNearlyZero())
    {
        FRotator NewRot = Direction.Rotation();
        Owner->SetActorRotation(NewRot);
    }
}
