// TRexDinosaur.cpp — Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260702_002
// Tyrannosaurus Rex full implementation — apex predator behavior

#include "Dinosaurs/TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATRexDinosaur::ATRexDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // TRex species data — set in constructor so CDO has correct values
    SpeciesData.SpeciesName = FName("Tyrannosaurus Rex");
    SpeciesData.MaxHealth = 500.0f;
    SpeciesData.MoveSpeed = 350.0f;
    SpeciesData.AttackDamage = 80.0f;
    SpeciesData.DetectionRadius = 1500.0f;
    SpeciesData.Diet = EDinoSpeciesDiet::Carnivore;
    SpeciesData.bIsPredator = true;

    // TRex physical dimensions — large capsule
    GetCapsuleComponent()->SetCapsuleHalfHeight(120.0f);
    GetCapsuleComponent()->SetCapsuleRadius(60.0f);

    // Movement — powerful but not agile
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = SpeciesData.MoveSpeed;
        MoveComp->MaxAcceleration = 800.0f;
        MoveComp->BrakingDecelerationWalking = 600.0f;
        MoveComp->RotationRate = FRotator(0.0f, 120.0f, 0.0f); // Slow turning
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->MaxStepHeight = 80.0f;   // Can step over large obstacles
        MoveComp->SetWalkableFloorAngle(45.0f);
        MoveComp->GravityScale = 1.2f;     // Heavy creature
    }
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // Initialize TRex-specific timers
    // Roar shortly after spawn to announce territory
    GetWorld()->GetTimerManager().SetTimer(
        RoarTimer,
        this,
        &ATRexDinosaur::Roar,
        FMath::RandRange(3.0f, 8.0f),
        false
    );

    UE_LOG(LogTemp, Log, TEXT("TRex spawned: %s | Health=%.0f | Speed=%.0f"),
        *GetName(), SpeciesData.MaxHealth, SpeciesData.MoveSpeed);
}

void ATRexDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // During charge — move toward target at high speed
    if (bIsCharging && ChargeTarget && IsAlive())
    {
        FVector ToTarget = (ChargeTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        float ChargeSpeed = SpeciesData.MoveSpeed * ChargeSpeedMultiplier;
        AddMovementInput(ToTarget, 1.0f);

        // Check if we've reached the target
        float DistToTarget = FVector::Dist(GetActorLocation(), ChargeTarget->GetActorLocation());
        if (DistToTarget < 200.0f)
        {
            // Impact! Deal charge damage
            UGameplayStatics::ApplyDamage(
                ChargeTarget,
                ChargeDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
            EndCharge();
        }
    }
}

// ============================================================
// BEHAVIOR HOOKS — Override from ADinosaurBase
// ============================================================

void ATRexDinosaur::OnIdle()
{
    // TRex idle: stand still, occasionally look around
    // In future: play idle animation, scan for prey
    UE_LOG(LogTemp, Verbose, TEXT("TRex %s: IDLE — scanning territory"), *GetName());
}

void ATRexDinosaur::OnRoam()
{
    // TRex roam: patrol slowly, mark territory
    // Movement handled by base class random walk
    UE_LOG(LogTemp, Verbose, TEXT("TRex %s: ROAMING"), *GetName());

    // Occasionally roar while roaming
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastRoarTime > RoarCooldown)
    {
        if (FMath::RandBool())
        {
            Roar();
        }
    }
}

void ATRexDinosaur::OnAlert()
{
    // TRex alert: lock onto target, prepare charge
    UE_LOG(LogTemp, Log, TEXT("TRex %s: ALERT — target acquired"), *GetName());

    // Roar when alerted (intimidation)
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastRoarTime > RoarCooldown * 0.5f)
    {
        Roar();
    }
}

void ATRexDinosaur::OnAttack()
{
    // TRex attack: charge if cooldown ready, otherwise bite
    float CurrentTime = GetWorld()->GetTimeSeconds();

    if (!bIsCharging && (CurrentTime - LastChargeTime > ChargeCooldown))
    {
        // Find nearest player/prey
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
        if (PlayerPawn)
        {
            float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
            if (DistToPlayer < SpeciesData.DetectionRadius)
            {
                StartChargeAttack(PlayerPawn);
                return;
            }
        }
    }

    // Fallback: bite attack
    PerformBiteAttack();
}

void ATRexDinosaur::OnFlee()
{
    // TRex rarely flees — only when health < 20%
    // Move away from threat at reduced speed
    UE_LOG(LogTemp, Warning, TEXT("TRex %s: FLEEING (low health)"), *GetName());
}

void ATRexDinosaur::OnDeath()
{
    // TRex death: cancel charge, clear timers
    bIsCharging = false;
    ChargeTarget = nullptr;
    GetWorld()->GetTimerManager().ClearTimer(ChargeTimer);
    GetWorld()->GetTimerManager().ClearTimer(RoarTimer);

    UE_LOG(LogTemp, Log, TEXT("TRex %s: DEAD — resources available for scavengers"), *GetName());
}

// ============================================================
// TREX-SPECIFIC ABILITIES
// ============================================================

void ATRexDinosaur::StartChargeAttack(AActor* Target)
{
    if (!Target || bIsCharging) return;

    bIsCharging = true;
    ChargeTarget = Target;
    LastChargeTime = GetWorld()->GetTimeSeconds();

    // Boost movement speed for charge
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = SpeciesData.MoveSpeed * ChargeSpeedMultiplier;
    }

    // End charge after 3 seconds max
    GetWorld()->GetTimerManager().SetTimer(
        ChargeTimer,
        this,
        &ATRexDinosaur::EndCharge,
        3.0f,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("TRex %s: CHARGING at %s"), *GetName(), *Target->GetName());
}

void ATRexDinosaur::EndCharge()
{
    bIsCharging = false;
    ChargeTarget = nullptr;

    // Restore normal movement speed
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = SpeciesData.MoveSpeed;
    }

    GetWorld()->GetTimerManager().ClearTimer(ChargeTimer);
    UE_LOG(LogTemp, Log, TEXT("TRex %s: Charge ended"), *GetName());
}

void ATRexDinosaur::Roar()
{
    if (!IsAlive()) return;

    LastRoarTime = GetWorld()->GetTimeSeconds();

    // Alert all nearby predators within RoarRadius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADinosaurBase::StaticClass(), NearbyActors);

    int32 AlertCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < RoarRadius)
        {
            ADinosaurBase* OtherDino = Cast<ADinosaurBase>(Actor);
            if (OtherDino && OtherDino->IsAlive())
            {
                OtherDino->AlertDinosaur();
                AlertCount++;
            }
        }
    }

    // Debug visual: draw roar sphere
    DrawDebugSphere(GetWorld(), GetActorLocation(), RoarRadius, 16,
        FColor::Red, false, 2.0f, 0, 5.0f);

    UE_LOG(LogTemp, Log, TEXT("TRex %s: ROAR — alerted %d nearby dinosaurs"), *GetName(), AlertCount);

    // Schedule next roar
    GetWorld()->GetTimerManager().SetTimer(
        RoarTimer,
        this,
        &ATRexDinosaur::Roar,
        RoarCooldown + FMath::RandRange(-3.0f, 3.0f),
        false
    );
}

void ATRexDinosaur::StompAttack()
{
    if (!IsAlive()) return;

    // Find all actors in stomp radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    int32 StaggerCount = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist < StompRadius)
        {
            UGameplayStatics::ApplyDamage(
                Actor,
                StompStaggerDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
            StaggerCount++;
        }
    }

    // Debug visual: draw stomp sphere
    DrawDebugSphere(GetWorld(), GetActorLocation(), StompRadius, 12,
        FColor::Orange, false, 1.5f, 0, 3.0f);

    UE_LOG(LogTemp, Log, TEXT("TRex %s: STOMP — staggered %d creatures"), *GetName(), StaggerCount);
}

void ATRexDinosaur::PerformBiteAttack()
{
    // Standard bite — damage to nearest target in melee range
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    float MeleeRange = GetCapsuleComponent()->GetScaledCapsuleRadius() * 3.0f;

    if (DistToPlayer < MeleeRange)
    {
        UGameplayStatics::ApplyDamage(
            PlayerPawn,
            SpeciesData.AttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );
        UE_LOG(LogTemp, Log, TEXT("TRex %s: BITE — dealt %.0f damage"), *GetName(), SpeciesData.AttackDamage);
    }
}
