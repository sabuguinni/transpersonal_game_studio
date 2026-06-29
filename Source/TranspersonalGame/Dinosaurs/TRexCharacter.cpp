// TRexCharacter.cpp
// Transpersonal Game Studio — Core Systems Programmer (Agent #3)
// Cycle: PROD_CYCLE_AUTO_20260629_009

#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- TRex Species Identity ---
    Species = EDinosaurSpecies::TRex;
    Diet = EDinosaurDiet::Carnivore;

    // --- TRex Survival Stats ---
    MaxHealth = 2000.0f;
    CurrentHealth = 2000.0f;
    AttackDamage = 150.0f;
    AttackRange = 280.0f;
    DetectionRange = 2500.0f;
    AggressionRange = 1800.0f;
    HungerDrainRate = 1.5f;   // Apex predator needs more food
    StaminaRegenRate = 8.0f;  // Slower regen — high energy cost

    // --- TRex Movement ---
    // Walk speed — TRex patrol pace
    GetCharacterMovement()->MaxWalkSpeed = 350.0f;
    // Jump height — TRex cannot jump (too heavy)
    GetCharacterMovement()->JumpZVelocity = 0.0f;
    GetCharacterMovement()->bCanWalkOffLedges = true;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 120.0f, 0.0f);

    // --- TRex Capsule ---
    GetCapsuleComponent()->SetCapsuleHalfHeight(220.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // --- Default State ---
    CurrentBehaviorState = EDinosaurBehaviorState::Idle;
    bIsEnraged = false;
    bRoarOnCooldown = false;
    TimeSinceLastRoar = 0.0f;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // TRex starts in Wandering state — patrols territory
    CurrentBehaviorState = EDinosaurBehaviorState::Wandering;
    UE_LOG(LogTemp, Log, TEXT("TRex [%s] spawned — MaxHealth=%.0f, AttackDamage=%.0f, DetectionRange=%.0f"),
        *GetName(), MaxHealth, AttackDamage, DetectionRange);
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Track roar cooldown
    if (bRoarOnCooldown)
    {
        TimeSinceLastRoar += DeltaTime;
        if (TimeSinceLastRoar >= RoarCooldown)
        {
            bRoarOnCooldown = false;
            TimeSinceLastRoar = 0.0f;
        }
    }

    // Check enrage threshold every tick
    CheckEnrageThreshold();
}

void ATRexCharacter::OnAlerted()
{
    // Override base alert — TRex charges immediately
    if (CurrentBehaviorState == EDinosaurBehaviorState::Dead) return;

    CurrentBehaviorState = EDinosaurBehaviorState::Chasing;

    // Increase movement speed to charge speed
    GetCharacterMovement()->MaxWalkSpeed = ChargeSpeed;

    // Roar on alert if not on cooldown
    if (!bRoarOnCooldown)
    {
        PerformRoar();
    }

    UE_LOG(LogTemp, Warning, TEXT("TRex [%s] ALERTED — initiating charge at speed %.0f"),
        *GetName(), ChargeSpeed);
}

void ATRexCharacter::PerformRoar()
{
    if (bRoarOnCooldown) return;
    if (CurrentBehaviorState == EDinosaurBehaviorState::Dead) return;

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROAR — radius=%.0f, fear=%.2f"),
        *GetName(), RoarRadius, RoarFearIntensity);

    // Find all actors within roar radius
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= RoarRadius)
        {
            // Log fear application — actual fear system handled by SurvivalComponent
            UE_LOG(LogTemp, Log, TEXT("TRex roar affects [%s] at distance %.0f — fear intensity %.2f"),
                *Actor->GetName(), Distance, RoarFearIntensity);
        }
    }

    // Start roar cooldown
    bRoarOnCooldown = true;
    TimeSinceLastRoar = 0.0f;

    // Debug visualization in editor
#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), RoarRadius, 16, FColor::Orange, false, 3.0f);
#endif
}

void ATRexCharacter::PerformStompAttack()
{
    if (CurrentBehaviorState == EDinosaurBehaviorState::Dead) return;

    UWorld* World = GetWorld();
    if (!World) return;

    float StompDamage = AttackDamage * StompDamageMultiplier;

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] STOMP — radius=%.0f, damage=%.0f"),
        *GetName(), StompRadius, StompDamage);

    // Find all actors within stomp radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Distance <= StompRadius)
        {
            // Apply stomp damage via UE5 damage system
            UGameplayStatics::ApplyDamage(
                Actor,
                StompDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );

            UE_LOG(LogTemp, Log, TEXT("TRex stomp hit [%s] for %.0f damage"),
                *Actor->GetName(), StompDamage);
        }
    }

    // Debug visualization
#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), StompRadius, 12, FColor::Red, false, 2.0f);
#endif
}

void ATRexCharacter::EnterEnragedState()
{
    if (bIsEnraged) return; // Already enraged
    if (CurrentBehaviorState == EDinosaurBehaviorState::Dead) return;

    bIsEnraged = true;
    CurrentBehaviorState = EDinosaurBehaviorState::Attacking;

    // Boost movement speed
    float NewSpeed = GetCharacterMovement()->MaxWalkSpeed * EnragedSpeedMultiplier;
    GetCharacterMovement()->MaxWalkSpeed = NewSpeed;

    // Boost attack damage
    AttackDamage *= 1.25f;

    UE_LOG(LogTemp, Warning, TEXT("TRex [%s] ENRAGED — speed=%.0f, damage=%.0f"),
        *GetName(), NewSpeed, AttackDamage);

    // Immediate roar on enrage
    PerformRoar();
}

void ATRexCharacter::CheckEnrageThreshold()
{
    if (bIsEnraged) return;
    if (CurrentBehaviorState == EDinosaurBehaviorState::Dead) return;
    if (MaxHealth <= 0.0f) return;

    float HealthPercent = CurrentHealth / MaxHealth;
    if (HealthPercent < 0.30f)
    {
        EnterEnragedState();
    }
}

void ATRexCharacter::ResetRoarCooldown()
{
    bRoarOnCooldown = false;
    TimeSinceLastRoar = 0.0f;
}
