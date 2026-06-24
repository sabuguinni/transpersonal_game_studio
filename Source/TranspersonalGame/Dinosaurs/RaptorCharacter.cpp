// RaptorCharacter.cpp
// Performance Optimizer #04 — Cycle PROD_CYCLE_AUTO_20260624_001
// Full implementation of ARaptorCharacter pack-hunter

#include "Dinosaurs/RaptorCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ARaptorCharacter::ARaptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Capsule — raptor is lean and fast ────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(35.0f, 75.0f);

    // ── Movement — pack hunter: high speed, agile ────────────────────────────
    UCharacterMovementComponent* Move = GetCharacterMovement();
    if (Move)
    {
        Move->MaxWalkSpeed          = 900.0f;   // cm/s — faster than T-Rex
        Move->MaxAcceleration       = 4096.0f;
        Move->BrakingDecelerationWalking = 2048.0f;
        Move->JumpZVelocity         = 700.0f;   // raptors can leap
        Move->AirControl            = 0.4f;
        Move->Mass                  = 80.0f;    // kg — lightweight predator
        Move->RotationRate          = FRotator(0.0f, 540.0f, 0.0f);
        Move->bOrientRotationToMovement = true;
    }

    // ── Pack system defaults ─────────────────────────────────────────────────
    PackCallRadius      = 3000.0f;  // 30 m
    PackCallCooldown    = 8.0f;     // seconds
    LastPackCallTime    = -999.0f;  // allow immediate first call
    MaxPackResponders   = 4;

    // ── Pounce defaults ──────────────────────────────────────────────────────
    PounceSpeed  = 1400.0f; // cm/s launch velocity
    PounceDamage = 35.0f;
    PounceRange  = 400.0f;  // 4 m
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ARaptorCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Lock species — prevents Blueprint subclass from accidentally overriding
    // before ApplySpeciesStats() fires in DinosaurBase::BeginPlay()
    Species = EDinosaurSpecies::Velociraptor;

    // Tag for AI queries — Combat AI (#12) can use this to identify pack members
    Tags.Add(FName("Raptor"));
    Tags.Add(FName("PackHunter"));
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformPackCall
// ─────────────────────────────────────────────────────────────────────────────

void ARaptorCharacter::PerformPackCall_Implementation()
{
    if (!CanPerformPackCall())
    {
        return;
    }

    LastPackCallTime = GetWorld()->GetTimeSeconds();

    // Find all nearby raptors
    TArray<ARaptorCharacter*> PackMembers = GetNearbyPackMembers();

    // Determine current attack target (stored in DinosaurBase as AttackTarget)
    AActor* MyTarget = AttackTarget;

    int32 Alerted = 0;
    for (ARaptorCharacter* Member : PackMembers)
    {
        if (!Member || Member == this) continue;
        if (Alerted >= MaxPackResponders) break;

        // Signal pack member to converge on the same target
        Member->AttackTarget = MyTarget;
        Member->Tags.Add(FName("PackAlerted"));
        ++Alerted;
    }

#if WITH_EDITOR
    // Visual debug — show pack call radius as a sphere
    DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        PackCallRadius,
        24,
        FColor::Orange,
        false,
        3.0f,
        0,
        2.0f
    );
    UE_LOG(LogTemp, Log,
           TEXT("ARaptorCharacter [%s]: PackCall — alerted %d/%d members (target: %s)"),
           *GetActorLabel(),
           Alerted,
           PackMembers.Num(),
           MyTarget ? *MyTarget->GetActorLabel() : TEXT("None"));
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformPounce
// ─────────────────────────────────────────────────────────────────────────────

void ARaptorCharacter::PerformPounce_Implementation(AActor* Target)
{
    if (!Target) return;

    const FVector MyLoc     = GetActorLocation();
    const FVector TargetLoc = Target->GetActorLocation();
    const float   Dist      = FVector::Dist(MyLoc, TargetLoc);

    if (Dist > PounceRange)
    {
#if WITH_EDITOR
        UE_LOG(LogTemp, Warning,
               TEXT("ARaptorCharacter [%s]: Pounce out of range (%.0f > %.0f)"),
               *GetActorLabel(), Dist, PounceRange);
#endif
        return;
    }

    // Calculate launch direction toward target with upward arc
    FVector Direction = (TargetLoc - MyLoc).GetSafeNormal();
    Direction.Z = 0.5f; // arc component
    Direction.Normalize();

    // Apply launch velocity
    LaunchCharacter(Direction * PounceSpeed, true, true);

    // Line trace at apex to apply damage (deferred via timer)
    FTimerHandle PounceHitTimer;
    GetWorldTimerManager().SetTimer(PounceHitTimer, [this, Target]()
    {
        if (!Target || !IsValid(Target)) return;

        FHitResult Hit;
        const FVector Start = GetActorLocation();
        const FVector End   = Target->GetActorLocation();

        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
        {
            if (Hit.GetActor() == Target)
            {
                UGameplayStatics::ApplyPointDamage(
                    Target,
                    PounceDamage,
                    (End - Start).GetSafeNormal(),
                    Hit,
                    GetController(),
                    this,
                    nullptr
                );

#if WITH_EDITOR
                DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint,
                              FColor::Red, false, 1.5f, 0, 2.0f);
                UE_LOG(LogTemp, Log,
                       TEXT("ARaptorCharacter [%s]: Pounce HIT %s for %.1f dmg"),
                       *GetActorLabel(), *Target->GetActorLabel(), PounceDamage);
#endif
            }
        }
    }, 0.35f, false); // 350ms — approximate apex time
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformClawSlash
// ─────────────────────────────────────────────────────────────────────────────

void ARaptorCharacter::PerformClawSlash_Implementation()
{
    // Claw slash: short-range sweep in front of the raptor
    const FVector  Start     = GetActorLocation();
    const FVector  Forward   = GetActorForwardVector();
    const float    SlashRange = 150.0f; // 1.5 m
    const float    SlashDmg   = AttackDamage * 0.6f;

    // Sphere sweep for multi-hit (claw arc)
    TArray<FHitResult> Hits;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    const FVector End = Start + Forward * SlashRange;
    GetWorld()->SweepMultiByChannel(
        Hits,
        Start,
        End,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(60.0f),
        Params
    );

    for (const FHitResult& Hit : Hits)
    {
        AActor* HitActor = Hit.GetActor();
        if (!HitActor || HitActor == this) continue;

        UGameplayStatics::ApplyPointDamage(
            HitActor,
            SlashDmg,
            Forward,
            Hit,
            GetController(),
            this,
            nullptr
        );

#if WITH_EDITOR
        UE_LOG(LogTemp, Log,
               TEXT("ARaptorCharacter [%s]: ClawSlash HIT %s for %.1f dmg"),
               *GetActorLabel(), *HitActor->GetActorLabel(), SlashDmg);
#endif
    }

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), End, 60.0f, 12, FColor::Cyan, false, 1.0f, 0, 1.5f);
#endif
}

// ─────────────────────────────────────────────────────────────────────────────
// Utility
// ─────────────────────────────────────────────────────────────────────────────

bool ARaptorCharacter::CanPerformPackCall() const
{
    const float Now = GetWorld()->GetTimeSeconds();
    return (Now - LastPackCallTime) >= PackCallCooldown;
}

TArray<ARaptorCharacter*> ARaptorCharacter::GetNearbyPackMembers() const
{
    TArray<ARaptorCharacter*> Result;

    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARaptorCharacter::StaticClass(), Found);

    for (AActor* A : Found)
    {
        ARaptorCharacter* R = Cast<ARaptorCharacter>(A);
        if (!R || R == this) continue;

        const float Dist = FVector::Dist(GetActorLocation(), R->GetActorLocation());
        if (Dist <= PackCallRadius)
        {
            Result.Add(R);
        }
    }

    return Result;
}
