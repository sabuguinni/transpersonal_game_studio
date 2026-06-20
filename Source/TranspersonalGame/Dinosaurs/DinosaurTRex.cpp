// DinosaurTRex.cpp
// Core Systems Programmer #03 — PROD_CYCLE_AUTO_20260620_004
// T-Rex implementation: stomp AoE, roar stun, territory defense, slow turn rate

#include "Dinosaurs/DinosaurTRex.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

ADinosaurTRex::ADinosaurTRex()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Species identity ---
    Species = EDinosaurSpecies::TyrannosaurusRex;
    BehaviorState = EDinosaurBehavior::Idle;

    // --- T-Rex stats: apex predator ---
    MaxHealth = 2000.0f;
    CurrentHealth = 2000.0f;
    AttackDamage = 200.0f;
    DetectionRadius = 4000.0f;
    AttackRange = 300.0f;
    MoveSpeed = 350.0f;

    // --- Movement: powerful but not agile ---
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = MoveSpeed;
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.0f, MaxTurnRate, 0.0f);
        MoveComp->JumpZVelocity = 0.0f;       // T-Rex cannot jump
        MoveComp->GravityScale = 1.5f;         // Heavy — falls fast
        MoveComp->MaxAcceleration = 600.0f;    // Slow to accelerate
        MoveComp->BrakingDecelerationWalking = 800.0f;
        MoveComp->Mass = 8000.0f;              // 8 metric tons
    }

    // --- Capsule: massive body ---
    UCapsuleComponent* Capsule = GetCapsuleComponent();
    if (Capsule)
    {
        Capsule->SetCapsuleHalfHeight(250.0f);
        Capsule->SetCapsuleRadius(120.0f);
    }

    // --- Cooldown timers start at max (ready to use immediately) ---
    TimeSinceLastStomp = StompCooldown;
    TimeSinceLastRoar = RoarCooldown;
}

void ADinosaurTRex::BeginPlay()
{
    Super::BeginPlay();

    // Claim territory at spawn location
    ClaimTerritory();

    UE_LOG(LogTemp, Log, TEXT("ADinosaurTRex[%s] spawned — Health=%.0f, Territory radius=%.0fcm"),
        *GetActorLabel(), CurrentHealth, TerritoryRadius);
}

void ADinosaurTRex::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Advance cooldown timers
    TimeSinceLastStomp += DeltaTime;
    TimeSinceLastRoar += DeltaTime;

    // Territory defense check — if player enters territory, become aggressive
    if (BehaviorState == EDinosaurBehavior::Idle || BehaviorState == EDinosaurBehavior::Patrol)
    {
        APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
        if (PC && PC->GetPawn())
        {
            FVector PlayerLoc = PC->GetPawn()->GetActorLocation();
            if (IsLocationInTerritory(PlayerLoc))
            {
                float DistToPlayer = FVector::Dist(GetActorLocation(), PlayerLoc);
                if (DistToPlayer <= DetectionRadius)
                {
                    BehaviorState = EDinosaurBehavior::Attacking;
                    bIsDefendingTerritory = true;

                    // Roar when first detecting intruder
                    if (CanRoar())
                    {
                        PerformRoar();
                    }
                }
            }
        }
    }
}

// --- Stomp AoE ---

bool ADinosaurTRex::CanStomp() const
{
    return TimeSinceLastStomp >= StompCooldown;
}

void ADinosaurTRex::PerformStomp()
{
    if (!CanStomp()) return;

    TimeSinceLastStomp = 0.0f;

    UWorld* World = GetWorld();
    if (!World) return;

    FVector StompOrigin = GetActorLocation();

    // Find all actors in stomp radius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(StompRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        StompOrigin,
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        SphereShape,
        QueryParams
    );

    int32 HitCount = 0;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!HitActor || HitActor == this) continue;

        // Apply stomp damage
        UGameplayStatics::ApplyDamage(
            HitActor,
            StompDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );

        // Apply knockback to primitives with physics
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Overlap.GetComponent());
        if (PrimComp && PrimComp->IsSimulatingPhysics())
        {
            FVector KnockDir = (HitActor->GetActorLocation() - StompOrigin).GetSafeNormal();
            KnockDir.Z = 0.4f; // Slight upward component
            PrimComp->AddImpulse(KnockDir * StompKnockbackForce, NAME_None, true);
        }

        HitCount++;
    }

    UE_LOG(LogTemp, Log, TEXT("ADinosaurTRex[%s] STOMP — hit %d actors in %.0fcm radius"),
        *GetActorLabel(), HitCount, StompRadius);

#if WITH_EDITOR
    // Debug visualization in editor
    DrawDebugSphere(World, StompOrigin, StompRadius, 16, FColor::Orange, false, 2.0f, 0, 5.0f);
#endif
}

// --- Roar ---

bool ADinosaurTRex::CanRoar() const
{
    return TimeSinceLastRoar >= RoarCooldown;
}

void ADinosaurTRex::PerformRoar()
{
    if (!CanRoar()) return;

    TimeSinceLastRoar = 0.0f;
    BehaviorState = EDinosaurBehavior::Attacking;

    UWorld* World = GetWorld();
    if (!World) return;

    // Find all actors in roar stun radius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape SphereShape = FCollisionShape::MakeSphere(RoarStunRadius);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        SphereShape,
        QueryParams
    );

    // Apply stun via damage event (stun logic handled by SurvivalComponent on character)
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!HitActor || HitActor == this) continue;

        // Apply minimal damage — the stun effect is the real impact
        UGameplayStatics::ApplyDamage(
            HitActor,
            5.0f,   // Roar does minimal direct damage
            GetController(),
            this,
            UDamageType::StaticClass()
        );
    }

    UE_LOG(LogTemp, Log, TEXT("ADinosaurTRex[%s] ROAR — stun radius %.0fcm, duration %.1fs"),
        *GetActorLabel(), RoarStunRadius, RoarStunDuration);

#if WITH_EDITOR
    DrawDebugSphere(World, GetActorLocation(), RoarStunRadius, 24, FColor::Red, false, 3.0f, 0, 3.0f);
#endif
}

// --- Territory ---

void ADinosaurTRex::ClaimTerritory()
{
    TerritoryCenter = GetActorLocation();
    bIsDefendingTerritory = false;

    UE_LOG(LogTemp, Log, TEXT("ADinosaurTRex[%s] claimed territory at (%.0f, %.0f, %.0f) radius=%.0fcm"),
        *GetActorLabel(),
        TerritoryCenter.X, TerritoryCenter.Y, TerritoryCenter.Z,
        TerritoryRadius);
}

bool ADinosaurTRex::IsLocationInTerritory(FVector Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= TerritoryRadius;
}

// --- Death override ---

void ADinosaurTRex::OnDeath_Implementation()
{
    // Call base — disables collision, enables ragdoll, sets 10s lifespan
    Super::OnDeath_Implementation();

    bIsDefendingTerritory = false;
    BehaviorState = EDinosaurBehavior::Dead;

    // Ground shake: apply radial impulse to nearby physics objects
    UWorld* World = GetWorld();
    if (World)
    {
        TArray<FOverlapResult> Overlaps;
        FCollisionShape SphereShape = FCollisionShape::MakeSphere(600.0f);
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(this);

        World->OverlapMultiByChannel(
            Overlaps,
            GetActorLocation(),
            FQuat::Identity,
            ECollisionChannel::ECC_WorldDynamic,
            SphereShape,
            QueryParams
        );

        for (const FOverlapResult& Overlap : Overlaps)
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Overlap.GetComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PrimComp->AddRadialImpulse(GetActorLocation(), 600.0f, 2000.0f,
                    ERadialImpulseFalloff::RIF_Linear, true);
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("ADinosaurTRex[%s] DIED — ground shake triggered"),
        *GetActorLabel());
}
