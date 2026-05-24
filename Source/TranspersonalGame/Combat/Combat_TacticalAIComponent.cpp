#include "Combat_TacticalAIComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombat_TacticalAIComponent::UCombat_TacticalAIComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;

    // Initialize tactical parameters
    TacticalUpdateInterval = 0.5f;
    FormationRadius = 300.0f;
    CoordinationRange = 1500.0f;
    StateChangeDelay = 2.0f;

    // Initialize internal state
    LastTacticalUpdate = 0.0f;
    StateTimer = 0.0f;
    bInCombat = false;
    CurrentTarget = nullptr;
    PackLeader = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    LastTargetUpdateTime = 0.0f;

    // Initialize tactical data
    TacticalData = FCombat_TacticalData();
}

void UCombat_TacticalAIComponent::BeginPlay()
{
    Super::BeginPlay();

    // Get AI controller and behavior tree components
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (AAIController* AIController = Cast<AAIController>(OwnerPawn->GetController()))
        {
            BehaviorTreeComponent = AIController->GetBehaviorTreeComponent();
            BlackboardComponent = AIController->GetBlackboardComponent();
        }
    }

    // Initialize blackboard values
    UpdateBlackboardValues();

    UE_LOG(LogTemp, Warning, TEXT("Combat_TacticalAIComponent initialized for %s"), 
           GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UCombat_TacticalAIComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    StateTimer += DeltaTime;

    // Update tactical assessment periodically
    if (CurrentTime - LastTacticalUpdate >= TacticalUpdateInterval)
    {
        UpdateTacticalAssessment();
        LastTacticalUpdate = CurrentTime;
    }

    // Execute current state logic
    ExecuteStateLogic();

    // Process pack coordination
    if (TacticalData.PackSize > 1)
    {
        ProcessPackCoordination();
    }

    // Update blackboard values
    UpdateBlackboardValues();
}

void UCombat_TacticalAIComponent::SetTacticalState(ECombat_TacticalState NewState)
{
    if (TacticalData.CurrentState != NewState && CanChangeState())
    {
        ECombat_TacticalState OldState = TacticalData.CurrentState;
        TacticalData.CurrentState = NewState;
        TacticalData.LastStateChangeTime = GetWorld()->GetTimeSeconds();
        StateTimer = 0.0f;

        UE_LOG(LogTemp, Log, TEXT("%s: Tactical state changed from %d to %d"), 
               *GetOwner()->GetName(), (int32)OldState, (int32)NewState);

        // Broadcast state change to pack members if leader
        if (TacticalData.bIsPackLeader)
        {
            BroadcastTacticalCommand(NewState);
        }
    }
}

ECombat_TacticalState UCombat_TacticalAIComponent::GetTacticalState() const
{
    return TacticalData.CurrentState;
}

void UCombat_TacticalAIComponent::UpdateTacticalAssessment()
{
    if (!GetOwner()) return;

    // Find potential threats/targets
    TArray<AActor*> PotentialTargets;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), PotentialTargets);

    AActor* BestTarget = nullptr;
    float BestThreatScore = 0.0f;
    FVector OwnerLocation = GetOwner()->GetActorLocation();

    for (AActor* Target : PotentialTargets)
    {
        if (Target == GetOwner()) continue;

        float Distance = FVector::Dist(OwnerLocation, Target->GetActorLocation());
        if (Distance > TacticalData.EngagementRange * 2.0f) continue;

        // Simple threat scoring based on distance and type
        float ThreatScore = 1.0f / (Distance + 1.0f);
        
        // Prefer human players as targets
        if (Target->IsA<APawn>() && Cast<APawn>(Target)->IsPlayerControlled())
        {
            ThreatScore *= 3.0f;
        }

        if (ThreatScore > BestThreatScore)
        {
            BestThreatScore = ThreatScore;
            BestTarget = Target;
        }
    }

    // Update current target
    if (BestTarget != CurrentTarget)
    {
        CurrentTarget = BestTarget;
        if (CurrentTarget)
        {
            LastKnownTargetLocation = CurrentTarget->GetActorLocation();
            LastTargetUpdateTime = GetWorld()->GetTimeSeconds();
        }
    }

    // Determine appropriate tactical state based on situation
    if (CurrentTarget)
    {
        float TargetDistance = FVector::Dist(OwnerLocation, CurrentTarget->GetActorLocation());
        
        if (TargetDistance <= TacticalData.EngagementRange)
        {
            // Close enough to engage
            if (TacticalData.PackSize > 1 && !TacticalData.bIsPackLeader)
            {
                SetTacticalState(ECombat_TacticalState::Coordinate);
            }
            else if (TargetDistance <= TacticalData.EngagementRange * 0.5f)
            {
                SetTacticalState(ECombat_TacticalState::Engage);
            }
            else
            {
                SetTacticalState(ECombat_TacticalState::Hunt);
            }
        }
        else
        {
            SetTacticalState(ECombat_TacticalState::Patrol);
        }
    }
    else
    {
        SetTacticalState(ECombat_TacticalState::Idle);
    }
}

void UCombat_TacticalAIComponent::ExecuteTacticalManeuver()
{
    if (!CurrentTarget) return;

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector OptimalPosition = CalculateOptimalPosition(TargetLocation);

    // Update movement target based on current state
    switch (TacticalData.CurrentState)
    {
        case ECombat_TacticalState::Flank:
            {
                bool bLeftFlank = FMath::RandBool();
                OptimalPosition = GetFlankingPosition(TargetLocation, bLeftFlank);
            }
            break;

        case ECombat_TacticalState::Engage:
            OptimalPosition = TargetLocation;
            break;

        case ECombat_TacticalState::Retreat:
            {
                FVector OwnerLocation = GetOwner()->GetActorLocation();
                FVector RetreatDirection = (OwnerLocation - TargetLocation).GetSafeNormal();
                OptimalPosition = OwnerLocation + RetreatDirection * TacticalData.FlankingDistance;
            }
            break;

        default:
            break;
    }

    UpdateMovementTarget();
}

void UCombat_TacticalAIComponent::SetFormation(ECombat_Formation NewFormation)
{
    TacticalData.PreferredFormation = NewFormation;
    UpdateFormationPositions();
}

FVector UCombat_TacticalAIComponent::GetFormationPosition(int32 MemberIndex)
{
    if (!GetOwner()) return FVector::ZeroVector;

    FVector BasePosition = GetOwner()->GetActorLocation();
    
    switch (TacticalData.PreferredFormation)
    {
        case ECombat_Formation::Line:
            return BasePosition + FVector(0, MemberIndex * 200.0f, 0);

        case ECombat_Formation::Circle:
            {
                float Angle = (2.0f * PI * MemberIndex) / TacticalData.PackSize;
                return BasePosition + FVector(
                    FormationRadius * FMath::Cos(Angle),
                    FormationRadius * FMath::Sin(Angle),
                    0
                );
            }

        case ECombat_Formation::Wedge:
            {
                int32 Row = MemberIndex / 2;
                int32 Side = MemberIndex % 2;
                return BasePosition + FVector(
                    -Row * 150.0f,
                    (Side == 0 ? -1 : 1) * Row * 100.0f,
                    0
                );
            }

        default:
            return BasePosition;
    }
}

void UCombat_TacticalAIComponent::UpdateFormationPositions()
{
    if (!TacticalData.bIsPackLeader) return;

    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (PackMembers[i])
        {
            FVector FormationPos = GetFormationPosition(i + 1); // Leader is position 0
            // Here you would typically send movement commands to pack members
        }
    }
}

void UCombat_TacticalAIComponent::RegisterPackMember(UCombat_TacticalAIComponent* Member)
{
    if (Member && !PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
        TacticalData.PackSize = PackMembers.Num() + 1; // +1 for self
        
        // Set this as pack leader if first member or if not already set
        if (!TacticalData.bIsPackLeader && PackMembers.Num() == 1)
        {
            TacticalData.bIsPackLeader = true;
        }

        Member->PackLeader = this;
        UE_LOG(LogTemp, Log, TEXT("Pack member registered. Pack size: %d"), TacticalData.PackSize);
    }
}

void UCombat_TacticalAIComponent::RemovePackMember(UCombat_TacticalAIComponent* Member)
{
    if (PackMembers.Remove(Member) > 0)
    {
        TacticalData.PackSize = PackMembers.Num() + 1;
        if (Member)
        {
            Member->PackLeader = nullptr;
        }
    }
}

void UCombat_TacticalAIComponent::BroadcastTacticalCommand(ECombat_TacticalState Command)
{
    for (UCombat_TacticalAIComponent* Member : PackMembers)
    {
        if (Member)
        {
            Member->SetTacticalState(Command);
        }
    }
}

void UCombat_TacticalAIComponent::ProcessThreatData(const FVector& ThreatLocation, float ThreatLevel)
{
    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), ThreatLocation);
    
    if (ShouldEngageThreat(ThreatLevel, Distance))
    {
        // Find the actual threat actor
        TArray<AActor*> NearbyActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);
        
        for (AActor* Actor : NearbyActors)
        {
            if (FVector::Dist(Actor->GetActorLocation(), ThreatLocation) < 100.0f)
            {
                CurrentTarget = Actor;
                break;
            }
        }
        
        SetTacticalState(ECombat_TacticalState::Hunt);
    }
    else if (ThreatLevel > 0.8f) // High threat, consider retreat
    {
        SetTacticalState(ECombat_TacticalState::Retreat);
    }
}

bool UCombat_TacticalAIComponent::ShouldEngageThreat(float ThreatLevel, float Distance) const
{
    // Engage if threat is moderate and within range, or if pack is large enough
    return (ThreatLevel > 0.3f && Distance <= TacticalData.EngagementRange) ||
           (TacticalData.PackSize >= 3 && ThreatLevel > 0.2f);
}

FVector UCombat_TacticalAIComponent::CalculateOptimalPosition(const FVector& ThreatLocation)
{
    if (!GetOwner()) return FVector::ZeroVector;

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector ToThreat = (ThreatLocation - OwnerLocation).GetSafeNormal();
    
    // Calculate position based on current state and pack coordination
    switch (TacticalData.CurrentState)
    {
        case ECombat_TacticalState::Engage:
            return ThreatLocation - ToThreat * 200.0f; // Close but not too close

        case ECombat_TacticalState::Flank:
            return GetFlankingPosition(ThreatLocation, FMath::RandBool());

        case ECombat_TacticalState::Retreat:
            return OwnerLocation - ToThreat * TacticalData.FlankingDistance;

        default:
            return OwnerLocation + ToThreat * 100.0f; // Move slightly toward threat
    }
}

void UCombat_TacticalAIComponent::UpdateBlackboardValues()
{
    if (!BlackboardComponent) return;

    BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), (uint8)TacticalData.CurrentState);
    BlackboardComponent->SetValueAsEnum(TEXT("Formation"), (uint8)TacticalData.PreferredFormation);
    BlackboardComponent->SetValueAsInt(TEXT("PackSize"), TacticalData.PackSize);
    BlackboardComponent->SetValueAsBool(TEXT("IsPackLeader"), TacticalData.bIsPackLeader);
    BlackboardComponent->SetValueAsBool(TEXT("InCombat"), bInCombat);

    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("CurrentTarget"), CurrentTarget);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
    }
    else
    {
        BlackboardComponent->ClearValue(TEXT("CurrentTarget"));
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
    }
}

void UCombat_TacticalAIComponent::ProcessPackCoordination()
{
    if (!TacticalData.bIsPackLeader) return;

    // Coordinate pack behavior based on current situation
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Check if pack members are too spread out
    FVector LeaderLocation = GetOwner()->GetActorLocation();
    bool bPackScattered = false;
    
    for (UCombat_TacticalAIComponent* Member : PackMembers)
    {
        if (Member && Member->GetOwner())
        {
            float Distance = FVector::Dist(LeaderLocation, Member->GetOwner()->GetActorLocation());
            if (Distance > CoordinationRange)
            {
                bPackScattered = true;
                break;
            }
        }
    }

    // Adjust formation if scattered
    if (bPackScattered && TacticalData.CurrentState != ECombat_TacticalState::Retreat)
    {
        SetFormation(ECombat_Formation::Circle);
        BroadcastTacticalCommand(ECombat_TacticalState::Coordinate);
    }
}

void UCombat_TacticalAIComponent::ExecuteStateLogic()
{
    switch (TacticalData.CurrentState)
    {
        case ECombat_TacticalState::Hunt:
            if (StateTimer > 5.0f && CurrentTarget)
            {
                float Distance = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
                if (Distance <= TacticalData.EngagementRange * 0.7f)
                {
                    SetTacticalState(ECombat_TacticalState::Engage);
                }
            }
            break;

        case ECombat_TacticalState::Engage:
            if (StateTimer > 3.0f && TacticalData.PackSize > 1)
            {
                SetTacticalState(ECombat_TacticalState::Flank);
            }
            break;

        case ECombat_TacticalState::Flank:
            if (StateTimer > 4.0f)
            {
                SetTacticalState(ECombat_TacticalState::Engage);
            }
            break;

        case ECombat_TacticalState::Retreat:
            if (StateTimer > 6.0f)
            {
                SetTacticalState(ECombat_TacticalState::Patrol);
            }
            break;

        default:
            break;
    }
}

bool UCombat_TacticalAIComponent::CanChangeState() const
{
    return StateTimer >= StateChangeDelay;
}

FVector UCombat_TacticalAIComponent::GetFlankingPosition(const FVector& TargetLocation, bool bLeftFlank)
{
    if (!GetOwner()) return FVector::ZeroVector;

    FVector OwnerLocation = GetOwner()->GetActorLocation();
    FVector ToTarget = (TargetLocation - OwnerLocation).GetSafeNormal();
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector);
    
    if (!bLeftFlank)
    {
        FlankDirection *= -1.0f;
    }

    return TargetLocation + FlankDirection * TacticalData.FlankingDistance;
}

void UCombat_TacticalAIComponent::UpdateMovementTarget()
{
    // This would typically interface with the movement component or AI controller
    // For now, we just update the blackboard
    if (BlackboardComponent && CurrentTarget)
    {
        FVector OptimalPos = CalculateOptimalPosition(CurrentTarget->GetActorLocation());
        BlackboardComponent->SetValueAsVector(TEXT("MovementTarget"), OptimalPos);
    }
}