// TRexCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Tyrannosaurus Rex — apex predator implementation.
// Inherits ADinosaurBase; overrides stats and adds roar/stomp abilities.

#include "World/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Override base species identity
    Species = EEng_DinoSpecies::TRex;
    Diet    = EEng_DinoDiet::Carnivore;

    // T-Rex stats — apex predator values
    DinoStats.MaxHealth        = 2000.0f;
    DinoStats.CurrentHealth    = 2000.0f;
    DinoStats.AttackDamage     = 200.0f;
    DinoStats.AttackRange      = 220.0f;    // Bite range (cm)
    DinoStats.MoveSpeed        = 600.0f;    // ~22 km/h — realistic T-Rex sprint
    DinoStats.DetectionRadius  = 3000.0f;  // Excellent vision
    DinoStats.FleeHealthThresh = 0.15f;    // T-Rex almost never flees
    DinoStats.AttackCooldown   = 2.5f;     // Slow but devastating
    DinoStats.Mass             = 8000.0f;  // kg — adult T-Rex
    DinoStats.bIsPack          = false;    // Solitary hunter

    // Movement — T-Rex is large and deliberate
    GetCharacterMovement()->MaxWalkSpeed        = DinoStats.MoveSpeed;
    GetCharacterMovement()->MaxAcceleration     = 800.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 1200.0f;
    GetCharacterMovement()->RotationRate        = FRotator(0.0f, 120.0f, 0.0f);
    GetCharacterMovement()->bOrientRotationToMovement = true;

    // Capsule size — T-Rex is enormous
    // Note: actual capsule resize happens in BeginPlay after mesh is set
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Ensure stats are applied after any Blueprint overrides
    InitializeSpeciesStats();

    RoarCooldownRemaining = FMath::RandRange(0.0f, RoarCooldown * 0.5f); // Stagger initial roar
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Roar cooldown countdown
    if (RoarCooldownRemaining > 0.0f)
    {
        RoarCooldownRemaining -= DeltaTime;
    }

    // Roar suppression countdown
    if (bRoarActive)
    {
        RoarSuppressRemaining -= DeltaTime;
        if (RoarSuppressRemaining <= 0.0f)
        {
            bRoarActive = false;
        }
    }

    // Auto-roar when hunting and cooldown is ready
    if (CurrentState == EEng_DinoState::Hunt && RoarCooldownRemaining <= 0.0f)
    {
        PerformRoar();
    }
}

void ATRexCharacter::PerformRoar()
{
    if (RoarCooldownRemaining > 0.0f) return;

    RoarCooldownRemaining = RoarCooldown;
    bRoarActive = true;
    RoarSuppressRemaining = RoarSuppressDuration;

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter::PerformRoar — %s roars! Suppressing prey in %.0f radius for %.1fs"),
        *GetActorLabel(), DinoStats.DetectionRadius, RoarSuppressDuration);

    // Radial fear effect — find all actors in detection radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist > DinoStats.DetectionRadius) continue;

        // Apply fear to any ADinosaurBase that is prey
        ADinosaurBase* OtherDino = Cast<ADinosaurBase>(Actor);
        if (OtherDino && OtherDino->Diet == EEng_DinoDiet::Herbivore)
        {
            // Force flee state on herbivores within range
            OtherDino->CurrentState = EEng_DinoState::Flee;
            OtherDino->CurrentTarget = this;
            UE_LOG(LogTemp, Verbose, TEXT("  Roar suppressed: %s"), *OtherDino->GetActorLabel());
        }
    }

#if WITH_EDITOR
    // Debug sphere for roar radius
    DrawDebugSphere(GetWorld(), GetActorLocation(), DinoStats.DetectionRadius,
        16, FColor::Orange, false, 3.0f, 0, 5.0f);
#endif
}

void ATRexCharacter::PerformStomp()
{
    FVector StompLocation = GetActorLocation();
    StompLocation.Z -= GetCapsuleComponent() ? GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 100.0f;

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter::PerformStomp — %s stomps at (%.0f, %.0f, %.0f)"),
        *GetActorLabel(), StompLocation.X, StompLocation.Y, StompLocation.Z);

    // Radial damage
    TArray<AActor*> IgnoredActors;
    IgnoredActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        StompDamage,
        StompLocation,
        StompRadius,
        UDamageType::StaticClass(),
        IgnoredActors,
        this,
        GetInstigatorController(),
        true // full damage at center, falloff to edge
    );

    // Camera shake for nearby players
    if (StompCameraShakeClass)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC)
        {
            float DistToPlayer = FVector::Dist(GetActorLocation(),
                PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : FVector::ZeroVector);
            float ShakeScale = FMath::Clamp(1.0f - (DistToPlayer / 3000.0f), 0.0f, 1.0f);
            if (ShakeScale > 0.01f)
            {
                PC->ClientStartCameraShake(StompCameraShakeClass, ShakeScale);
            }
        }
    }

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), StompLocation, StompRadius,
        12, FColor::Red, false, 2.0f, 0, 3.0f);
#endif
}

void ATRexCharacter::InitializeSpeciesStats()
{
    // Ensure movement component reflects current stats
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = DinoStats.MoveSpeed;
    }

    // Ensure health is clamped to max
    DinoStats.CurrentHealth = FMath::Min(DinoStats.CurrentHealth, DinoStats.MaxHealth);
}
