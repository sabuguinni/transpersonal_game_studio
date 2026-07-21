#include "AI/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ATRexCharacter::ATRexCharacter()
{
    // ── TRex-specific stats ──────────────────────────────────────────────────
    MaxHealth        = 2000.f;
    CurrentHealth    = 2000.f;
    AttackDamage     = 200.f;
    DetectionRadius  = 3000.f;
    AttackRadius     = 350.f;
    RoamSpeed        = 400.f;
    SprintSpeed      = 1200.f;
    NormalSprintSpeed = 1200.f;

    // TRex is a carnivore — hunts on sight
    bIsCarnivore = true;
    bIsPackHunter = false;

    // Species tag
    DinoSpecies = EEng_DinoSpecies::TRex;

    // Large collision capsule
    GetCapsuleComponent()->SetCapsuleHalfHeight(180.f);
    GetCapsuleComponent()->SetCapsuleRadius(80.f);

    // Movement defaults
    GetCharacterMovement()->MaxWalkSpeed = RoamSpeed;
    GetCharacterMovement()->BrakingDecelerationWalking = 800.f;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 120.f, 0.f);

    PrimaryActorTick.bCanEverTick = true;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter[%s]: BeginPlay — Health=%.0f, Damage=%.0f, DetectRadius=%.0f"),
        *GetName(), CurrentHealth, AttackDamage, DetectionRadius);
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // TRex-specific tick logic (charge, roar cooldowns) handled via timers
}

// ── Roar ─────────────────────────────────────────────────────────────────────

void ATRexCharacter::PerformRoar()
{
    if (bIsRoaring) return;

    bIsRoaring = true;
    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter[%s]: ROAR initiated — 0.8s wind-up"), *GetName());

    // Pause movement during roar
    GetCharacterMovement()->MaxWalkSpeed = 0.f;

    // Schedule roar completion
    GetWorldTimerManager().SetTimer(
        RoarTimerHandle,
        this,
        &ATRexCharacter::OnRoarComplete,
        0.8f,
        false
    );
}

void ATRexCharacter::OnRoarComplete()
{
    bIsRoaring = false;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter[%s]: Roar complete — attacking!"), *GetName());
}

// ── Stomp ─────────────────────────────────────────────────────────────────────

void ATRexCharacter::PerformStomp()
{
    UWorld* World = GetWorld();
    if (!World) return;

    const FVector StompOrigin = GetActorLocation();
    const float StompDamage = AttackDamage * StompDamageMultiplier;

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter[%s]: STOMP — radius=%.0f, damage=%.0f"),
        *GetName(), StompRadius, StompDamage);

    // Radial damage to all actors in stomp radius
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        World,
        StompDamage,
        StompOrigin,
        StompRadius,
        UDamageType::StaticClass(),
        IgnoreActors,
        this,
        GetController(),
        true
    );

#if WITH_EDITOR
    // Debug visualisation in editor
    DrawDebugSphere(World, StompOrigin, StompRadius, 16, FColor::Orange, false, 1.5f);
#endif
}

// ── Charge ────────────────────────────────────────────────────────────────────

void ATRexCharacter::StartCharge()
{
    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter[%s]: CHARGE started — speed boost for %.1fs"),
        *GetName(), ChargeDuration);

    // Boost speed
    GetCharacterMovement()->MaxWalkSpeed = 1800.f;

    // Schedule charge end
    GetWorldTimerManager().SetTimer(
        ChargeTimerHandle,
        this,
        &ATRexCharacter::OnChargeComplete,
        ChargeDuration,
        false
    );
}

void ATRexCharacter::OnChargeComplete()
{
    GetCharacterMovement()->MaxWalkSpeed = NormalSprintSpeed;
    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter[%s]: Charge complete — speed restored"), *GetName());
}
