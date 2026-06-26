#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    InitTRexStats();
}

void ATRexCharacter::InitTRexStats()
{
    // T-Rex is an apex carnivore — high health, high damage, slow but powerful
    DinoStats.MaxHealth        = 1200.0f;
    DinoStats.CurrentHealth    = 1200.0f;
    DinoStats.AttackDamage     = 120.0f;
    DinoStats.AttackRange      = 280.0f;
    DinoStats.DetectionRadius  = 2500.0f;
    DinoStats.MoveSpeed        = 480.0f;   // ~17 km/h — realistic T-Rex sprint
    DinoStats.ChaseSpeed       = 680.0f;   // ~24 km/h at full sprint
    DinoStats.Mass             = 8000.0f;  // kg
    DinoStats.FleeHealthThreshold = 0.15f; // Only flees below 15% health
    DinoStats.AttackCooldown   = 2.5f;

    // Species identity
    Species     = EEng_DinoSpecies::TRex;
    DietType    = EEng_DinoDiet::Carnivore;
    bIsAggressive = true;

    // Movement component tuning
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed          = DinoStats.MoveSpeed;
        GetCharacterMovement()->MaxAcceleration       = 800.0f;
        GetCharacterMovement()->BrakingDecelerationWalking = 600.0f;
        GetCharacterMovement()->RotationRate          = FRotator(0.0f, 120.0f, 0.0f);
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->GravityScale          = 1.2f;
    }
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // T-Rex starts in Idle, will transition to Patrol after 3s
    CurrentState = EEng_DinoState::Idle;
    LastRoarTime = -RoarCooldown; // Allow immediate first roar

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter [%s] spawned — Health: %.0f, DetectionRadius: %.0f"),
        *GetActorLabel(), DinoStats.MaxHealth, DinoStats.DetectionRadius);
}

void ATRexCharacter::PerformRoar()
{
    const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (Now - LastRoarTime < RoarCooldown)
    {
        return; // Still on cooldown
    }

    bIsRoaring = true;
    LastRoarTime = Now;

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter [%s] ROAR — stunning prey within %.0f units"),
        *GetActorLabel(), DinoStats.DetectionRadius * 0.6f);

    // Find all actors within roar radius and apply stun (via gameplay tag or damage event)
    TArray<AActor*> NearbyActors;
    const float RoarRadius = DinoStats.DetectionRadius * 0.6f;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    for (AActor* Target : NearbyActors)
    {
        if (!Target || Target == this) continue;

        const float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
        if (Dist <= RoarRadius)
        {
            // Apply a small fear damage event — gameplay systems can intercept this
            UGameplayStatics::ApplyDamage(
                Target,
                5.0f,          // Minimal direct damage from roar
                GetController(),
                this,
                UDamageType::StaticClass()
            );
        }
    }

    // Clear roar flag after animation would finish (simplified — no animation system yet)
    FTimerHandle RoarTimer;
    GetWorld()->GetTimerManager().SetTimer(RoarTimer, [this]()
    {
        bIsRoaring = false;
    }, 2.5f, false);
}

void ATRexCharacter::PerformStomp()
{
    if (!GetWorld()) return;

    const FVector StompOrigin = GetActorLocation();

    UE_LOG(LogTemp, Log, TEXT("ATRexCharacter [%s] STOMP — AoE %.0f radius, %.0f damage"),
        *GetActorLabel(), StompRadius, StompDamage);

    // Radial damage centered on feet
    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        StompDamage,
        StompOrigin,
        StompRadius,
        UDamageType::StaticClass(),
        TArray<AActor*>{ this }, // Ignore self
        this,
        GetController(),
        true // Full damage at center, falloff to edge
    );

#if WITH_EDITOR
    // Debug visualization in editor
    DrawDebugSphere(GetWorld(), StompOrigin, StompRadius, 12, FColor::Orange, false, 2.0f);
#endif
}
