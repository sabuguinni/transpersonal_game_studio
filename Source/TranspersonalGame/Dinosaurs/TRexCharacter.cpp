// TRexCharacter.cpp
// Core Systems Programmer #03 — Cycle AUTO_20260620_005
// Tyrannosaurus Rex — apex predator implementation
// Inherits DinosaurBase (APawn subclass)

#include "TRexCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // T-Rex specific capsule — large, heavy predator
    if (GetCapsuleComponent())
    {
        GetCapsuleComponent()->SetCapsuleHalfHeight(200.0f);
        GetCapsuleComponent()->SetCapsuleRadius(80.0f);
    }

    // Movement — powerful but not agile
    if (GetMovementComponent())
    {
        UFloatingPawnMovement* MoveComp = Cast<UFloatingPawnMovement>(GetMovementComponent());
        if (MoveComp)
        {
            MoveComp->MaxSpeed = 1200.0f;       // ~43 km/h — realistic T-Rex estimate
            MoveComp->Acceleration = 400.0f;    // Slow to accelerate (heavy body)
            MoveComp->Deceleration = 600.0f;
        }
    }

    // T-Rex stats — apex predator
    MaxHealth = 1500.0f;
    CurrentHealth = 1500.0f;
    MaxStamina = 80.0f;
    CurrentStamina = 80.0f;
    bIsPredator = true;
    DetectionRadius = 4000.0f;   // 40m detection range
    AttackRange = 300.0f;        // 3m bite range
    TerritoryRadius = 8000.0f;   // 80m territory

    // T-Rex specific
    RoarCooldown = 30.0f;
    StompCooldown = 8.0f;
    bIsRoaring = false;
    bIsStomping = false;
    RoarStunRadius = 1500.0f;
    RoarStunDuration = 3.0f;
    StompDamageRadius = 400.0f;
    StompDamage = 120.0f;
    TurnRateDegreesPerSec = 45.0f;  // Limited turn rate — realistic
    FieldOfViewDegrees = 90.0f;     // Binocular forward vision
    bHasRoared = false;
    LastRoarTime = -9999.0f;
    LastStompTime = -9999.0f;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Start roar timer — T-Rex roars periodically to assert territory
    GetWorldTimerManager().SetTimer(
        RoarTimerHandle,
        this,
        &ATRexCharacter::CheckRoarOpportunity,
        RoarCooldown,
        true,
        FMath::RandRange(5.0f, 15.0f)  // Random initial delay
    );

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter[%s] spawned — apex predator active. DetectionRadius=%.0f TerritoryRadius=%.0f"),
        *GetActorLabel(), DetectionRadius, TerritoryRadius);
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Limited turn rate — T-Rex cannot spin quickly
    EnforceTurnRateLimit(DeltaTime);
}

float ATRexCharacter::GetAttackDamage() const
{
    // T-Rex bite force — 57,000 N historically, translated to 80 damage base
    // Scales with stamina: tired T-Rex bites less hard
    float StaminaFactor = FMath::Clamp(CurrentStamina / MaxStamina, 0.5f, 1.0f);
    return 80.0f * StaminaFactor;
}

bool ATRexCharacter::CanAttack() const
{
    // T-Rex needs stamina and target must be in front (limited FOV)
    if (!Super::CanAttack()) return false;
    if (CurrentStamina < 10.0f) return false;
    return true;
}

void ATRexCharacter::PerformAttack(AActor* Target)
{
    if (!Target || !CanAttack()) return;

    // Check target is within FOV
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector Forward = GetActorForwardVector();
    float DotProduct = FVector::DotProduct(Forward, ToTarget);
    float HalfFOVCos = FMath::Cos(FMath::DegreesToRadians(FieldOfViewDegrees * 0.5f));

    if (DotProduct < HalfFOVCos)
    {
        // Target not in FOV — T-Rex must turn first
        UE_LOG(LogTemp, Verbose, TEXT("TRex[%s]: Target outside FOV — turning"), *GetActorLabel());
        return;
    }

    Super::PerformAttack(Target);

    // Drain stamina on attack
    CurrentStamina = FMath::Max(0.0f, CurrentStamina - 15.0f);

    UE_LOG(LogTemp, Log, TEXT("TRex[%s]: BITE on %s — damage=%.1f stamina=%.1f"),
        *GetActorLabel(), *Target->GetActorLabel(), GetAttackDamage(), CurrentStamina);
}

void ATRexCharacter::PerformRoar()
{
    float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastRoarTime < RoarCooldown) return;
    if (bIsRoaring) return;

    bIsRoaring = true;
    LastRoarTime = Now;
    bHasRoared = true;

    UE_LOG(LogTemp, Log, TEXT("TRex[%s]: ROAR — stun radius=%.0f duration=%.1fs"),
        *GetActorLabel(), RoarStunRadius, RoarStunDuration);

    // Apply stun to all actors within RoarStunRadius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= RoarStunRadius)
        {
            // Notify actor of roar stun (they implement their own stun response)
            Actor->ReceiveAnyDamage(0.0f, nullptr, nullptr, this);  // Zero damage, just event trigger
            UE_LOG(LogTemp, Verbose, TEXT("TRex roar stunned: %s (dist=%.0f)"), *Actor->GetActorLabel(), Dist);
        }
    }

    // Reset roar state after animation duration
    FTimerHandle RoarEndHandle;
    GetWorldTimerManager().SetTimer(RoarEndHandle, [this]()
    {
        bIsRoaring = false;
    }, 2.5f, false);

    OnRoarPerformed();
}

void ATRexCharacter::PerformStomp()
{
    float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastStompTime < StompCooldown) return;
    if (bIsStomping) return;

    bIsStomping = true;
    LastStompTime = Now;

    UE_LOG(LogTemp, Log, TEXT("TRex[%s]: STOMP — AoE radius=%.0f damage=%.1f"),
        *GetActorLabel(), StompDamageRadius, StompDamage);

    // AoE damage in stomp radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    FVector StompLocation = GetActorLocation();
    StompLocation.Z -= 100.0f;  // Ground level

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(StompLocation, Actor->GetActorLocation());
        if (Dist <= StompDamageRadius)
        {
            float FalloffDamage = StompDamage * (1.0f - (Dist / StompDamageRadius) * 0.5f);
            UGameplayStatics::ApplyDamage(Actor, FalloffDamage, GetController(), this, nullptr);
            UE_LOG(LogTemp, Verbose, TEXT("TRex stomp hit: %s dist=%.0f dmg=%.1f"), *Actor->GetActorLabel(), Dist, FalloffDamage);
        }
    }

    // Reset stomp state
    FTimerHandle StompEndHandle;
    GetWorldTimerManager().SetTimer(StompEndHandle, [this]()
    {
        bIsStomping = false;
    }, 1.5f, false);

    OnStompPerformed();
}

bool ATRexCharacter::IsTargetInFOV(AActor* Target) const
{
    if (!Target) return false;
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector Forward = GetActorForwardVector();
    float Dot = FVector::DotProduct(Forward, ToTarget);
    float HalfFOVCos = FMath::Cos(FMath::DegreesToRadians(FieldOfViewDegrees * 0.5f));
    return Dot >= HalfFOVCos;
}

void ATRexCharacter::EnforceTurnRateLimit(float DeltaTime)
{
    // Clamp yaw rotation change per frame to TurnRateDegreesPerSec
    // This is enforced at the movement level — actual implementation
    // depends on the controller driving rotation. This stub records
    // the constraint for BehaviorTree tasks to respect.
    // BT tasks should use TurnRateDegreesPerSec when rotating toward target.
    (void)DeltaTime;
}

void ATRexCharacter::CheckRoarOpportunity()
{
    // Roar when stamina is high and not currently in combat
    if (CurrentStamina > MaxStamina * 0.7f && BehaviorState != ECore_DinoState::Attacking)
    {
        PerformRoar();
    }
}

void ATRexCharacter::OnDinosaurSpawned_Implementation()
{
    Super::OnDinosaurSpawned_Implementation();
    UE_LOG(LogTemp, Log, TEXT("TRex[%s]: APEX PREDATOR ONLINE"), *GetActorLabel());
}

void ATRexCharacter::OnDinosaurDeath_Implementation()
{
    Super::OnDinosaurDeath_Implementation();
    GetWorldTimerManager().ClearTimer(RoarTimerHandle);
    UE_LOG(LogTemp, Log, TEXT("TRex[%s]: FALLEN — territory open"), *GetActorLabel());
}

void ATRexCharacter::OnRoarPerformed_Implementation()
{
    // Blueprint override point — play roar animation + sound
}

void ATRexCharacter::OnStompPerformed_Implementation()
{
    // Blueprint override point — play stomp animation + ground shake VFX
}
