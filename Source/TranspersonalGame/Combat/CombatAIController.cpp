#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Combat_EnemyPawn.h"

// Blackboard Keys
const FName ACombatAIController::TargetActorKey = TEXT("TargetActor");
const FName ACombatAIController::AIStateKey = TEXT("AIState");
const FName ACombatAIController::TacticalRoleKey = TEXT("TacticalRole");
const FName ACombatAIController::PackLeaderKey = TEXT("PackLeader");

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 1500.0f;
        SightConfig->LoseSightRadius = 1600.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = 1200.0f;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Initialize state
    CurrentAIState = ECombat_AIState::Patrol;
    CurrentTarget = nullptr;
    PackLeader = nullptr;
    LastTargetUpdateTime = 0.0f;
    StateChangeTime = 0.0f;

    // Set default tactical data
    TacticalData.Role = ECombat_TacticalRole::Solo;
    TacticalData.FlankingDistance = 500.0f;
    TacticalData.AttackRange = 200.0f;
    TacticalData.RetreatHealthThreshold = 0.3f;
    TacticalData.bCanCallForHelp = true;
    TacticalData.MaxPackSize = 3;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAI();
    
    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdatePerception();
    UpdateTacticalBehavior(DeltaTime);
    ProcessPackCoordination();
}

void ACombatAIController::InitializeAI()
{
    if (Blackboard && BlackboardComponent)
    {
        UseBlackboard(BlackboardAsset);
        
        // Initialize blackboard values
        BlackboardComponent->SetValueAsEnum(AIStateKey, static_cast<uint8>(CurrentAIState));
        BlackboardComponent->SetValueAsEnum(TacticalRoleKey, static_cast<uint8>(TacticalData.Role));
    }

    if (BehaviorTree && BehaviorTreeComponent)
    {
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::SetAIState(ECombat_AIState NewState)
{
    if (CurrentAIState != NewState)
    {
        ECombat_AIState PreviousState = CurrentAIState;
        CurrentAIState = NewState;
        StateChangeTime = GetWorld()->GetTimeSeconds();

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(AIStateKey, static_cast<uint8>(CurrentAIState));
        }

        // Log state change for debugging
        if (GEngine)
        {
            FString StateName = UEnum::GetValueAsString(CurrentAIState);
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                FString::Printf(TEXT("%s: AI State changed to %s"), 
                *GetPawn()->GetName(), *StateName));
        }

        // Handle specific state transitions
        switch (NewState)
        {
            case ECombat_AIState::Combat:
                if (TacticalData.bCanCallForHelp && TacticalData.Role == ECombat_TacticalRole::Alpha)
                {
                    CallForBackup();
                }
                break;
            case ECombat_AIState::Flanking:
                ExecuteFlankingManeuver();
                break;
            case ECombat_AIState::Retreating:
                // Clear current target when retreating
                SetTarget(nullptr);
                break;
        }
    }
}

void ACombatAIController::SetTarget(APawn* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        CurrentTarget = NewTarget;
        LastTargetUpdateTime = GetWorld()->GetTimeSeconds();

        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TargetActorKey, CurrentTarget);
        }

        // Change AI state based on target
        if (CurrentTarget)
        {
            if (CurrentAIState == ECombat_AIState::Patrol)
            {
                SetAIState(ECombat_AIState::Alert);
            }
        }
        else
        {
            if (CurrentAIState == ECombat_AIState::Combat || CurrentAIState == ECombat_AIState::Hunting)
            {
                SetAIState(ECombat_AIState::Patrol);
            }
        }
    }
}

void ACombatAIController::ExecuteFlankingManeuver()
{
    if (!CurrentTarget || !GetPawn())
    {
        return;
    }

    FVector TargetLocation = CurrentTarget->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - MyLocation).GetSafeNormal();

    // Calculate flanking position based on role
    FVector FlankingDirection;
    switch (TacticalData.Role)
    {
        case ECombat_TacticalRole::Beta:
            // Flank left
            FlankingDirection = FVector::CrossProduct(DirectionToTarget, FVector::UpVector);
            break;
        case ECombat_TacticalRole::Gamma:
            // Flank right
            FlankingDirection = FVector::CrossProduct(FVector::UpVector, DirectionToTarget);
            break;
        default:
            // Alpha or Solo - direct approach
            FlankingDirection = DirectionToTarget;
            break;
    }

    FVector FlankingPosition = TargetLocation + (FlankingDirection * TacticalData.FlankingDistance);
    
    // Move to flanking position
    MoveToLocation(FlankingPosition, 50.0f);
}

void ACombatAIController::CallForBackup()
{
    if (!GetPawn() || PackMembers.Num() >= TacticalData.MaxPackSize)
    {
        return;
    }

    // Find nearby AI controllers that can join the pack
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatAIController::StaticClass(), FoundActors);

    for (AActor* Actor : FoundActors)
    {
        ACombatAIController* OtherAI = Cast<ACombatAIController>(Actor);
        if (OtherAI && OtherAI != this && !OtherAI->IsInPack())
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), OtherAI->GetPawn()->GetActorLocation());
            if (Distance <= 1000.0f) // Within backup call range
            {
                OtherAI->JoinPack(this);
                if (PackMembers.Num() >= TacticalData.MaxPackSize)
                {
                    break;
                }
            }
        }
    }
}

bool ACombatAIController::ShouldRetreat() const
{
    if (!GetPawn())
    {
        return false;
    }

    // Check health threshold
    if (ACombat_EnemyPawn* EnemyPawn = Cast<ACombat_EnemyPawn>(GetPawn()))
    {
        float HealthRatio = EnemyPawn->GetHealthRatio();
        if (HealthRatio <= TacticalData.RetreatHealthThreshold)
        {
            return true;
        }
    }

    // Check if outnumbered
    if (CurrentTarget && IsInPack())
    {
        // Count nearby enemies
        TArray<AActor*> NearbyEnemies;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacter::StaticClass(), NearbyEnemies);
        
        int32 EnemyCount = 0;
        for (AActor* Actor : NearbyEnemies)
        {
            if (Actor != GetPawn() && FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation()) <= 500.0f)
            {
                EnemyCount++;
            }
        }

        if (EnemyCount > PackMembers.Num() + 1) // +1 for self
        {
            return true;
        }
    }

    return false;
}

void ACombatAIController::JoinPack(ACombatAIController* NewPackLeader)
{
    if (!NewPackLeader || NewPackLeader == this)
    {
        return;
    }

    // Leave current pack if in one
    LeavePack();

    // Join new pack
    PackLeader = NewPackLeader;
    NewPackLeader->PackMembers.AddUnique(this);

    // Assign tactical role based on pack position
    int32 PackPosition = NewPackLeader->PackMembers.Num();
    switch (PackPosition)
    {
        case 1:
            TacticalData.Role = ECombat_TacticalRole::Beta;
            break;
        case 2:
            TacticalData.Role = ECombat_TacticalRole::Gamma;
            break;
        default:
            TacticalData.Role = ECombat_TacticalRole::Gamma;
            break;
    }

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TacticalRoleKey, static_cast<uint8>(TacticalData.Role));
        BlackboardComponent->SetValueAsObject(PackLeaderKey, PackLeader);
    }

    // Share target with pack leader
    if (NewPackLeader->GetCurrentTarget())
    {
        SetTarget(NewPackLeader->GetCurrentTarget());
        SetAIState(ECombat_AIState::Combat);
    }
}

void ACombatAIController::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->PackMembers.Remove(this);
        PackLeader = nullptr;
    }

    // Remove all pack members if this was the leader
    for (ACombatAIController* Member : PackMembers)
    {
        if (Member)
        {
            Member->PackLeader = nullptr;
            Member->TacticalData.Role = ECombat_TacticalRole::Solo;
            
            if (Member->BlackboardComponent)
            {
                Member->BlackboardComponent->SetValueAsEnum(TacticalRoleKey, static_cast<uint8>(ECombat_TacticalRole::Solo));
                Member->BlackboardComponent->SetValueAsObject(PackLeaderKey, nullptr);
            }
        }
    }
    PackMembers.Empty();

    // Reset to solo role
    TacticalData.Role = ECombat_TacticalRole::Solo;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(TacticalRoleKey, static_cast<uint8>(TacticalData.Role));
        BlackboardComponent->SetValueAsObject(PackLeaderKey, nullptr);
    }
}

void ACombatAIController::UpdatePerception()
{
    if (!AIPerceptionComponent || !GetPawn())
    {
        return;
    }

    // Get currently perceived actors
    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(nullptr, PerceivedActors);

    // Find the closest hostile target
    APawn* BestTarget = nullptr;
    float ClosestDistance = TacticalData.AttackRange * 2.0f;

    for (AActor* Actor : PerceivedActors)
    {
        if (APawn* Pawn = Cast<APawn>(Actor))
        {
            if (Pawn != GetPawn() && Pawn->IsA<ACharacter>())
            {
                float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Pawn->GetActorLocation());
                if (Distance < ClosestDistance)
                {
                    BestTarget = Pawn;
                    ClosestDistance = Distance;
                }
            }
        }
    }

    // Update target if we found a better one
    if (BestTarget && BestTarget != CurrentTarget)
    {
        SetTarget(BestTarget);
    }
}

void ACombatAIController::UpdateTacticalBehavior(float DeltaTime)
{
    if (!CurrentTarget || !GetPawn())
    {
        return;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    // State machine logic
    switch (CurrentAIState)
    {
        case ECombat_AIState::Alert:
            if (DistanceToTarget <= TacticalData.AttackRange * 1.5f)
            {
                SetAIState(ECombat_AIState::Combat);
            }
            else if (DistanceToTarget <= 800.0f)
            {
                SetAIState(ECombat_AIState::Hunting);
            }
            break;

        case ECombat_AIState::Hunting:
            if (DistanceToTarget <= TacticalData.AttackRange)
            {
                SetAIState(ECombat_AIState::Combat);
            }
            else if (DistanceToTarget > 1200.0f)
            {
                SetAIState(ECombat_AIState::Patrol);
            }
            break;

        case ECombat_AIState::Combat:
            if (ShouldRetreat())
            {
                SetAIState(ECombat_AIState::Retreating);
            }
            else if (DistanceToTarget > TacticalData.AttackRange * 2.0f && IsInPack())
            {
                SetAIState(ECombat_AIState::Flanking);
            }
            break;

        case ECombat_AIState::Flanking:
            if (DistanceToTarget <= TacticalData.AttackRange)
            {
                SetAIState(ECombat_AIState::Combat);
            }
            break;

        case ECombat_AIState::Retreating:
            if (DistanceToTarget > 1000.0f)
            {
                SetAIState(ECombat_AIState::Patrol);
            }
            break;
    }
}

void ACombatAIController::ProcessPackCoordination()
{
    if (!IsInPack())
    {
        return;
    }

    // Pack leader coordinates the attack
    if (TacticalData.Role == ECombat_TacticalRole::Alpha && CurrentTarget)
    {
        // Share target with pack members
        for (ACombatAIController* Member : PackMembers)
        {
            if (Member && Member->GetCurrentTarget() != CurrentTarget)
            {
                Member->SetTarget(CurrentTarget);
            }
        }

        // Coordinate tactical positions
        float TimeSinceStateChange = GetWorld()->GetTimeSeconds() - StateChangeTime;
        if (CurrentAIState == ECombat_AIState::Combat && TimeSinceStateChange > 2.0f)
        {
            // Order flanking maneuver
            for (ACombatAIController* Member : PackMembers)
            {
                if (Member && Member->GetAIState() == ECombat_AIState::Combat)
                {
                    Member->SetAIState(ECombat_AIState::Flanking);
                }
            }
        }
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // This is called automatically when perception component detects changes
    // The main logic is handled in UpdatePerception()
}