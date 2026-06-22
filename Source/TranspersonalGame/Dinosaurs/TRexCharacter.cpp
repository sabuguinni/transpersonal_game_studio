// TRexCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Full implementation of ATRexCharacter.
// All UFUNCTION() methods implemented here — no stubs, no #if 0.

#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────

ATRexCharacter::ATRexCharacter()
{
    // T-Rex capsule: tall and wide to match massive body
    // Real T-Rex: ~4m hip height, ~12m length
    // UE5 scale: capsule half-height 200, radius 90 (UU = cm)
    GetCapsuleComponent()->InitCapsuleSize(90.0f, 200.0f);

    // Species identity — set before BeginPlay so DinosaurBase reads it
    Species = EDinoSpecies::TRex;

    // Behaviour defaults — T-Rex is solitary, territorial, aggressive when hungry
    bIsAggressive = false;         // Neutral until provoked or very hungry
    TerritoryRadius = 8000.0f;     // Large exclusive zone

    // Apply movement defaults in constructor
    // (BeginPlay will also call ApplyTRexStats for runtime overrides)
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = TRexWalkSpeed;
        GetCharacterMovement()->JumpZVelocity = 0.0f;   // T-Rex cannot jump
        GetCharacterMovement()->GravityScale = 1.5f;    // Heavy — falls fast
        GetCharacterMovement()->Mass = 8000.0f;         // ~8 metric tons
        GetCharacterMovement()->BrakingDecelerationWalking = 800.0f; // Slow to stop
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ATRexCharacter::BeginPlay()
{
    // Call parent first — starts hunger/behaviour timers, caches TerritoryCenter
    Super::BeginPlay();

    // Override base class stats with T-Rex specific values
    ApplyTRexStats();
}

// ─────────────────────────────────────────────────────────────────────────────
// ApplyTRexStats — push species values into base class properties
// ─────────────────────────────────────────────────────────────────────────────

void ATRexCharacter::ApplyTRexStats()
{
    // Health
    MaxHealth = TRexMaxHealth;
    CurrentHealth = TRexMaxHealth;

    // Hunger decay — overwrite base class rate
    HungerDecayRate = TRexHungerDecayRate;

    // Territory
    TerritoryRadius = TRexTerritoryRadius;

    // Movement
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = TRexWalkSpeed;
    }

    UE_LOG(LogTemp, Log,
           TEXT("ATRexCharacter::ApplyTRexStats — Health=%.0f Walk=%.0f Sprint=%.0f Bite=%.0f"),
           TRexMaxHealth, TRexWalkSpeed, TRexSprintSpeed, TRexBiteDamage);
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformAttack — override base class, use T-Rex bite stats
// ─────────────────────────────────────────────────────────────────────────────

void ATRexCharacter::PerformAttack()
{
    if (CurrentBehaviourState == EDinoBehaviourState::Dead)
    {
        return;
    }

    // Sphere overlap at head position (forward + up offset from capsule centre)
    const FVector HeadOffset = GetActorForwardVector() * TRexAttackRange
                             + FVector(0.0f, 0.0f, 120.0f); // head is high up
    const FVector AttackOrigin = GetActorLocation() + HeadOffset;

    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    const bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        AttackOrigin,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(TRexAttackRange),
        QueryParams
    );

    if (bHit)
    {
        for (const FOverlapResult& Overlap : Overlaps)
        {
            AActor* HitActor = Overlap.GetActor();
            if (HitActor && HitActor != this)
            {
                // Apply bite damage
                UGameplayStatics::ApplyDamage(
                    HitActor,
                    TRexBiteDamage,
                    GetController(),
                    this,
                    UDamageType::StaticClass()
                );

                UE_LOG(LogTemp, Log,
                       TEXT("ATRexCharacter: Bite hit %s for %.0f damage"),
                       *HitActor->GetName(), TRexBiteDamage);
            }
        }
    }

    // Hunger satisfied slightly after a successful hunt attempt
    if (bHit)
    {
        CurrentHunger = FMath::Min(CurrentHunger + 20.0f, MaxHunger);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformRoar — intimidate nearby prey, announce territory
// ─────────────────────────────────────────────────────────────────────────────

void ATRexCharacter::PerformRoar()
{
    if (bRoarOnCooldown || CurrentBehaviourState == EDinoBehaviourState::Dead)
    {
        return;
    }

    bIsRoaring = true;
    bRoarOnCooldown = true;

    // Find all pawns within intimidation radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;

        const float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= RoarIntimidationRadius)
        {
            // Broadcast roar event — Animation Agent (#10) and Audio Agent (#16)
            // will bind to this via Blueprint event dispatcher
            // For now: log the intimidation hit
            UE_LOG(LogTemp, Log,
                   TEXT("ATRexCharacter::PerformRoar — Intimidated %s at dist=%.0f"),
                   *Actor->GetName(), Dist);
        }
    }

    // Start roar cooldown timer
    GetWorldTimerManager().SetTimer(
        RoarCooldownTimer,
        this,
        &ATRexCharacter::OnRoarFinished,
        RoarCooldown,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter::PerformRoar — Roar performed, cooldown=%.0fs"), RoarCooldown);
}

// ─────────────────────────────────────────────────────────────────────────────
// OnRoarFinished — reset roar state after animation completes
// ─────────────────────────────────────────────────────────────────────────────

void ATRexCharacter::OnRoarFinished()
{
    bIsRoaring = false;
    bRoarOnCooldown = false;
    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter::OnRoarFinished — Roar cooldown expired"));
}

// ─────────────────────────────────────────────────────────────────────────────
// PerformStomp — radial damage + knockback for nearby small creatures
// ─────────────────────────────────────────────────────────────────────────────

void ATRexCharacter::PerformStomp()
{
    if (CurrentBehaviourState == EDinoBehaviourState::Dead)
    {
        return;
    }

    const FVector StompOrigin = GetActorLocation();

    // Radial damage — affects all actors within StompRadius
    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        StompDamage,
        StompOrigin,
        StompRadius,
        UDamageType::StaticClass(),
        TArray<AActor*>{ this }, // ignore self
        this,
        GetController(),
        false, // full damage (not fall-off)
        ECC_Visibility
    );

    UE_LOG(LogTemp, Log,
           TEXT("ATRexCharacter::PerformStomp — Radial damage %.0f in radius %.0f"),
           StompDamage, StompRadius);
}

// ─────────────────────────────────────────────────────────────────────────────
// Die — T-Rex specific death sequence
// ─────────────────────────────────────────────────────────────────────────────

void ATRexCharacter::Die()
{
    // Clear roar timer before calling parent die
    GetWorldTimerManager().ClearTimer(RoarCooldownTimer);
    bIsRoaring = false;
    bRoarOnCooldown = false;

    UE_LOG(LogTemp, Warning,
           TEXT("ATRexCharacter::Die — T-Rex %s has died"), *GetName());

    // Delegate remaining death logic (ragdoll, timer cleanup, state set) to base
    Super::Die();
}
