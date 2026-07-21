// RaptorDinosaur.cpp
// Agent #05 — Procedural World Generator | PROD_CYCLE_AUTO_20260620_008
// Implements pack-hunting Velociraptor behaviour

#include "RaptorDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

ARaptorDinosaur::ARaptorDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // Raptor stats — fast, fragile, pack hunter
    MaxHealth = 80.0f;
    CurrentHealth = 80.0f;
    AttackDamage = 40.0f;
    bIsPackLeader = false;
    PackSize = 0;
    LeapCooldown = 5.0f;
    bLeapOnCooldown = false;

    // High movement speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 800.0f;
        GetCharacterMovement()->JumpZVelocity = 600.0f;
        GetCharacterMovement()->GravityScale = 1.2f;
    }
}

void ARaptorDinosaur::BeginPlay()
{
    Super::BeginPlay();
}

void ARaptorDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

// ─── Combat ───────────────────────────────────────────────────────────────────

void ARaptorDinosaur::Attack_Implementation(AActor* Target)
{
    if (!Target || !IsAlive())
    {
        return;
    }

    // Base claw attack
    UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, nullptr);

    // Raptors apply a bleed DoT (5 dmg/s for 3s)
    UWorld* W = GetWorld();
    if (W)
    {
        AActor* WeakTarget = Target;
        float BleedDamage = 5.0f;
        int32 BleedTicks = 3;

        for (int32 i = 1; i <= BleedTicks; ++i)
        {
            float Delay = static_cast<float>(i);
            FTimerHandle BleedHandle;
            W->GetTimerManager().SetTimer(BleedHandle, [WeakTarget, BleedDamage]()
            {
                if (IsValid(WeakTarget))
                {
                    UGameplayStatics::ApplyDamage(WeakTarget, BleedDamage, nullptr, nullptr, nullptr);
                }
            }, Delay, false);
        }
    }
}

void ARaptorDinosaur::OnDeath_Implementation()
{
    // Notify pack leader that a member died
    if (!bIsPackLeader && IsValid(PackLeader))
    {
        PackLeader->PackSize = FMath::Max(0, PackLeader->PackSize - 1);
    }

    // Clear leap cooldown timer
    GetWorld()->GetTimerManager().ClearTimer(LeapCooldownHandle);

    Super::OnDeath_Implementation();
}

// ─── Leap Attack ──────────────────────────────────────────────────────────────

void ARaptorDinosaur::LeapAttack(AActor* Target)
{
    if (!Target || bLeapOnCooldown || !IsAlive())
    {
        return;
    }

    // Direction toward target
    FVector ToTarget = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector LeapImpulse = ToTarget * 1200.0f + FVector(0.0f, 0.0f, 500.0f);

    // Launch the raptor toward the target
    LaunchCharacter(LeapImpulse, true, true);

    // Apply bonus damage on leap
    float LeapDamage = AttackDamage * 1.5f;
    UGameplayStatics::ApplyDamage(Target, LeapDamage, GetController(), this, nullptr);

    // Start cooldown
    bLeapOnCooldown = true;
    GetWorld()->GetTimerManager().SetTimer(LeapCooldownHandle, this,
        &ARaptorDinosaur::ResetLeapCooldown, LeapCooldown, false);
}

void ARaptorDinosaur::ResetLeapCooldown()
{
    bLeapOnCooldown = false;
}

// ─── Pack Behaviour ───────────────────────────────────────────────────────────

void ARaptorDinosaur::JoinPack(ARaptorDinosaur* Leader)
{
    if (!IsValid(Leader) || Leader == this)
    {
        return;
    }

    PackLeader = Leader;
    bIsPackLeader = false;

    // Increment leader's pack count
    Leader->PackSize = FMath::Min(Leader->PackSize + 1, 6); // max pack of 6

    // Calculate flank offset based on pack size for coordinated attacks
    int32 MemberIndex = Leader->PackSize;
    float FlankAngle = (MemberIndex % 2 == 0) ? 45.0f : -45.0f;
    float FlankRadius = 300.0f + (MemberIndex / 2) * 150.0f;

    FlankOffset = FVector(
        FMath::Cos(FMath::DegreesToRadians(FlankAngle)) * FlankRadius,
        FMath::Sin(FMath::DegreesToRadians(FlankAngle)) * FlankRadius,
        0.0f
    );
}

bool ARaptorDinosaur::IsAlive() const
{
    return CurrentHealth > 0.0f;
}
