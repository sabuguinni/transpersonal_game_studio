// TRexCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// T-Rex species implementation: apex predator stats, charge attack, roar ability

#include "TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATRexCharacter::ATRexCharacter()
{
    // Species identity
    SpeciesName = FName("Tyrannosaurus Rex");

    // Apex predator combat stats
    MaxHealth       = 2000.0f;
    CurrentHealth   = 2000.0f;
    AttackDamage    = 150.0f;
    AttackRange     = 300.0f;
    AttackCooldown  = 2.5f;

    // Movement — T-Rex is powerful but not fast
    PatrolSpeed     = 200.0f;
    ChaseSpeed      = 550.0f;
    TerritoryRadius = 3500.0f;
    ThreatDetectRadius = 2500.0f;

    // Charge attack parameters
    bIsCharging         = false;
    ChargeSpeed         = 900.0f;
    ChargeDamage        = 300.0f;
    ChargeCooldown      = 12.0f;
    ChargeRange         = 1800.0f;
    bChargeReady        = true;

    // Roar parameters
    RoarRadius          = 2000.0f;
    RoarFearDuration    = 5.0f;
    RoarCooldown        = 20.0f;
    bRoarReady          = true;

    // Capsule — T-Rex is enormous
    GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // Movement component
    GetCharacterMovement()->MaxWalkSpeed = PatrolSpeed;
    GetCharacterMovement()->MaxAcceleration = 800.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 1200.0f;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 180.0f, 0.0f);
    GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Start roar cooldown timer on spawn (don't roar immediately)
    GetWorldTimerManager().SetTimer(
        RoarCooldownHandle,
        this,
        &ATRexCharacter::OnRoarCooldownExpired,
        RoarCooldown,
        false
    );
    bRoarReady = false;
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // During charge, apply extra forward force
    if (bIsCharging)
    {
        FVector Forward = GetActorForwardVector();
        AddMovementInput(Forward, 1.0f);
    }
}

void ATRexCharacter::PerformAttack()
{
    // Override: T-Rex checks for charge opportunity before normal attack
    if (bChargeReady)
    {
        APawn* Target = GetCurrentTarget();
        if (Target)
        {
            float DistToTarget = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
            if (DistToTarget > AttackRange && DistToTarget <= ChargeRange)
            {
                StartCharge(Target);
                return;
            }
        }
    }

    // Fall back to base attack
    Super::PerformAttack();
}

void ATRexCharacter::StartCharge(APawn* Target)
{
    if (!Target || bIsCharging) return;

    bIsCharging   = true;
    bChargeReady  = false;

    // Face the target
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FRotator LookAt  = ToTarget.Rotation();
    SetActorRotation(LookAt);

    // Boost movement speed for charge
    GetCharacterMovement()->MaxWalkSpeed = ChargeSpeed;

    // Apply damage at end of charge (3 seconds)
    FTimerHandle ChargeEndHandle;
    GetWorldTimerManager().SetTimer(
        ChargeEndHandle,
        this,
        &ATRexCharacter::EndCharge,
        3.0f,
        false
    );

    // Charge cooldown
    GetWorldTimerManager().SetTimer(
        ChargeCooldownHandle,
        this,
        &ATRexCharacter::OnChargeCooldownExpired,
        ChargeCooldown,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] CHARGING at %s"), *GetName(), *Target->GetName());
}

void ATRexCharacter::EndCharge()
{
    bIsCharging = false;

    // Restore normal chase speed
    GetCharacterMovement()->MaxWalkSpeed = ChaseSpeed;

    // Deal charge damage to anything in melee range
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= AttackRange * 1.5f)
        {
            UGameplayStatics::ApplyDamage(
                Actor,
                ChargeDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
            UE_LOG(LogTemp, Log, TEXT("TRex CHARGE HIT: %s for %.0f dmg"), *Actor->GetName(), ChargeDamage);
        }
    }
}

void ATRexCharacter::PerformRoar()
{
    if (!bRoarReady) return;

    bRoarReady = false;

    // Apply fear to all pawns in roar radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    int32 FearCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Tag the actor as feared (gameplay tag or simple property set)
            // For now, apply a small stun via velocity knockback
            APawn* Pawn = Cast<APawn>(Actor);
            if (Pawn && Pawn->GetMovementComponent())
            {
                FVector AwayDir = (Pawn->GetActorLocation() - GetActorLocation()).GetSafeNormal();
                Pawn->GetMovementComponent()->Velocity += AwayDir * 600.0f;
            }
            FearCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TRex ROAR: feared %d pawns within %.0f units"), FearCount, RoarRadius);

    // Restart roar cooldown
    GetWorldTimerManager().SetTimer(
        RoarCooldownHandle,
        this,
        &ATRexCharacter::OnRoarCooldownExpired,
        RoarCooldown,
        false
    );
}

void ATRexCharacter::OnChargeCooldownExpired()
{
    bChargeReady = true;
    UE_LOG(LogTemp, Log, TEXT("TRex [%s] charge READY"), *GetName());
}

void ATRexCharacter::OnRoarCooldownExpired()
{
    bRoarReady = true;
    // Immediately roar when cooldown expires if in combat
    PerformRoar();
}

APawn* ATRexCharacter::GetCurrentTarget() const
{
    // Delegate to base class threat detection
    // The base class tracks the current chase target via its AI state
    // We expose it here for charge logic
    TArray<AActor*> NearbyPawns;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyPawns);

    APawn* Closest = nullptr;
    float  MinDist = ThreatDetectRadius;

    for (AActor* Actor : NearbyPawns)
    {
        if (Actor == this) continue;
        // Skip other dinosaurs (T-Rex only attacks player/prey)
        if (Actor->IsA(ADinosaurBase::StaticClass())) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < MinDist)
        {
            MinDist = Dist;
            Closest = Cast<APawn>(Actor);
        }
    }

    return Closest;
}
