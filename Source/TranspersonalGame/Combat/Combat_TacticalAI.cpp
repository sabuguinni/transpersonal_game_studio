#include "Combat_TacticalAI.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

UCombat_TacticalAI::UCombat_TacticalAI()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize tactical parameters
    CurrentTacticalState = ECombat_TacticalState::Patrol;
    TacticalUpdateInterval = 0.5f;
    LastTacticalUpdate = 0.0f;

    // Threat assessment parameters
    ThreatDetectionRange = 2000.0f;
    ThreatMemoryDuration = 10.0f;

    // Pack coordination parameters
    bCanJoinPack = true;
    MaxPackSize = 6;

    // Combat parameters
    AttackRange = 300.0f;
    RetreatThreshold = 0.3f;
    AmbushDistance = 800.0f;
    bPreferAmbushTactics = false;

    // Initialize components
    BlackboardComp = nullptr;
    PerceptionComp = nullptr;
}

void UCombat_TacticalAI::BeginPlay()
{
    Super::BeginPlay();

    // Get AI controller and its components
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
        {
            BlackboardComp = AIController->GetBlackboardComponent();
            PerceptionComp = AIController->GetPerceptionComponent();
        }
    }

    // Initialize pack data
    PackData.PackMembers.Empty();
    PackData.PackLeader = nullptr;
    PackData.RallyPoint = GetOwner()->GetActorLocation();
    PackData.PackState = ECombat_TacticalState::Patrol;
}

void UCombat_TacticalAI::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    UpdateTacticalState(DeltaTime);
}

void UCombat_TacticalAI::UpdateTacticalState(float DeltaTime)
{
    LastTacticalUpdate += DeltaTime;
    
    if (LastTacticalUpdate >= TacticalUpdateInterval)
    {
        UpdateThreatAssessment(DeltaTime);
        UpdatePackCoordination(DeltaTime);
        ExecuteTacticalManeuver();
        CleanupOldThreats(DeltaTime);
        
        LastTacticalUpdate = 0.0f;
    }
}

void UCombat_TacticalAI::AssessThreat(AActor* PotentialThreat)
{
    if (!PotentialThreat || PotentialThreat == GetOwner())
    {
        return;
    }

    // Calculate threat level based on distance, size, and type
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), PotentialThreat->GetActorLocation());
    float ThreatLevel = 1.0f - (Distance / ThreatDetectionRange);

    // Check if this is a player threat
    bool bIsPlayer = PotentialThreat->IsA<APawn>() && Cast<APawn>(PotentialThreat)->IsPlayerControlled();
    if (bIsPlayer)
    {
        ThreatLevel *= 1.5f; // Players are more threatening
    }

    // Update or add threat assessment
    FCombat_ThreatAssessment* ExistingThreat = KnownThreats.FindByPredicate([PotentialThreat](const FCombat_ThreatAssessment& Threat)
    {
        return Threat.ThreatActor == PotentialThreat;
    });

    if (ExistingThreat)
    {
        ExistingThreat->ThreatLevel = ThreatLevel;
        ExistingThreat->ThreatLocation = PotentialThreat->GetActorLocation();
        ExistingThreat->LastSeenTime = GetWorld()->GetTimeSeconds();
    }
    else
    {
        FCombat_ThreatAssessment NewThreat;
        NewThreat.ThreatLevel = ThreatLevel;
        NewThreat.ThreatLocation = PotentialThreat->GetActorLocation();
        NewThreat.ThreatActor = PotentialThreat;
        NewThreat.LastSeenTime = GetWorld()->GetTimeSeconds();
        NewThreat.bIsPlayerThreat = bIsPlayer;
        
        KnownThreats.Add(NewThreat);
    }

    // Update tactical state based on threat level
    if (ThreatLevel > 0.7f && CurrentTacticalState == ECombat_TacticalState::Patrol)
    {
        SetTacticalState(ECombat_TacticalState::Hunt);
    }
    else if (ThreatLevel > 0.9f)
    {
        SetTacticalState(ECombat_TacticalState::Engage);
    }
}

void UCombat_TacticalAI::CoordinateWithPack()
{
    if (!bCanJoinPack || PackData.PackMembers.Num() == 0)
    {
        return;
    }

    // If we're the pack leader, coordinate pack behavior
    if (IsPackLeader())
    {
        // Find the highest priority threat
        FCombat_ThreatAssessment* HighestThreat = nullptr;
        float HighestThreatLevel = 0.0f;

        for (FCombat_ThreatAssessment& Threat : KnownThreats)
        {
            if (Threat.ThreatLevel > HighestThreatLevel)
            {
                HighestThreat = &Threat;
                HighestThreatLevel = Threat.ThreatLevel;
            }
        }

        // Coordinate pack response to threat
        if (HighestThreat && HighestThreatLevel > 0.5f)
        {
            PackData.RallyPoint = HighestThreat->ThreatLocation;
            BroadcastPackSignal(ECombat_TacticalState::Hunt);
        }
    }
    else if (PackData.PackLeader)
    {
        // Follow pack leader's coordination
        if (UCombat_TacticalAI* LeaderAI = PackData.PackLeader->FindComponentByClass<UCombat_TacticalAI>())
        {
            SetTacticalState(LeaderAI->PackData.PackState);
        }
    }
}

void UCombat_TacticalAI::ExecuteTacticalManeuver()
{
    switch (CurrentTacticalState)
    {
        case ECombat_TacticalState::Hunt:
        {
            // Find and pursue the highest priority threat
            FCombat_ThreatAssessment* PrimaryThreat = nullptr;
            float HighestThreatLevel = 0.0f;

            for (FCombat_ThreatAssessment& Threat : KnownThreats)
            {
                if (Threat.ThreatLevel > HighestThreatLevel)
                {
                    PrimaryThreat = &Threat;
                    HighestThreatLevel = Threat.ThreatLevel;
                }
            }

            if (PrimaryThreat && PrimaryThreat->ThreatActor)
            {
                if (bPreferAmbushTactics && FVector::Dist(GetOwner()->GetActorLocation(), PrimaryThreat->ThreatLocation) > AmbushDistance)
                {
                    ExecuteAmbushTactic(PrimaryThreat->ThreatActor);
                }
                else
                {
                    ExecuteFlankingManeuver(PrimaryThreat->ThreatActor);
                }
            }
            break;
        }

        case ECombat_TacticalState::Engage:
        {
            // Direct combat engagement
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsBool(TEXT("ShouldAttack"), true);
                BlackboardComp->SetValueAsFloat(TEXT("AttackRange"), AttackRange);
            }
            break;
        }

        case ECombat_TacticalState::Retreat:
        {
            // Find safe position and retreat
            FVector SafePosition = FindCoverPosition(GetOwner()->GetActorLocation());
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsVector(TEXT("RetreatLocation"), SafePosition);
                BlackboardComp->SetValueAsBool(TEXT("ShouldRetreat"), true);
            }
            break;
        }

        case ECombat_TacticalState::Ambush:
        {
            // Wait in ambush position
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsBool(TEXT("ShouldAmbush"), true);
                BlackboardComp->SetValueAsFloat(TEXT("AmbushRange"), AmbushDistance);
            }
            break;
        }

        default:
            break;
    }
}

bool UCombat_TacticalAI::ShouldEngageTarget(AActor* Target)
{
    if (!Target)
    {
        return false;
    }

    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Target->GetActorLocation());
    
    // Check if target is within attack range
    if (Distance > AttackRange)
    {
        return false;
    }

    // Check if we have tactical advantage (pack support, cover, etc.)
    bool bHasPackSupport = PackData.PackMembers.Num() > 1;
    bool bHasCover = IsPositionSafe(GetOwner()->GetActorLocation());
    
    return bHasPackSupport || bHasCover || Distance < AttackRange * 0.5f;
}

FVector UCombat_TacticalAI::CalculateOptimalPosition(AActor* Target)
{
    if (!Target)
    {
        return GetOwner()->GetActorLocation();
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector CurrentLocation = GetOwner()->GetActorLocation();
    
    // Calculate flanking position
    FVector DirectionToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
    FVector RightVector = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
    
    // Try positions to the right and left of the target
    TArray<FVector> CandidatePositions;
    CandidatePositions.Add(TargetLocation + RightVector * AttackRange * 0.8f);
    CandidatePositions.Add(TargetLocation - RightVector * AttackRange * 0.8f);
    CandidatePositions.Add(TargetLocation + DirectionToTarget * AttackRange * 1.2f);
    
    // Find the safest position
    for (const FVector& Position : CandidatePositions)
    {
        if (IsPositionSafe(Position))
        {
            return Position;
        }
    }
    
    return CurrentLocation;
}

void UCombat_TacticalAI::SetTacticalState(ECombat_TacticalState NewState)
{
    if (CurrentTacticalState != NewState)
    {
        CurrentTacticalState = NewState;
        
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(NewState));
        }
        
        // Log state change for debugging
        UE_LOG(LogTemp, Log, TEXT("Tactical AI State Changed to: %d"), static_cast<int32>(NewState));
    }
}

void UCombat_TacticalAI::JoinPack(APawn* Leader)
{
    if (!Leader || !bCanJoinPack)
    {
        return;
    }

    if (UCombat_TacticalAI* LeaderAI = Leader->FindComponentByClass<UCombat_TacticalAI>())
    {
        if (LeaderAI->PackData.PackMembers.Num() < MaxPackSize)
        {
            LeaderAI->PackData.PackMembers.AddUnique(Cast<APawn>(GetOwner()));
            PackData.PackLeader = Leader;
            PackData.PackMembers = LeaderAI->PackData.PackMembers;
        }
    }
}

void UCombat_TacticalAI::LeavePack()
{
    if (PackData.PackLeader)
    {
        if (UCombat_TacticalAI* LeaderAI = PackData.PackLeader->FindComponentByClass<UCombat_TacticalAI>())
        {
            LeaderAI->PackData.PackMembers.Remove(Cast<APawn>(GetOwner()));
        }
    }
    
    PackData.PackLeader = nullptr;
    PackData.PackMembers.Empty();
}

bool UCombat_TacticalAI::IsPackLeader() const
{
    return PackData.PackLeader == GetOwner();
}

void UCombat_TacticalAI::BroadcastPackSignal(ECombat_TacticalState Signal)
{
    for (APawn* PackMember : PackData.PackMembers)
    {
        if (PackMember && PackMember != GetOwner())
        {
            if (UCombat_TacticalAI* MemberAI = PackMember->FindComponentByClass<UCombat_TacticalAI>())
            {
                MemberAI->SetTacticalState(Signal);
            }
        }
    }
}

void UCombat_TacticalAI::UpdateThreatAssessment(float DeltaTime)
{
    // Update threat levels based on current conditions
    for (FCombat_ThreatAssessment& Threat : KnownThreats)
    {
        if (Threat.ThreatActor && IsValid(Threat.ThreatActor))
        {
            float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Threat.ThreatActor->GetActorLocation());
            Threat.ThreatLevel = FMath::Max(0.0f, 1.0f - (Distance / ThreatDetectionRange));
            Threat.ThreatLocation = Threat.ThreatActor->GetActorLocation();
        }
    }
}

void UCombat_TacticalAI::CleanupOldThreats(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    KnownThreats.RemoveAll([CurrentTime, this](const FCombat_ThreatAssessment& Threat)
    {
        return (CurrentTime - Threat.LastSeenTime) > ThreatMemoryDuration || !IsValid(Threat.ThreatActor);
    });
}

void UCombat_TacticalAI::UpdatePackCoordination(float DeltaTime)
{
    // Remove invalid pack members
    PackData.PackMembers.RemoveAll([](APawn* Member)
    {
        return !IsValid(Member);
    });

    // If pack leader is invalid, try to become leader or leave pack
    if (PackData.PackLeader && !IsValid(PackData.PackLeader))
    {
        if (PackData.PackMembers.Num() > 0)
        {
            PackData.PackLeader = Cast<APawn>(GetOwner());
        }
        else
        {
            LeavePack();
        }
    }
}

FVector UCombat_TacticalAI::FindCoverPosition(FVector FromLocation)
{
    // Simple cover finding - look for positions away from threats
    FVector SafeDirection = FVector::ZeroVector;
    
    for (const FCombat_ThreatAssessment& Threat : KnownThreats)
    {
        if (Threat.ThreatActor)
        {
            FVector AwayFromThreat = (FromLocation - Threat.ThreatLocation).GetSafeNormal();
            SafeDirection += AwayFromThreat * Threat.ThreatLevel;
        }
    }
    
    SafeDirection.Normalize();
    return FromLocation + SafeDirection * 1000.0f;
}

bool UCombat_TacticalAI::IsPositionSafe(FVector Position)
{
    // Check if position is away from known threats
    for (const FCombat_ThreatAssessment& Threat : KnownThreats)
    {
        if (Threat.ThreatActor)
        {
            float Distance = FVector::Dist(Position, Threat.ThreatLocation);
            if (Distance < AttackRange * 1.5f && Threat.ThreatLevel > 0.5f)
            {
                return false;
            }
        }
    }
    
    return true;
}

void UCombat_TacticalAI::ExecuteFlankingManeuver(AActor* Target)
{
    if (!Target || !BlackboardComp)
    {
        return;
    }
    
    FVector OptimalPosition = CalculateOptimalPosition(Target);
    BlackboardComp->SetValueAsVector(TEXT("FlankPosition"), OptimalPosition);
    BlackboardComp->SetValueAsBool(TEXT("ShouldFlank"), true);
}

void UCombat_TacticalAI::ExecuteAmbushTactic(AActor* Target)
{
    if (!Target || !BlackboardComp)
    {
        return;
    }
    
    SetTacticalState(ECombat_TacticalState::Ambush);
    BlackboardComp->SetValueAsVector(TEXT("AmbushPosition"), GetOwner()->GetActorLocation());
    BlackboardComp->SetValueAsObject(TEXT("AmbushTarget"), Target);
}