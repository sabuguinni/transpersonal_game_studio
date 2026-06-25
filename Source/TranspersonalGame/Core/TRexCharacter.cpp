// TRexCharacter.cpp — Tyrannosaurus Rex implementation
// Agent #03 — Core Systems Programmer — PROD_CYCLE_AUTO_20260625_002

#include "Core/TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"
#include "Engine/World.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // === Species identity ===
    Species = EDinoSpecies::TRex;
    bIsPredator = true;

    // === T-Rex base stats ===
    MaxHealth       = 500.0f;
    CurrentHealth   = 500.0f;
    AttackDamage    = 120.0f;
    SenseRadius     = 3000.0f;
    AttackRadius    = 250.0f;
    MaxStamina      = 100.0f;
    CurrentStamina  = 100.0f;
    HungerDecayRate = 2.0f;   // Apex predator burns energy fast
    ThirstDecayRate = 1.5f;

    // === Movement — T-Rex is fast in a straight line ===
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed    = 900.0f;
        GetCharacterMovement()->JumpZVelocity   = 0.0f;   // T-Rex cannot jump
        GetCharacterMovement()->bCanWalkOffLedges = true;
        GetCharacterMovement()->RotationRate    = FRotator(0.0f, 180.0f, 0.0f);
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->GravityScale    = 1.2f;   // Heavy — falls faster
        GetCharacterMovement()->Mass            = 8000.0f; // 8 tonnes
    }

    // === Scale — T-Rex is enormous ===
    SetActorScale3D(FVector(3.0f, 3.0f, 3.0f));

    // Solitary — no pack
    bIsPack = false;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Initialise roar cooldown (can roar immediately on spawn)
    TimeSinceLastRoar = RoarCooldown;

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s spawned — Health=%.0f AttackDmg=%.0f SenseRadius=%.0f"),
        *GetActorLabel(), MaxHealth, AttackDamage, SenseRadius);
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Track roar cooldown
    if (TimeSinceLastRoar < RoarCooldown)
    {
        TimeSinceLastRoar += DeltaTime;
    }

    // Tick charge movement
    if (bIsCharging)
    {
        TickCharge(DeltaTime);
    }
}

// ============================================================
// CHARGE ATTACK
// ============================================================

void ATRexCharacter::StartCharge(AActor* Target)
{
    if (!Target || bIsCharging) return;

    // Stamina check
    if (CurrentStamina < ChargeStaminaCost)
    {
        UE_LOG(LogTemp, Warning, TEXT("[TRex] %s cannot charge — insufficient stamina (%.0f/%.0f)"),
            *GetActorLabel(), CurrentStamina, ChargeStaminaCost);
        return;
    }

    bIsCharging   = true;
    ChargeTarget  = Target;
    CurrentStamina -= ChargeStaminaCost;

    // Boost movement speed for charge
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 900.0f * ChargeSpeedMultiplier;
    }

    // Auto-stop charge after 4 seconds (prevent infinite charge)
    GetWorldTimerManager().SetTimer(ChargeTimerHandle, this, &ATRexCharacter::StopCharge, 4.0f, false);

    OnChargeStart();

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s CHARGING at %s"),
        *GetActorLabel(), *Target->GetActorLabel());
}

void ATRexCharacter::TickCharge(float DeltaTime)
{
    if (!ChargeTarget.Get())
    {
        StopCharge();
        return;
    }

    // Move toward target
    FVector Direction = (ChargeTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    AddMovementInput(Direction, 1.0f);

    // Check impact distance
    float DistToTarget = FVector::Dist(GetActorLocation(), ChargeTarget->GetActorLocation());
    if (DistToTarget <= AttackRadius * 1.5f)
    {
        OnChargeImpact(ChargeTarget.Get());
        StopCharge();
    }
}

void ATRexCharacter::OnChargeImpact(AActor* HitActor)
{
    if (!HitActor) return;

    float TotalDamage = AttackDamage + ChargeImpactBonusDamage;
    UGameplayStatics::ApplyDamage(HitActor, TotalDamage, GetController(), this, nullptr);

    OnChargeImpactEvent(HitActor);

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s CHARGE IMPACT on %s — Damage=%.0f"),
        *GetActorLabel(), *HitActor->GetActorLabel(), TotalDamage);
}

void ATRexCharacter::StopCharge()
{
    if (!bIsCharging) return;

    bIsCharging  = false;
    ChargeTarget = nullptr;

    // Restore normal speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 900.0f;
    }

    GetWorldTimerManager().ClearTimer(ChargeTimerHandle);

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s charge stopped"), *GetActorLabel());
}

// ============================================================
// ROAR
// ============================================================

bool ATRexCharacter::CanRoar() const
{
    return TimeSinceLastRoar >= RoarCooldown;
}

void ATRexCharacter::PerformRoar()
{
    if (!CanRoar()) return;

    TimeSinceLastRoar = 0.0f;

    // Apply stun to nearby prey
    ApplyRoarStun();

    // Notify Blueprint for animation + sound
    OnRoar();

    UE_LOG(LogTemp, Log, TEXT("[TRex] %s ROARS — stunning prey within %.0f units"),
        *GetActorLabel(), RoarRadius);
}

void ATRexCharacter::ApplyRoarStun()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Find all actors within roar radius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape Sphere = FCollisionShape::MakeSphere(RoarRadius);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        Sphere,
        Params
    );

    int32 StunnedCount = 0;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Target = Overlap.GetActor();
        if (!Target) continue;

        // Apply stun via custom event (Blueprint handles animation/AI pause)
        // For now: apply a small damage as fear response
        UGameplayStatics::ApplyDamage(Target, 5.0f, GetController(), this, nullptr);
        StunnedCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("[TRex] Roar stunned %d actors"), StunnedCount);

#if WITH_EDITOR
    // Debug sphere in editor
    DrawDebugSphere(World, GetActorLocation(), RoarRadius, 16, FColor::Orange, false, 3.0f);
#endif
}
