#include "TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ATRexDinosaur::ATRexDinosaur()
{
    // T-Rex apex predator stats
    DinoStats.MaxHealth         = 5000.f;
    DinoStats.CurrentHealth     = 5000.f;
    DinoStats.AttackDamage      = 400.f;
    DinoStats.MoveSpeed         = 800.f;
    DinoStats.Mass              = 12000.f;
    DinoStats.DetectionRadius   = 3500.f;
    DinoStats.AttackRadius      = 350.f;
    DinoStats.DietType          = ECore_DinosaurDiet::Carnivore;

    // Solitary — never pack hunts
    bIsPackHunter = false;

    // Large territory for apex predator
    TerritoryRadius = 8000.f;

    // T-Rex capsule: large collision
    GetCapsuleComponent()->SetCapsuleHalfHeight(180.f);
    GetCapsuleComponent()->SetCapsuleRadius(80.f);

    // Movement config
    GetCharacterMovement()->MaxWalkSpeed = DinoStats.MoveSpeed;
    GetCharacterMovement()->Mass = DinoStats.Mass;
    GetCharacterMovement()->GravityScale = 1.2f;
    GetCharacterMovement()->MaxAcceleration = 600.f;
    GetCharacterMovement()->BrakingDecelerationWalking = 800.f;
    GetCharacterMovement()->RotationRate = FRotator(0.f, 120.f, 0.f);
    GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // T-Rex starts in Patrolling state
    SetBehaviorState(ECore_DinosaurBehaviorState::Patrolling);
}

void ATRexDinosaur::PerformRoar()
{
    if (bRoarOnCooldown || !IsAlive()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    bRoarOnCooldown = true;

    // Find all actors in roar radius (2000 units) and apply stun/fear
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= 2000.f)
        {
            // Apply roar effect — can be extended via Blueprint interface
            // For now, log the effect for debugging
            UE_LOG(LogTemp, Log, TEXT("TRex Roar: affecting %s at distance %.0f"), *Actor->GetName(), Dist);
        }
    }

    // Switch to Aggressive state
    SetBehaviorState(ECore_DinosaurBehaviorState::Aggressive);

    // Start roar cooldown
    World->GetTimerManager().SetTimer(
        RoarCooldownHandle,
        this,
        &ATRexDinosaur::ResetRoarCooldown,
        RoarCooldownDuration,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("TRex %s performed ROAR — aggressive state activated"), *GetName());
}

void ATRexDinosaur::PerformStomp()
{
    if (!IsAlive()) return;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector StompOrigin = GetActorLocation();

    // Find all actors in stomp radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    int32 StompHits = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this) continue;

        float Dist = FVector::Dist(StompOrigin, Actor->GetActorLocation());
        if (Dist <= StompRadius)
        {
            // Apply stomp damage
            UGameplayStatics::ApplyDamage(
                Actor,
                StompDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
            StompHits++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("TRex %s STOMP — hit %d actors in radius %.0f"), *GetName(), StompHits, StompRadius);
}

void ATRexDinosaur::UpdateBehavior()
{
    // Call base behavior first
    Super::UpdateBehavior();

    // T-Rex specific: charge when hunting target is far
    if (CurrentBehaviorState == ECore_DinosaurBehaviorState::Hunting && CurrentTarget)
    {
        float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

        if (DistToTarget > ChargeSwitchDistance)
        {
            // Charge speed — faster than normal hunting
            float ChargeSpeed = DinoStats.MoveSpeed * ChargeSpeedMultiplier;
            GetCharacterMovement()->MaxWalkSpeed = FMath::Clamp(ChargeSpeed, 0.f, 2000.f);
        }
        else
        {
            // Close enough — normal attack speed
            GetCharacterMovement()->MaxWalkSpeed = DinoStats.MoveSpeed * 1.5f;

            // Attempt roar when closing in
            if (!bRoarOnCooldown)
            {
                PerformRoar();
            }
        }
    }
}

void ATRexDinosaur::ResetRoarCooldown()
{
    bRoarOnCooldown = false;
    UE_LOG(LogTemp, Log, TEXT("TRex %s roar cooldown reset"), *GetName());
}
