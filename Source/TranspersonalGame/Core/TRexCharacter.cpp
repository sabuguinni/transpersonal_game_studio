// TRexCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// Tyrannosaurus Rex implementation

#include "Core/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATRexCharacter::ATRexCharacter()
{
    // ── Species identity ──────────────────────────────────────────────────────
    // EDinoSpecies is defined in DinosaurBase.h — TyrannosaurusRex = 0
    Species = EDinoSpecies::TyrannosaurusRex;
    bIsAggressive = true;
    bIsPackHunter = false;

    // ── Survival stats ────────────────────────────────────────────────────────
    MaxHealth = 1000.0f;
    Health    = 1000.0f;
    MaxStamina = 600.0f;
    Stamina   = 600.0f;
    BiteDamage = 150.0f;

    // ── Movement ──────────────────────────────────────────────────────────────
    WalkSpeed = 350.0f;
    RunSpeed  = 900.0f;

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = WalkSpeed;
        MoveComp->MaxAcceleration = 1200.0f;
        MoveComp->BrakingDecelerationWalking = 800.0f;
        MoveComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->GravityScale = 1.2f;   // Heavy — falls faster
    }

    // ── Capsule — large bipedal predator ──────────────────────────────────────
    UCapsuleComponent* Cap = GetCapsuleComponent();
    if (Cap)
    {
        Cap->InitCapsuleSize(120.0f, 280.0f);
    }

    // ── AI auto-possession ────────────────────────────────────────────────────
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Wire BehaviorTree into AI controller if set
    if (TRexBehaviorTree)
    {
        BehaviorTreeAsset = TRexBehaviorTree;
    }
}

// ── Roar ──────────────────────────────────────────────────────────────────────

bool ATRexCharacter::CanRoar() const
{
    return (GetWorld()->GetTimeSeconds() - LastRoarTime) >= RoarCooldown;
}

void ATRexCharacter::PerformRoar()
{
    if (!CanRoar()) return;

    LastRoarTime = GetWorld()->GetTimeSeconds();

    // Find all actors within RoarRadius and apply stun
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Apply stun via gameplay tag or interface — stub for BT integration
            // Full stun implementation wired in Combat Agent (#12)
            Actor->Tags.AddUnique(FName("Stunned"));
        }
    }

    // Fire Blueprint event for animation + audio
    OnTRexRoar();
}

// ── Stomp ─────────────────────────────────────────────────────────────────────

void ATRexCharacter::PerformStomp()
{
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= StompRadius)
        {
            // Apply radial damage — Combat Agent (#12) wires full damage system
            UGameplayStatics::ApplyRadialDamage(
                GetWorld(),
                StompDamage,
                GetActorLocation(),
                StompRadius,
                nullptr,   // DamageType — wired by Combat Agent
                TArray<AActor*>(),
                this,
                GetController(),
                true       // DoFullDamage at centre
            );
        }
    }

    OnTRexStomp();
}
