// TyrannosaurusRex.cpp
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260630_005
// Full implementation of Tyrannosaurus Rex — apex predator

#include "Dinosaurs/TyrannosaurusRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
    // Species identity
    Species = EDinosaurSpecies::TyrannosaurusRex;
    DinoSize = EDinosaurSize::Huge;

    // Survival stats — apex predator
    MaxHealth = 2000.0f;
    CurrentHealth = 2000.0f;
    MaxStamina = 600.0f;
    CurrentStamina = 600.0f;
    AttackDamage = 150.0f;
    AttackRange = 250.0f;

    // Movement — powerful but not fast
    GetCharacterMovement()->MaxWalkSpeed = 350.0f;
    GetCharacterMovement()->MaxAcceleration = 800.0f;
    GetCharacterMovement()->BrakingDecelerationWalking = 600.0f;
    GetCharacterMovement()->GravityScale = 1.4f;
    GetCharacterMovement()->JumpZVelocity = 0.0f; // TRex cannot jump
    GetCharacterMovement()->bCanWalkOffLedges = true;

    // Capsule — massive body
    GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
    GetCapsuleComponent()->SetCapsuleRadius(100.0f);

    // Territory and detection — large predator needs large territory
    TerritoryRadius = 3500.0f;
    DetectionRange = 2500.0f;
    AggressionLevel = 0.9f; // Very aggressive

    // Roar ability
    RoarRadius = 1500.0f;
    RoarCooldown = 30.0f;

    // Stomp
    StompRadius = 300.0f;
}

void ATyrannosaurusRex::BeginPlay()
{
    Super::BeginPlay();

    // Start periodic roar timer — TRex announces its presence
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            RoarTimerHandle,
            this,
            &ATyrannosaurusRex::PeriodicRoar,
            RoarCooldown,
            true, // looping
            10.0f // first roar after 10 seconds
        );
    }

    UE_LOG(LogTemp, Log, TEXT("TyrannosaurusRex [%s] spawned — HP:%.0f DMG:%.0f Speed:%.0f Territory:%.0f"),
        *GetName(), MaxHealth, AttackDamage,
        GetCharacterMovement()->MaxWalkSpeed, TerritoryRadius);
}

void ATyrannosaurusRex::Roar()
{
    if (CurrentAIState == EDinoAIState::Dead) return;

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROARS — radius %.0f"), *GetName(), RoarRadius);

    // Apply fear to nearby characters within roar radius
    // Agent #12 (Combat AI) will implement full fear system via Blueprint override
    // For now: log all actors within roar radius
    if (UWorld* World = GetWorld())
    {
        TArray<FOverlapResult> Overlaps;
        FCollisionShape Sphere = FCollisionShape::MakeSphere(RoarRadius);
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(this);

        World->OverlapMultiByChannel(
            Overlaps,
            GetActorLocation(),
            FQuat::Identity,
            ECC_Pawn,
            Sphere,
            Params
        );

        for (const FOverlapResult& Overlap : Overlaps)
        {
            if (AActor* NearbyActor = Overlap.GetActor())
            {
                UE_LOG(LogTemp, Verbose, TEXT("TRex roar affects: %s"), *NearbyActor->GetName());
            }
        }

#if WITH_EDITOR
        // Debug sphere in editor
        DrawDebugSphere(World, GetActorLocation(), RoarRadius, 16,
            FColor::Red, false, 3.0f, 0, 5.0f);
#endif
    }
}

void ATyrannosaurusRex::PeriodicRoar()
{
    // Only roar when active (not dead, not fleeing)
    if (CurrentAIState != EDinoAIState::Dead &&
        CurrentAIState != EDinoAIState::Fleeing)
    {
        Roar();
    }
}

float ATyrannosaurusRex::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    // TRex enrages when damaged — speed burst and immediate aggression
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (ActualDamage > 0.0f && CurrentAIState != EDinoAIState::Dead)
    {
        // Enrage: temporary speed boost
        float EnragedSpeed = GetCharacterMovement()->MaxWalkSpeed * 1.3f;
        GetCharacterMovement()->MaxWalkSpeed = FMath::Min(EnragedSpeed, 600.0f);

        // Immediate roar on being hit
        Roar();

        UE_LOG(LogTemp, Warning, TEXT("TRex [%s] ENRAGED — speed boosted to %.0f"),
            *GetName(), GetCharacterMovement()->MaxWalkSpeed);

        // Reset speed after 8 seconds
        if (UWorld* World = GetWorld())
        {
            FTimerHandle EnrageResetHandle;
            World->GetTimerManager().SetTimer(EnrageResetHandle, [this]()
            {
                if (IsValid(this) && CurrentAIState != EDinoAIState::Dead)
                {
                    GetCharacterMovement()->MaxWalkSpeed = 350.0f;
                }
            }, 8.0f, false);
        }
    }

    return ActualDamage;
}
