// TRexCharacter.cpp
// Transpersonal Game Studio — Core Systems Programmer (#03)
// Tyrannosaurus Rex implementation — apex predator of the prehistoric world

#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // TRex stats — apex predator
    MaxHealth = 2000.0f;
    CurrentHealth = 2000.0f;
    AttackDamage = 150.0f;
    DetectionRadius = 2500.0f;
    AttackRange = 300.0f;
    MoveSpeed = 600.0f;

    // TRex-specific properties
    ChargeSpeedMultiplier = 3.0f;
    StompRadius = 400.0f;
    StompDamage = 80.0f;
    BiteCooldown = 2.5f;
    TerritorialRadius = 5000.0f;
    bIsCharging = false;

    // Movement — TRex is powerful but not agile
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
        GetCharacterMovement()->JumpZVelocity = 0.0f; // TRex cannot jump
        GetCharacterMovement()->GravityScale = 1.5f;
        GetCharacterMovement()->Mass = 8000.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 200.0f;
    }

    // Species identification
    Species = EDinoSpecies::TRex;
    DietType = EDinoDietType::Carnivore;
    bIsPredator = true;
    bIsPackAnimal = false;
    HungerDecayRate = 0.5f; // Slower hunger — large body mass
    StaminaRegenRate = 8.0f;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Record territory center at spawn location
    TerritoryCenter = GetActorLocation();

    // TRex patrols a large territory
    if (GetWorld())
    {
        // Initial territorial roar on spawn
        FTimerHandle InitRoarTimer;
        GetWorld()->GetTimerManager().SetTimer(
            InitRoarTimer,
            this,
            &ATRexCharacter::TerritorialRoar,
            3.0f,
            false
        );
    }
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Check if any actors entered territory
    if (BehaviorState == EDinoBehaviorState::Idle || BehaviorState == EDinoBehaviorState::Patrolling)
    {
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

        for (AActor* Actor : NearbyActors)
        {
            if (Actor == this) continue;
            if (!IsValid(Actor)) continue;

            float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Dist < DetectionRadius && IsInTerritory(Actor))
            {
                // Aggression toward player or herbivores in territory
                AController* PC = GetWorld()->GetFirstPlayerController();
                if (PC && PC->GetPawn() == Actor)
                {
                    SetBehaviorState(EDinoBehaviorState::Hunting);
                    CurrentTarget = Actor;
                    break;
                }
            }
        }
    }
}

void ATRexCharacter::PerformBiteAttack()
{
    if (!CurrentTarget || !IsValid(CurrentTarget)) return;
    if (BehaviorState == EDinoBehaviorState::Dead) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget > AttackRange * 1.5f) return; // Must be close enough

    // Apply massive bite damage
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        AttackDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    // Consume stamina
    CurrentStamina = FMath::Max(0.0f, CurrentStamina - 30.0f);

    // Bite cooldown
    GetWorld()->GetTimerManager().SetTimer(
        BiteAttackTimer,
        this,
        &ATRexCharacter::ResetBiteCooldown,
        BiteCooldown,
        false
    );

    SetBehaviorState(EDinoBehaviorState::Attacking);
}

void ATRexCharacter::PerformStompAttack()
{
    if (BehaviorState == EDinoBehaviorState::Dead) return;

    // AoE stomp — damages everything in StompRadius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    int32 HitCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        if (!IsValid(Actor)) continue;

        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= StompRadius)
        {
            UGameplayStatics::ApplyDamage(
                Actor,
                StompDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
            HitCount++;
        }
    }

    // Stomp costs significant stamina
    CurrentStamina = FMath::Max(0.0f, CurrentStamina - 50.0f);

    // Stomp cooldown (longer than bite)
    GetWorld()->GetTimerManager().SetTimer(
        StompAttackTimer,
        this,
        &ATRexCharacter::ResetStompCooldown,
        8.0f,
        false
    );
}

void ATRexCharacter::TerritorialRoar()
{
    // Base roar (alerts pack — TRex is solo but roar scares prey)
    Roar();

    // Additional effect: nearby small dinosaurs flee
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurBase::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        ADinosaurBase* OtherDino = Cast<ADinosaurBase>(Actor);
        if (!OtherDino) continue;

        float Dist = FVector::Dist(GetActorLocation(), OtherDino->GetActorLocation());
        if (Dist < 3000.0f)
        {
            // Smaller dinosaurs flee from TRex roar
            if (OtherDino->MaxHealth < 500.0f)
            {
                OtherDino->SetBehaviorState(EDinoBehaviorState::Fleeing);
            }
        }
    }
}

void ATRexCharacter::ChargeTowardTarget(AActor* Target)
{
    if (!Target || !IsValid(Target)) return;
    if (CurrentHealth < MaxHealth * 0.5f) return; // Only charge when healthy

    StartCharge(Target);
}

bool ATRexCharacter::IsFearless_Implementation() const
{
    // TRex fears nothing — apex predator
    return true;
}

bool ATRexCharacter::IsInTerritory(AActor* Actor) const
{
    if (!Actor) return false;
    float Dist = FVector::Dist(TerritoryCenter, Actor->GetActorLocation());
    return Dist <= TerritorialRadius;
}

void ATRexCharacter::ResetBiteCooldown()
{
    // Bite is ready again — handled by AI behavior tree polling
}

void ATRexCharacter::ResetStompCooldown()
{
    // Stomp is ready again
}

void ATRexCharacter::StartCharge(AActor* Target)
{
    if (bIsCharging) return;

    bIsCharging = true;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed * ChargeSpeedMultiplier;
    }

    // Charge lasts 3 seconds max
    FTimerHandle ChargeTimer;
    GetWorld()->GetTimerManager().SetTimer(
        ChargeTimer,
        this,
        &ATRexCharacter::EndCharge,
        3.0f,
        false
    );
}

void ATRexCharacter::EndCharge()
{
    bIsCharging = false;
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
    }
}
