// TRexCharacter.cpp
// Core Systems Programmer #03 — Transpersonal Game Studio
// T-Rex apex predator — full implementation
// Cycle: PROD_CYCLE_AUTO_20260630_010

#include "Dinosaurs/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // ─── T-Rex Base Stats ───────────────────────────────────────────────────
    MaxHealth           = 2000.0f;
    CurrentHealth       = 2000.0f;
    AttackDamage        = 150.0f;
    AttackRange         = 250.0f;       // Bite range (cm)
    DetectionRadius     = 2500.0f;      // T-Rex has excellent vision
    AggroRadius         = 1800.0f;
    WalkSpeed           = 400.0f;
    RunSpeed            = 800.0f;
    MaxHunger           = 100.0f;
    CurrentHunger       = 80.0f;
    HungerDecayRate     = 0.5f;         // Slower decay — large metabolism

    // ─── T-Rex Specific Stats ───────────────────────────────────────────────
    RoarRadius          = 1500.0f;
    RoarFearDuration    = 8.0f;
    StompRadius         = 350.0f;
    StompDamage         = 80.0f;
    RoarCooldown        = 30.0f;
    TerritoryRadius     = 5000.0f;
    bIsAmbushing        = false;
    bRoarOnCooldown     = false;
    LastStompTime       = 0.0f;
    StompCooldown       = 5.0f;

    // ─── Capsule Size (large predator) ──────────────────────────────────────
    GetCapsuleComponent()->SetCapsuleHalfHeight(180.0f);
    GetCapsuleComponent()->SetCapsuleRadius(80.0f);

    // ─── Movement ───────────────────────────────────────────────────────────
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed          = WalkSpeed;
        MoveComp->MaxAcceleration       = 600.0f;
        MoveComp->BrakingDecelerationWalking = 800.0f;
        MoveComp->RotationRate          = FRotator(0.0f, 180.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->GravityScale          = 1.2f;    // Heavy — falls fast
    }
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Record territory center at spawn location
    TerritoryCenter = GetActorLocation();

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter [%s] spawned at territory center (%.0f, %.0f, %.0f)"),
        *GetName(), TerritoryCenter.X, TerritoryCenter.Y, TerritoryCenter.Z);
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Ambush logic: if Idle and player is within 3x detection radius but not aggro radius,
    // enter ambush mode (crouch/slow movement)
    if (CurrentState == EDinoState::Idle || CurrentState == EDinoState::Patrol)
    {
        UWorld* World = GetWorld();
        if (World)
        {
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
            if (PlayerPawn)
            {
                float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
                // Enter ambush if player is in outer detection zone but not yet aggro
                if (DistToPlayer < DetectionRadius * 2.0f && DistToPlayer > AggroRadius)
                {
                    if (!bIsAmbushing)
                    {
                        bIsAmbushing = true;
                        // Slow down to ambush walk speed
                        UCharacterMovementComponent* MoveComp = GetCharacterMovement();
                        if (MoveComp)
                        {
                            MoveComp->MaxWalkSpeed = WalkSpeed * 0.4f;
                        }
                    }
                }
                else
                {
                    if (bIsAmbushing)
                    {
                        bIsAmbushing = false;
                        UCharacterMovementComponent* MoveComp = GetCharacterMovement();
                        if (MoveComp)
                        {
                            MoveComp->MaxWalkSpeed = WalkSpeed;
                        }
                    }
                }
            }
        }
    }
}

void ATRexCharacter::PerformRoar()
{
    if (bRoarOnCooldown) return;

    UWorld* World = GetWorld();
    if (!World) return;

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter [%s] ROAR — fear radius %.0f cm"), *GetName(), RoarRadius);

    // Find all actors within roar radius
    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        World,
        GetActorLocation(),
        RoarRadius,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ this },
        OverlappingActors
    );

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor != this)
        {
            ApplyFearToActor(Actor, RoarFearDuration);
        }
    }

    // Start roar cooldown
    bRoarOnCooldown = true;
    World->GetTimerManager().SetTimer(
        RoarCooldownTimer,
        this,
        &ATRexCharacter::ResetRoarCooldown,
        RoarCooldown,
        false
    );

    // Debug sphere
#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), RoarRadius, 16, FColor::Orange, false, 3.0f);
#endif
}

void ATRexCharacter::PerformStomp()
{
    UWorld* World = GetWorld();
    if (!World) return;

    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastStompTime < StompCooldown) return;

    LastStompTime = CurrentTime;

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter [%s] STOMP — AoE radius %.0f, damage %.0f"),
        *GetName(), StompRadius, StompDamage);

    // AoE damage in front of T-Rex
    FVector StompCenter = GetActorLocation() + GetActorForwardVector() * 200.0f;

    TArray<AActor*> OverlappingActors;
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
        World,
        StompCenter,
        StompRadius,
        ObjectTypes,
        nullptr,
        TArray<AActor*>{ this },
        OverlappingActors
    );

    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor != this)
        {
            UGameplayStatics::ApplyDamage(
                Actor,
                StompDamage,
                GetController(),
                this,
                UDamageType::StaticClass()
            );
            UE_LOG(LogTemp, Log, TEXT("  Stomp hit: %s for %.0f damage"), *Actor->GetName(), StompDamage);
        }
    }

#if WITH_EDITOR
    DrawDebugSphere(World, StompCenter, StompRadius, 12, FColor::Red, false, 2.0f);
#endif
}

void ATRexCharacter::PerformBite()
{
    // Bite is the primary single-target attack — delegates to base PerformAttack
    PerformAttack();

    UE_LOG(LogTemp, Log, TEXT("TRexCharacter [%s] BITE — damage %.0f"), *GetName(), AttackDamage);
}

void ATRexCharacter::PerformAttack()
{
    // Override base attack: T-Rex uses bite (high damage, short range)
    if (!CurrentTarget) return;

    float DistToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
    if (DistToTarget <= AttackRange)
    {
        UGameplayStatics::ApplyDamage(
            CurrentTarget,
            AttackDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );

        UE_LOG(LogTemp, Log, TEXT("TRexCharacter [%s] bit %s for %.0f damage"),
            *GetName(), *CurrentTarget->GetName(), AttackDamage);

        // After a successful bite, try roar if not on cooldown
        if (!bRoarOnCooldown)
        {
            PerformRoar();
        }
    }
}

void ATRexCharacter::ApplyFearToActor(AActor* Target, float Duration)
{
    if (!Target) return;

    // Apply fear via game tag or property if the target supports it
    // For now, log the fear application — SurvivalComponent integration will handle this
    UE_LOG(LogTemp, Log, TEXT("TRexCharacter: Fear applied to %s for %.1f seconds"),
        *Target->GetName(), Duration);

    // TODO: Call Target->GetComponentByClass<USurvivalComponent>()->ApplyFear(Duration)
    // when SurvivalComponent integration is complete
}

void ATRexCharacter::ResetRoarCooldown()
{
    bRoarOnCooldown = false;
    UE_LOG(LogTemp, Log, TEXT("TRexCharacter [%s] roar cooldown reset"), *GetName());
}
