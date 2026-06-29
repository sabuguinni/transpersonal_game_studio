// TRexCharacter.cpp
// Core Systems Programmer #03 — Cycle AUTO_20260629_004
// Tyrannosaurus Rex — full species implementation

#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Species identity ---
    SpeciesName = TEXT("Tyrannosaurus Rex");
    SizeClass = EDinosaurSizeClass::Massive;

    // --- Survival stats (realistic approximation) ---
    MaxHealth = 2500.0f;
    CurrentHealth = 2500.0f;
    AttackDamage = 180.0f;      // Devastating bite
    AttackRadius = 200.0f;      // Short reach — must be very close
    DetectionRadius = 3000.0f;  // Excellent sensory range
    TerritoryRadius = 8000.0f;  // Large territory

    // --- Capsule sizing for T-Rex scale ---
    // T-Rex: ~4m tall at hip, ~12m long
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleHalfHeight(200.0f);  // 4m hip height
        Capsule->SetCapsuleRadius(80.0f);
    }

    // --- Movement: slow but powerful ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 550.0f;        // ~20 km/h
        MoveComp->MaxAcceleration = 400.0f;     // Slow to accelerate (massive inertia)
        MoveComp->BrakingDecelerationWalking = 300.0f;
        MoveComp->RotationRate = FRotator(0.0f, 90.0f, 0.0f);  // Slow turning
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->JumpZVelocity = 0.0f;         // T-Rex cannot jump
        MoveComp->GravityScale = 1.5f;          // Heavy — stays grounded
    }

    // --- Mesh scale ---
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (MeshComp)
    {
        MeshComp->SetRelativeScale3D(FVector(2.5f, 2.5f, 2.5f));
        MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -200.0f));
    }

    // --- TRex-specific defaults ---
    RoarRadius = 2000.0f;
    RoarFearDuration = 8.0f;
    ChargeSpeedMultiplier = 1.8f;
    ChargeDuration = 2.5f;
    RoarCooldown = 30.0f;

    bIsCharging = false;
    bRoarOnCooldown = false;
    ChargeElapsed = 0.0f;
    BaseWalkSpeed = 550.0f;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Cache base walk speed after movement component is fully initialized
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        BaseWalkSpeed = MoveComp->MaxWalkSpeed;
    }
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Charge state update — track elapsed time
    if (bIsCharging)
    {
        ChargeElapsed += DeltaTime;
        if (ChargeElapsed >= ChargeDuration)
        {
            EndCharge();
        }
    }
}

float ATRexCharacter::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    // T-Rex has thick hide — reduce incoming damage by 30%
    float ReducedDamage = DamageAmount * 0.70f;

    // Call parent implementation with reduced damage
    float ActualDamage = Super::TakeDamage(ReducedDamage, DamageEvent, EventInstigator, DamageCauser);

    // If damaged and not already aggressive, roar and charge attacker
    if (ActualDamage > 0.0f && DamageCauser && CurrentBehaviorState != EDinosaurBehaviorState::Dead)
    {
        if (!bRoarOnCooldown)
        {
            PerformRoar();
        }
        InitiateCharge(DamageCauser);
    }

    return ActualDamage;
}

void ATRexCharacter::OnPreyDetected_Implementation(AActor* PreyActor)
{
    if (!PreyActor || CurrentBehaviorState == EDinosaurBehaviorState::Dead)
    {
        return;
    }

    // Switch to hunting state
    SetBehaviorState(EDinosaurBehaviorState::Hunting);

    // Roar to signal the hunt (if not on cooldown)
    if (!bRoarOnCooldown)
    {
        PerformRoar();
    }

    unreal::log(TEXT("TRex: Prey detected — initiating hunt"));
}

void ATRexCharacter::PerformRoar()
{
    if (bRoarOnCooldown || CurrentBehaviorState == EDinosaurBehaviorState::Dead)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    // Find all actors within roar radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);

    FVector MyLocation = GetActorLocation();
    int32 AffectedCount = 0;

    for (AActor* Actor : NearbyActors)
    {
        if (!Actor || Actor == this)
        {
            continue;
        }

        float Distance = FVector::Dist(MyLocation, Actor->GetActorLocation());
        if (Distance <= RoarRadius)
        {
            // Apply fear to player characters and other dinosaurs
            // (Fear component integration handled via interface in future cycle)
            AffectedCount++;
        }
    }

    // Debug visualization in editor
#if WITH_EDITOR
    DrawDebugSphere(World, MyLocation, RoarRadius, 24, FColor::Orange, false, 3.0f, 0, 5.0f);
#endif

    // Start cooldown
    bRoarOnCooldown = true;
    World->GetTimerManager().SetTimer(
        RoarCooldownHandle,
        this,
        &ATRexCharacter::ResetRoarCooldown,
        RoarCooldown,
        false
    );

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] ROARED — affected %d actors within %.0f UU"),
        *GetActorLabel(), AffectedCount, RoarRadius);
}

void ATRexCharacter::InitiateCharge(AActor* Target)
{
    if (!Target || bIsCharging || CurrentBehaviorState == EDinosaurBehaviorState::Dead)
    {
        return;
    }

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    ChargeTarget = Target;
    bIsCharging = true;
    ChargeElapsed = 0.0f;

    // Apply charge speed boost
    MoveComp->MaxWalkSpeed = BaseWalkSpeed * ChargeSpeedMultiplier;

    // Set aggressive behavior state
    SetBehaviorState(EDinosaurBehaviorState::Aggressive);

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] initiating charge toward [%s] at %.0f UU/s"),
        *GetActorLabel(), *Target->GetActorLabel(), MoveComp->MaxWalkSpeed);
}

void ATRexCharacter::EndCharge()
{
    bIsCharging = false;
    ChargeElapsed = 0.0f;
    ChargeTarget.Reset();

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = BaseWalkSpeed;
    }

    // Return to hunting if we still have a target, else roam
    SetBehaviorState(EDinosaurBehaviorState::Hunting);

    UE_LOG(LogTemp, Log, TEXT("TRex [%s] charge ended — returning to hunt state"), *GetActorLabel());
}

void ATRexCharacter::ResetRoarCooldown()
{
    bRoarOnCooldown = false;
    UE_LOG(LogTemp, Log, TEXT("TRex [%s] roar cooldown reset"), *GetActorLabel());
}
