// VelociraptorCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Velociraptor: fast pack-hunter, flanking AI, leap attack, bleed DoT

#include "Dinosaurs/VelociraptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor — species stats
// ─────────────────────────────────────────────────────────────────────────────

AVelociraptorCharacter::AVelociraptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Species identity ──────────────────────────────────────────────────────
    Species         = EDinoSpecies::Velociraptor;
    bIsCarnivore    = true;
    bIsPackHunter   = true;

    // ── Survival stats ────────────────────────────────────────────────────────
    MaxHealth       = 400.f;
    CurrentHealth   = 400.f;
    MaxHunger       = 100.f;
    CurrentHunger   = 80.f;
    HungerDecayRate = 1.2f;   // Raptors burn energy fast

    // ── Combat stats ─────────────────────────────────────────────────────────
    AttackDamage    = 80.f;
    AttackRange     = 200.f;
    DetectionRadius = 1800.f;
    AggressionLevel = 0.85f;  // Very aggressive

    // ── Movement ─────────────────────────────────────────────────────────────
    PatrolSpeed     = 350.f;
    ChaseSpeed      = 900.f;
    AttackCooldown  = 1.2f;

    // ── UCharacterMovementComponent ───────────────────────────────────────────
    if (UCharacterMovementComponent* CMC = GetCharacterMovement())
    {
        CMC->MaxWalkSpeed           = ChaseSpeed;
        CMC->JumpZVelocity          = 600.f;   // Raptors can leap
        CMC->AirControl             = 0.4f;
        CMC->GravityScale           = 1.2f;
        CMC->MaxAcceleration        = 3000.f;  // Snappy acceleration
        CMC->BrakingDecelerationWalking = 2000.f;
        CMC->RotationRate           = FRotator(0.f, 720.f, 0.f);
        CMC->bOrientRotationToMovement = true;
    }

    // ── Scale — raptors are smaller than player expects ───────────────────────
    SetActorScale3D(FVector(1.5f, 1.5f, 1.5f));
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Elect pack alpha among nearby raptors
    ElectPackAlpha();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Apply bleed DoT if active
    TickBleed(DeltaTime);

    // Non-alpha raptors move to their assigned flank position
    if (!bIsPackAlpha && FlankTarget && AssignedFlankIndex >= 0)
    {
        MoveToFlankPosition(DeltaTime);
    }

    // Alpha coordinates pack attack
    if (bIsPackAlpha && CurrentState == EDinoState::Chasing && CurrentTarget)
    {
        CoordinatePackAttack();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformAttack override — use leap if in range, else standard bite
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::PerformAttack_Implementation()
{
    if (!CurrentTarget) return;

    const float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= LeapAttackRange && CanLeap())
    {
        ExecuteLeapAttack();
    }
    else
    {
        // Standard bite — delegate to base class damage pipeline
        Super::PerformAttack_Implementation();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CanLeap
// ─────────────────────────────────────────────────────────────────────────────

bool AVelociraptorCharacter::CanLeap() const
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
    return (Now - LastLeapTime) >= LeapCooldown && !bIsLeaping;
}

// ─────────────────────────────────────────────────────────────────────────────
// ExecuteLeapAttack
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::ExecuteLeapAttack()
{
    if (!CurrentTarget || !CanLeap()) return;

    bIsLeaping = true;
    LastLeapTime = GetWorld()->GetTimeSeconds();

    // Direction toward target
    const FVector ToTarget = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    const FVector LeapVelocity = ToTarget * LeapImpulse + FVector(0.f, 0.f, 400.f);

    // Apply launch impulse
    LaunchCharacter(LeapVelocity, true, true);

    // Apply bite damage immediately on leap contact
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    // Start bleed DoT on target
    BleedTarget = CurrentTarget;
    BleedTimeRemaining = BleedDuration;

    // Notify Blueprint (animation, sound, VFX)
    OnLeapAttack(CurrentTarget);

    // Reset leap flag after short delay (landing)
    FTimerHandle LeapResetTimer;
    GetWorldTimerManager().SetTimer(LeapResetTimer, [this]()
    {
        bIsLeaping = false;
    }, 0.8f, false);
}

// ─────────────────────────────────────────────────────────────────────────────
// BroadcastPackCall
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::BroadcastPackCall()
{
    if (!GetWorld()) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVelociraptorCharacter::StaticClass(), NearbyActors);

    int32 ResponseCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        if (ResponseCount >= MaxPackResponseCount) break;

        AVelociraptorCharacter* PackMember = Cast<AVelociraptorCharacter>(Actor);
        if (!PackMember) continue;

        const float Dist = FVector::Dist(GetActorLocation(), PackMember->GetActorLocation());
        if (Dist <= PackCallRadius)
        {
            // Assign this raptor as their alpha and give them the target
            PackMember->PackAlpha = this;
            PackMember->AssignFlankPosition(ResponseCount, CurrentTarget);
            OnPackMemberArrived(PackMember);
            ResponseCount++;
        }
    }

    OnPackCallIssued();
}

// ─────────────────────────────────────────────────────────────────────────────
// AssignFlankPosition
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::AssignFlankPosition(int32 FlankIndex, AActor* SharedTarget)
{
    AssignedFlankIndex = FlankIndex;
    FlankTarget = SharedTarget;
    CurrentTarget = SharedTarget;
    CurrentState = EDinoState::Chasing;
}

// ─────────────────────────────────────────────────────────────────────────────
// MoveToFlankPosition (internal)
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::MoveToFlankPosition(float DeltaTime)
{
    if (!FlankTarget) return;

    // Calculate flank offset based on assigned index
    // 0 = left flank, 1 = right flank, 2 = rear
    const float AngleOffsets[] = { -FlankAngleDegrees, FlankAngleDegrees, 180.f };
    const float Angle = AngleOffsets[FMath::Clamp(AssignedFlankIndex, 0, 2)];

    const FVector TargetForward = FlankTarget->GetActorForwardVector();
    const FVector RotatedDir = TargetForward.RotateAngleAxis(Angle, FVector::UpVector);
    const FVector FlankPos = FlankTarget->GetActorLocation() + RotatedDir * FlankRadius;

    // Move toward flank position
    const FVector ToFlank = (FlankPos - GetActorLocation()).GetSafeNormal();
    AddMovementInput(ToFlank, 1.0f);

    // If close enough to flank position and in attack range, perform attack
    const float DistToTarget = FVector::Dist(GetActorLocation(), FlankTarget->GetActorLocation());
    if (DistToTarget <= AttackRange)
    {
        PerformAttack();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TickBleed (internal)
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::TickBleed(float DeltaTime)
{
    if (BleedTimeRemaining <= 0.f || !BleedTarget) return;

    BleedTimeRemaining -= DeltaTime;

    // Apply bleed damage each tick (scaled to per-second rate)
    UGameplayStatics::ApplyDamage(
        BleedTarget,
        BleedDamagePerSecond * DeltaTime,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    if (BleedTimeRemaining <= 0.f)
    {
        BleedTarget = nullptr;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ElectPackAlpha (internal)
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::ElectPackAlpha()
{
    if (!GetWorld()) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVelociraptorCharacter::StaticClass(), NearbyActors);

    float HighestHealth = CurrentHealth;
    AVelociraptorCharacter* AlphaCandidate = this;

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        AVelociraptorCharacter* Other = Cast<AVelociraptorCharacter>(Actor);
        if (!Other) continue;

        const float Dist = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
        if (Dist <= PackCallRadius && Other->CurrentHealth > HighestHealth)
        {
            HighestHealth = Other->CurrentHealth;
            AlphaCandidate = Other;
        }
    }

    bIsPackAlpha = (AlphaCandidate == this);
    if (!bIsPackAlpha)
    {
        PackAlpha = AlphaCandidate;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// CoordinatePackAttack (internal — alpha only)
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::CoordinatePackAttack()
{
    if (!CurrentTarget || !GetWorld()) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AVelociraptorCharacter::StaticClass(), NearbyActors);

    int32 FlankIdx = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this || FlankIdx >= MaxPackResponseCount) break;
        AVelociraptorCharacter* Member = Cast<AVelociraptorCharacter>(Actor);
        if (!Member || Member->bIsPackAlpha) continue;

        const float Dist = FVector::Dist(GetActorLocation(), Member->GetActorLocation());
        if (Dist <= PackCallRadius && Member->CurrentTarget != CurrentTarget)
        {
            Member->AssignFlankPosition(FlankIdx, CurrentTarget);
            FlankIdx++;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Blueprint Native Event implementations
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::OnLeapAttack_Implementation(AActor* Target)
{
    // Blueprint handles animation notify, sound, VFX
    // C++ base: log for debugging
    if (Target)
    {
        UE_LOG(LogTemp, Log, TEXT("Raptor [%s] LEAP ATTACK on [%s]"),
            *GetActorLabel(), *Target->GetActorLabel());
    }
}

void AVelociraptorCharacter::OnPackCallIssued_Implementation()
{
    UE_LOG(LogTemp, Log, TEXT("Raptor [%s] issued PACK CALL"), *GetActorLabel());
}

void AVelociraptorCharacter::OnPackMemberArrived_Implementation(AVelociraptorCharacter* Member)
{
    if (Member)
    {
        UE_LOG(LogTemp, Log, TEXT("Pack member [%s] responded to alpha [%s]"),
            *Member->GetActorLabel(), *GetActorLabel());
    }
}
