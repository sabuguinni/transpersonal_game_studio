// TRex.cpp
// Core Systems Programmer #03 — Cycle AUTO_20260630_004
// Tyrannosaurus Rex implementation

#include "Dinosaurs/TRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATRex::ATRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Species Identity ─────────────────────────────────────────────────────
    Species = EDinosaurSpecies::TyrannosaurusRex;
    Diet = EDinosaurDiet::Carnivore;

    // ── Physical Scale ────────────────────────────────────────────────────────
    // T-Rex: 12m long, 4m tall at hip, 6m tall at head
    // UE5 units: 1 unit = 1cm, so 400cm capsule half-height
    GetCapsuleComponent()->SetCapsuleHalfHeight(400.0f);
    GetCapsuleComponent()->SetCapsuleRadius(120.0f);
    GetCapsuleComponent()->SetCollisionProfileName(TEXT("BlockAll"));

    // ── Survival Stats ────────────────────────────────────────────────────────
    MaxHealth = 1000.0f;
    CurrentHealth = 1000.0f;
    MaxHunger = 100.0f;
    CurrentHunger = 80.0f;
    MaxStamina = 100.0f;
    CurrentStamina = 100.0f;
    HungerDrainRate = 0.5f;  // Apex predator — needs large meals, infrequently

    // ── Combat Stats ─────────────────────────────────────────────────────────
    AttackDamage = 150.0f;
    AttackRange = 300.0f;    // Long reach — massive jaw
    AttackCooldown = 2.5f;   // Slow but devastating

    // ── Territory & Detection ─────────────────────────────────────────────────
    TerritoryRadius = 5000.0f;   // Huge territory — apex predator
    DetectionRadius = 2500.0f;   // Excellent smell/hearing

    // ── Movement ──────────────────────────────────────────────────────────────
    // T-Rex: estimated 17-25 km/h, UE5 units ~500-700 cm/s
    GetCharacterMovement()->MaxWalkSpeed = 550.0f;
    GetCharacterMovement()->MaxAcceleration = 800.0f;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 60.0f, 0.0f);  // Slow turn
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->GravityScale = 1.2f;  // Heavy — falls fast

    // ── Pack Behavior ─────────────────────────────────────────────────────────
    bIsSolitary = true;
    MaxPackSize = 1;
    PackCommunicationRadius = 0.0f;  // No pack coordination

    // ── Sensory ───────────────────────────────────────────────────────────────
    bMovementBasedVision = true;
    BloodSmellRadius = 3000.0f;
}

void ATRex::BeginPlay()
{
    Super::BeginPlay();

    // Record home territory center
    HomeLocation = GetActorLocation();

    // Periodic blood smell check — T-Rex can smell blood from far away
    GetWorldTimerManager().SetTimer(
        RoarTimerHandle,
        this,
        &ATRex::CheckForBloodSmell,
        10.0f,  // Check every 10 seconds
        true
    );

    // Initial roar to establish territory
    float InitialRoarDelay = FMath::RandRange(5.0f, 15.0f);
    GetWorldTimerManager().SetTimer(
        ChargeTimerHandle,
        this,
        &ATRex::PerformRoar,
        InitialRoarDelay,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("ATRex::BeginPlay — %s spawned at territory center (%.0f, %.0f, %.0f)"),
        *GetActorLabel(), HomeLocation.X, HomeLocation.Y, HomeLocation.Z);
}

void ATRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Charge logic — accelerate toward target
    if (bIsCharging && ChargeTarget)
    {
        ChargeTimer -= DeltaTime;

        FVector ToTarget = (ChargeTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        float ChargeSpeed = GetCharacterMovement()->MaxWalkSpeed * ChargeSpeedMultiplier;
        AddMovementInput(ToTarget, 1.0f);

        // End charge when time expires or close to target
        float DistToTarget = FVector::Dist(GetActorLocation(), ChargeTarget->GetActorLocation());
        if (ChargeTimer <= 0.0f || DistToTarget < AttackRange)
        {
            EndCharge();

            // Apply charge damage on impact
            if (DistToTarget < AttackRange && ChargeTarget)
            {
                FDamageEvent DmgEvent;
                ChargeTarget->TakeDamage(ChargeDamage, DmgEvent, GetController(), this);
            }
        }
    }
}

float ATRex::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // Enrage below 30% health — roar and charge immediately
    float HealthPercent = CurrentHealth / MaxHealth;
    if (HealthPercent < 0.3f && !bIsCharging)
    {
        PerformRoar();

        if (DamageCauser)
        {
            InitiateCharge(DamageCauser);
        }

        // Enrage: boost speed temporarily
        GetCharacterMovement()->MaxWalkSpeed = 750.0f;
        UE_LOG(LogTemp, Warning, TEXT("ATRex %s ENRAGED — health at %.0f%%"),
            *GetActorLabel(), HealthPercent * 100.0f);
    }

    return ActualDamage;
}

void ATRex::PerformRoar()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastRoarTime < RoarCooldown)
    {
        return;  // Still on cooldown
    }

    bIsRoaring = true;
    LastRoarTime = CurrentTime;

    UE_LOG(LogTemp, Log, TEXT("ATRex %s ROARS — radius %.0f"), *GetActorLabel(), RoarRadius);

    // Find all actors within roar radius and apply fear
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* NearActor : NearbyActors)
    {
        if (NearActor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), NearActor->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Apply roar fear — other dinosaurs and player will flee
            // This is a gameplay event — handled by the receiving actor's component
            UE_LOG(LogTemp, Verbose, TEXT("  Roar affects: %s (dist=%.0f)"),
                *NearActor->GetActorLabel(), Dist);
        }
    }

    // End roar after 3 seconds
    FTimerHandle RoarEndHandle;
    GetWorldTimerManager().SetTimer(RoarEndHandle, this, &ATRex::EndRoar, 3.0f, false);
}

void ATRex::InitiateCharge(AActor* Target)
{
    if (!Target || bIsCharging) return;

    bIsCharging = true;
    ChargeTarget = Target;
    ChargeTimer = ChargeDuration;

    // Boost movement speed for charge
    GetCharacterMovement()->MaxWalkSpeed = 550.0f * ChargeSpeedMultiplier;
    GetCharacterMovement()->MaxAcceleration = 3000.0f;

    UE_LOG(LogTemp, Log, TEXT("ATRex %s initiates CHARGE toward %s"),
        *GetActorLabel(), *Target->GetActorLabel());
}

void ATRex::EndCharge()
{
    bIsCharging = false;
    ChargeTarget = nullptr;
    ChargeTimer = 0.0f;

    // Restore normal movement
    GetCharacterMovement()->MaxWalkSpeed = 550.0f;
    GetCharacterMovement()->MaxAcceleration = 800.0f;

    UE_LOG(LogTemp, Log, TEXT("ATRex %s charge ended"), *GetActorLabel());
}

void ATRex::EndRoar()
{
    bIsRoaring = false;
}

void ATRex::CheckForBloodSmell()
{
    // Scan for bleeding/dead actors within smell radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* NearActor : NearbyActors)
    {
        if (NearActor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), NearActor->GetActorLocation());
        if (Dist <= BloodSmellRadius)
        {
            // Check if actor is a damaged creature (has low health)
            // This will be extended when SurvivalComponent is fully integrated
            // For now, log detection
            UE_LOG(LogTemp, Verbose, TEXT("ATRex %s smells potential prey: %s (dist=%.0f)"),
                *GetActorLabel(), *NearActor->GetActorLabel(), Dist);
        }
    }
}
