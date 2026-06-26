// DinosaurTRex.cpp — Tyrannosaurus Rex implementation
// Engine: Unreal Engine 5.5 | Module: TranspersonalGame

#include "DinosaurTRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // T-Rex species stats — apex predator
    MaxHealth = 2000.0f;
    CurrentHealth = 2000.0f;
    AttackDamage = 120.0f;
    AttackRange = 350.0f;
    DetectionRange = 2000.0f;
    bIsAggressive = true;

    // Movement — powerful but not agile
    WalkSpeed = 350.0f;
    SprintSpeed = 700.0f;
    TurnRate = 60.0f;

    // Survival stats
    MaxHunger = 200.0f;
    CurrentHunger = 200.0f;
    MaxThirst = 150.0f;
    CurrentThirst = 150.0f;
    HungerDrainRate = 0.8f;   // Large body needs more food
    ThirstDrainRate = 0.5f;

    // Capsule — large dinosaur
    GetCapsuleComponent()->SetCapsuleHalfHeight(180.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // Movement component
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, TurnRate, 0.0f);
        GetCharacterMovement()->JumpZVelocity = 0.0f;   // T-Rex cannot jump
        GetCharacterMovement()->GravityScale = 1.5f;     // Heavy animal
    }
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // T-Rex starts in roaming state
    SetBehaviorState(ECore_DinosaurBehaviorState::Roaming);

    // Initial roar to establish territory
    TimeSinceLastRoar = RoarCooldown; // Ready to roar immediately
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Advance cooldown timers
    TimeSinceLastStomp += DeltaTime;
    TimeSinceLastRoar += DeltaTime;

    // Territorial scan — check for nearby prey
    if (CurrentBehaviorState == ECore_DinosaurBehaviorState::Roaming ||
        CurrentBehaviorState == ECore_DinosaurBehaviorState::Idle)
    {
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

        for (AActor* Actor : NearbyActors)
        {
            if (Actor == this) continue;
            if (!IsValid(Actor)) continue;

            float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Dist < TerritorialRadius)
            {
                // Check if it's a player or prey (not another T-Rex)
                if (Actor->ActorHasTag(FName("Player")) || Actor->ActorHasTag(FName("Prey")))
                {
                    SetBehaviorState(ECore_DinosaurBehaviorState::Hunting);
                    if (!bHasRoaredThisAttack)
                    {
                        TriggerRoar();
                        bHasRoaredThisAttack = true;
                    }
                    break;
                }
            }
        }
    }
    else if (CurrentBehaviorState == ECore_DinosaurBehaviorState::Attacking)
    {
        // Stomp attack when in attack range
        if (TimeSinceLastStomp >= StompCooldown)
        {
            ExecuteStomp();
        }
    }
    else
    {
        // Reset roar flag when not hunting
        bHasRoaredThisAttack = false;
    }

    // Periodic roar during hunting
    if (CurrentBehaviorState == ECore_DinosaurBehaviorState::Hunting &&
        TimeSinceLastRoar >= RoarCooldown)
    {
        TriggerRoar();
    }
}

void ATRexCharacter::ExecuteStomp()
{
    if (TimeSinceLastStomp < StompCooldown) return;
    TimeSinceLastStomp = 0.0f;

    // AoE damage sphere in front of T-Rex
    FVector StompCenter = GetActorLocation() + GetActorForwardVector() * (AttackRange * 0.5f);

    TArray<AActor*> HitActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), HitActors);

    for (AActor* Target : HitActors)
    {
        if (Target == this) continue;
        if (!IsValid(Target)) continue;

        float Dist = FVector::Dist(StompCenter, Target->GetActorLocation());
        if (Dist <= StompRadius)
        {
            FDamageEvent DamageEvent;
            Target->TakeDamage(StompDamage, DamageEvent, GetController(), this);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter: Stomp executed at %s, radius=%.0f, damage=%.0f"),
        *StompCenter.ToString(), StompRadius, StompDamage);
}

void ATRexCharacter::TriggerRoar()
{
    TimeSinceLastRoar = 0.0f;

    // Frighten nearby prey — increase their fear stat if they have one
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    int32 FrightenedCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < TerritorialRadius * 1.5f)
        {
            // Tag actor as frightened (AI controllers can read this)
            Actor->Tags.AddUnique(FName("Frightened"));
            FrightenedCount++;
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter: ROAR! Frightened %d actors within %.0fm"),
        FrightenedCount, TerritorialRadius * 1.5f);
}

bool ATRexCharacter::IsTargetInTerritorialRange(AActor* Target) const
{
    if (!IsValid(Target)) return false;
    float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    return Dist <= TerritorialRadius;
}

void ATRexCharacter::OnDeath_Implementation()
{
    // Call base death (disables collision, sets lifespan)
    Super::OnDeath_Implementation();

    // T-Rex specific: remove frightened tags from nearby actors on death
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < TerritorialRadius)
        {
            Actor->Tags.Remove(FName("Frightened"));
        }
    }

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter: T-Rex died. Territory cleared."));
}
