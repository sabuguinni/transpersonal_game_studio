#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure sight
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Configure hearing
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    // Add senses to perception component
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Initialize state
    CurrentAIState = ECombat_AIState::Idle;
    CurrentThreatLevel = ECombat_ThreatLevel::None;
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    TimeSinceLastTargetSeen = 0.0f;
    bIsPackLeader = false;
    PackLeader = nullptr;
    
    // Initialize tactical data with defaults
    TacticalData = FCombat_TacticalData();
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
    }
    
    // Start behavior tree if available
    if (BehaviorTree && BlackboardAsset)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateAIState(DeltaTime);
    UpdateThreatAssessment();
    UpdatePackCoordination();
    
    // Update time since last target seen
    if (CurrentTarget)
    {
        TimeSinceLastTargetSeen += DeltaTime;
    }
}

void ACombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    // Initialize blackboard if available
    if (BlackboardAsset && GetBlackboardComponent())
    {
        GetBlackboardComponent()->InitializeBlackboard(*BlackboardAsset);
    }
}

void ACombatAIController::SetAIState(ECombat_AIState NewState)
{
    if (CurrentAIState != NewState)
    {
        ECombat_AIState PreviousState = CurrentAIState;
        CurrentAIState = NewState;
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(NewState));
        }
        
        // Handle state transitions
        switch (NewState)
        {
            case ECombat_AIState::Engaging:
                if (bIsPackLeader)
                {
                    CoordinatePackAttack();
                }
                break;
            case ECombat_AIState::Retreating:
                BroadcastToPackMembers(TEXT("Retreating"));
                break;
            case ECombat_AIState::Hunting:
                BroadcastToPackMembers(TEXT("Target acquired"));
                break;
        }
    }
}

void ACombatAIController::SetThreatLevel(ECombat_ThreatLevel NewThreatLevel)
{
    CurrentThreatLevel = NewThreatLevel;
    
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsEnum(TEXT("ThreatLevel"), static_cast<uint8>(NewThreatLevel));
    }
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    TimeSinceLastTargetSeen = 0.0f;
    
    if (NewTarget)
    {
        LastKnownTargetLocation = NewTarget->GetActorLocation();
        
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), NewTarget);
            GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
        }
    }
    else
    {
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->ClearValue(TEXT("TargetActor"));
        }
    }
}

void ACombatAIController::UpdateLastKnownTargetLocation(const FVector& Location)
{
    LastKnownTargetLocation = Location;
    TimeSinceLastTargetSeen = 0.0f;
    
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsVector(TEXT("TargetLocation"), Location);
    }
}

bool ACombatAIController::ShouldEngageTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    // Check if target is within engagement range
    if (DistanceToTarget > TacticalData.PreferredEngagementRange * 1.5f)
    {
        return false;
    }
    
    // Check health status
    if (ShouldRetreat())
    {
        return false;
    }
    
    // Check if we have line of sight
    if (!HasClearLineOfSight(CurrentTarget->GetActorLocation()))
    {
        return false;
    }
    
    return true;
}

bool ACombatAIController::ShouldFlankTarget() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return false;
    }
    
    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    // Only flank if we're at medium range
    return DistanceToTarget > TacticalData.MinimumEngagementRange && 
           DistanceToTarget < TacticalData.FlankingDistance &&
           TacticalData.bCanCoordinate;
}

bool ACombatAIController::ShouldRetreat() const
{
    if (!GetPawn())
    {
        return false;
    }
    
    // Check health-based retreat
    if (ACharacter* Character = Cast<ACharacter>(GetPawn()))
    {
        // Simplified health check - in real implementation would use actual health component
        float HealthRatio = 1.0f; // Placeholder
        return HealthRatio < TacticalData.RetreatHealthThreshold;
    }
    
    // Check if overwhelmed
    if (CurrentThreatLevel == ECombat_ThreatLevel::Critical)
    {
        return true;
    }
    
    return false;
}

FVector ACombatAIController::GetOptimalAttackPosition() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    }
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Position at preferred engagement range
    FVector OptimalPosition = TargetLocation - (DirectionToTarget * TacticalData.PreferredEngagementRange);
    
    // Check if position is navigable
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(OptimalPosition, NavLocation, FVector(500.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return OptimalPosition;
}

FVector ACombatAIController::GetFlankingPosition() const
{
    if (!CurrentTarget || !GetPawn())
    {
        return GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    }
    
    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate perpendicular direction for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector).GetSafeNormal();
    
    // Randomly choose left or right flank
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    FVector FlankPosition = TargetLocation + (FlankDirection * TacticalData.FlankingDistance);
    
    // Check if position is navigable
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld()))
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(FlankPosition, NavLocation, FVector(500.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return FlankPosition;
}

void ACombatAIController::RegisterPackMember(ACombatAIController* NewMember)
{
    if (NewMember && !PackMembers.Contains(NewMember))
    {
        PackMembers.Add(NewMember);
        NewMember->PackLeader = this;
        
        // If this is the first member, become pack leader
        if (!bIsPackLeader && PackMembers.Num() == 1)
        {
            SetAsPackLeader();
        }
    }
}

void ACombatAIController::RemovePackMember(ACombatAIController* Member)
{
    if (Member)
    {
        PackMembers.Remove(Member);
        Member->PackLeader = nullptr;
        
        // If no members left, stop being pack leader
        if (PackMembers.Num() == 0)
        {
            bIsPackLeader = false;
        }
    }
}

void ACombatAIController::SetAsPackLeader()
{
    bIsPackLeader = true;
    
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsBool(TEXT("IsPackLeader"), true);
    }
}

void ACombatAIController::CoordinatePackAttack()
{
    if (!bIsPackLeader || !CurrentTarget)
    {
        return;
    }
    
    // Assign roles to pack members
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        ACombatAIController* Member = PackMembers[i];
        if (Member && Member->GetPawn())
        {
            if (i % 2 == 0)
            {
                // Even indexed members flank left
                Member->SetAIState(ECombat_AIState::Flanking);
            }
            else
            {
                // Odd indexed members flank right
                Member->SetAIState(ECombat_AIState::Flanking);
            }
            
            Member->SetTarget(CurrentTarget);
        }
    }
    
    BroadcastToPackMembers(TEXT("Coordinated attack initiated"));
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is a potential target
            FAIStimulus Stimulus;
            if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    // New target detected
                    if (!CurrentTarget || CalculateTargetThreat(Actor) > CalculateTargetThreat(CurrentTarget))
                    {
                        SetTarget(Actor);
                        SetAIState(ECombat_AIState::Investigating);
                    }
                }
            }
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (Actor == CurrentTarget)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            UpdateLastKnownTargetLocation(Stimulus.StimulusLocation);
            
            if (CurrentAIState == ECombat_AIState::Investigating)
            {
                SetAIState(ECombat_AIState::Hunting);
            }
        }
        else
        {
            // Lost sight of target
            if (CurrentAIState == ECombat_AIState::Engaging)
            {
                SetAIState(ECombat_AIState::Investigating);
            }
        }
    }
}

void ACombatAIController::UpdateAIState(float DeltaTime)
{
    switch (CurrentAIState)
    {
        case ECombat_AIState::Idle:
            if (CurrentTarget)
            {
                SetAIState(ECombat_AIState::Investigating);
            }
            break;
            
        case ECombat_AIState::Investigating:
            if (CurrentTarget && TimeSinceLastTargetSeen < 2.0f)
            {
                SetAIState(ECombat_AIState::Hunting);
            }
            else if (TimeSinceLastTargetSeen > 10.0f)
            {
                SetTarget(nullptr);
                SetAIState(ECombat_AIState::Idle);
            }
            break;
            
        case ECombat_AIState::Hunting:
            if (ShouldEngageTarget())
            {
                SetAIState(ECombat_AIState::Engaging);
            }
            else if (ShouldRetreat())
            {
                SetAIState(ECombat_AIState::Retreating);
            }
            break;
            
        case ECombat_AIState::Engaging:
            if (ShouldRetreat())
            {
                SetAIState(ECombat_AIState::Retreating);
            }
            else if (ShouldFlankTarget())
            {
                SetAIState(ECombat_AIState::Flanking);
            }
            break;
            
        case ECombat_AIState::Retreating:
            // Check if safe to re-engage
            if (!ShouldRetreat() && CurrentTarget)
            {
                SetAIState(ECombat_AIState::Hunting);
            }
            break;
    }
}

void ACombatAIController::UpdateThreatAssessment()
{
    if (!CurrentTarget)
    {
        SetThreatLevel(ECombat_ThreatLevel::None);
        return;
    }
    
    float ThreatScore = CalculateTargetThreat(CurrentTarget);
    
    if (ThreatScore > 0.8f)
    {
        SetThreatLevel(ECombat_ThreatLevel::Critical);
    }
    else if (ThreatScore > 0.6f)
    {
        SetThreatLevel(ECombat_ThreatLevel::High);
    }
    else if (ThreatScore > 0.4f)
    {
        SetThreatLevel(ECombat_ThreatLevel::Medium);
    }
    else if (ThreatScore > 0.2f)
    {
        SetThreatLevel(ECombat_ThreatLevel::Low);
    }
    else
    {
        SetThreatLevel(ECombat_ThreatLevel::None);
    }
}

void ACombatAIController::UpdatePackCoordination()
{
    if (bIsPackLeader)
    {
        // Clean up invalid pack members
        PackMembers.RemoveAll([](ACombatAIController* Member)
        {
            return !IsValid(Member) || !IsValid(Member->GetPawn());
        });
    }
}

float ACombatAIController::CalculateTargetThreat(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return 0.0f;
    }
    
    float ThreatScore = 0.0f;
    
    // Distance factor (closer = more threatening)
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    float DistanceFactor = FMath::Clamp(1.0f - (Distance / 2000.0f), 0.0f, 1.0f);
    ThreatScore += DistanceFactor * 0.4f;
    
    // Line of sight factor
    if (HasClearLineOfSight(Target->GetActorLocation()))
    {
        ThreatScore += 0.3f;
    }
    
    // Movement factor (moving targets are more threatening)
    if (ACharacter* Character = Cast<ACharacter>(Target))
    {
        float Speed = Character->GetVelocity().Size();
        if (Speed > 100.0f)
        {
            ThreatScore += 0.3f;
        }
    }
    
    return FMath::Clamp(ThreatScore, 0.0f, 1.0f);
}

bool ACombatAIController::IsPositionSafe(const FVector& Position) const
{
    // Check for nearby threats
    if (GetWorld())
    {
        TArray<AActor*> NearbyActors;
        UKismetSystemLibrary::SphereOverlapActors(
            GetWorld(),
            Position,
            500.0f,
            TArray<TEnumAsByte<EObjectTypeQuery>>(),
            ACharacter::StaticClass(),
            TArray<AActor*>(),
            NearbyActors
        );
        
        // Position is unsafe if there are multiple threats nearby
        return NearbyActors.Num() < 2;
    }
    
    return true;
}

bool ACombatAIController::HasClearLineOfSight(const FVector& TargetLocation) const
{
    if (!GetPawn())
    {
        return false;
    }
    
    FVector StartLocation = GetPawn()->GetActorLocation();
    FVector EndLocation = TargetLocation;
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    QueryParams.bTraceComplex = false;
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECC_Visibility,
        QueryParams
    );
    
    return !bHit;
}

void ACombatAIController::BroadcastToPackMembers(const FString& Message)
{
    if (bIsPackLeader)
    {
        for (ACombatAIController* Member : PackMembers)
        {
            if (Member)
            {
                Member->ReceivePackCommand(Message, this);
            }
        }
    }
}

void ACombatAIController::ReceivePackCommand(const FString& Command, ACombatAIController* Sender)
{
    if (Sender == PackLeader)
    {
        // Process command from pack leader
        if (Command == TEXT("Coordinated attack initiated"))
        {
            // Respond to coordinated attack
        }
        else if (Command == TEXT("Retreating"))
        {
            // Consider retreating as well
            if (ShouldRetreat())
            {
                SetAIState(ECombat_AIState::Retreating);
            }
        }
    }
}