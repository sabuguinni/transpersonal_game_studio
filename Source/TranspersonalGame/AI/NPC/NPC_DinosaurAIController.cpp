#include "NPC_DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ANPC_DinosaurAIController::ANPC_DinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    SetPerceptionComponent(*AIPerceptionComponent);

    // Initialize default values
    DinosaurSpecies = ENPC_DinosaurSpecies::TRex;
    CurrentState = ENPC_DinosaurState::Idle;
    PatrolRadius = 5000.0f;
    ChaseDistance = 3000.0f;
    AttackDistance = 300.0f;
    SightRange = 2500.0f;
    HearingRange = 1500.0f;
    MovementSpeed = 600.0f;
    AggressionLevel = 0.5f;
    bIsPackAnimal = false;
    bIsNocturnal = false;
    bIsPackLeader = false;
    CurrentTarget = nullptr;
    PackLeader = nullptr;
    LastPlayerSightTime = 0.0f;
}

void ANPC_DinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAIPerception();
    SetupSpeciesDefaults();
    
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
        UpdateBlackboardValues();
    }
}

void ANPC_DinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        HomeLocation = InPawn->GetActorLocation();
        ConfigureBehaviorTree();
        StartPatrolling();
    }
}

void ANPC_DinosaurAIController::OnUnPossess()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
    
    Super::OnUnPossess();
}

void ANPC_DinosaurAIController::InitializeAIPerception()
{
    if (!AIPerceptionComponent)
        return;

    // Configure Sight
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRange;
        SightConfig->LoseSightRadius = SightRange * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure Hearing
    HearingConfig = CreateDefaultSubobject<UAIHearingConfig>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurAIController::OnPerceptionUpdated);
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurAIController::OnTargetPerceptionUpdated);
}

void ANPC_DinosaurAIController::ConfigureBehaviorTree()
{
    if (BehaviorTree && BlackboardAsset)
    {
        if (Blackboard)
        {
            Blackboard->InitializeBlackboard(*BlackboardAsset);
        }
        
        if (BehaviorTreeComponent)
        {
            BehaviorTreeComponent->StartTree(*BehaviorTree);
        }
    }
}

void ANPC_DinosaurAIController::UpdateBlackboardValues()
{
    if (!Blackboard)
        return;

    Blackboard->SetValueAsEnum(TEXT("DinosaurSpecies"), static_cast<uint8>(DinosaurSpecies));
    Blackboard->SetValueAsEnum(TEXT("CurrentState"), static_cast<uint8>(CurrentState));
    Blackboard->SetValueAsVector(TEXT("HomeLocation"), HomeLocation);
    Blackboard->SetValueAsFloat(TEXT("PatrolRadius"), PatrolRadius);
    Blackboard->SetValueAsFloat(TEXT("ChaseDistance"), ChaseDistance);
    Blackboard->SetValueAsFloat(TEXT("AttackDistance"), AttackDistance);
    Blackboard->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
    Blackboard->SetValueAsBool(TEXT("IsPackAnimal"), bIsPackAnimal);
    Blackboard->SetValueAsBool(TEXT("IsPackLeader"), bIsPackLeader);
    
    if (CurrentTarget)
    {
        Blackboard->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
        Blackboard->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
    }
    else
    {
        Blackboard->ClearValue(TEXT("TargetActor"));
        Blackboard->ClearValue(TEXT("TargetLocation"));
    }
}

void ANPC_DinosaurAIController::SetupSpeciesDefaults()
{
    switch (DinosaurSpecies)
    {
        case ENPC_DinosaurSpecies::TRex:
            PatrolRadius = 8000.0f;
            ChaseDistance = 4000.0f;
            AttackDistance = 400.0f;
            SightRange = 3500.0f;
            HearingRange = 2000.0f;
            MovementSpeed = 800.0f;
            AggressionLevel = 0.9f;
            bIsPackAnimal = false;
            bIsNocturnal = false;
            break;
            
        case ENPC_DinosaurSpecies::Velociraptor:
            PatrolRadius = 4000.0f;
            ChaseDistance = 2500.0f;
            AttackDistance = 200.0f;
            SightRange = 2000.0f;
            HearingRange = 1800.0f;
            MovementSpeed = 1200.0f;
            AggressionLevel = 0.8f;
            bIsPackAnimal = true;
            bIsNocturnal = false;
            break;
            
        case ENPC_DinosaurSpecies::Triceratops:
            PatrolRadius = 3000.0f;
            ChaseDistance = 1500.0f;
            AttackDistance = 350.0f;
            SightRange = 1800.0f;
            HearingRange = 1200.0f;
            MovementSpeed = 400.0f;
            AggressionLevel = 0.3f;
            bIsPackAnimal = true;
            bIsNocturnal = false;
            break;
            
        case ENPC_DinosaurSpecies::Brachiosaurus:
            PatrolRadius = 6000.0f;
            ChaseDistance = 1000.0f;
            AttackDistance = 500.0f;
            SightRange = 2500.0f;
            HearingRange = 1000.0f;
            MovementSpeed = 200.0f;
            AggressionLevel = 0.1f;
            bIsPackAnimal = false;
            bIsNocturnal = false;
            break;
            
        default:
            break;
    }
}

void ANPC_DinosaurAIController::SetDinosaurSpecies(ENPC_DinosaurSpecies NewSpecies)
{
    DinosaurSpecies = NewSpecies;
    SetupSpeciesDefaults();
    UpdateBlackboardValues();
}

void ANPC_DinosaurAIController::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        HandleStateTransition(NewState);
        CurrentState = NewState;
        UpdateBlackboardValues();
    }
}

void ANPC_DinosaurAIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (NewTarget)
    {
        LastKnownPlayerLocation = NewTarget->GetActorLocation();
        LastPlayerSightTime = GetWorld()->GetTimeSeconds();
    }
    UpdateBlackboardValues();
}

void ANPC_DinosaurAIController::ClearTarget()
{
    CurrentTarget = nullptr;
    UpdateBlackboardValues();
}

bool ANPC_DinosaurAIController::HasValidTarget() const
{
    return CurrentTarget && IsValid(CurrentTarget);
}

float ANPC_DinosaurAIController::GetDistanceToTarget() const
{
    if (!HasValidTarget() || !GetPawn())
        return -1.0f;
        
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool ANPC_DinosaurAIController::IsPlayerInSight() const
{
    if (!HasValidTarget())
        return false;
        
    return GetDistanceToTarget() <= SightRange;
}

bool ANPC_DinosaurAIController::IsPlayerInAttackRange() const
{
    if (!HasValidTarget())
        return false;
        
    return GetDistanceToTarget() <= AttackDistance;
}

void ANPC_DinosaurAIController::StartPatrolling()
{
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
    ClearTarget();
    
    FVector PatrolPoint = GetRandomPatrolPoint();
    if (Blackboard)
    {
        Blackboard->SetValueAsVector(TEXT("PatrolPoint"), PatrolPoint);
    }
}

void ANPC_DinosaurAIController::StartChasing(AActor* Target)
{
    SetTarget(Target);
    SetDinosaurState(ENPC_DinosaurState::Chasing);
    
    // Alert pack members if this is a pack animal
    if (bIsPackAnimal)
    {
        AlertPack(Target);
    }
}

void ANPC_DinosaurAIController::StartAttacking()
{
    if (HasValidTarget())
    {
        SetDinosaurState(ENPC_DinosaurState::Attacking);
    }
}

void ANPC_DinosaurAIController::ReturnToHome()
{
    SetDinosaurState(ENPC_DinosaurState::Returning);
    ClearTarget();
    
    if (Blackboard)
    {
        Blackboard->SetValueAsVector(TEXT("TargetLocation"), HomeLocation);
    }
}

void ANPC_DinosaurAIController::JoinPack(ANPC_DinosaurAIController* Leader)
{
    if (Leader && Leader != this)
    {
        LeavePack(); // Leave current pack first
        
        PackLeader = Leader;
        bIsPackLeader = false;
        Leader->AddPackMember(this);
        UpdateBlackboardValues();
    }
}

void ANPC_DinosaurAIController::LeavePack()
{
    if (PackLeader)
    {
        PackLeader->RemovePackMember(this);
        PackLeader = nullptr;
    }
    
    // Transfer leadership if this was the leader
    if (bIsPackLeader && PackMembers.Num() > 0)
    {
        ANPC_DinosaurAIController* NewLeader = PackMembers[0];
        NewLeader->BecomePackLeader();
        
        for (auto* Member : PackMembers)
        {
            if (Member != NewLeader)
            {
                NewLeader->AddPackMember(Member);
                Member->PackLeader = NewLeader;
            }
        }
    }
    
    PackMembers.Empty();
    bIsPackLeader = false;
    UpdateBlackboardValues();
}

void ANPC_DinosaurAIController::BecomePackLeader()
{
    bIsPackLeader = true;
    PackLeader = nullptr;
    UpdateBlackboardValues();
}

void ANPC_DinosaurAIController::AddPackMember(ANPC_DinosaurAIController* Member)
{
    if (Member && !PackMembers.Contains(Member))
    {
        PackMembers.Add(Member);
        Member->PackLeader = this;
        Member->bIsPackLeader = false;
    }
}

void ANPC_DinosaurAIController::RemovePackMember(ANPC_DinosaurAIController* Member)
{
    if (Member)
    {
        PackMembers.Remove(Member);
        Member->PackLeader = nullptr;
    }
}

void ANPC_DinosaurAIController::AlertPack(AActor* Threat)
{
    for (auto* Member : PackMembers)
    {
        if (Member && IsValid(Member))
        {
            Member->SetTarget(Threat);
            Member->SetDinosaurState(ENPC_DinosaurState::Chasing);
        }
    }
}

void ANPC_DinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Check if this is the player character
            if (Actor == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
            {
                float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
                
                if (Distance <= ChaseDistance && CurrentState != ENPC_DinosaurState::Attacking)
                {
                    StartChasing(Actor);
                }
                else if (Distance <= AttackDistance)
                {
                    StartAttacking();
                }
            }
        }
    }
}

void ANPC_DinosaurAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
        return;
        
    if (Stimulus.WasSuccessfullySensed())
    {
        if (Actor == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
        {
            SetTarget(Actor);
            LastPlayerSightTime = GetWorld()->GetTimeSeconds();
        }
    }
    else
    {
        // Lost sight of target
        if (Actor == CurrentTarget)
        {
            float TimeSinceLastSight = GetWorld()->GetTimeSeconds() - LastPlayerSightTime;
            if (TimeSinceLastSight > 10.0f) // Give up after 10 seconds
            {
                ReturnToHome();
            }
        }
    }
}

FVector ANPC_DinosaurAIController::GetRandomPatrolPoint()
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    FVector PatrolPoint = HomeLocation + (RandomDirection * RandomDistance);
    
    // Try to find a valid navigation point
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(PatrolPoint, NavLocation, FVector(1000.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return PatrolPoint;
}

bool ANPC_DinosaurAIController::IsLocationSafe(const FVector& Location) const
{
    // Basic safety check - avoid water, cliffs, etc.
    // This would be expanded based on the specific environment
    return true;
}

void ANPC_DinosaurAIController::HandleStateTransition(ENPC_DinosaurState NewState)
{
    // Handle any cleanup or initialization needed when changing states
    switch (NewState)
    {
        case ENPC_DinosaurState::Idle:
            ClearTarget();
            break;
            
        case ENPC_DinosaurState::Patrolling:
            ClearTarget();
            break;
            
        case ENPC_DinosaurState::Chasing:
            // Increase movement speed during chase
            if (GetPawn())
            {
                // This would be implemented based on the specific movement component
            }
            break;
            
        case ENPC_DinosaurState::Attacking:
            // Prepare for attack
            break;
            
        case ENPC_DinosaurState::Returning:
            ClearTarget();
            break;
            
        default:
            break;
    }
}