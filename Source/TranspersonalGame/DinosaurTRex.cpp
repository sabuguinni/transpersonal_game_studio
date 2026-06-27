// DinosaurTRex.cpp
// Tyrannosaurus Rex — apex predator implementation.
// Inherits from ADinosaurBase. Agent #12 will attach Behavior Trees.

#include "DinosaurTRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATRex::ATRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Species identity ────────────────────────────────────────────────────
    DinoStats.Species       = EEng_DinoSpecies::TyrannosaurusRex;
    DinoStats.MaxHealth     = 2500.0f;
    DinoStats.CurrentHealth = 2500.0f;
    DinoStats.AttackDamage  = 180.0f;
    DinoStats.MoveSpeed     = 650.0f;   // cm/s — fast for its size
    DinoStats.DetectionRadius = 3000.0f;
    DinoStats.TerritoryRadius = 8000.0f;
    DinoStats.bIsAggressive = true;
    DinoStats.bIsPack       = false;

    // ── Capsule sizing (large predator) ────────────────────────────────────
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 200.0f);

    // ── Movement ────────────────────────────────────────────────────────────
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed          = DinoStats.MoveSpeed;
        MoveComp->MaxAcceleration       = 800.0f;
        MoveComp->BrakingDecelerationWalking = 600.0f;
        MoveComp->JumpZVelocity         = 0.0f;   // TRex cannot jump
        MoveComp->GravityScale          = 1.2f;   // Heavy — falls faster
        MoveComp->RotationRate          = FRotator(0.0f, 180.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
    }

    // ── Actor label ─────────────────────────────────────────────────────────
    Tags.Add(FName("Dinosaur"));
    Tags.Add(FName("Predator"));
    Tags.Add(FName("TRex"));
}

void ATRex::BeginPlay()
{
    Super::BeginPlay();

    // Start in patrol mode — Combat AI (Agent #12) will override via BT
    SetBehaviorState(EEng_DinoBehaviorState::Patrolling);

    UE_LOG(LogTemp, Log, TEXT("ATRex [%s] spawned — Health:%.0f Attack:%.0f"),
        *GetName(), DinoStats.CurrentHealth, DinoStats.AttackDamage);
}

void ATRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Tick down roar cooldown
    if (RoarCooldownRemaining > 0.0f)
    {
        RoarCooldownRemaining -= DeltaTime;
    }
}

void ATRex::PerformRoar()
{
    if (RoarCooldownRemaining > 0.0f)
    {
        return; // Still on cooldown
    }

    RoarCooldownRemaining = RoarCooldown;

    // Find all actors within roar radius and apply stun tag
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("Player"), OverlappingActors);

    FVector Origin = GetActorLocation();
    for (AActor* Actor : OverlappingActors)
    {
        if (!Actor) continue;
        float Dist = FVector::Dist(Origin, Actor->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Apply stun via tag — Agent #12 Behavior Trees will read this tag
            Actor->Tags.AddUnique(FName("Stunned"));
            UE_LOG(LogTemp, Log, TEXT("ATRex roar stunned: %s for %.1fs"),
                *Actor->GetName(), RoarStunDuration);

            // Schedule stun removal (simple timer)
            FTimerHandle StunTimer;
            TWeakObjectPtr<AActor> WeakActor = Actor;
            GetWorldTimerManager().SetTimer(StunTimer, [WeakActor]()
            {
                if (WeakActor.IsValid())
                {
                    WeakActor->Tags.Remove(FName("Stunned"));
                }
            }, RoarStunDuration, false);
        }
    }

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), Origin, RoarRadius, 16, FColor::Orange, false, 2.0f);
#endif

    UE_LOG(LogTemp, Log, TEXT("ATRex [%s] ROAR — radius:%.0f stunned:%d actors"),
        *GetName(), RoarRadius, OverlappingActors.Num());
}

void ATRex::ChargeAttack(AActor* Target)
{
    if (!Target || bIsCharging) return;

    bIsCharging = true;

    // Boost movement speed during charge
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinoStats.MoveSpeed * 1.8f; // 180% speed during charge
    }

    // Apply charge damage (multiplied)
    float ChargeDmg = DinoStats.AttackDamage * ChargeDamageMultiplier;
    ApplyDamage(Target, ChargeDmg);

    // Reset charge state after 2 seconds
    FTimerHandle ChargeResetTimer;
    GetWorldTimerManager().SetTimer(ChargeResetTimer, [this]()
    {
        bIsCharging = false;
        UCharacterMovementComponent* MC = GetCharacterMovement();
        if (MC)
        {
            MC->MaxWalkSpeed = DinoStats.MoveSpeed;
        }
        UE_LOG(LogTemp, Log, TEXT("ATRex [%s] charge ended"), *GetName());
    }, 2.0f, false);

    UE_LOG(LogTemp, Log, TEXT("ATRex [%s] CHARGE on [%s] — damage:%.0f"),
        *GetName(), *Target->GetName(), ChargeDmg);
}
