// TRexCharacter.cpp — Tyrannosaurus Rex implementation
// Prehistoric Survival Game — Transpersonal Game Studio
// Agent #3 — Core Systems Programmer — PROD_CYCLE_AUTO_20260625_007

#include "TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // === Species identity ===
    DinosaurSpecies = EDinosaurSpecies::TRex;

    // === Apex predator stats ===
    MaxHealth        = 2000.f;
    CurrentHealth    = 2000.f;
    AttackDamage     = 200.f;
    MovementSpeed    = 400.f;   // Lumbering walk
    SprintSpeed      = 900.f;   // Terrifying charge
    DetectionRadius  = 4000.f;  // Sees prey from far
    AttackRange      = 350.f;   // Long bite reach
    MaxHunger        = 150.f;
    MaxThirst        = 100.f;
    HungerDrainRate  = 0.8f;    // Apex predator needs lots of food
    ThirstDrainRate  = 0.4f;

    // === Scale — TRex is massive ===
    SetActorScale3D(FVector(3.0f, 3.0f, 3.0f));

    // === Movement defaults ===
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
        GetCharacterMovement()->JumpZVelocity = 0.f;  // TRex cannot jump
        GetCharacterMovement()->bCanWalkOffLedges = true;
        GetCharacterMovement()->GravityScale = 1.5f;  // Heavy
    }
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();
    LastRoarTime = -RoarCooldown; // Allow immediate roar on spawn
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Auto-roar when attacking and cooldown elapsed
    if (CurrentBehaviorState == EDinosaurBehavior::Attacking)
    {
        float Now = GetWorld()->GetTimeSeconds();
        if ((Now - LastRoarTime) >= RoarCooldown)
        {
            PerformRoar();
        }
    }
}

void ATRexCharacter::PerformRoar()
{
    if (!GetWorld()) return;

    LastRoarTime = GetWorld()->GetTimeSeconds();

    // Find all actors in roar radius and apply fear
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Notify nearby actors — they can implement their own fear response
            Actor->ReceiveAnyDamage(0.f, nullptr, nullptr, this);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter::PerformRoar — radius=%.0f"), RoarRadius);
}

void ATRexCharacter::PerformStomp()
{
    if (!GetWorld()) return;

    FVector StompOrigin = GetActorLocation();

    // Radial damage in stomp radius
    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        StompDamage,
        StompOrigin,
        StompRadius,
        nullptr,   // DamageType — use default
        TArray<AActor*>(),
        this,
        GetController(),
        true       // DoFullDamage — no falloff
    );

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter::PerformStomp — damage=%.0f radius=%.0f"), StompDamage, StompRadius);
}

void ATRexCharacter::BeginCharge(FVector TargetLocation)
{
    if (bIsCharging) return;

    bIsCharging = true;

    // Sprint speed during charge
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    }

    // End charge after 3 seconds
    GetWorld()->GetTimerManager().SetTimer(
        ChargeTimerHandle,
        this,
        &ATRexCharacter::EndCharge,
        3.0f,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter::BeginCharge — target=(%.0f,%.0f,%.0f)"),
        TargetLocation.X, TargetLocation.Y, TargetLocation.Z);
}

void ATRexCharacter::EndCharge()
{
    bIsCharging = false;

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter::EndCharge"));
}
