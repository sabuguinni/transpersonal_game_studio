#include "DinosaurCombatComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"

UDinosaurCombatComponent::UDinosaurCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f; // 20Hz tick for combat precision
}

void UDinosaurCombatComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentPhase = ECombat_AttackPhase::None;
    PhaseTimer = 0.0f;
    LastAttackTime = 0.0f;
    TotalHitsDealt = 0;
}

void UDinosaurCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (CurrentPhase != ECombat_AttackPhase::None)
    {
        AdvanceAttackPhase(DeltaTime);
    }
}

// ============================================================
// Attack initiation
// ============================================================

bool UDinosaurCombatComponent::TryInitiateAttack()
{
    if (IsAttacking() || IsOnCooldown())
    {
        return false;
    }

    CurrentPhase = ECombat_AttackPhase::WindUp;
    PhaseTimer = AttackProfile.WindUpTime;
    OnAttackPhaseChanged(CurrentPhase);
    return true;
}

void UDinosaurCombatComponent::CancelAttack()
{
    if (CurrentPhase == ECombat_AttackPhase::WindUp || CurrentPhase == ECombat_AttackPhase::Strike)
    {
        CurrentPhase = ECombat_AttackPhase::Recovery;
        PhaseTimer = AttackProfile.RecoveryTime * 0.5f; // Shortened recovery on cancel
        OnAttackPhaseChanged(CurrentPhase);
    }
}

bool UDinosaurCombatComponent::IsAttacking() const
{
    return CurrentPhase == ECombat_AttackPhase::WindUp
        || CurrentPhase == ECombat_AttackPhase::Strike
        || CurrentPhase == ECombat_AttackPhase::Recovery;
}

bool UDinosaurCombatComponent::IsOnCooldown() const
{
    return CurrentPhase == ECombat_AttackPhase::Cooldown;
}

// ============================================================
// Phase state machine
// ============================================================

void UDinosaurCombatComponent::AdvanceAttackPhase(float DeltaTime)
{
    PhaseTimer -= DeltaTime;

    if (PhaseTimer > 0.0f)
    {
        return;
    }

    // Phase transition
    switch (CurrentPhase)
    {
    case ECombat_AttackPhase::WindUp:
        CurrentPhase = ECombat_AttackPhase::Strike;
        PhaseTimer = AttackProfile.StrikeTime;
        ExecuteStrike();
        break;

    case ECombat_AttackPhase::Strike:
        CurrentPhase = ECombat_AttackPhase::Recovery;
        PhaseTimer = AttackProfile.RecoveryTime;
        break;

    case ECombat_AttackPhase::Recovery:
        CurrentPhase = ECombat_AttackPhase::Cooldown;
        PhaseTimer = AttackProfile.CooldownTime;
        LastAttackTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        break;

    case ECombat_AttackPhase::Cooldown:
        CurrentPhase = ECombat_AttackPhase::None;
        PhaseTimer = 0.0f;
        break;

    default:
        CurrentPhase = ECombat_AttackPhase::None;
        break;
    }

    OnAttackPhaseChanged(CurrentPhase);
}

// ============================================================
// Strike execution — sweep trace hit detection
// ============================================================

void UDinosaurCombatComponent::ExecuteStrike()
{
    FCombat_HitResult Result = PerformSweepAttack();

    if (Result.bHit && Result.HitActor)
    {
        TotalHitsDealt++;
        ApplyDamageToTarget(Result.HitActor, Result.HitLocation);
        OnAttackHit(Result);
    }
    else
    {
        OnAttackMiss();
    }
}

FCombat_HitResult UDinosaurCombatComponent::PerformSweepAttack()
{
    FCombat_HitResult OutResult;
    OutResult.bHit = false;

    AActor* Owner = GetOwner();
    if (!Owner) return OutResult;

    UWorld* World = GetWorld();
    if (!World) return OutResult;

    const FVector Origin = Owner->GetActorLocation();
    const FVector Forward = Owner->GetActorForwardVector();
    const float Range = AttackProfile.AttackRange;
    const float HalfAngle = FMath::DegreesToRadians(AttackProfile.AttackAngleDegrees * 0.5f);

    // Capsule sweep in front of the dinosaur
    FVector SweepEnd = Origin + Forward * Range;
    FCollisionShape SweepShape = FCollisionShape::MakeSphere(Range * 0.5f);

    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Owner);

    bool bAnyHit = World->SweepMultiByChannel(
        Hits,
        Origin,
        SweepEnd,
        FQuat::Identity,
        ECC_Pawn,
        SweepShape,
        Params
    );

    if (bDebugDrawAttackSweep)
    {
        DrawDebugSphere(World, SweepEnd, Range * 0.5f, 12, FColor::Red, false, 1.0f);
        DrawDebugLine(World, Origin, SweepEnd, FColor::Orange, false, 1.0f, 0, 3.0f);
    }

    if (!bAnyHit) return OutResult;

    // Filter by angle — only hit targets in front of the dinosaur
    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor) continue;

        FVector ToTarget = (HitActor->GetActorLocation() - Origin).GetSafeNormal();
        float DotProduct = FVector::DotProduct(Forward, ToTarget);
        float AngleToTarget = FMath::Acos(DotProduct);

        if (AngleToTarget <= HalfAngle)
        {
            OutResult.bHit = true;
            OutResult.HitActor = HitActor;
            OutResult.HitLocation = Hit.ImpactPoint;
            OutResult.DamageDealt = AttackProfile.BaseDamage;
            break; // First valid target in arc
        }
    }

    return OutResult;
}

void UDinosaurCombatComponent::ApplyDamageToTarget(AActor* Target, const FVector& HitLoc)
{
    if (!Target) return;

    AActor* Owner = GetOwner();
    AController* InstigatorController = nullptr;

    if (ACharacter* DinoChar = Cast<ACharacter>(Owner))
    {
        InstigatorController = DinoChar->GetController();
    }

    // Apply damage — TranspersonalCharacter receives this via TakeDamage
    UGameplayStatics::ApplyPointDamage(
        Target,
        AttackProfile.BaseDamage,
        HitLoc,
        FHitResult(),
        InstigatorController,
        Owner,
        UDamageType::StaticClass()
    );

    // Apply knockback if target is a character
    if (ACharacter* TargetChar = Cast<ACharacter>(Target))
    {
        UCharacterMovementComponent* Movement = TargetChar->GetCharacterMovement();
        if (Movement)
        {
            FVector KnockDir = (Target->GetActorLocation() - HitLoc).GetSafeNormal();
            KnockDir.Z = 0.4f; // Slight upward component
            KnockDir.Normalize();
            Movement->AddImpulse(KnockDir * AttackProfile.KnockbackForce, true);
        }
    }
}

// ============================================================
// Species preset configurations
// ============================================================

void UDinosaurCombatComponent::SetAttackProfile(const FCombat_AttackProfile& NewProfile)
{
    AttackProfile = NewProfile;
}

void UDinosaurCombatComponent::SetTRexDefaults()
{
    AttackProfile.AttackType      = ECombat_DinoAttackType::Bite;
    AttackProfile.BaseDamage      = 85.0f;  // Lethal — player has 100 HP
    AttackProfile.AttackRange     = 350.0f;
    AttackProfile.AttackAngleDegrees = 70.0f;
    AttackProfile.WindUpTime      = 0.8f;   // Slow wind-up — telegraphed
    AttackProfile.StrikeTime      = 0.15f;  // Extremely fast strike window
    AttackProfile.RecoveryTime    = 1.2f;   // Long recovery — exploit window
    AttackProfile.CooldownTime    = 3.5f;
    AttackProfile.KnockbackForce  = 1200.0f;
}

void UDinosaurCombatComponent::SetRaptorDefaults()
{
    AttackProfile.AttackType      = ECombat_DinoAttackType::Claw;
    AttackProfile.BaseDamage      = 25.0f;  // Lower per hit — but attacks frequently
    AttackProfile.AttackRange     = 150.0f;
    AttackProfile.AttackAngleDegrees = 90.0f; // Wide arc — hard to dodge
    AttackProfile.WindUpTime      = 0.25f;  // Very fast — hard to read
    AttackProfile.StrikeTime      = 0.1f;
    AttackProfile.RecoveryTime    = 0.3f;
    AttackProfile.CooldownTime    = 1.2f;   // Rapid succession attacks
    AttackProfile.KnockbackForce  = 400.0f;
}

void UDinosaurCombatComponent::SetBrachiosaurusDefaults()
{
    AttackProfile.AttackType      = ECombat_DinoAttackType::Stomp;
    AttackProfile.BaseDamage      = 60.0f;
    AttackProfile.AttackRange     = 400.0f;
    AttackProfile.AttackAngleDegrees = 360.0f; // Stomp hits all around
    AttackProfile.WindUpTime      = 1.5f;   // Very slow — only attacks if cornered
    AttackProfile.StrikeTime      = 0.3f;
    AttackProfile.RecoveryTime    = 2.0f;
    AttackProfile.CooldownTime    = 6.0f;   // Rarely attacks
    AttackProfile.KnockbackForce  = 1500.0f;
}
