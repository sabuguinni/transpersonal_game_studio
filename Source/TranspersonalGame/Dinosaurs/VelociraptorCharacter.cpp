#include "VelociraptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

AVelociraptorCharacter::AVelociraptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Raptor stats (override DinosaurBase defaults) ──────────────────────
    DinoStats.MaxHealth       = 200.0f;
    DinoStats.CurrentHealth   = 200.0f;
    DinoStats.AttackDamage    = 45.0f;
    DinoStats.DetectionRadius = 1800.0f;
    DinoStats.AttackRadius    = 150.0f;
    DinoStats.PatrolSpeed     = 300.0f;
    DinoStats.ChaseSpeed      = 800.0f;
    DinoStats.Species         = EEng_DinoSpecies::Velociraptor;

    // ── Pack defaults ──────────────────────────────────────────────────────
    PackIndex        = 0;
    FlankAngleOffset = 0.0f;

    // ── Leap defaults ──────────────────────────────────────────────────────
    LeapRange        = 400.0f;
    LeapImpulse      = 1200.0f;
    LeapCooldown     = 3.0f;
    LeapCooldownTimer = 0.0f;
    bIsLeaping       = false;

    // ── Movement ──────────────────────────────────────────────────────────
    GetCharacterMovement()->MaxWalkSpeed        = DinoStats.PatrolSpeed;
    GetCharacterMovement()->JumpZVelocity       = 600.0f;
    GetCharacterMovement()->AirControl          = 0.4f;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate        = FRotator(0.0f, 540.0f, 0.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::BeginPlay()
{
    Super::BeginPlay();
    // Scale: raptors are smaller than T-Rex
    SetActorScale3D(FVector(1.5f, 1.5f, 1.5f));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Tick leap cooldown
    if (LeapCooldownTimer > 0.0f)
    {
        LeapCooldownTimer -= DeltaTime;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TickChase — flanking movement override
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::TickChase(float DeltaTime)
{
    if (!CurrentTarget)
    {
        // Fall back to base chase if no target
        Super::TickChase(DeltaTime);
        return;
    }

    // Alpha (index 0) charges directly; flankers move to offset positions
    if (PackIndex == 0)
    {
        Super::TickChase(DeltaTime);
    }
    else
    {
        UpdateFlankMovement(DeltaTime);
    }

    // All pack members attempt leap when close enough
    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget <= LeapRange)
    {
        AttemptLeap(CurrentTarget);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// TickAttack — leap attack override
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::TickAttack(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinoState(EEng_DinoState::Idle);
        return;
    }

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

    if (DistToTarget <= DinoStats.AttackRadius)
    {
        // In melee range — perform standard attack
        PerformAttack();
    }
    else if (DistToTarget <= LeapRange)
    {
        // In leap range — leap at target
        AttemptLeap(CurrentTarget);
    }
    else
    {
        // Target moved out of attack range — chase again
        SetDinoState(EEng_DinoState::Chase);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// OnDinoStateChanged — raptor-specific reactions
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::OnDinoStateChanged_Implementation(EEng_DinoState NewState)
{
    Super::OnDinoStateChanged_Implementation(NewState);

    switch (NewState)
    {
    case EEng_DinoState::Chase:
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.ChaseSpeed;
        // Alpha alerts the pack when it starts chasing
        if (PackIndex == 0 && CurrentTarget)
        {
            AlertPack(CurrentTarget);
        }
        break;

    case EEng_DinoState::Patrol:
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.PatrolSpeed;
        bIsLeaping = false;
        break;

    case EEng_DinoState::Flee:
        // Raptors flee at max speed
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.ChaseSpeed * 1.2f;
        break;

    default:
        break;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// OnDinoDied — pack scatter
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::OnDinoDied_Implementation()
{
    // Surviving pack members flee when a packmate dies
    for (AVelociraptorCharacter* Member : PackMembers)
    {
        if (Member && Member != this && Member->IsAlive())
        {
            Member->SetDinoState(EEng_DinoState::Flee);
        }
    }

    Super::OnDinoDied_Implementation();
}

// ─────────────────────────────────────────────────────────────────────────────
// JoinPack
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::JoinPack(const TArray<AVelociraptorCharacter*>& Pack, int32 Index)
{
    PackMembers      = Pack;
    PackIndex        = Index;

    // Assign flank angles: alpha=0°, flanker1=90°, flanker2=-90°
    const float Angles[] = { 0.0f, 90.0f, -90.0f };
    if (Index >= 0 && Index < 3)
    {
        FlankAngleOffset = Angles[Index];
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// AlertPack
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::AlertPack(AActor* Target)
{
    if (!Target) return;

    for (AVelociraptorCharacter* Member : PackMembers)
    {
        if (Member && Member != this && Member->IsAlive())
        {
            Member->CurrentTarget = Target;
            Member->SetDinoState(EEng_DinoState::Chase);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// ComputeFlankPosition
// ─────────────────────────────────────────────────────────────────────────────

FVector AVelociraptorCharacter::ComputeFlankPosition(AActor* Target) const
{
    if (!Target) return GetActorLocation();

    // Direction from target to this raptor, rotated by flank angle
    FVector ToRaptor = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal();
    FVector FlankDir = ToRaptor.RotateAngleAxis(FlankAngleOffset, FVector::UpVector);

    // Flank position is 300 units from the target at the flank angle
    const float FlankDistance = 300.0f;
    return Target->GetActorLocation() + FlankDir * FlankDistance;
}

// ─────────────────────────────────────────────────────────────────────────────
// AttemptLeap
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::AttemptLeap(AActor* Target)
{
    if (!Target || bIsLeaping || LeapCooldownTimer > 0.0f) return;

    bIsLeaping        = true;
    LeapCooldownTimer = LeapCooldown;

    // Direction toward target
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    ToTarget.Z       = 0.4f; // slight upward arc

    // Apply launch velocity
    LaunchCharacter(ToTarget * LeapImpulse, true, true);

    // Deal damage on leap impact (slight delay for arc)
    FTimerHandle LeapImpactTimer;
    GetWorldTimerManager().SetTimer(LeapImpactTimer, [this, Target]()
    {
        if (Target && IsAlive())
        {
            float DistOnLand = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
            if (DistOnLand <= DinoStats.AttackRadius * 1.5f)
            {
                UGameplayStatics::ApplyDamage(Target, DinoStats.AttackDamage * 1.5f,
                    GetController(), this, nullptr);
            }
        }
        bIsLeaping = false;
    }, 0.6f, false);
}

// ─────────────────────────────────────────────────────────────────────────────
// UpdateFlankMovement (private)
// ─────────────────────────────────────────────────────────────────────────────

void AVelociraptorCharacter::UpdateFlankMovement(float DeltaTime)
{
    if (!CurrentTarget) return;

    FVector FlankPos = ComputeFlankPosition(CurrentTarget);
    FVector ToFlank  = (FlankPos - GetActorLocation()).GetSafeNormal();

    // Add movement input toward flank position
    AddMovementInput(ToFlank, 1.0f);

    // Face the target while flanking
    FRotator LookAt = UKismetMathLibrary::FindLookAtRotation(
        GetActorLocation(), CurrentTarget->GetActorLocation());
    SetActorRotation(FRotator(0.0f, LookAt.Yaw, 0.0f));
}
