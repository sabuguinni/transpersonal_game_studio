#include "Dinosaurs/TRex.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ATRex::ATRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // Apply TRex stats immediately in constructor
    ApplyTRexStats();

    // TRex detection sphere — wider than base (1000cm)
    if (DetectionSphere)
    {
        DetectionSphere->SetSphereRadius(1000.0f);
    }

    // TRex movement — slower top speed but massive momentum
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 400.0f;       // Patrol speed
        MoveComp->Mass = 8000.0f;              // 8 tonnes
        MoveComp->MaxStepHeight = 120.0f;      // Can step over large obstacles
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, 90.0f, 0.0f); // Slow turning radius
        MoveComp->GravityScale = 1.2f;
    }
}

void ATRex::ApplyTRexStats()
{
    // Override base DinosaurStats with TRex-specific values
    DinosaurStats.MaxHealth = 2000.0f;
    DinosaurStats.CurrentHealth = 2000.0f;
    DinosaurStats.AttackDamage = 250.0f;
    DinosaurStats.AttackRange = 300.0f;
    DinosaurStats.PatrolSpeed = 400.0f;
    DinosaurStats.SprintSpeed = 720.0f;     // Charge speed
    DinosaurStats.BodyMass = 8000.0f;
    DinosaurStats.bIsCarnivore = true;
    DinosaurStats.bIsPackHunter = false;
    DinosaurStats.AggressionLevel = 0.9f;   // Very aggressive
    DinosaurStats.DetectionRadius = 1000.0f;

    Species = EEng_DinosaurSpecies::TRex;
}

void ATRex::BeginPlay()
{
    Super::BeginPlay();

    // Ensure stats are correct after BeginPlay base init
    ApplyTRexStats();
    DinosaurStats.CurrentHealth = DinosaurStats.MaxHealth;

    UE_LOG(LogTemp, Log, TEXT("ATRex::BeginPlay — %s initialized. Health=%.0f Damage=%.0f"),
        *GetName(), DinosaurStats.CurrentHealth, DinosaurStats.AttackDamage);
}

void ATRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update charge movement if charging
    if (bIsCharging)
    {
        UpdateCharge(DeltaTime);
    }
}

float ATRex::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
                         AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // TRex enrages when below 50% health — roar immediately
    if (DinosaurStats.CurrentHealth < DinosaurStats.MaxHealth * 0.5f && !bRoarOnCooldown)
    {
        PerformRoar();
    }

    return ActualDamage;
}

void ATRex::PerformRoar()
{
    if (bRoarOnCooldown) return;

    UWorld* World = GetWorld();
    if (!World) return;

    bRoarOnCooldown = true;

    UE_LOG(LogTemp, Log, TEXT("ATRex::PerformRoar — %s ROARS! Stun radius=%.0fcm"), *GetName(), RoarStunRadius);

    // Find all actors within roar radius and apply stun (via damage event with stun tag)
    TArray<AActor*> OverlappingActors;
    UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), OverlappingActors);

    for (AActor* Target : OverlappingActors)
    {
        if (Target == this) continue;

        float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
        if (Distance <= RoarStunRadius)
        {
            // Apply minimal roar damage as stun signal (gameplay systems can intercept)
            UGameplayStatics::ApplyDamage(Target, 10.0f, GetController(), this, nullptr);
            UE_LOG(LogTemp, Log, TEXT("ATRex::PerformRoar — Stun applied to %s (dist=%.0f)"),
                *Target->GetName(), Distance);
        }
    }

    // Reset roar cooldown
    World->GetTimerManager().SetTimer(RoarCooldownTimer, [this]()
    {
        bRoarOnCooldown = false;
        UE_LOG(LogTemp, Log, TEXT("ATRex::RoarCooldown — Roar ready again"));
    }, RoarCooldown, false);
}

void ATRex::InitiateCharge(const FVector& TargetLocation)
{
    if (bIsCharging) return;

    UWorld* World = GetWorld();
    if (!World) return;

    bIsCharging = true;
    ChargeTarget = TargetLocation;

    // Boost movement speed for charge
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinosaurStats.SprintSpeed * ChargeSpeedMultiplier;
    }

    UE_LOG(LogTemp, Log, TEXT("ATRex::InitiateCharge — Charging toward %s"), *TargetLocation.ToString());

    // End charge after 3 seconds
    World->GetTimerManager().SetTimer(ChargeTimer, this, &ATRex::EndCharge, 3.0f, false);
}

void ATRex::EndCharge()
{
    if (!bIsCharging) return;

    bIsCharging = false;
    ChargeTarget = FVector::ZeroVector;

    // Restore patrol speed
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = DinosaurStats.PatrolSpeed;
    }

    UE_LOG(LogTemp, Log, TEXT("ATRex::EndCharge — Charge ended, returning to patrol speed"));
}

void ATRex::UpdateCharge(float DeltaTime)
{
    if (!bIsCharging || ChargeTarget.IsZero()) return;

    // Move toward charge target
    FVector Direction = (ChargeTarget - GetActorLocation()).GetSafeNormal();
    float DistToTarget = FVector::Dist(GetActorLocation(), ChargeTarget);

    if (DistToTarget < 200.0f)
    {
        // Reached target — end charge and attack
        EndCharge();

        // Perform attack at charge destination
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
        for (AActor* Target : NearbyActors)
        {
            if (Target == this) continue;
            if (FVector::Dist(GetActorLocation(), Target->GetActorLocation()) < DinosaurStats.AttackRange)
            {
                UGameplayStatics::ApplyDamage(Target, DinosaurStats.AttackDamage * 1.5f,
                    GetController(), this, nullptr);
                UE_LOG(LogTemp, Log, TEXT("ATRex::ChargeImpact — Hit %s for %.0f damage"),
                    *Target->GetName(), DinosaurStats.AttackDamage * 1.5f);
            }
        }
        return;
    }

    // Add movement input toward target
    AddMovementInput(Direction, 1.0f);
}
