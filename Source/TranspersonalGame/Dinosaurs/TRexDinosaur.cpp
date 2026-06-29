// TRexDinosaur.cpp
// Transpersonal Game Studio — Core Systems Programmer #03
// Cycle: PROD_CYCLE_AUTO_20260629_011
// Full implementation of ATRexDinosaur — no stubs, all methods implemented

#include "Dinosaurs/TRexDinosaur.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "TimerManager.h"

// ─── Constructor ──────────────────────────────────────────────────────────────

ATRexDinosaur::ATRexDinosaur()
{
    PrimaryActorTick.bCanEverTick = true;

    // Species identity — set on CDO so Blueprint subclasses inherit defaults
    Species = EDinosaurSpecies::TyrannosaurusRex;

    // T-Rex base stats (realistic approximations)
    MaxHealth         = 1500.0f;
    CurrentHealth     = 1500.0f;
    BaseMoveSpeed     = 450.0f;   // ~25 km/h — realistic T-Rex top speed
    DetectionRadius   = 3500.0f;  // Scent-extended detection
    AttackDamage      = 120.0f;   // Bite damage (set in CombatStats too)
    AttackRange       = 250.0f;   // Jaw reach
    DamageResistance  = 0.35f;    // Thick hide — 35% damage reduction
    HungerDecayRate   = 0.8f;     // Large body — slower relative hunger decay
    TerritoryRadius   = 5000.0f;  // Large territory

    // Apply movement speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.0f, 120.0f, 0.0f); // Slow turning
        GetCharacterMovement()->JumpZVelocity = 0.0f; // T-Rex cannot jump
        GetCharacterMovement()->GravityScale = 1.2f;  // Heavy body
    }
}

// ─── BeginPlay ────────────────────────────────────────────────────────────────

void ATRexDinosaur::BeginPlay()
{
    Super::BeginPlay();

    // Initialise charge state
    ChargeState = ECore_TRexChargeState::Idle;
    TimeSinceLastCharge = CombatStats.ChargeCooldownSeconds; // Ready to charge immediately
    ChargePhaseTimer = 0.0f;
    bHasActiveTarget = false;

    // Initialise sensory state
    ActiveSenseMode = ECore_TRexSenseMode::Visual;
    SensoryStats.bTrackingScent = false;
    SensoryStats.LastScentPosition = GetActorLocation();

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur [%s] BeginPlay — Health: %.0f, Territory: %.0f cm radius"),
        *GetName(), CurrentHealth, TerritoryRadius);
}

// ─── Tick ─────────────────────────────────────────────────────────────────────

void ATRexDinosaur::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsDead) return;

    // Advance charge cooldown
    TimeSinceLastCharge += DeltaTime;

    // Tick active charge phase
    if (ChargeState != ECore_TRexChargeState::Idle)
    {
        TickCharge(DeltaTime);
    }

    // Update scent tracking
    UpdateScentTracking(DeltaTime);

    // Multi-sense prey detection (every tick — T-Rex is always alert)
    AActor* DetectedPrey = nullptr;
    if (DetectPreyMultiSense(DetectedPrey))
    {
        bHasActiveTarget = true;
        CurrentPrey = DetectedPrey;

        // If we can charge and prey is at medium range, initiate charge
        if (CanCharge())
        {
            float DistToPrey = FVector::Dist(GetActorLocation(), DetectedPrey->GetActorLocation());
            if (DistToPrey > AttackRange && DistToPrey < DetectionRadius * 0.7f)
            {
                InitiateCharge(DetectedPrey->GetActorLocation());
            }
        }
    }
    else
    {
        bHasActiveTarget = false;
        // Don't null CurrentPrey immediately — maintain last known position for scent tracking
    }
}

// ─── Combat — Charge ──────────────────────────────────────────────────────────

void ATRexDinosaur::InitiateCharge(FVector TargetLocation)
{
    if (!CanCharge()) return;
    if (ChargeState != ECore_TRexChargeState::Idle) return;

    ChargeTargetLocation = TargetLocation;
    TimeSinceLastCharge = 0.0f;

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur [%s] Initiating charge toward (%.0f, %.0f, %.0f)"),
        *GetName(),
        ChargeTargetLocation.X, ChargeTargetLocation.Y, ChargeTargetLocation.Z);

    BeginWindup();
}

bool ATRexDinosaur::CanCharge() const
{
    return TimeSinceLastCharge >= CombatStats.ChargeCooldownSeconds
        && ChargeState == ECore_TRexChargeState::Idle
        && !bIsDead;
}

void ATRexDinosaur::BeginWindup()
{
    ChargeState = ECore_TRexChargeState::Winding;
    ChargePhaseTimer = CombatStats.ChargeWindupSeconds;

    // Slow down during windup (T-Rex gathering momentum)
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed * 0.3f;
    }
}

void ATRexDinosaur::BeginChargePhase()
{
    ChargeState = ECore_TRexChargeState::Charging;
    ChargePhaseTimer = CombatStats.ChargeDurationSeconds;

    // Full charge speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed * CombatStats.ChargeSpeedMultiplier;
    }

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur [%s] CHARGING — speed: %.0f cm/s"),
        *GetName(), BaseMoveSpeed * CombatStats.ChargeSpeedMultiplier);
}

void ATRexDinosaur::BeginRecovery()
{
    ChargeState = ECore_TRexChargeState::Recovering;
    ChargePhaseTimer = 2.0f; // 2 second recovery stumble

    // Reduced speed during recovery
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed * 0.5f;
    }
}

void ATRexDinosaur::EndCharge()
{
    ChargeState = ECore_TRexChargeState::Idle;
    ChargePhaseTimer = 0.0f;

    // Restore normal speed
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
    }
}

void ATRexDinosaur::TickCharge(float DeltaTime)
{
    ChargePhaseTimer -= DeltaTime;

    switch (ChargeState)
    {
    case ECore_TRexChargeState::Winding:
        if (ChargePhaseTimer <= 0.0f)
        {
            BeginChargePhase();
        }
        break;

    case ECore_TRexChargeState::Charging:
        // During charge, perform stomp damage check every frame
        PerformStomp();

        if (ChargePhaseTimer <= 0.0f)
        {
            BeginRecovery();
        }
        break;

    case ECore_TRexChargeState::Recovering:
        if (ChargePhaseTimer <= 0.0f)
        {
            EndCharge();
        }
        break;

    default:
        break;
    }
}

// ─── Combat — Stomp ───────────────────────────────────────────────────────────

void ATRexDinosaur::PerformStomp()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Sphere overlap for stomp radius
    TArray<FOverlapResult> Overlaps;
    FCollisionShape StompSphere = FCollisionShape::MakeSphere(CombatStats.StompRadiusCm);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        StompSphere,
        QueryParams
    );

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* HitActor = Overlap.GetActor();
        if (!HitActor || HitActor == this) continue;

        // Apply stomp damage
        UGameplayStatics::ApplyDamage(
            HitActor,
            CombatStats.StompDamage,
            GetController(),
            this,
            UDamageType::StaticClass()
        );

        // Knockback — push actor away from T-Rex
        FVector KnockbackDir = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        ACharacter* HitChar = Cast<ACharacter>(HitActor);
        if (HitChar && HitChar->GetCharacterMovement())
        {
            HitChar->GetCharacterMovement()->AddImpulse(KnockbackDir * 1200.0f, true);
        }
    }
}

// ─── Combat — Bite ────────────────────────────────────────────────────────────

void ATRexDinosaur::PerformBite()
{
    if (!CurrentPrey) return;

    float DistToPrey = FVector::Dist(GetActorLocation(), CurrentPrey->GetActorLocation());
    if (DistToPrey > AttackRange) return;

    UGameplayStatics::ApplyDamage(
        CurrentPrey,
        CombatStats.BiteDamage,
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur [%s] BITE — dealt %.0f damage to %s"),
        *GetName(), CombatStats.BiteDamage, *CurrentPrey->GetName());
}

// ─── Combat — Territorial Roar ────────────────────────────────────────────────

void ATRexDinosaur::TerritorialRoar()
{
    // Call base roar (handles cooldown and behavior state)
    Roar();

    // Additional T-Rex effect: intimidate all prey within 1500cm
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<FOverlapResult> Overlaps;
    FCollisionShape RoarSphere = FCollisionShape::MakeSphere(1500.0f);
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    World->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        RoarSphere,
        QueryParams
    );

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* NearbyActor = Overlap.GetActor();
        if (!NearbyActor || NearbyActor == this) continue;

        // Trigger flee response on other dinosaurs (non-T-Rex)
        ADinosaurBase* NearbyDino = Cast<ADinosaurBase>(NearbyActor);
        if (NearbyDino && NearbyDino->Species != EDinosaurSpecies::TyrannosaurusRex)
        {
            NearbyDino->UpdateBehaviorState(EDinosaurBehavior::Fleeing);
            UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur roar triggered flee on: %s"), *NearbyDino->GetName());
        }
    }
}

// ─── Senses — Multi-Modal Detection ──────────────────────────────────────────

bool ATRexDinosaur::DetectPreyMultiSense(AActor*& OutDetectedPrey)
{
    // Priority order: Scent > Vibration > Visual (T-Rex biology)
    if (CheckScentDetection(OutDetectedPrey))
    {
        ActiveSenseMode = ECore_TRexSenseMode::Scent;
        return true;
    }
    if (CheckVibrationDetection(OutDetectedPrey))
    {
        ActiveSenseMode = ECore_TRexSenseMode::Vibration;
        return true;
    }
    if (CheckVisualDetection(OutDetectedPrey))
    {
        ActiveSenseMode = ECore_TRexSenseMode::Visual;
        return true;
    }

    return false;
}

bool ATRexDinosaur::CheckVisualDetection(AActor*& OutPrey) const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    // T-Rex visual detection is movement-based — only detects moving actors
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return false;

    float Dist = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist > SensoryStats.VisualRadiusCm) return false;

    // Check if player is moving (velocity threshold)
    float PlayerSpeed = PlayerPawn->GetVelocity().Size();
    if (PlayerSpeed < 50.0f) return false; // Stationary player not detected visually

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams TraceParams;
    TraceParams.AddIgnoredActor(this);
    TraceParams.AddIgnoredActor(PlayerPawn);

    bool bBlocked = World->LineTraceSingleByChannel(
        HitResult,
        GetActorLocation() + FVector(0, 0, 150.0f), // Eye height
        PlayerPawn->GetActorLocation(),
        ECollisionChannel::ECC_Visibility,
        TraceParams
    );

    if (!bBlocked)
    {
        OutPrey = PlayerPawn;
        return true;
    }

    return false;
}

bool ATRexDinosaur::CheckVibrationDetection(AActor*& OutPrey) const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return false;

    float Dist = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist > SensoryStats.VibrationRadiusCm) return false;

    // Vibration detection: player running on ground triggers this
    float PlayerSpeed = PlayerPawn->GetVelocity().Size();
    if (PlayerSpeed < 200.0f) return false; // Only running triggers vibration

    // Ground contact check — only works if player is on the ground
    ACharacter* PlayerChar = Cast<ACharacter>(PlayerPawn);
    if (PlayerChar && PlayerChar->GetCharacterMovement())
    {
        if (!PlayerChar->GetCharacterMovement()->IsMovingOnGround()) return false;
    }

    OutPrey = PlayerPawn;
    return true;
}

bool ATRexDinosaur::CheckScentDetection(AActor*& OutPrey) const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return false;

    float Dist = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
    if (Dist > SensoryStats.ScentRadiusCm) return false;

    // Scent detection: always active within range (wind direction would reduce this in full sim)
    OutPrey = PlayerPawn;
    return true;
}

void ATRexDinosaur::UpdateScentTracking(float DeltaTime)
{
    ScentTrackingTimer += DeltaTime;

    // Update scent position every 0.5 seconds
    if (ScentTrackingTimer < 0.5f) return;
    ScentTrackingTimer = 0.0f;

    if (CurrentPrey)
    {
        SensoryStats.LastScentPosition = CurrentPrey->GetActorLocation();
        SensoryStats.bTrackingScent = true;
    }
    else if (SensoryStats.bTrackingScent)
    {
        // Scent fades — stop tracking after losing prey for a while
        float DistToLastScent = FVector::Dist(GetActorLocation(), SensoryStats.LastScentPosition);
        if (DistToLastScent < 200.0f)
        {
            // Reached last scent position — prey has moved on
            SensoryStats.bTrackingScent = false;
            UE_LOG(LogTemp, Log, TEXT("ATRexDinosaur [%s] lost scent trail"), *GetName());
        }
    }
}
