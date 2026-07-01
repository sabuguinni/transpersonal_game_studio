#include "TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATRexDinosaur::ATRexDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Capsule: massive T-Rex body ---
    GetCapsuleComponent()->InitCapsuleSize(120.0f, 200.0f);

    // --- Stats override: apex predator ---
    DinoStats.MaxHealth = 8000.0f;
    DinoStats.CurrentHealth = 8000.0f;
    DinoStats.AttackDamage = 80.0f;
    DinoStats.DetectionRadius = 2000.0f;
    DinoStats.AttackRadius = 250.0f;
    DinoStats.MoveSpeed = 550.0f;
    DinoStats.Mass = 8000.0f;
    DinoStats.bIsCarnivore = true;

    Species = EEng_DinosaurSpecies::TRex;

    // --- Movement ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 120.0f, 0.0f);
        MoveComp->Mass = DinoStats.Mass;
        MoveComp->JumpZVelocity = 0.0f; // T-Rex cannot jump
        MoveComp->MaxStepHeight = 80.0f;
    }

    bUseControllerRotationYaw = false;
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();

    BaseWalkSpeed = DinoStats.MoveSpeed;
    RoarCooldownRemaining = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur::BeginPlay — TRex spawned. HP=%.0f, Damage=%.0f, Detection=%.0fcm"),
        DinoStats.CurrentHealth, DinoStats.AttackDamage, DinoStats.DetectionRadius);
}

void ATRexDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateAbilityCooldowns(DeltaTime);

    // Auto-roar when entering hunting state and roar is ready
    if (CurrentBehaviorState == EEng_DinosaurBehaviorState::Hunting && RoarCooldownRemaining <= 0.0f)
    {
        ExecuteRoar();
    }

    // Auto-charge when close to target
    if (CurrentBehaviorState == EEng_DinosaurBehaviorState::Attacking && !bIsCharging)
    {
        if (CurrentTarget)
        {
            float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
            if (DistToTarget <= ChargeActivationDistance)
            {
                InitiateCharge();
            }
        }
    }
}

void ATRexDinosaur::ExecuteRoar()
{
    if (RoarCooldownRemaining > 0.0f) return;

    RoarCooldownRemaining = RoarCooldown;

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur::ExecuteRoar — ROAR! Radius=%.0fcm, StunDuration=%.1fs"),
        RoarRadius, RoarStunDuration);

    // Find all actors within roar radius
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Apply stun via damage event (stun handled by character's SurvivalComponent)
            UGameplayStatics::ApplyDamage(Actor, 0.0f, GetController(), this, UDamageType::StaticClass());
            UE_LOG(LogTemp, Log, TEXT("  Roar stun applied to %s (dist=%.0fcm)"), *Actor->GetName(), Dist);
        }
    }

#if WITH_EDITOR
    // Debug sphere in editor
    DrawDebugSphere(GetWorld(), GetActorLocation(), RoarRadius, 16, FColor::Orange, false, 2.0f);
#endif
}

void ATRexDinosaur::InitiateCharge()
{
    if (bIsCharging) return;
    if (!CurrentTarget) return;

    bIsCharging = true;
    ChargeDurationRemaining = MaxChargeDuration;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed * ChargeSpeedMultiplier;
    }

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur::InitiateCharge — Charging at %s! Speed=%.0f"),
        *CurrentTarget->GetName(), DinoStats.MoveSpeed * ChargeSpeedMultiplier);
}

void ATRexDinosaur::ExecuteStomp()
{
    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur::ExecuteStomp — Stomp! Damage=%.0f, Radius=%.0fcm"),
        StompDamage, StompRadius);

    // AoE damage in stomp radius
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        StompDamage,
        GetActorLocation(),
        StompRadius,
        UDamageType::StaticClass(),
        IgnoredActors,
        this,
        GetController(),
        true // full damage at center
    );

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), GetActorLocation(), StompRadius, 12, FColor::Red, false, 1.5f);
#endif
}

void ATRexDinosaur::UpdateAbilityCooldowns(float DeltaTime)
{
    // Tick roar cooldown
    if (RoarCooldownRemaining > 0.0f)
    {
        RoarCooldownRemaining = FMath::Max(0.0f, RoarCooldownRemaining - DeltaTime);
    }

    // Tick charge duration
    if (bIsCharging)
    {
        ChargeDurationRemaining -= DeltaTime;
        if (ChargeDurationRemaining <= 0.0f)
        {
            bIsCharging = false;
            UCharacterMovementComponent* MoveComp = GetCharacterMovement();
            if (MoveComp)
            {
                MoveComp->MaxWalkSpeed = BaseWalkSpeed;
            }
            UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur: Charge ended — returning to normal speed"));
        }
    }
}
