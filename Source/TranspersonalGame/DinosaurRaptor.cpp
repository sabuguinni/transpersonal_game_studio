// DinosaurRaptor.cpp
// Velociraptor — pack predator, fast flanker, jump attacker
// Agent #04 Performance Optimizer — Cycle PROD_CYCLE_AUTO_20260627_003

#include "DinosaurRaptor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

AVelociraptorDinosaur::AVelociraptorDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Capsule (raptor silhouette: lean and fast) ───────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(35.0f, 80.0f);

    // ── Movement: fast, agile, no jump penalty ───────────────────────────────
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 900.0f;          // Fast sprint
        MoveComp->MaxAcceleration = 3000.0f;      // Rapid direction change
        MoveComp->BrakingDecelerationWalking = 2000.0f;
        MoveComp->JumpZVelocity = 600.0f;         // Raptors can leap
        MoveComp->AirControl = 0.4f;              // Some air steering
        MoveComp->GravityScale = 1.2f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // Snappy turning
    }

    // ── Dinosaur base stats ──────────────────────────────────────────────────
    // (DinosaurBase properties — set here if they exist on base class)
    // Health = 400, AttackDamage = 60 — set via UPROPERTY defaults on base

    // ── Pack defaults ────────────────────────────────────────────────────────
    PackRole = 0;          // Default: leader (overridden when joining a pack)
    PackLeader = nullptr;
    MaxPackSize = 3;

    // ── Combat defaults ──────────────────────────────────────────────────────
    JumpAttackDamage = 80.0f;
    JumpAttackRange = 300.0f;
    JumpAttackCooldown = 4.0f;
    bIsJumping = false;
    JumpAttackTimer = 0.0f;

    // ── Flanking defaults ────────────────────────────────────────────────────
    FlankOffset = FVector::ZeroVector;
    CoordinationRadius = 1500.0f;
}

void AVelociraptorDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // If we have a leader, register with them
    if (PackLeader && PackLeader != this)
    {
        PackLeader->RegisterPackMember(this);
    }
}

void AVelociraptorDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Tick down jump attack cooldown
    if (JumpAttackTimer > 0.0f)
    {
        JumpAttackTimer -= DeltaTime;
    }

#if WITH_EDITOR
    // Debug: draw coordination radius in editor
    if (PackRole == 0) // Leader only
    {
        DrawDebugSphere(
            GetWorld(),
            GetActorLocation(),
            CoordinationRadius,
            16,
            FColor::Yellow,
            false,
            -1.0f,
            0,
            1.0f
        );
    }
    // Draw flank offset target
    if (FlankOffset != FVector::ZeroVector)
    {
        DrawDebugSphere(
            GetWorld(),
            GetFlankPosition(),
            40.0f,
            8,
            FColor::Cyan,
            false,
            -1.0f,
            0,
            1.5f
        );
    }
#endif
}

void AVelociraptorDinosaur::PerformJumpAttack(AActor* Target)
{
    if (!Target || bIsJumping || JumpAttackTimer > 0.0f)
    {
        return;
    }

    FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
    float Distance = ToTarget.Size();

    if (Distance > JumpAttackRange)
    {
        return; // Out of range
    }

    bIsJumping = true;
    JumpAttackTimer = JumpAttackCooldown;

    // Launch toward target with upward arc
    ToTarget.Normalize();
    FVector LaunchVelocity = ToTarget * 800.0f + FVector(0.0f, 0.0f, 400.0f);
    LaunchCharacter(LaunchVelocity, true, true);

    // Apply damage if close enough
    if (Distance <= JumpAttackRange * 0.5f)
    {
        UGameplayStatics::ApplyDamage(
            Target,
            JumpAttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );
    }

    // Reset jump state after 1.2 seconds
    GetWorldTimerManager().SetTimer(
        JumpResetTimer,
        this,
        &AVelociraptorDinosaur::ResetJumpState,
        1.2f,
        false
    );
}

void AVelociraptorDinosaur::CoordinateFlank(AActor* Target)
{
    if (!Target || PackRole != 0) // Only leader coordinates
    {
        return;
    }

    FVector TargetLoc = Target->GetActorLocation();
    FVector ToTarget = TargetLoc - GetActorLocation();
    ToTarget.Normalize();

    // Perpendicular flanking vectors
    FVector Right = FVector::CrossProduct(ToTarget, FVector::UpVector);
    Right.Normalize();

    // Assign flank positions to pack members
    // Member 0 (index 0): left flank
    // Member 1 (index 1): right flank
    // Leader: direct frontal approach
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (!PackMembers[i])
        {
            continue;
        }

        FVector Offset;
        if (i == 0)
        {
            // Left flanker: 90° left of target, 400cm out
            Offset = -Right * 400.0f + ToTarget * 200.0f;
        }
        else if (i == 1)
        {
            // Right flanker: 90° right of target, 400cm out
            Offset = Right * 400.0f + ToTarget * 200.0f;
        }
        else
        {
            // Additional members: rear pressure
            Offset = -ToTarget * 300.0f;
        }

        PackMembers[i]->FlankOffset = TargetLoc + Offset;
    }
}

bool AVelociraptorDinosaur::RegisterPackMember(AVelociraptorDinosaur* NewMember)
{
    if (!NewMember || PackMembers.Num() >= MaxPackSize - 1)
    {
        return false;
    }

    if (!PackMembers.Contains(NewMember))
    {
        PackMembers.Add(NewMember);
        // Assign role based on index
        NewMember->PackRole = PackMembers.Num(); // 1=Left, 2=Right, etc.
        NewMember->PackLeader = this;
        return true;
    }

    return false;
}

FVector AVelociraptorDinosaur::GetFlankPosition() const
{
    // If FlankOffset is set (by leader), return it
    // Otherwise return current location (no flanking target)
    if (FlankOffset != FVector::ZeroVector)
    {
        return FlankOffset;
    }
    return GetActorLocation();
}

void AVelociraptorDinosaur::ResetJumpState()
{
    bIsJumping = false;
}
