// RaptorCharacter.cpp
// Performance Optimizer #04 — PROD_CYCLE_AUTO_20260624_002
// Full implementation of ARaptorCharacter — pack-hunter Velociraptor

#include "Dinosaurs/RaptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
ARaptorCharacter::ARaptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Species stats ──────────────────────────────────────────────────
    Species         = EPerf_DinosaurSpecies::Velociraptor;
    MaxHealth       = 400.0f;
    CurrentHealth   = 400.0f;
    AttackDamage    = 60.0f;
    DetectionRange  = 2500.0f;
    AttackRange     = 180.0f;

    // ── Movement — fast pack hunter ────────────────────────────────────
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed          = 900.0f;   // Sprint speed
        Move->MaxAcceleration       = 2400.0f;
        Move->BrakingDecelerationWalking = 1800.0f;
        Move->JumpZVelocity         = 620.0f;   // Raptors can leap
        Move->AirControl            = 0.35f;
        Move->NavAgentProps.bCanJump = true;
    }

    // ── Pack defaults ──────────────────────────────────────────────────
    PackLeader              = nullptr;
    PackHuntTarget          = nullptr;
    bIsFlanking             = false;
    FlankAngleDegrees       = 45.0f;
    PackCoordinationRadius  = 3000.0f;
    bLeapOnCooldown         = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────
void ARaptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Start pack coordination timer (every 2 seconds)
    GetWorldTimerManager().SetTimer(
        PackCoordTimerHandle,
        this,
        &ARaptorCharacter::CoordinatePack,
        2.0f,
        true
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────
void ARaptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // If flanking, visualise flank vector in debug
#if WITH_EDITOR
    if (bIsFlanking && PackHuntTarget)
    {
        DrawDebugLine(
            GetWorld(),
            GetActorLocation(),
            PackHuntTarget->GetActorLocation(),
            FColor::Orange,
            false, 0.1f, 0, 2.0f
        );
    }
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformLeapAttack
// ─────────────────────────────────────────────────────────────────────────────
void ARaptorCharacter::PerformLeapAttack()
{
    if (bLeapOnCooldown || !PackHuntTarget) return;

    // Compute launch vector toward target
    FVector ToTarget = (PackHuntTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector LaunchVel = ToTarget * 1200.0f + FVector(0.0f, 0.0f, 500.0f);
    LaunchCharacter(LaunchVel, true, true);

    // Apply damage at end of leap (simplified: immediate radial)
    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        AttackDamage,
        PackHuntTarget->GetActorLocation(),
        AttackRange,
        nullptr,
        TArray<AActor*>(),
        this,
        GetInstigatorController(),
        true
    );

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), PackHuntTarget->GetActorLocation(), AttackRange, 12, FColor::Red, false, 1.5f);
#endif

    // Cooldown 4 seconds
    bLeapOnCooldown = true;
    GetWorldTimerManager().SetTimer(
        LeapCooldownHandle,
        [this]() { bLeapOnCooldown = false; },
        4.0f,
        false
    );
}

// ─────────────────────────────────────────────────────────────────────────────
// EmitPackScreech
// ─────────────────────────────────────────────────────────────────────────────
void ARaptorCharacter::EmitPackScreech()
{
    // Find all raptors within PackCoordinationRadius and alert them
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), NearbyActors);

    int32 AlertCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= PackCoordinationRadius)
        {
            ARaptorCharacter* Raptor = Cast<ARaptorCharacter>(Actor);
            if (Raptor && PackHuntTarget)
            {
                Raptor->PackHuntTarget = PackHuntTarget;
                Raptor->PackLeader = this;
                AlertCount++;
            }
        }
    }

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), GetActorLocation(), PackCoordinationRadius, 16, FColor::Yellow, false, 2.0f);
    UE_LOG(LogTemp, Log, TEXT("RaptorScreech: Alerted %d pack members"), AlertCount);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginFlankManoeuvre
// ─────────────────────────────────────────────────────────────────────────────
void ARaptorCharacter::BeginFlankManoeuvre(AActor* Target)
{
    if (!Target) return;

    PackHuntTarget = Target;
    bIsFlanking = true;

    // Compute flank position: rotate FlankAngleDegrees around target
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector FlankDir = ToTarget.RotateAngleAxis(FlankAngleDegrees, FVector::UpVector);
    FVector FlankPos = Target->GetActorLocation() - FlankDir * AttackRange * 1.5f;

    // Move toward flank position (AI controller would handle this in full impl)
    // Here we set a debug marker
#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), FlankPos, 60.0f, 8, FColor::Cyan, false, 3.0f);
    UE_LOG(LogTemp, Log, TEXT("Raptor %s flanking at angle %.1f deg"), *GetActorLabel(), FlankAngleDegrees);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// RegisterPackMember
// ─────────────────────────────────────────────────────────────────────────────
void ARaptorCharacter::RegisterPackMember(ARaptorCharacter* NewMember)
{
    if (!NewMember || NewMember == this) return;
    if (!PackMembers.Contains(NewMember))
    {
        PackMembers.Add(NewMember);
        NewMember->PackLeader = this;
        UE_LOG(LogTemp, Log, TEXT("Pack registered: %s -> leader %s"), *NewMember->GetActorLabel(), *GetActorLabel());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CoordinatePack (timer callback)
// ─────────────────────────────────────────────────────────────────────────────
void ARaptorCharacter::CoordinatePack()
{
    // Only the leader coordinates
    if (PackLeader != nullptr) return;
    if (!PackHuntTarget) return;

    BroadcastHuntTarget(PackHuntTarget);

    // Assign flank angles to members
    int32 MemberCount = PackMembers.Num();
    for (int32 i = 0; i < MemberCount; ++i)
    {
        if (PackMembers[i])
        {
            float Angle = (MemberCount > 1)
                ? FMath::Lerp(-90.0f, 90.0f, (float)i / (float)(MemberCount - 1))
                : 0.0f;
            PackMembers[i]->FlankAngleDegrees = Angle;
            PackMembers[i]->BeginFlankManoeuvre(PackHuntTarget);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// BroadcastHuntTarget
// ─────────────────────────────────────────────────────────────────────────────
void ARaptorCharacter::BroadcastHuntTarget(AActor* Target)
{
    for (ARaptorCharacter* Member : PackMembers)
    {
        if (Member)
        {
            Member->PackHuntTarget = Target;
        }
    }
}
