// TRexCharacter.cpp
// Performance Optimizer #04 — Agent #04
// T-Rex apex predator implementation

#include "TRexCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

ATRexCharacter::ATRexCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // T-Rex physical scale — large apex predator
    GetCapsuleComponent()->InitCapsuleSize(80.0f, 200.0f);

    // Movement tuning — heavy but fast in short bursts
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 600.0f;          // ~22 km/h patrol speed
        MoveComp->MaxAcceleration = 800.0f;
        MoveComp->BrakingDecelerationWalking = 600.0f;
        MoveComp->RotationRate = FRotator(0.0f, 180.0f, 0.0f);
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->Mass = 8000.0f;                 // 8 tonnes
        MoveComp->GravityScale = 1.2f;            // Heavier gravity feel
    }

    // T-Rex specific defaults
    CurrentPhase = ETRexPhase::Idle;
    ChargeData.ChargeSpeed = 1400.0f;
    ChargeData.ChargeDuration = 3.0f;
    ChargeData.ChargeImpactRadius = 150.0f;
    ChargeData.ChargeImpactForce = 5000.0f;
    ChargeData.bIsCharging = false;
    ChargeData.ChargeTarget = nullptr;

    RoarData.RoarRadius = 2000.0f;
    RoarData.RoarFearMultiplier = 2.5f;
    RoarData.RoarCooldown = 15.0f;
    RoarData.bIsRoaring = false;
    RoarData.LastRoarTime = -999.0f;

    TerritoryRadius = 5000.0f;
    TerritoryCenter = FVector::ZeroVector;
    bTerritoryInitialized = false;

    // Override base class survival stats for apex predator
    MaxHealth = 2000.0f;
    CurrentHealth = 2000.0f;
    AttackDamage = 250.0f;
    DetectionRange = 3000.0f;
    AttackRange = 300.0f;
}

void ATRexCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to spawn location
    if (!bTerritoryInitialized)
    {
        TerritoryCenter = GetActorLocation();
        bTerritoryInitialized = true;
    }

    // Start in idle phase
    SetCombatPhase(ETRexPhase::Idle);
}

void ATRexCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update charge state
    if (ChargeData.bIsCharging)
    {
        UpdateCharge(DeltaTime);
    }

    // Evaluate combat behaviour every 0.5s (performance: not every frame)
    static float BehaviourTimer = 0.0f;
    BehaviourTimer += DeltaTime;
    if (BehaviourTimer >= 0.5f)
    {
        BehaviourTimer = 0.0f;
        EvaluateCombatBehaviour();
    }
}

void ATRexCharacter::BeginCharge(AActor* Target)
{
    if (!Target || ChargeData.bIsCharging)
    {
        return;
    }

    ChargeData.bIsCharging = true;
    ChargeData.ChargeTarget = Target;
    ChargeData.ChargeStartTime = GetWorld()->GetTimeSeconds();

    // Boost movement speed during charge
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = ChargeData.ChargeSpeed;
    }

    SetCombatPhase(ETRexPhase::Charging);
    OnChargeBegin.Broadcast(Target);
}

void ATRexCharacter::UpdateCharge(float DeltaTime)
{
    if (!ChargeData.ChargeTarget)
    {
        EndCharge();
        return;
    }

    float ElapsedTime = GetWorld()->GetTimeSeconds() - ChargeData.ChargeStartTime;
    if (ElapsedTime >= ChargeData.ChargeDuration)
    {
        EndCharge();
        return;
    }

    // Check for impact
    float DistToTarget = FVector::Dist(GetActorLocation(), ChargeData.ChargeTarget->GetActorLocation());
    if (DistToTarget <= ChargeData.ChargeImpactRadius)
    {
        OnChargeImpact(ChargeData.ChargeTarget);
        EndCharge();
    }
}

void ATRexCharacter::EndCharge()
{
    ChargeData.bIsCharging = false;
    ChargeData.ChargeTarget = nullptr;

    // Restore normal speed
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (MoveComp)
    {
        MoveComp->MaxWalkSpeed = 600.0f;
    }

    if (CurrentPhase == ETRexPhase::Charging)
    {
        SetCombatPhase(ETRexPhase::Attacking);
    }
}

void ATRexCharacter::OnChargeImpact(AActor* ImpactTarget)
{
    if (!ImpactTarget)
    {
        return;
    }

    // Apply knockback
    ApplyKnockback(ImpactTarget, ChargeData.ChargeImpactForce);

    // Apply damage
    UGameplayStatics::ApplyDamage(
        ImpactTarget,
        AttackDamage * 1.5f,  // Charge deals 150% damage
        GetController(),
        this,
        UDamageType::StaticClass()
    );

    OnChargeImpactEvent.Broadcast(ImpactTarget);
}

void ATRexCharacter::PerformRoar()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (!CanRoar())
    {
        return;
    }

    RoarData.bIsRoaring = true;
    RoarData.LastRoarTime = CurrentTime;

    // Find all actors in roar radius and apply fear
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), NearbyActors);

    int32 FrightenedCount = 0;
    for (AActor* NearbyActor : NearbyActors)
    {
        if (!NearbyActor || NearbyActor == this)
        {
            continue;
        }

        float Dist = FVector::Dist(GetActorLocation(), NearbyActor->GetActorLocation());
        if (Dist <= RoarData.RoarRadius)
        {
            // Apply fear to DinosaurBase subclasses
            ADinosaurBase* DinoActor = Cast<ADinosaurBase>(NearbyActor);
            if (DinoActor && DinoActor != this)
            {
                DinoActor->FearLevel = FMath::Min(DinoActor->FearLevel + RoarData.RoarFearMultiplier * 20.0f, 100.0f);
                FrightenedCount++;
            }
        }
    }

    OnRoarPerformed.Broadcast(RoarData.RoarRadius);

    // Roar animation ends after 2s
    FTimerHandle RoarTimer;
    GetWorld()->GetTimerManager().SetTimer(RoarTimer, [this]()
    {
        RoarData.bIsRoaring = false;
    }, 2.0f, false);
}

bool ATRexCharacter::IsCharging() const
{
    return ChargeData.bIsCharging;
}

bool ATRexCharacter::CanRoar() const
{
    if (RoarData.bIsRoaring)
    {
        return false;
    }
    float CurrentTime = GetWorld()->GetTimeSeconds();
    return (CurrentTime - RoarData.LastRoarTime) >= RoarData.RoarCooldown;
}

void ATRexCharacter::SetTerritoryCenter(FVector NewCenter)
{
    TerritoryCenter = NewCenter;
    bTerritoryInitialized = true;
}

bool ATRexCharacter::IsInTerritory(FVector Location) const
{
    return FVector::Dist(Location, TerritoryCenter) <= TerritoryRadius;
}

void ATRexCharacter::SetCombatPhase(ETRexPhase NewPhase)
{
    if (CurrentPhase == NewPhase)
    {
        return;
    }

    ETRexPhase OldPhase = CurrentPhase;
    CurrentPhase = NewPhase;
    OnPhaseChanged.Broadcast(OldPhase, NewPhase);

    // Apply phase-specific movement modifiers
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    switch (NewPhase)
    {
        case ETRexPhase::Idle:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        case ETRexPhase::Patrolling:
            MoveComp->MaxWalkSpeed = 300.0f;
            break;
        case ETRexPhase::Hunting:
            MoveComp->MaxWalkSpeed = 700.0f;
            break;
        case ETRexPhase::Attacking:
            MoveComp->MaxWalkSpeed = 800.0f;
            break;
        case ETRexPhase::Charging:
            MoveComp->MaxWalkSpeed = ChargeData.ChargeSpeed;
            break;
        case ETRexPhase::Roaring:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
        case ETRexPhase::Retreating:
            MoveComp->MaxWalkSpeed = 500.0f;
            break;
        case ETRexPhase::Feeding:
            MoveComp->MaxWalkSpeed = 0.0f;
            break;
    }
}

void ATRexCharacter::EvaluateCombatBehaviour()
{
    // Health-based phase transitions
    float HealthPct = CurrentHealth / MaxHealth;

    if (HealthPct < 0.2f && CurrentPhase != ETRexPhase::Retreating)
    {
        // Critical health — retreat
        SetCombatPhase(ETRexPhase::Retreating);
        return;
    }

    // Threat detection — use base class sphere check
    AActor* Threat = DetectNearestThreat();
    if (Threat)
    {
        float ThreatDist = FVector::Dist(GetActorLocation(), Threat->GetActorLocation());

        if (ThreatDist <= AttackRange)
        {
            if (CurrentPhase != ETRexPhase::Attacking && CurrentPhase != ETRexPhase::Charging)
            {
                // Close range — attack or charge
                if (CanRoar() && FMath::RandBool())
                {
                    SetCombatPhase(ETRexPhase::Roaring);
                    PerformRoar();
                }
                else
                {
                    BeginCharge(Threat);
                }
            }
        }
        else if (ThreatDist <= DetectionRange)
        {
            if (CurrentPhase == ETRexPhase::Idle || CurrentPhase == ETRexPhase::Patrolling)
            {
                SetCombatPhase(ETRexPhase::Hunting);
            }
        }
    }
    else
    {
        // No threat — return to patrol if not already
        if (CurrentPhase == ETRexPhase::Hunting || CurrentPhase == ETRexPhase::Attacking)
        {
            SetCombatPhase(ETRexPhase::Patrolling);
        }
    }
}

void ATRexCharacter::ApplyKnockback(AActor* Target, float Force)
{
    if (!Target)
    {
        return;
    }

    ACharacter* TargetChar = Cast<ACharacter>(Target);
    if (TargetChar)
    {
        FVector KnockbackDir = (Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        KnockbackDir.Z = 0.4f;  // Slight upward component
        KnockbackDir.Normalize();

        UCharacterMovementComponent* TargetMove = TargetChar->GetCharacterMovement();
        if (TargetMove)
        {
            TargetMove->AddImpulse(KnockbackDir * Force, true);
        }
    }
}

AActor* ATRexCharacter::DetectNearestThreat()
{
    // Simple sphere overlap — Agent #11 will replace with UAIPerceptionComponent
    TArray<FOverlapResult> Overlaps;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(this);

    bool bHit = GetWorld()->OverlapMultiByChannel(
        Overlaps,
        GetActorLocation(),
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(DetectionRange),
        QueryParams
    );

    AActor* NearestThreat = nullptr;
    float NearestDist = FLT_MAX;

    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OtherActor = Overlap.GetActor();
        if (!OtherActor || OtherActor == this)
        {
            continue;
        }

        // Treat player characters as threats
        ACharacter* OtherChar = Cast<ACharacter>(OtherActor);
        if (OtherChar && !Cast<ADinosaurBase>(OtherChar))
        {
            float Dist = FVector::Dist(GetActorLocation(), OtherActor->GetActorLocation());
            if (Dist < NearestDist)
            {
                NearestDist = Dist;
                NearestThreat = OtherActor;
            }
        }
    }

    return NearestThreat;
}
