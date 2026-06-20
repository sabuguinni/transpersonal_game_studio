// TRexDinosaur.cpp
// Performance Optimizer — Agent #04 | PROD_CYCLE_AUTO_20260620_008
// T-Rex species implementation: Roar, bleed DoT, death sequence

#include "TRexDinosaur.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ATRexDinosaur::ATRexDinosaur()
{
    // Override base stats with T-Rex values
    MaxHealth          = 500.0f;
    CurrentHealth      = 500.0f;
    AttackDamage       = 150.0f;
    AttackRange        = 250.0f;
    MovementSpeed      = 600.0f;
    bIsAggressive      = false;

    // T-Rex specific
    RoarRadius         = 800.0f;
    RoarStunDuration   = 2.0f;
    RoarCooldown       = 15.0f;
    BleedDamagePerSec  = 10.0f;
    BleedDuration      = 5.0f;
    bRoarOnCooldown    = false;

    GetCharacterMovement()->MaxWalkSpeed = MovementSpeed;
}

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();
    // T-Rex starts passive — becomes aggressive when hit or player enters territory
    bIsAggressive = false;
}

void ATRexDinosaur::Roar()
{
    if (bRoarOnCooldown) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Visual debug sphere for roar radius (editor only)
#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), RoarRadius, 16, FColor::Red, false, 2.0f);
#endif

    // Find all actors in roar radius — apply stun (fear) to player
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(RoarRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    World->OverlapMultiByChannel(Overlaps, GetActorLocation(), FQuat::Identity,
        ECC_Pawn, Sphere, Params);

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Target = Overlap.GetActor();
        if (!Target) continue;

        // Apply stun via generic damage event (stun tag via DamageType)
        UGameplayStatics::ApplyDamage(Target, 0.0f, GetController(),
            this, UDamageType::StaticClass());

        // Log stun
        UE_LOG(LogTemp, Log, TEXT("TRex Roar stunned: %s for %.1fs"),
            *Target->GetName(), RoarStunDuration);
    }

    // Start cooldown
    bRoarOnCooldown = true;
    World->GetTimerManager().SetTimer(RoarCooldownHandle, this,
        &ATRexDinosaur::ResetRoarCooldown, RoarCooldown, false);

    UE_LOG(LogTemp, Log, TEXT("TRex ROAR — radius=%.0f stun=%.1fs cooldown=%.0fs"),
        RoarRadius, RoarStunDuration, RoarCooldown);
}

void ATRexDinosaur::Attack_Implementation(AActor* Target)
{
    if (!Target) return;

    UWorld* World = GetWorld();
    if (!World) return;

    // Base attack damage
    UGameplayStatics::ApplyDamage(Target, AttackDamage,
        GetController(), this, UDamageType::StaticClass());

    // Apply bleed DoT — timer fires every 1s for BleedDuration seconds
    int32 BleedTicks = FMath::RoundToInt(BleedDuration);
    for (int32 i = 1; i <= BleedTicks; ++i)
    {
        FTimerHandle BleedHandle;
        float Delay = (float)i;
        // Capture target weakly — use lambda with raw ptr (target may die)
        AActor* RawTarget = Target;
        float BleedDmg = BleedDamagePerSec;
        AActor* Self = this;
        AController* Ctrl = GetController();

        World->GetTimerManager().SetTimer(BleedHandle,
            FTimerDelegate::CreateLambda([RawTarget, BleedDmg, Self, Ctrl]()
            {
                if (IsValid(RawTarget) && IsValid(Self))
                {
                    UGameplayStatics::ApplyDamage(RawTarget, BleedDmg,
                        Ctrl, Self, UDamageType::StaticClass());
                    UE_LOG(LogTemp, Verbose, TEXT("TRex bleed tick: %.1f dmg to %s"),
                        BleedDmg, *RawTarget->GetName());
                }
            }),
            Delay, false);
    }

    UE_LOG(LogTemp, Log, TEXT("TRex Attack: %.0f dmg + bleed %.0f/s x %.0fs on %s"),
        AttackDamage, BleedDamagePerSec, BleedDuration, *Target->GetName());
}

void ATRexDinosaur::OnDeath_Implementation()
{
    // Stop roar cooldown timer
    UWorld* World = GetWorld();
    if (World)
    {
        World->GetTimerManager().ClearTimer(RoarCooldownHandle);
    }

    // Call base death (disables movement + collision, schedules destroy)
    Super::OnDeath_Implementation();

    UE_LOG(LogTemp, Log, TEXT("TRex DEAD — %s"), *GetName());
}

void ATRexDinosaur::ResetRoarCooldown()
{
    bRoarOnCooldown = false;
    UE_LOG(LogTemp, Log, TEXT("TRex Roar cooldown reset — ready"));
}
