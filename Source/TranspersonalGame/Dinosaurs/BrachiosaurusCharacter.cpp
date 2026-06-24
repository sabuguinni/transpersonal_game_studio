// BrachiosaurusCharacter.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260624_002
// Full implementation of ABrachiosaurusCharacter

#include "Dinosaurs/BrachiosaurusCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

ABrachiosaurusCharacter::ABrachiosaurusCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Species Identity ──────────────────────────────────────────────────
    Species         = EWorld_DinosaurSpecies::Brachiosaurus;
    MaxHealth       = 3000.0f;
    CurrentHealth   = 3000.0f;
    AttackDamage    = 20.0f;   // Tail swipe only — passive herbivore
    DetectionRange  = 5000.0f; // Long-range awareness

    // ── Movement — slow, lumbering giant ─────────────────────────────────
    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed          = 300.0f;
        Move->MaxAcceleration       = 200.0f;
        Move->BrakingDecelerationWalking = 400.0f;
        Move->RotationRate          = FRotator(0.0f, 60.0f, 0.0f); // Slow turning
        Move->bOrientRotationToMovement = true;
        Move->GravityScale          = 1.2f; // Heavy
    }

    // ── Combat / Behaviour defaults ───────────────────────────────────────
    FleeRadius             = 3000.0f;
    TailSwipeRadius        = 600.0f;
    StompKnockbackImpulse  = 1200.0f;
    TailSwipeCooldown      = 8.0f;
    bIsFleeing             = false;
    bTailSwipeReady        = true;
    FleeTargetLocation     = FVector::ZeroVector;
}

void ABrachiosaurusCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Start periodic flee evaluation every 3 seconds
    GetWorldTimerManager().SetTimer(
        FleeEvaluationTimer,
        this,
        &ABrachiosaurusCharacter::EvaluateFleeResponse,
        3.0f,
        true,
        1.0f
    );
}

void ABrachiosaurusCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // If fleeing, move toward flee target
    if (bIsFleeing && !FleeTargetLocation.IsZero())
    {
        FVector Direction = (FleeTargetLocation - GetActorLocation()).GetSafeNormal();
        AddMovementInput(Direction, 1.0f);

        // Stop fleeing when close enough to target
        float DistToTarget = FVector::Dist(GetActorLocation(), FleeTargetLocation);
        if (DistToTarget < 500.0f)
        {
            bIsFleeing = false;
            FleeTargetLocation = FVector::ZeroVector;
        }
    }
}

void ABrachiosaurusCharacter::PerformTailSwipe()
{
    if (!bTailSwipeReady)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // AoE damage in tail swipe arc (behind and to the sides)
    TArray<AActor*> IgnoreActors;
    IgnoreActors.Add(this);

    UGameplayStatics::ApplyRadialDamage(
        World,
        AttackDamage,
        GetActorLocation(),
        TailSwipeRadius,
        UDamageType::StaticClass(),
        IgnoreActors,
        this,
        GetController(),
        false,
        ECC_Visibility
    );

    // Visual debug in editor
#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), TailSwipeRadius, 16, FColor::Orange, false, 2.0f);
#endif

    // Start cooldown
    bTailSwipeReady = false;
    GetWorldTimerManager().SetTimer(
        TailSwipeCooldownTimer,
        [this]() { bTailSwipeReady = true; },
        TailSwipeCooldown,
        false
    );
}

void ABrachiosaurusCharacter::EvaluateFleeResponse()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Scan for carnivore threats within FleeRadius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADinosaurBase::StaticClass(), NearbyActors);

    FVector ThreatLocation = FVector::ZeroVector;
    bool bThreatFound = false;

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this)
        {
            continue;
        }

        ADinosaurBase* OtherDino = Cast<ADinosaurBase>(Actor);
        if (!OtherDino)
        {
            continue;
        }

        // Only flee from carnivores (TRex, Raptor)
        if (OtherDino->Species == EWorld_DinosaurSpecies::TyrannosaurusRex ||
            OtherDino->Species == EWorld_DinosaurSpecies::Velociraptor)
        {
            float Dist = FVector::Dist(GetActorLocation(), OtherDino->GetActorLocation());
            if (Dist < FleeRadius)
            {
                ThreatLocation = OtherDino->GetActorLocation();
                bThreatFound = true;

                // Tail swipe if threat is very close
                if (Dist < TailSwipeRadius * 1.5f)
                {
                    PerformTailSwipe();
                }
                break;
            }
        }
    }

    if (bThreatFound)
    {
        // Flee away from threat
        FVector FleeDir = (GetActorLocation() - ThreatLocation).GetSafeNormal();
        FleeTargetLocation = GetActorLocation() + FleeDir * 4000.0f;
        bIsFleeing = true;

        // Warn herd
        EmitHerdWarning();
        CoordinateHerdFlee(ThreatLocation);
    }
    else
    {
        bIsFleeing = false;
    }
}

void ABrachiosaurusCharacter::PerformGroundStomp()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Knockback all actors within stomp radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    const float StompRadius = 800.0f;

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this)
        {
            continue;
        }

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < StompRadius)
        {
            // Apply knockback via radial damage
            UGameplayStatics::ApplyRadialDamage(
                World,
                AttackDamage * 0.5f,
                GetActorLocation(),
                StompRadius,
                UDamageType::StaticClass(),
                TArray<AActor*>{this},
                this,
                GetController(),
                true,
                ECC_Visibility
            );
            break; // Single radial call covers all
        }
    }

#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), StompRadius, 16, FColor::Red, false, 2.0f);
#endif
}

void ABrachiosaurusCharacter::EmitHerdWarning()
{
    UpdateHerdMembers();

    for (ABrachiosaurusCharacter* Member : HerdMembers)
    {
        if (Member && Member != this)
        {
            // Trigger flee evaluation on herd member immediately
            Member->EvaluateFleeResponse();
        }
    }
}

void ABrachiosaurusCharacter::UpdateHerdMembers()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    HerdMembers.Empty();

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, ABrachiosaurusCharacter::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (Actor == this)
        {
            continue;
        }

        ABrachiosaurusCharacter* Member = Cast<ABrachiosaurusCharacter>(Actor);
        if (Member)
        {
            float Dist = FVector::Dist(GetActorLocation(), Member->GetActorLocation());
            if (Dist < DetectionRange)
            {
                HerdMembers.Add(Member);
            }
        }
    }
}

void ABrachiosaurusCharacter::CoordinateHerdFlee(const FVector& ThreatLocation)
{
    for (ABrachiosaurusCharacter* Member : HerdMembers)
    {
        if (!Member || Member == this)
        {
            continue;
        }

        // Point each herd member away from the threat
        FVector FleeDir = (Member->GetActorLocation() - ThreatLocation).GetSafeNormal();
        Member->FleeTargetLocation = Member->GetActorLocation() + FleeDir * 4000.0f;
        Member->bIsFleeing = true;
    }
}
