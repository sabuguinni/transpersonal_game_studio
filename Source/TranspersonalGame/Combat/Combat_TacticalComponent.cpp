#include "Combat_TacticalComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalComponent::UCombat_TacticalComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update for performance
}

void UCombat_TacticalComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize tactical state
    TacticalState.CurrentStance = ECombatStance::Neutral;
    TacticalState.CombatReadiness = 0.0f;
    TacticalState.bInCombat = false;
    TacticalState.LastCombatTime = -CombatCooldownTime;
    
    UE_LOG(LogTemp, Log, TEXT("Combat Tactical Component initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_TacticalComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateTacticalState(DeltaTime);
}

void UCombat_TacticalComponent::UpdateTacticalState(float DeltaTime)
{
    if (!GetOwner())
        return;
    
    // Update threat assessments
    UpdateThreatAssessments(DeltaTime);
    
    // Clean up invalid threats
    CleanupInvalidThreats();
    
    // Update combat readiness based on nearby threats
    float MaxThreatLevel = 0.0f;
    for (const auto& ThreatPair : ThreatMap)
    {
        if (ThreatPair.Value.ThreatLevel > MaxThreatLevel)
        {
            MaxThreatLevel = ThreatPair.Value.ThreatLevel;
        }
    }
    
    // Smooth combat readiness transition
    float TargetReadiness = FMath::Clamp(MaxThreatLevel, 0.0f, 1.0f);
    TacticalState.CombatReadiness = FMath::FInterpTo(TacticalState.CombatReadiness, TargetReadiness, DeltaTime, 2.0f);
    
    // Auto-engage if readiness exceeds threshold
    if (TacticalState.CombatReadiness > AggressionThreshold && !TacticalState.bInCombat)
    {
        // Find highest threat target
        AActor* HighestThreat = nullptr;
        float HighestThreatLevel = 0.0f;
        
        for (const auto& ThreatPair : ThreatMap)
        {
            if (ThreatPair.Value.ThreatLevel > HighestThreatLevel && ThreatPair.Value.bIsHostile)
            {
                HighestThreat = ThreatPair.Key;
                HighestThreatLevel = ThreatPair.Value.ThreatLevel;
            }
        }
        
        if (HighestThreat)
        {
            EnterCombatMode(HighestThreat);
        }
    }
    
    // Auto-exit combat if no threats remain
    if (TacticalState.bInCombat && TacticalState.CombatReadiness < 0.2f)
    {
        ExitCombatMode();
    }
}

void UCombat_TacticalComponent::AssessThreat(AActor* PotentialThreat)
{
    if (!PotentialThreat || !GetOwner())
        return;
    
    FCombat_ThreatAssessment Assessment;
    Assessment.Distance = FVector::Dist(GetOwner()->GetActorLocation(), PotentialThreat->GetActorLocation());
    Assessment.LastKnownPosition = PotentialThreat->GetActorLocation();
    Assessment.ThreatLevel = CalculateThreatLevel(PotentialThreat);
    Assessment.bIsHostile = Assessment.ThreatLevel > 0.3f; // Threshold for hostility
    Assessment.AggressionLevel = FMath::Clamp(1.0f - (Assessment.Distance / ThreatDetectionRadius), 0.0f, 1.0f);
    
    ThreatMap.Add(PotentialThreat, Assessment);
    
    UE_LOG(LogTemp, Log, TEXT("Threat assessed: %s - Level: %.2f, Distance: %.0f"), 
           *PotentialThreat->GetName(), Assessment.ThreatLevel, Assessment.Distance);
}

void UCombat_TacticalComponent::EnterCombatMode(AActor* Target)
{
    if (!Target || TacticalState.bInCombat)
        return;
    
    TacticalState.bInCombat = true;
    TacticalState.PrimaryTarget = Target;
    TacticalState.CurrentStance = ECombatStance::Aggressive;
    
    // Add to hostile targets if not already present
    if (!TacticalState.HostileTargets.Contains(Target))
    {
        TacticalState.HostileTargets.Add(Target);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("%s entering combat with %s"), 
           *GetOwner()->GetName(), *Target->GetName());
}

void UCombat_TacticalComponent::ExitCombatMode()
{
    if (!TacticalState.bInCombat)
        return;
    
    TacticalState.bInCombat = false;
    TacticalState.PrimaryTarget = nullptr;
    TacticalState.CurrentStance = ECombatStance::Neutral;
    TacticalState.LastCombatTime = GetWorld()->GetTimeSeconds();
    TacticalState.HostileTargets.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("%s exiting combat mode"), *GetOwner()->GetName());
}

FVector UCombat_TacticalComponent::GetOptimalPosition(AActor* Target, float PreferredDistance)
{
    if (!Target || !GetOwner())
        return GetOwner()->GetActorLocation();
    
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector TargetLocation = Target->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    
    // Calculate optimal position based on combat stance
    FVector OptimalPosition;
    
    switch (TacticalState.CurrentStance)
    {
        case ECombatStance::Aggressive:
            // Move closer for attack
            OptimalPosition = TargetLocation - (DirectionToTarget * (PreferredDistance * 0.7f));
            break;
            
        case ECombatStance::Defensive:
            // Maintain distance
            OptimalPosition = TargetLocation - (DirectionToTarget * (PreferredDistance * 1.3f));
            break;
            
        case ECombatStance::Flanking:
            // Move to side for flanking
            {
                FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
                OptimalPosition = TargetLocation - (DirectionToTarget * PreferredDistance) + (RightVector * PreferredDistance * 0.5f);
            }
            break;
            
        default:
            OptimalPosition = TargetLocation - (DirectionToTarget * PreferredDistance);
            break;
    }
    
    // Ensure position is safe
    if (!IsPositionSafe(OptimalPosition, Target))
    {
        // Fallback to current position if optimal position is unsafe
        OptimalPosition = OwnerLocation;
    }
    
    return OptimalPosition;
}

bool UCombat_TacticalComponent::ShouldEngageTarget(AActor* Target)
{
    if (!Target || !GetOwner())
        return false;
    
    // Check cooldown
    if (GetWorld()->GetTimeSeconds() - TacticalState.LastCombatTime < CombatCooldownTime)
        return false;
    
    // Check distance
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    if (Distance > CombatEngagementRadius)
        return false;
    
    // Check threat level
    if (ThreatMap.Contains(Target))
    {
        const FCombat_ThreatAssessment& Assessment = ThreatMap[Target];
        return Assessment.bIsHostile && Assessment.ThreatLevel > AggressionThreshold;
    }
    
    return false;
}

void UCombat_TacticalComponent::SetCombatStance(ECombatStance NewStance)
{
    TacticalState.CurrentStance = NewStance;
    UE_LOG(LogTemp, Log, TEXT("%s changed combat stance to %d"), 
           *GetOwner()->GetName(), (int32)NewStance);
}

void UCombat_TacticalComponent::CoordinateWithPack(const TArray<AActor*>& PackMembers)
{
    if (!bUsePackTactics || PackMembers.Num() == 0)
        return;
    
    // Simple pack coordination - spread out around target
    if (TacticalState.PrimaryTarget)
    {
        int32 MyIndex = PackMembers.Find(GetOwner());
        if (MyIndex != INDEX_NONE)
        {
            float AngleStep = 360.0f / PackMembers.Num();
            float MyAngle = MyIndex * AngleStep;
            
            // Calculate flanking position based on pack formation
            FVector FlankingPos = GetFlankingPosition(TacticalState.PrimaryTarget, PackMembers);
            
            UE_LOG(LogTemp, Log, TEXT("Pack coordination: %s taking angle %.1f"), 
                   *GetOwner()->GetName(), MyAngle);
        }
    }
}

FVector UCombat_TacticalComponent::GetFlankingPosition(AActor* Target, const TArray<AActor*>& PackMembers)
{
    if (!Target || !GetOwner())
        return GetOwner()->GetActorLocation();
    
    int32 MyIndex = PackMembers.Find(GetOwner());
    if (MyIndex == INDEX_NONE)
        return GetOwner()->GetActorLocation();
    
    FVector TargetLocation = Target->GetActorLocation();
    float AngleStep = 360.0f / FMath::Max(PackMembers.Num(), 1);
    float MyAngle = MyIndex * AngleStep;
    
    // Convert angle to radians and calculate position
    float AngleRad = FMath::DegreesToRadians(MyAngle);
    FVector Offset = FVector(
        FMath::Cos(AngleRad) * PreferredCombatDistance,
        FMath::Sin(AngleRad) * PreferredCombatDistance,
        0.0f
    );
    
    return TargetLocation + Offset;
}

FCombat_ThreatAssessment UCombat_TacticalComponent::GetThreatAssessment(AActor* Target) const
{
    if (ThreatMap.Contains(Target))
    {
        return ThreatMap[Target];
    }
    
    return FCombat_ThreatAssessment();
}

FVector UCombat_TacticalComponent::CalculateOptimalAttackVector(AActor* Target) const
{
    if (!Target || !GetOwner())
        return FVector::ZeroVector;
    
    FVector TargetLocation = Target->GetActorLocation();
    FVector OwnerLocation = GetOwner()->GetActorLocation();
    
    // Calculate attack vector based on target's movement
    if (APawn* TargetPawn = Cast<APawn>(Target))
    {
        FVector TargetVelocity = TargetPawn->GetVelocity();
        if (!TargetVelocity.IsNearlyZero())
        {
            // Lead the target
            float PredictionTime = 1.0f; // 1 second prediction
            FVector PredictedPosition = TargetLocation + (TargetVelocity * PredictionTime);
            return (PredictedPosition - OwnerLocation).GetSafeNormal();
        }
    }
    
    return (TargetLocation - OwnerLocation).GetSafeNormal();
}

bool UCombat_TacticalComponent::IsPositionSafe(const FVector& Position, AActor* Target) const
{
    if (!GetWorld())
        return false;
    
    // Basic safety check - ensure position is not inside obstacles
    FHitResult HitResult;
    FVector Start = Position + FVector(0, 0, 100);
    FVector End = Position - FVector(0, 0, 100);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_WorldStatic
    );
    
    return bHit; // Position is safe if we hit ground
}

float UCombat_TacticalComponent::CalculateThreatLevel(AActor* Target) const
{
    if (!Target || !GetOwner())
        return 0.0f;
    
    float ThreatLevel = 0.0f;
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    // Distance-based threat (closer = more threatening)
    float DistanceThreat = FMath::Clamp(1.0f - (Distance / ThreatDetectionRadius), 0.0f, 1.0f);
    ThreatLevel += DistanceThreat * 0.4f;
    
    // Size-based threat (larger actors are more threatening)
    if (UPrimitiveComponent* TargetPrimitive = Target->FindComponentByClass<UPrimitiveComponent>())
    {
        FVector BoundsSize = TargetPrimitive->Bounds.BoxExtent;
        float SizeFactor = FMath::Clamp(BoundsSize.Size() / 1000.0f, 0.0f, 1.0f);
        ThreatLevel += SizeFactor * 0.3f;
    }
    
    // Movement-based threat (fast-moving targets are more threatening)
    if (APawn* TargetPawn = Cast<APawn>(Target))
    {
        float Speed = TargetPawn->GetVelocity().Size();
        float SpeedThreat = FMath::Clamp(Speed / 1000.0f, 0.0f, 1.0f);
        ThreatLevel += SpeedThreat * 0.3f;
    }
    
    return FMath::Clamp(ThreatLevel, 0.0f, 1.0f);
}

void UCombat_TacticalComponent::UpdateThreatAssessments(float DeltaTime)
{
    for (auto& ThreatPair : ThreatMap)
    {
        AActor* ThreatActor = ThreatPair.Key;
        FCombat_ThreatAssessment& Assessment = ThreatPair.Value;
        
        if (IsValid(ThreatActor))
        {
            // Update distance and position
            Assessment.Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatActor->GetActorLocation());
            Assessment.LastKnownPosition = ThreatActor->GetActorLocation();
            
            // Recalculate threat level
            Assessment.ThreatLevel = CalculateThreatLevel(ThreatActor);
            Assessment.AggressionLevel = FMath::Clamp(1.0f - (Assessment.Distance / ThreatDetectionRadius), 0.0f, 1.0f);
            
            // Update hostility based on current threat level
            Assessment.bIsHostile = Assessment.ThreatLevel > 0.3f;
        }
    }
}

void UCombat_TacticalComponent::CleanupInvalidThreats()
{
    TArray<AActor*> InvalidThreats;
    
    for (const auto& ThreatPair : ThreatMap)
    {
        AActor* ThreatActor = ThreatPair.Key;
        const FCombat_ThreatAssessment& Assessment = ThreatPair.Value;
        
        // Remove invalid actors or actors too far away
        if (!IsValid(ThreatActor) || Assessment.Distance > ThreatDetectionRadius * 1.5f)
        {
            InvalidThreats.Add(ThreatActor);
        }
    }
    
    for (AActor* InvalidThreat : InvalidThreats)
    {
        ThreatMap.Remove(InvalidThreat);
        TacticalState.HostileTargets.Remove(InvalidThreat);
        
        if (TacticalState.PrimaryTarget == InvalidThreat)
        {
            TacticalState.PrimaryTarget = nullptr;
        }
    }
}