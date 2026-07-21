// VelociraptorCharacter.cpp — Velociraptor pack hunter implementation
// Agent #04 — Performance Optimizer | PROD_CYCLE_AUTO_20260625_007

#include "VelociraptorCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"

AVelociraptorCharacter::AVelociraptorCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f; // 20Hz — fast enough for agile predator

    // --- Survival stats (inherited from ADinosaurBase) ---
    MaxHealth = 300.f;
    CurrentHealth = 300.f;
    AttackDamage = 80.f;
    DetectionRadius = 2000.f;

    // --- Pack hunting ---
    PackRadius = 1500.f;
    PackBonusDamageMultiplier = 1.5f;
    NearbyPackCount = 0;

    // --- Leap attack ---
    LeapRange = 400.f;
    LeapDamage = 60.f;
    LeapCooldown = 4.f;
    bCanLeap = true;

    // --- Movement: fast, agile, low gravity ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 700.f;         // ~25 km/h sprint
        MoveComp->MaxAcceleration = 3000.f;     // Very quick acceleration
        MoveComp->BrakingDecelerationWalking = 2000.f;
        MoveComp->JumpZVelocity = 500.f;        // Can leap/jump
        MoveComp->GravityScale = 1.0f;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, 720.f, 0.f); // Fast turning
    }

    // Scale: smaller predator
    SetActorScale3D(FVector(1.5f, 1.5f, 1.5f));

    // Species tag for AI behavior tree queries
    // DinosaurSpecies = EDinosaurSpecies::Velociraptor; // set when enum is available
}

void AVelociraptorCharacter::BeginPlay()
{
    Super::BeginPlay();
    bCanLeap = true;
}

void AVelociraptorCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update pack count every tick (TickInterval=0.05s keeps this cheap)
    UpdatePackCount();
}

void AVelociraptorCharacter::UpdatePackCount()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AVelociraptorCharacter::StaticClass(), NearbyActors);

    int32 Count = 0;
    for (AActor* Actor : NearbyActors)
    {
        if (Actor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        if (Dist <= PackRadius)
        {
            Count++;
        }
    }
    NearbyPackCount = Count;
}

float AVelociraptorCharacter::GetEffectiveAttackDamage() const
{
    if (NearbyPackCount >= 2)
    {
        return AttackDamage * PackBonusDamageMultiplier;
    }
    return AttackDamage;
}

void AVelociraptorCharacter::PerformLeapAttack(AActor* Target)
{
    if (!Target || !bCanLeap) return;

    float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
    if (Dist > LeapRange) return;

    // Apply leap damage (with pack bonus)
    float FinalDamage = LeapDamage;
    if (NearbyPackCount >= 2)
    {
        FinalDamage *= PackBonusDamageMultiplier;
    }

    UGameplayStatics::ApplyDamage(
        Target,
        FinalDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    // Launch self toward target (leap physics)
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector LaunchVelocity = ToTarget * 800.f + FVector(0.f, 0.f, 300.f);
    LaunchCharacter(LaunchVelocity, true, true);

    // Start cooldown
    bCanLeap = false;
    GetWorldTimerManager().SetTimer(
        LeapCooldownTimer,
        this,
        &AVelociraptorCharacter::ResetLeapCooldown,
        LeapCooldown,
        false
    );
}

void AVelociraptorCharacter::ResetLeapCooldown()
{
    bCanLeap = true;
}

void AVelociraptorCharacter::EmitDistressCall()
{
    // Broadcast zero-damage event to all nearby raptors — they converge on this location
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> NearbyRaptors;
    UGameplayStatics::GetAllActorsOfClass(World, AVelociraptorCharacter::StaticClass(), NearbyRaptors);

    for (AActor* Raptor : NearbyRaptors)
    {
        if (Raptor == this) continue;
        float Dist = FVector::Dist(GetActorLocation(), Raptor->GetActorLocation());
        if (Dist <= PackRadius * 2.f) // Distress call reaches twice the pack radius
        {
            // Zero-damage notification — listeners implement OnAnyDamage to respond
            UGameplayStatics::ApplyDamage(
                Raptor,
                0.f,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
        }
    }
}
