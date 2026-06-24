// TRexCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio

#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // Species identity
    Species = EDinoSpecies::TyrannosaurusRex;

    // Apex predator stats
    MaxHealth       = 2000.0f;
    CurrentHealth   = 2000.0f;
    AttackDamage    = 150.0f;
    AttackRange     = 350.0f;
    DetectionRange  = 4000.0f;
    MaxStamina      = 200.0f;
    CurrentStamina  = 200.0f;

    // TRex is large but not the fastest — burst speed
    GetCharacterMovement()->MaxWalkSpeed        = 600.0f;
    GetCharacterMovement()->MaxAcceleration     = 800.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 400.0f;
    GetCharacterMovement()->JumpZVelocity       = 0.0f;   // TRex cannot jump
    GetCharacterMovement()->bCanWalkOffLedges   = false;

    // Scale — TRex is massive
    SetActorScale3D(FVector(3.5f, 3.5f, 3.5f));
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Schedule periodic roar every 15-25 seconds when hunting
    GetWorldTimerManager().SetTimer(
        RoarTimerHandle,
        this,
        &ATRexCharacter::MaybeRoar,
        20.0f,
        true,
        10.0f  // First roar after 10s
    );
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    // TRex-specific tick logic can be added here
    // Parent handles behavior state machine, stamina regen, combat
}

void ATRexCharacter::PerformRoar()
{
    if (bIsRoaring || !IsAlive()) return;

    bIsRoaring = true;

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROARS — radius %.0fcm"), *GetActorLabel(), RoarRadius);

    // Find all actors in roar radius and apply fear
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    FVector MyLoc = GetActorLocation();
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(MyLoc, Actor->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Apply fear damage type — TranspersonalCharacter listens for this
            UGameplayStatics::ApplyDamage(
                Actor,
                0.0f,  // No direct damage — fear effect only (handled by SurvivalComponent)
                GetController(),
                this,
                UDamageType::StaticClass()
            );
            UE_LOG(LogTemp, Verbose, TEXT("TRex roar reached: %s (dist=%.0f)"), *Actor->GetActorLabel(), Dist);
        }
    }

    // Debug sphere in editor
#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), MyLoc, RoarRadius, 16, FColor::Red, false, 3.0f);
#endif

    // Reset roar flag after animation would finish (~3s)
    FTimerHandle ResetHandle;
    GetWorldTimerManager().SetTimer(ResetHandle, [this]() { bIsRoaring = false; }, 3.0f, false);
}

void ATRexCharacter::PerformStomp()
{
    if (!IsAlive()) return;

    FVector StompOrigin = GetActorLocation();
    StompOrigin.Z -= 50.0f;  // Ground level

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] STOMPS — radius %.0fcm, damage %.0f"),
        *GetActorLabel(), StompRadius, StompDamage);

    // Radial damage at stomp point
    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        StompDamage,
        StompOrigin,
        StompRadius,
        UDamageType::StaticClass(),
        TArray<AActor*>(),  // Ignore list (empty = hit all)
        this,
        GetController(),
        true  // Do full damage at center, falloff toward edge
    );

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), StompOrigin, StompRadius, 12, FColor::Orange, false, 2.0f);
#endif
}

void ATRexCharacter::MaybeRoar()
{
    // Only roar when hunting or in combat
    if (CurrentState == EDinoState::Hunting || CurrentState == EDinoState::Attacking)
    {
        PerformRoar();
    }
}
