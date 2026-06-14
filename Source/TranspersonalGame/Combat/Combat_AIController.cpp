#include "Combat_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISightConfig.h"
#include "Perception/AIHearingConfig.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "DrawDebugHelpers.h"

ACombat_AIController::ACombat_AIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create behavior tree component
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    
    // Create blackboard component
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));

    // Create AI perception component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure sight perception
    UAISightConfig* SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 2000.0f;
        SightConfig->LoseSightRadius = 2200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->DetectionByAffiliation.bNeutral = true;
        SightConfig->DetectionByAffiliation.bFriendly = false;
        SightConfig->DetectionByAffiliation.bEnemy = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // Configure hearing perception
    UAIHearingConfig* HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1500.0f;
        HearingConfig->DetectionByAffiliation.bNeutral = true;
        HearingConfig->DetectionByAffiliation.bFriendly = false;
        HearingConfig->DetectionByAffiliation.bEnemy = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Initialize state
    CurrentState = ECombat_AIState::Idle;
    CurrentTarget = nullptr;
    StateChangeTimer = 0.0f;
    LastKnownTargetLocation = FVector::ZeroVector;

    // Initialize tactical data
    TacticalData.AggressionLevel = 0.7f;
    TacticalData.CautiousDistance = 1200.0f;
    TacticalData.AttackRange = 400.0f;
    TacticalData.FlankingRadius = 900.0f;
    TacticalData.bCanCoordinate = true;
    TacticalData.PackSize = 1;
}

void ACombat_AIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnPerceptionUpdated);
    }

    // Start behavior tree if assigned
    if (BehaviorTree && BlackboardComponent)
    {
        UseBlackboard(BehaviorTree->BlackboardAsset);
        RunBehaviorTree(BehaviorTree);
    }

    // Set initial state
    SetAIState(ECombat_AIState::Patrol);
}

void ACombat_AIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateTacticalState(DeltaTime);
    ProcessPackCoordination();

    StateChangeTimer += DeltaTime;

    // Debug drawing
    if (GetPawn())
    {
        FVector PawnLocation = GetPawn()->GetActorLocation();
        
        // Draw perception radius
        DrawDebugCircle(GetWorld(), PawnLocation, TacticalData.CautiousDistance, 32, FColor::Yellow, false, 0.1f, 0, 5.0f, FVector(0, 1, 0), FVector(1, 0, 0));
        
        // Draw attack range
        DrawDebugCircle(GetWorld(), PawnLocation, TacticalData.AttackRange, 16, FColor::Red, false, 0.1f, 0, 3.0f, FVector(0, 1, 0), FVector(1, 0, 0));
        
        // Draw state
        FString StateString = UEnum::GetValueAsString(CurrentState);
        DrawDebugString(GetWorld(), PawnLocation + FVector(0, 0, 200), StateString, nullptr, FColor::White, 0.1f);
    }
}

void ACombat_AIController::SetAIState(ECombat_AIState NewState)
{
    if (CurrentState != NewState)
    {
        ECombat_AIState OldState = CurrentState;
        CurrentState = NewState;
        StateChangeTimer = 0.0f;

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(NewState));
        }

        // State transition logic
        switch (NewState)
        {
        case ECombat_AIState::Hunting:
            if (CurrentTarget && BlackboardComponent)
            {
                BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
                LastKnownTargetLocation = CurrentTarget->GetActorLocation();
                BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
            }
            break;

        case ECombat_AIState::Attacking:
            CoordinatePackAttack();
            break;

        case ECombat_AIState::Retreating:
            if (BlackboardComponent)
            {
                FVector RetreatLocation = GetPawn()->GetActorLocation() + (GetPawn()->GetActorLocation() - LastKnownTargetLocation).GetSafeNormal() * 1500.0f;
                BlackboardComponent->SetValueAsVector(TEXT("RetreatLocation"), RetreatLocation);
            }
            break;
        }
    }
}

void ACombat_AIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
        
        if (NewTarget)
        {
            LastKnownTargetLocation = NewTarget->GetActorLocation();
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), LastKnownTargetLocation);
            
            // Transition to hunting state
            SetAIState(ECombat_AIState::Hunting);
        }
        else
        {
            SetAIState(ECombat_AIState::Patrol);
        }
    }
}

void ACombat_AIController::AddPackMember(AActor* Member)
{
    if (Member && !PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
        TacticalData.PackSize = PackMembers.Num();
    }
}

void ACombat_AIController::RemovePackMember(AActor* Member)
{
    if (Member)
    {
        PackMembers.Remove(Member);
        TacticalData.PackSize = PackMembers.Num();
    }
}

bool ACombat_AIController::IsInPackFormation() const
{
    if (PackMembers.Num() < 2 || !GetPawn())
    {
        return false;
    }

    FVector MyLocation = GetPawn()->GetActorLocation();
    float FormationRadius = TacticalData.FlankingRadius;

    for (AActor* Member : PackMembers)
    {
        if (Member && Member != GetPawn())
        {
            float Distance = FVector::Dist(MyLocation, Member->GetActorLocation());
            if (Distance > FormationRadius)
            {
                return false;
            }
        }
    }

    return true;
}

FVector ACombat_AIController::GetFlankingPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return FVector::ZeroVector;
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector ToTarget = (TargetLocation - MyLocation).GetSafeNormal();
    
    // Calculate flanking position 90 degrees to the right or left
    FVector FlankDirection = FVector::CrossProduct(ToTarget, FVector::UpVector);
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f; // Random left or right
    }
    
    return TargetLocation + FlankDirection * TacticalData.FlankingRadius;
}

bool ACombat_AIController::ShouldRetreat() const
{
    if (!GetPawn())
    {
        return false;
    }

    // Check health (assuming pawn has health component)
    ACharacter* Character = Cast<ACharacter>(GetPawn());
    if (Character)
    {
        // Retreat if heavily damaged or outnumbered
        float HealthRatio = 1.0f; // Would get from health component
        int32 NearbyEnemies = 1; // Would count nearby enemies
        
        return (HealthRatio < 0.3f) || (NearbyEnemies > TacticalData.PackSize * 2);
    }

    return false;
}

void ACombat_AIController::CoordinatePackAttack()
{
    if (!TacticalData.bCanCoordinate || PackMembers.Num() < 2)
    {
        return;
    }

    // Coordinate attack patterns with pack members
    for (int32 i = 0; i < PackMembers.Num(); ++i)
    {
        if (AActor* Member = PackMembers[i])
        {
            if (ACombat_AIController* MemberController = Cast<ACombat_AIController>(Member->GetInstigatorController()))
            {
                // Assign different roles based on pack position
                if (i == 0)
                {
                    MemberController->SetAIState(ECombat_AIState::Attacking); // Leader attacks directly
                }
                else if (i % 2 == 1)
                {
                    MemberController->SetAIState(ECombat_AIState::Flanking); // Flank left
                }
                else
                {
                    MemberController->SetAIState(ECombat_AIState::Flanking); // Flank right
                }
            }
        }
    }
}

void ACombat_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is a player or enemy
            if (Actor->ActorHasTag(TEXT("Player")) || Actor->ActorHasTag(TEXT("Enemy")))
            {
                FAIStimulus Stimulus;
                if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
                {
                    if (Stimulus.WasSuccessfullySensed())
                    {
                        SetTarget(Actor);
                    }
                    else if (CurrentTarget == Actor)
                    {
                        // Lost sight of target
                        LastKnownTargetLocation = Stimulus.StimulusLocation;
                        SetAIState(ECombat_AIState::Investigating);
                    }
                }
            }
        }
    }
}

void ACombat_AIController::UpdateTacticalState(float DeltaTime)
{
    if (!GetPawn())
    {
        return;
    }

    switch (CurrentState)
    {
    case ECombat_AIState::Hunting:
        if (CurrentTarget)
        {
            float DistanceToTarget = GetDistanceToTarget(CurrentTarget);
            
            if (DistanceToTarget <= TacticalData.AttackRange)
            {
                SetAIState(ECombat_AIState::Attacking);
            }
            else if (!CanSeeTarget(CurrentTarget) && StateChangeTimer > 5.0f)
            {
                SetAIState(ECombat_AIState::Investigating);
            }
        }
        break;

    case ECombat_AIState::Attacking:
        if (ShouldRetreat())
        {
            SetAIState(ECombat_AIState::Retreating);
        }
        else if (CurrentTarget && GetDistanceToTarget(CurrentTarget) > TacticalData.AttackRange * 1.5f)
        {
            SetAIState(ECombat_AIState::Hunting);
        }
        break;

    case ECombat_AIState::Investigating:
        if (StateChangeTimer > 10.0f)
        {
            SetAIState(ECombat_AIState::Patrol);
        }
        break;

    case ECombat_AIState::Retreating:
        if (StateChangeTimer > 8.0f)
        {
            SetAIState(ECombat_AIState::Patrol);
        }
        break;
    }
}

void ACombat_AIController::ProcessPackCoordination()
{
    if (!TacticalData.bCanCoordinate || PackMembers.Num() < 2)
    {
        return;
    }

    // Remove null or destroyed pack members
    PackMembers.RemoveAll([](AActor* Member) {
        return !IsValid(Member);
    });

    TacticalData.PackSize = PackMembers.Num();
}

FVector ACombat_AIController::CalculateOptimalPosition(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return GetPawn()->GetActorLocation();
    }

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Calculate position based on current state
    switch (CurrentState)
    {
    case ECombat_AIState::Flanking:
        return GetFlankingPosition(Target);
        
    case ECombat_AIState::Attacking:
        {
            FVector ToTarget = (TargetLocation - MyLocation).GetSafeNormal();
            return TargetLocation - ToTarget * TacticalData.AttackRange * 0.8f;
        }
        
    case ECombat_AIState::Retreating:
        {
            FVector AwayFromTarget = (MyLocation - TargetLocation).GetSafeNormal();
            return MyLocation + AwayFromTarget * 1000.0f;
        }
        
    default:
        return MyLocation;
    }
}

bool ACombat_AIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent)
    {
        return false;
    }

    FAIStimulus Stimulus;
    return AIPerceptionComponent->GetActorsPerception(Target, Stimulus) && Stimulus.WasSuccessfullySensed();
}

float ACombat_AIController::GetDistanceToTarget(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return MAX_FLT;
    }

    return FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
}