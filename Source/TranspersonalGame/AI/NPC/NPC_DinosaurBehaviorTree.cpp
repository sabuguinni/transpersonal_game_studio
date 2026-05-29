#include "NPC_DinosaurBehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UNPC_DinosaurBehaviorTree::UNPC_DinosaurBehaviorTree()
{
    Species = ENPC_DinosaurSpecies::TRex;
    CurrentState = ENPC_DinosaurState::Idle;
    StateTimer = 0.0f;
    HomeLocation = FVector::ZeroVector;
    BehaviorTreeAsset = nullptr;
    BlackboardComp = nullptr;
}

void UNPC_DinosaurBehaviorTree::InitializeBehavior(APawn* OwnerPawn)
{
    if (!OwnerPawn)
    {
        return;
    }

    HomeLocation = OwnerPawn->GetActorLocation();
    
    // Generate patrol points around home location
    Memory.PatrolPoints.Empty();
    for (int32 i = 0; i < 6; i++)
    {
        float Angle = (i * 60.0f) * PI / 180.0f;
        FVector PatrolPoint = HomeLocation + FVector(
            FMath::Cos(Angle) * Stats.PatrolRadius,
            FMath::Sin(Angle) * Stats.PatrolRadius,
            0.0f
        );
        Memory.PatrolPoints.Add(PatrolPoint);
    }

    SetupSpeciesBehavior();
    SetState(ENPC_DinosaurState::Patrolling);
}

void UNPC_DinosaurBehaviorTree::UpdateBehavior(float DeltaTime)
{
    StateTimer += DeltaTime;

    // Update stats over time
    Stats.Hunger = FMath::Max(0.0f, Stats.Hunger - (DeltaTime * 0.5f));
    Stats.Energy = FMath::Min(100.0f, Stats.Energy + (DeltaTime * 0.2f));

    // State-specific behavior updates
    switch (CurrentState)
    {
        case ENPC_DinosaurState::Idle:
            if (StateTimer > 5.0f)
            {
                SetState(ENPC_DinosaurState::Patrolling);
            }
            break;

        case ENPC_DinosaurState::Patrolling:
            if (Stats.Hunger < 20.0f)
            {
                SetState(ENPC_DinosaurState::Hunting);
            }
            else if (Stats.Energy < 30.0f)
            {
                SetState(ENPC_DinosaurState::Sleeping);
            }
            break;

        case ENPC_DinosaurState::Hunting:
            if (Stats.Hunger > 70.0f)
            {
                SetState(ENPC_DinosaurState::Patrolling);
            }
            break;

        case ENPC_DinosaurState::Sleeping:
            Stats.Energy = FMath::Min(100.0f, Stats.Energy + (DeltaTime * 2.0f));
            if (Stats.Energy > 80.0f)
            {
                SetState(ENPC_DinosaurState::Idle);
            }
            break;

        case ENPC_DinosaurState::Fleeing:
            if (StateTimer > 10.0f)
            {
                SetState(ENPC_DinosaurState::Patrolling);
            }
            break;
    }
}

void UNPC_DinosaurBehaviorTree::SetState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        StateTimer = 0.0f;

        // Update blackboard if available
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(TEXT("CurrentState"), static_cast<uint8>(CurrentState));
        }
    }
}

bool UNPC_DinosaurBehaviorTree::CanSeePlayer(APawn* OwnerPawn, APawn* Player)
{
    if (!OwnerPawn || !Player)
    {
        return false;
    }

    float Distance = GetDistanceToPlayer(OwnerPawn, Player);
    if (Distance > Stats.DetectionRange)
    {
        return false;
    }

    // Line of sight check
    FVector Start = OwnerPawn->GetActorLocation();
    FVector End = Player->GetActorLocation();
    
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(Player);

    bool bHit = OwnerPawn->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        Start,
        End,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return !bHit; // Can see if no obstruction
}

float UNPC_DinosaurBehaviorTree::GetDistanceToPlayer(APawn* OwnerPawn, APawn* Player)
{
    if (!OwnerPawn || !Player)
    {
        return 99999.0f;
    }

    return FVector::Dist(OwnerPawn->GetActorLocation(), Player->GetActorLocation());
}

FVector UNPC_DinosaurBehaviorTree::GetNextPatrolPoint()
{
    if (Memory.PatrolPoints.Num() == 0)
    {
        return HomeLocation;
    }

    Memory.CurrentPatrolIndex = (Memory.CurrentPatrolIndex + 1) % Memory.PatrolPoints.Num();
    return Memory.PatrolPoints[Memory.CurrentPatrolIndex];
}

void UNPC_DinosaurBehaviorTree::UpdateMemory(APawn* Player, float DeltaTime)
{
    if (Player)
    {
        Memory.LastPlayerLocation = Player->GetActorLocation();
        Memory.TimeSincePlayerSeen = 0.0f;
        Memory.ThreatLevel = FMath::Min(100.0f, Memory.ThreatLevel + (DeltaTime * 10.0f));
    }
    else
    {
        Memory.TimeSincePlayerSeen += DeltaTime;
        if (Memory.TimeSincePlayerSeen > 30.0f)
        {
            Memory.ThreatLevel = FMath::Max(0.0f, Memory.ThreatLevel - (DeltaTime * 5.0f));
        }
    }
}

void UNPC_DinosaurBehaviorTree::SetupSpeciesBehavior()
{
    switch (Species)
    {
        case ENPC_DinosaurSpecies::TRex:
            SetupTRexBehavior();
            break;
        case ENPC_DinosaurSpecies::Velociraptor:
            SetupVelociraptorBehavior();
            break;
        case ENPC_DinosaurSpecies::Triceratops:
            SetupTriceratopsBehavior();
            break;
        case ENPC_DinosaurSpecies::Brachiosaurus:
            SetupBrachiosaurusBehavior();
            break;
    }
}

void UNPC_DinosaurBehaviorTree::SetupTRexBehavior()
{
    Stats.Aggression = 80.0f;
    Stats.Fear = 5.0f;
    Stats.PatrolRadius = 8000.0f;
    Stats.DetectionRange = 4000.0f;
    Stats.AttackRange = 400.0f;
}

void UNPC_DinosaurBehaviorTree::SetupVelociraptorBehavior()
{
    Stats.Aggression = 90.0f;
    Stats.Fear = 20.0f;
    Stats.PatrolRadius = 6000.0f;
    Stats.DetectionRange = 5000.0f;
    Stats.AttackRange = 200.0f;
}

void UNPC_DinosaurBehaviorTree::SetupTriceratopsBehavior()
{
    Stats.Aggression = 40.0f;
    Stats.Fear = 30.0f;
    Stats.PatrolRadius = 4000.0f;
    Stats.DetectionRange = 2500.0f;
    Stats.AttackRange = 300.0f;
}

void UNPC_DinosaurBehaviorTree::SetupBrachiosaurusBehavior()
{
    Stats.Aggression = 10.0f;
    Stats.Fear = 60.0f;
    Stats.PatrolRadius = 10000.0f;
    Stats.DetectionRange = 3000.0f;
    Stats.AttackRange = 500.0f;
}

// Behavior Tree Task Implementations
UNPC_BTTask_Patrol::UNPC_BTTask_Patrol()
{
    NodeName = TEXT("Dinosaur Patrol");
}

EBTNodeResult::Type UNPC_BTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    // Get next patrol point and move to it
    FVector PatrolPoint = BlackboardComp->GetValueAsVector(TEXT("PatrolPoint"));
    if (PatrolPoint.IsZero())
    {
        // Generate a random patrol point
        FVector CurrentLocation = AIController->GetPawn()->GetActorLocation();
        float Radius = 3000.0f;
        float Angle = FMath::RandRange(0.0f, 360.0f) * PI / 180.0f;
        
        PatrolPoint = CurrentLocation + FVector(
            FMath::Cos(Angle) * Radius,
            FMath::Sin(Angle) * Radius,
            0.0f
        );
        
        BlackboardComp->SetValueAsVector(TEXT("PatrolPoint"), PatrolPoint);
    }

    // Move to patrol point
    AIController->MoveToLocation(PatrolPoint, 100.0f);

    return EBTNodeResult::Succeeded;
}

UNPC_BTTask_ChasePlayer::UNPC_BTTask_ChasePlayer()
{
    NodeName = TEXT("Chase Player");
}

EBTNodeResult::Type UNPC_BTTask_ChasePlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    APawn* Player = Cast<APawn>(BlackboardComp->GetValueAsObject(TEXT("Player")));
    if (!Player)
    {
        return EBTNodeResult::Failed;
    }

    // Chase the player
    AIController->MoveToActor(Player, 200.0f);

    return EBTNodeResult::Succeeded;
}

UNPC_BTTask_AttackPlayer::UNPC_BTTask_AttackPlayer()
{
    NodeName = TEXT("Attack Player");
}

EBTNodeResult::Type UNPC_BTTask_AttackPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    APawn* Player = Cast<APawn>(BlackboardComp->GetValueAsObject(TEXT("Player")));
    if (!Player)
    {
        return EBTNodeResult::Failed;
    }

    // Perform attack animation/logic
    // This would trigger attack animations and deal damage
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur attacking player!"));

    return EBTNodeResult::Succeeded;
}

UNPC_BTDecorator_CanSeePlayer::UNPC_BTDecorator_CanSeePlayer()
{
    NodeName = TEXT("Can See Player");
    SightRange = 3000.0f;
    SightAngle = 90.0f;
}

bool UNPC_BTDecorator_CanSeePlayer::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return false;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        return false;
    }

    APawn* Player = Cast<APawn>(BlackboardComp->GetValueAsObject(TEXT("Player")));
    if (!Player)
    {
        return false;
    }

    APawn* OwnerPawn = AIController->GetPawn();
    FVector OwnerLocation = OwnerPawn->GetActorLocation();
    FVector PlayerLocation = Player->GetActorLocation();
    
    float Distance = FVector::Dist(OwnerLocation, PlayerLocation);
    if (Distance > SightRange)
    {
        return false;
    }

    // Check angle
    FVector OwnerForward = OwnerPawn->GetActorForwardVector();
    FVector ToPlayer = (PlayerLocation - OwnerLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(OwnerForward, ToPlayer);
    float AngleRadians = FMath::Acos(DotProduct);
    float AngleDegrees = FMath::RadiansToDegrees(AngleRadians);

    if (AngleDegrees > SightAngle / 2.0f)
    {
        return false;
    }

    // Line of sight check
    FHitResult HitResult;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OwnerPawn);
    QueryParams.AddIgnoredActor(Player);

    bool bHit = OwnerPawn->GetWorld()->LineTraceSingleByChannel(
        HitResult,
        OwnerLocation,
        PlayerLocation,
        ECollisionChannel::ECC_Visibility,
        QueryParams
    );

    return !bHit; // Can see if no obstruction
}