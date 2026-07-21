// TRexDinosaur.cpp
// Core Systems Programmer #03 — Cycle AUTO_20260702_008
// Tyrannosaurus Rex — full species implementation

#include "Dinosaurs/TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRexDinosaur::ATRexDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // Species-specific stats — apex predator
    SpeciesName = FText::FromString(TEXT("Tyrannosaurus Rex"));
    MaxHealth = 2000.0f;
    CurrentHealth = 2000.0f;
    AttackDamage = 150.0f;
    PatrolSpeed = 200.0f;   // Slow patrol
    RunSpeed = 650.0f;       // Fast charge
    DetectionRange = 1200.0f;
    AttackRange = 250.0f;
    DinosaurSize = EEng_DinosaurSize::Massive;
    bIsPredator = true;
    bIsTerritory = true;
    TerritoryRadius = 2500.0f;
    ExperienceReward = 500;

    // Movement component setup
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = PatrolSpeed;
        MoveComp->MaxAcceleration = 800.0f;
        MoveComp->BrakingDecelerationWalking = 600.0f;
        MoveComp->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->GravityScale = 1.2f;
        MoveComp->Mass = 8000.0f;  // 8 tonnes
    }

    // Scale the capsule for TRex size
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleHalfHeight(180.0f);
        Capsule->SetCapsuleRadius(80.0f);
    }
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // Start periodic prey scan every 1.5 seconds
    GetWorldTimerManager().SetTimer(
        PreyScanTimer,
        this,
        &ATRexDinosaur::ScanForPrey,
        1.5f,
        true,
        2.0f  // Initial delay
    );

    UE_LOG(LogTemp, Log, TEXT("TRexDinosaur BeginPlay: %s — MaxHealth=%.0f, AttackDamage=%.0f"),
        *GetActorLabel(), MaxHealth, AttackDamage);
}

void ATRexDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Charge movement — push toward ThreatTarget
    if (bIsCharging && ThreatTarget)
    {
        FVector Direction = (ThreatTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        float ChargeSpeed = RunSpeed * ChargeSpeedMultiplier;
        AddMovementInput(Direction, 1.0f);

        UCharacterMovementComponent* MoveComp = GetCharacterMovement();
        if (MoveComp)
        {
            MoveComp->MaxWalkSpeed = ChargeSpeed;
        }

        // Check if close enough to deal charge damage
        float DistToTarget = FVector::Dist(GetActorLocation(), ThreatTarget->GetActorLocation());
        if (DistToTarget <= AttackRange + 50.0f)
        {
            // Impact — deal bonus damage
            float TotalDamage = AttackDamage + ChargeBonusDamage;
            UGameplayStatics::ApplyDamage(ThreatTarget, TotalDamage, GetController(), this, nullptr);
            bIsCharging = false;

            UCharacterMovementComponent* MC = GetCharacterMovement();
            if (MC) MC->MaxWalkSpeed = RunSpeed;

            UE_LOG(LogTemp, Log, TEXT("TRex charge impact! Dealt %.0f damage to %s"),
                TotalDamage, *ThreatTarget->GetActorLabel());
        }
    }
}

void ATRexDinosaur::PerformRoar()
{
    if (bIsRoaring) return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastRoarTime < RoarCooldown) return;

    bIsRoaring = true;
    LastRoarTime = CurrentTime;

    UE_LOG(LogTemp, Log, TEXT("TRex ROAR at %s — stunning prey within %.0f units"),
        *GetActorLabel(), RoarRadius);

    // Sphere overlap — find all actors within RoarRadius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(RoarRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        SphereShape,
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor && HitActor != this)
            {
                // Apply stun via gameplay tag or simple movement disable
                // For now, log the stun — full stun system via GameplayAbilities in future
                UE_LOG(LogTemp, Log, TEXT("TRex roar stuns: %s for %.1fs"),
                    *HitActor->GetActorLabel(), RoarStunDuration);
            }
        }
    }

    // End roar after 1.5 seconds
    FTimerHandle RoarEndTimer;
    GetWorldTimerManager().SetTimer(RoarEndTimer, [this]()
    {
        bIsRoaring = false;
    }, 1.5f, false);
}

void ATRexDinosaur::InitiateCharge(AActor* Target)
{
    if (!Target || bIsCharging) return;

    ThreatTarget = Target;
    bIsCharging = true;
    CurrentBehavior = EEng_DinosaurBehavior::Aggressive;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = RunSpeed * ChargeSpeedMultiplier;
    }

    UE_LOG(LogTemp, Log, TEXT("TRex initiating charge toward %s"), *Target->GetActorLabel());

    // Auto-cancel charge after 4 seconds if no impact
    GetWorldTimerManager().SetTimer(ChargeTimer, [this]()
    {
        if (bIsCharging)
        {
            bIsCharging = false;
            UCharacterMovementComponent* MC = GetCharacterMovement();
            if (MC) MC->MaxWalkSpeed = RunSpeed;
            UE_LOG(LogTemp, Log, TEXT("TRex charge cancelled (timeout)"));
        }
    }, 4.0f, false);
}

void ATRexDinosaur::OnPreyDetected_Implementation(AActor* Prey)
{
    if (!Prey) return;

    UE_LOG(LogTemp, Warning, TEXT("TRex [%s] detected prey: %s — initiating hunt"),
        *GetActorLabel(), *Prey->GetActorLabel());

    ThreatTarget = Prey;
    CurrentBehavior = EEng_DinosaurBehavior::Aggressive;

    // Roar first, then charge
    PerformRoar();

    FTimerHandle ChargeDelayTimer;
    GetWorldTimerManager().SetTimer(ChargeDelayTimer, [this, Prey]()
    {
        InitiateCharge(Prey);
    }, 2.0f, false);
}

void ATRexDinosaur::ScanForPrey()
{
    if (CurrentBehavior == EEng_DinosaurBehavior::Dead) return;
    if (CurrentBehavior == EEng_DinosaurBehavior::Aggressive) return;  // Already hunting

    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(PreyDetectionRange);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECC_Pawn,
        SphereShape,
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor && HitActor != this)
            {
                // Trigger prey detection
                OnPreyDetected(HitActor);
                break;  // Hunt one target at a time
            }
        }
    }
}
