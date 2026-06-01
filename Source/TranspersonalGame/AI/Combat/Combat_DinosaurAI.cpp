#include "Combat_DinosaurAI.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AISense_Hearing.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "NavigationSystem.h"

ACombat_DinosaurAI::ACombat_DinosaurAI()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Initialize state
    CurrentState = ECombat_DinosaurState::Idle;
    CurrentTarget = nullptr;
    PatrolPoint = FVector::ZeroVector;
    StateChangeTime = 0.0f;

    // Default settings
    SightRange = 2000.0f;
    HearingRange = 1500.0f;
    AttackRange = 300.0f;
    FleeRange = 500.0f;
    PatrolRadius = 1000.0f;
    AggressionLevel = 0.5f;
    FearLevel = 0.3f;
    bIsPackHunter = false;
    bIsTerritorial = true;
}

void ACombat_DinosaurAI::BeginPlay()
{
    Super::BeginPlay();

    InitializeAIPerception();
    SetupBehaviorTree();

    // Start with idle state
    CurrentState = ECombat_DinosaurState::Idle;
    StateChangeTime = GetWorld()->GetTimeSeconds();

    // Set initial patrol point
    if (GetPawn())
    {
        PatrolPoint = GetPawn()->GetActorLocation();
    }
}

void ACombat_DinosaurAI::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateThreatAssessment();
    UpdateCombatState();
    UpdateBlackboard();
}

void ACombat_DinosaurAI::InitializeAIPerception()
{
    if (!AIPerceptionComponent)
    {
        return;
    }

    // Configure sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = SightRange;
        SightConfig->LoseSightRadius = SightRange + 200.0f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Configure hearing sense
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    if (HearingConfig)
    {
        HearingConfig->HearingRange = HearingRange;
        HearingConfig->SetMaxAge(3.0f);
        HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
        HearingConfig->DetectionByAffiliation.bDetectFriendlies = false;
        HearingConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*HearingConfig);
    }

    // Set dominant sense
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Bind perception events
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurAI::OnTargetPerceptionUpdated);
}

void ACombat_DinosaurAI::SetupBehaviorTree()
{
    if (BlackboardComponent && DinosaurBlackboard)
    {
        BlackboardComponent->InitializeBlackboard(*DinosaurBlackboard);
    }

    // Start behavior tree based on dinosaur type
    if (BehaviorTreeComponent && TRexBehaviorTree)
    {
        BehaviorTreeComponent->StartTree(*TRexBehaviorTree);
    }
}

void ACombat_DinosaurAI::SetDinosaurType(EDinosaurSpecies DinosaurType)
{
    switch (DinosaurType)
    {
        case EDinosaurSpecies::TRex:
            AggressionLevel = 0.9f;
            FearLevel = 0.1f;
            AttackRange = 400.0f;
            SightRange = 2500.0f;
            bIsPackHunter = false;
            bIsTerritorial = true;
            if (BehaviorTreeComponent && TRexBehaviorTree)
            {
                BehaviorTreeComponent->StartTree(*TRexBehaviorTree);
            }
            break;

        case EDinosaurSpecies::Velociraptor:
            AggressionLevel = 0.8f;
            FearLevel = 0.4f;
            AttackRange = 200.0f;
            SightRange = 2000.0f;
            bIsPackHunter = true;
            bIsTerritorial = false;
            if (BehaviorTreeComponent && VelociraptorBehaviorTree)
            {
                BehaviorTreeComponent->StartTree(*VelociraptorBehaviorTree);
            }
            break;

        case EDinosaurSpecies::Triceratops:
            AggressionLevel = 0.4f;
            FearLevel = 0.3f;
            AttackRange = 300.0f;
            SightRange = 1800.0f;
            bIsPackHunter = false;
            bIsTerritorial = true;
            if (BehaviorTreeComponent && TriceratopsBehaviorTree)
            {
                BehaviorTreeComponent->StartTree(*TriceratopsBehaviorTree);
            }
            break;

        case EDinosaurSpecies::Brachiosaurus:
            AggressionLevel = 0.2f;
            FearLevel = 0.6f;
            AttackRange = 500.0f;
            SightRange = 2200.0f;
            bIsPackHunter = false;
            bIsTerritorial = false;
            if (BehaviorTreeComponent && BrachiosaurusBehaviorTree)
            {
                BehaviorTreeComponent->StartTree(*BrachiosaurusBehaviorTree);
            }
            break;

        default:
            break;
    }

    // Reinitialize perception with new ranges
    InitializeAIPerception();
}

void ACombat_DinosaurAI::UpdateThreatAssessment()
{
    if (!AIPerceptionComponent)
    {
        return;
    }

    TArray<AActor*> PerceivedActors;
    AIPerceptionComponent->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    // Clear old threats
    DetectedThreats.Empty();

    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (AActor* Actor : PerceivedActors)
    {
        if (!Actor || Actor == GetPawn())
        {
            continue;
        }

        FCombat_ThreatInfo ThreatInfo;
        ThreatInfo.ThreatActor = Actor;
        ThreatInfo.ThreatLevel = CalculateThreatLevel(Actor);
        ThreatInfo.LastSeenTime = CurrentTime;
        ThreatInfo.LastKnownLocation = Actor->GetActorLocation();
        
        if (GetPawn())
        {
            ThreatInfo.DistanceToThreat = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
        }

        if (ThreatInfo.ThreatLevel > ECombat_ThreatLevel::None)
        {
            DetectedThreats.Add(ThreatInfo);
        }
    }

    ProcessThreatList();
}

ECombat_ThreatLevel ACombat_DinosaurAI::CalculateThreatLevel(AActor* ThreatActor)
{
    if (!ThreatActor || !GetPawn())
    {
        return ECombat_ThreatLevel::None;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), ThreatActor->GetActorLocation());
    ECombat_ThreatLevel ThreatLevel = ECombat_ThreatLevel::None;

    // Check if it's a player character
    if (ThreatActor->IsA<ACharacter>() && ThreatActor->GetClass()->GetName().Contains("Character"))
    {
        if (Distance < AttackRange)
        {
            ThreatLevel = ECombat_ThreatLevel::Extreme;
        }
        else if (Distance < FleeRange)
        {
            ThreatLevel = ECombat_ThreatLevel::High;
        }
        else if (Distance < SightRange * 0.5f)
        {
            ThreatLevel = ECombat_ThreatLevel::Medium;
        }
        else
        {
            ThreatLevel = ECombat_ThreatLevel::Low;
        }
    }

    // Modify threat level based on dinosaur personality
    if (AggressionLevel > 0.7f)
    {
        // Aggressive dinosaurs see higher threats as opportunities
        if (ThreatLevel == ECombat_ThreatLevel::High)
        {
            ThreatLevel = ECombat_ThreatLevel::Medium;
        }
    }
    else if (FearLevel > 0.6f)
    {
        // Fearful dinosaurs escalate threat levels
        if (ThreatLevel == ECombat_ThreatLevel::Medium)
        {
            ThreatLevel = ECombat_ThreatLevel::High;
        }
        else if (ThreatLevel == ECombat_ThreatLevel::Low)
        {
            ThreatLevel = ECombat_ThreatLevel::Medium;
        }
    }

    return ThreatLevel;
}

void ACombat_DinosaurAI::ProcessThreatList()
{
    if (DetectedThreats.Num() == 0)
    {
        CurrentTarget = nullptr;
        if (CurrentState == ECombat_DinosaurState::Hunting || CurrentState == ECombat_DinosaurState::Attacking)
        {
            CurrentState = ECombat_DinosaurState::Patrolling;
            StateChangeTime = GetWorld()->GetTimeSeconds();
        }
        return;
    }

    // Find the highest priority threat
    AActor* BestTarget = FindBestTarget();
    
    if (BestTarget != CurrentTarget)
    {
        CurrentTarget = BestTarget;
        StateChangeTime = GetWorld()->GetTimeSeconds();

        if (CurrentTarget)
        {
            ECombat_ThreatLevel TargetThreatLevel = CalculateThreatLevel(CurrentTarget);
            
            if (TargetThreatLevel >= ECombat_ThreatLevel::High && AggressionLevel > FearLevel)
            {
                EngageCombat(CurrentTarget);
            }
            else if (TargetThreatLevel >= ECombat_ThreatLevel::Medium && FearLevel > AggressionLevel)
            {
                FleeFromThreat(CurrentTarget);
            }
            else
            {
                CurrentState = ECombat_DinosaurState::Hunting;
            }
        }
    }
}

AActor* ACombat_DinosaurAI::FindBestTarget()
{
    AActor* BestTarget = nullptr;
    float HighestPriority = 0.0f;

    for (const FCombat_ThreatInfo& Threat : DetectedThreats)
    {
        if (!Threat.ThreatActor)
        {
            continue;
        }

        float Priority = 0.0f;

        // Base priority on threat level
        switch (Threat.ThreatLevel)
        {
            case ECombat_ThreatLevel::Extreme:
                Priority = 100.0f;
                break;
            case ECombat_ThreatLevel::High:
                Priority = 75.0f;
                break;
            case ECombat_ThreatLevel::Medium:
                Priority = 50.0f;
                break;
            case ECombat_ThreatLevel::Low:
                Priority = 25.0f;
                break;
            default:
                Priority = 0.0f;
                break;
        }

        // Modify priority based on distance (closer = higher priority)
        if (Threat.DistanceToThreat > 0.0f)
        {
            Priority *= (1.0f - (Threat.DistanceToThreat / SightRange));
        }

        // Aggressive dinosaurs prefer closer targets
        if (AggressionLevel > 0.6f)
        {
            Priority *= (1.0f + AggressionLevel);
        }

        if (Priority > HighestPriority)
        {
            HighestPriority = Priority;
            BestTarget = Threat.ThreatActor;
        }
    }

    return BestTarget;
}

void ACombat_DinosaurAI::EngageCombat(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    CurrentTarget = Target;
    CurrentState = ECombat_DinosaurState::Attacking;
    StateChangeTime = GetWorld()->GetTimeSeconds();

    // Move towards target
    MoveToActor(Target, AttackRange * 0.8f);
}

void ACombat_DinosaurAI::FleeFromThreat(AActor* ThreatActor)
{
    if (!ThreatActor || !GetPawn())
    {
        return;
    }

    CurrentState = ECombat_DinosaurState::Fleeing;
    StateChangeTime = GetWorld()->GetTimeSeconds();

    // Calculate flee direction (opposite from threat)
    FVector FleeDirection = GetPawn()->GetActorLocation() - ThreatActor->GetActorLocation();
    FleeDirection.Normalize();

    FVector FleeLocation = GetPawn()->GetActorLocation() + (FleeDirection * FleeRange * 2.0f);

    // Find navigable location
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(FleeLocation, NavLocation, FVector(500.0f, 500.0f, 500.0f)))
        {
            MoveToLocation(NavLocation.Location);
        }
    }
}

void ACombat_DinosaurAI::StartPatrolling()
{
    CurrentState = ECombat_DinosaurState::Patrolling;
    StateChangeTime = GetWorld()->GetTimeSeconds();

    FVector NewPatrolPoint = GetRandomPatrolPoint();
    MoveToLocation(NewPatrolPoint);
}

FVector ACombat_DinosaurAI::GetRandomPatrolPoint()
{
    if (!GetPawn())
    {
        return FVector::ZeroVector;
    }

    FVector RandomDirection = UKismetMathLibrary::RandomUnitVector();
    RandomDirection.Z = 0.0f; // Keep on ground level
    RandomDirection.Normalize();

    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    FVector NewPatrolPoint = PatrolPoint + (RandomDirection * RandomDistance);

    // Try to find navigable location
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSystem)
    {
        FNavLocation NavLocation;
        if (NavSystem->ProjectPointToNavigation(NewPatrolPoint, NavLocation, FVector(1000.0f, 1000.0f, 500.0f)))
        {
            return NavLocation.Location;
        }
    }

    return NewPatrolPoint;
}

bool ACombat_DinosaurAI::IsInAttackRange(AActor* Target)
{
    if (!Target || !GetPawn())
    {
        return false;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

void ACombat_DinosaurAI::PerformAttack()
{
    if (!CurrentTarget || !IsInAttackRange(CurrentTarget))
    {
        return;
    }

    // Trigger attack animation/behavior
    // This would typically trigger an animation montage or behavior tree task
    UE_LOG(LogTemp, Log, TEXT("Dinosaur %s performing attack on %s"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
           CurrentTarget ? *CurrentTarget->GetName() : TEXT("Unknown"));

    // Face the target
    if (GetPawn())
    {
        FVector LookDirection = CurrentTarget->GetActorLocation() - GetPawn()->GetActorLocation();
        LookDirection.Z = 0.0f;
        LookDirection.Normalize();

        FRotator NewRotation = LookDirection.Rotation();
        GetPawn()->SetActorRotation(NewRotation);
    }
}

void ACombat_DinosaurAI::UpdateCombatState()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    float TimeSinceStateChange = CurrentTime - StateChangeTime;

    switch (CurrentState)
    {
        case ECombat_DinosaurState::Idle:
            if (TimeSinceStateChange > 3.0f)
            {
                StartPatrolling();
            }
            break;

        case ECombat_DinosaurState::Patrolling:
            if (HasReachedDestination())
            {
                CurrentState = ECombat_DinosaurState::Idle;
                StateChangeTime = CurrentTime;
            }
            break;

        case ECombat_DinosaurState::Attacking:
            if (CurrentTarget && IsInAttackRange(CurrentTarget))
            {
                PerformAttack();
            }
            else if (!CurrentTarget)
            {
                CurrentState = ECombat_DinosaurState::Patrolling;
                StateChangeTime = CurrentTime;
            }
            break;

        case ECombat_DinosaurState::Fleeing:
            if (TimeSinceStateChange > 5.0f || HasReachedDestination())
            {
                CurrentState = ECombat_DinosaurState::Patrolling;
                StateChangeTime = CurrentTime;
            }
            break;

        default:
            break;
    }
}

void ACombat_DinosaurAI::UpdateBlackboard()
{
    if (!BlackboardComponent)
    {
        return;
    }

    // Update blackboard with current state information
    BlackboardComponent->SetValueAsEnum(TEXT("CurrentState"), static_cast<uint8>(CurrentState));
    
    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
    }
    else
    {
        BlackboardComponent->ClearValue(TEXT("TargetActor"));
    }

    BlackboardComponent->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
    BlackboardComponent->SetValueAsFloat(TEXT("FearLevel"), FearLevel);
    BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), CurrentTarget != nullptr);
    BlackboardComponent->SetValueAsVector(TEXT("PatrolPoint"), PatrolPoint);
}

void ACombat_DinosaurAI::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    // This function can be called from Blueprint or other systems
    UpdateThreatAssessment();
}

void ACombat_DinosaurAI::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s detected %s"), 
               GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
               *Actor->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Dinosaur %s lost sight of %s"), 
               GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
               *Actor->GetName());
    }

    // Trigger threat assessment update
    UpdateThreatAssessment();
}