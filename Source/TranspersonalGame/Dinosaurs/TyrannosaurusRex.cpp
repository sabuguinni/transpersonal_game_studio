// TyrannosaurusRex.cpp — Tyrannosaurus Rex species implementation
// Agent #03 — Core Systems Programmer — Cycle AUTO_20260628_010

#include "Dinosaurs/TyrannosaurusRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATyrannosaurusRex::ATyrannosaurusRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // ── Species identity ──────────────────────────────────────────────────
    Species    = EEng_DinosaurSpecies::TRex;
    Diet       = EEng_DinosaurDiet::Carnivore;
    MaxHealth  = 500.0f;
    Health     = 500.0f;
    AttackDamage = 25.0f;
    AttackRange  = 200.0f;   // cm — bite reach
    AttackCooldown = 3.0f;   // seconds between bites

    // ── Movement ──────────────────────────────────────────────────────────
    WalkSpeed = 400.0f;
    RunSpeed  = 550.0f;

    if (UCharacterMovementComponent* Move = GetCharacterMovement())
    {
        Move->MaxWalkSpeed          = WalkSpeed;
        Move->JumpZVelocity         = 0.0f;   // TRex cannot jump
        Move->bCanWalkOffLedges     = true;
        Move->NavAgentProps.AgentRadius = 120.0f;
        Move->NavAgentProps.AgentHeight = 600.0f;
        Move->RotationRate          = FRotator(0.0f, 180.0f, 0.0f);
        Move->bOrientRotationToMovement = true;
    }

    // ── Capsule ───────────────────────────────────────────────────────────
    if (UCapsuleComponent* Cap = GetCapsuleComponent())
    {
        Cap->SetCapsuleHalfHeight(300.0f);
        Cap->SetCapsuleRadius(120.0f);
    }

    // ── Hunger / thirst decay (slower for apex predator) ─────────────────
    HungerDecayRate = 1.5f;
    ThirstDecayRate = 1.0f;
}

void ATyrannosaurusRex::BeginPlay()
{
    Super::BeginPlay();

    // Record home territory centre
    HomeLocation = GetActorLocation();
}

void ATyrannosaurusRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ── Charge locomotion ─────────────────────────────────────────────────
    if (bIsCharging)
    {
        ChargeTimeRemaining -= DeltaTime;

        if (ChargeTimeRemaining <= 0.0f)
        {
            // End charge — restore normal speed
            bIsCharging = false;
            if (UCharacterMovementComponent* Move = GetCharacterMovement())
            {
                Move->MaxWalkSpeed = RunSpeed;
            }
        }
        else
        {
            // Apply charge velocity
            const float ChargeSpeed = RunSpeed * ChargeSpeedMultiplier;
            AddMovementInput(ChargeDirection, 1.0f);
            if (UCharacterMovementComponent* Move = GetCharacterMovement())
            {
                Move->MaxWalkSpeed = ChargeSpeed;
            }
        }
    }
}

void ATyrannosaurusRex::PerformRoar()
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (Now - LastRoarTime < RoarCooldown)
    {
        return;   // Still on cooldown
    }
    LastRoarTime = Now;

    // Find all actors within roar radius and apply brief stun
    // (Stun is communicated via gameplay tag / event — simplified here)
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), OverlappingActors);

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor == this) continue;
        const float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Notify actor of roar stun (Blueprint event or interface call)
            // For now log the effect — full stun requires GameplayAbilities
            UE_LOG(LogTemp, Log, TEXT("TRex ROAR stuns %s for %.1fs"), *Actor->GetName(), RoarStunDuration);
        }
    }

#if WITH_EDITOR
    DrawDebugSphere(GetWorld(), GetActorLocation(), RoarRadius, 16, FColor::Orange, false, 2.0f);
#endif
}

void ATyrannosaurusRex::BeginCharge(FVector TargetLocation)
{
    if (bIsCharging) return;

    const FVector ToTarget = (TargetLocation - GetActorLocation()).GetSafeNormal();
    ChargeDirection     = ToTarget;
    ChargeTimeRemaining = ChargeDuration;
    bIsCharging         = true;

    UE_LOG(LogTemp, Log, TEXT("TRex begins charge toward %s"), *TargetLocation.ToString());
}
