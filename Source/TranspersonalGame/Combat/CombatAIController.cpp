#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Set default values
    SightRadius = 1500.0f;
    LoseSightRadius = 1600.0f;
    PeripheralVisionAngleDegrees = 90.0f;
    HearingRange = 1200.0f;
    
    CurrentTarget = nullptr;
    LastPlayerSightTime = 0.0f;

    // Initialize tactical data
    TacticalData.CurrentState = ECombat_TacticalState::Patrol;
    TacticalData.ThreatLevel = 0.0f;
    TacticalData.EngagementDistance = 800.0f;
    TacticalData.bIsPackLeader = false;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Setup AI Perception
    if (AIPerceptionComponent)
    {
        // Configure sight
        UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
        if (SightConfig)
        {
            SightConfig->SightRadius = SightRadius;
            SightConfig->LoseSightRadius = LoseSightRadius;
            SightConfig->PeripheralVisionAngleDegrees = PeripheralVisionAngleDegrees;
            SightConfig->SetMaxAge(5.0f);
            SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
            SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
            SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
            SightConfig->DetectionByAffiliation.bDetectEnemies = true;

            AIPerceptionComponent->ConfigureSense(*SightConfig);
        }

        // Configure hearing
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

        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnTargetPerceptionUpdated);
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    }

    // Start behavior tree if available
    if (CombatBehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(CombatBehaviorTree);
    }
}

void ACombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    UpdateTacticalBehavior(DeltaTime);
}

void ACombatAIController::SetTacticalState(ECombat_TacticalState NewState)
{
    if (TacticalData.CurrentState != NewState)
    {
        TacticalData.CurrentState = NewState;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("TacticalState"), static_cast<uint8>(NewState));
        }

        // Log state change for debugging
        UE_LOG(LogTemp, Warning, TEXT("Combat AI State Changed to: %d"), static_cast<int32>(NewState));
    }
}

void ACombatAIController::UpdateThreatLevel(float NewThreatLevel)
{
    TacticalData.ThreatLevel = FMath::Clamp(NewThreatLevel, 0.0f, 100.0f);
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("ThreatLevel"), TacticalData.ThreatLevel);
    }

    // Adjust behavior based on threat level
    if (TacticalData.ThreatLevel > 80.0f)
    {
        SetTacticalState(ECombat_TacticalState::Retreat);
    }
    else if (TacticalData.ThreatLevel > 50.0f)
    {
        SetTacticalState(ECombat_TacticalState::Engage);
    }
    else if (TacticalData.ThreatLevel > 20.0f)
    {
        SetTacticalState(ECombat_TacticalState::Investigate);
    }
}

void ACombatAIController::SetPackLeader(bool bIsLeader)
{
    TacticalData.bIsPackLeader = bIsLeader;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("IsPackLeader"), bIsLeader);
    }
}

FVector ACombatAIController::GetFlankingPosition(const FVector& PlayerLocation, float FlankDistance)
{
    if (!GetPawn())
    {
        return FVector::ZeroVector;
    }

    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector DirectionToPlayer = (PlayerLocation - MyLocation).GetSafeNormal();
    
    // Calculate perpendicular vector for flanking
    FVector FlankDirection = FVector::CrossProduct(DirectionToPlayer, FVector::UpVector).GetSafeNormal();
    
    // Randomize flanking side
    if (FMath::RandBool())
    {
        FlankDirection *= -1.0f;
    }
    
    FVector FlankPosition = PlayerLocation + (FlankDirection * FlankDistance);
    
    // Ensure position is on navmesh
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(FlankPosition, NavLocation, FVector(500.0f, 500.0f, 500.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return FlankPosition;
}

bool ACombatAIController::CanSeePlayer() const
{
    if (!AIPerceptionComponent || !CurrentTarget)
    {
        return false;
    }

    FActorPerceptionBlueprintInfo PerceptionInfo;
    return AIPerceptionComponent->GetActorsPerception(CurrentTarget, PerceptionInfo) && 
           PerceptionInfo.LastSensedStimuli.Num() > 0 &&
           PerceptionInfo.LastSensedStimuli[0].WasSuccessfullySensed();
}

void ACombatAIController::CoordinatePackAttack()
{
    if (!TacticalData.bIsPackLeader || !CurrentTarget)
    {
        return;
    }

    // Find pack members in range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatAIController::StaticClass(), FoundActors);

    PackMembers.Empty();
    for (AActor* Actor : FoundActors)
    {
        ACombatAIController* OtherController = Cast<ACombatAIController>(Actor);
        if (OtherController && OtherController != this && 
            FVector::Dist(GetPawn()->GetActorLocation(), OtherController->GetPawn()->GetActorLocation()) < 2000.0f)
        {
            PackMembers.Add(OtherController);
        }
    }

    // Assign roles to pack members
    for (int32 i = 0; i < PackMembers.Num(); i++)
    {
        if (PackMembers[i])
        {
            if (i == 0)
            {
                // First member flanks left
                PackMembers[i]->SetTacticalState(ECombat_TacticalState::Flank);
                FVector FlankPos = GetFlankingPosition(CurrentTarget->GetActorLocation(), 600.0f);
                if (PackMembers[i]->BlackboardComponent)
                {
                    PackMembers[i]->BlackboardComponent->SetValueAsVector(TEXT("FlankPosition"), FlankPos);
                }
            }
            else if (i == 1)
            {
                // Second member flanks right
                PackMembers[i]->SetTacticalState(ECombat_TacticalState::Flank);
                FVector FlankPos = GetFlankingPosition(CurrentTarget->GetActorLocation(), -600.0f);
                if (PackMembers[i]->BlackboardComponent)
                {
                    PackMembers[i]->BlackboardComponent->SetValueAsVector(TEXT("FlankPosition"), FlankPos);
                }
            }
            else
            {
                // Others engage directly
                PackMembers[i]->SetTacticalState(ECombat_TacticalState::Engage);
            }
        }
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            CurrentTarget = Actor;
            TacticalData.LastKnownPlayerLocation = Actor->GetActorLocation();
            LastPlayerSightTime = GetWorld()->GetTimeSeconds();
            
            if (BlackboardComponent)
            {
                BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), CurrentTarget);
                BlackboardComponent->SetValueAsVector(TEXT("LastKnownPlayerLocation"), TacticalData.LastKnownPlayerLocation);
            }
            
            // Increase threat level when player is detected
            UpdateThreatLevel(TacticalData.ThreatLevel + 25.0f);
            break;
        }
    }
}

void ACombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor || !Actor->IsA<ACharacter>())
    {
        return;
    }

    if (Stimulus.WasSuccessfullySensed())
    {
        CurrentTarget = Actor;
        TacticalData.LastKnownPlayerLocation = Actor->GetActorLocation();
        LastPlayerSightTime = GetWorld()->GetTimeSeconds();
        
        // Player spotted - coordinate pack if leader
        if (TacticalData.bIsPackLeader)
        {
            CoordinatePackAttack();
        }
    }
    else
    {
        // Lost sight - investigate last known position
        if (TacticalData.CurrentState == ECombat_TacticalState::Engage)
        {
            SetTacticalState(ECombat_TacticalState::Investigate);
        }
    }
}

void ACombatAIController::UpdateTacticalBehavior(float DeltaTime)
{
    if (!GetPawn() || !CurrentTarget)
    {
        return;
    }

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    float TimeSinceLastSight = GetWorld()->GetTimeSeconds() - LastPlayerSightTime;

    // Update threat level based on distance and time
    if (CanSeePlayer())
    {
        if (DistanceToTarget < TacticalData.EngagementDistance)
        {
            UpdateThreatLevel(TacticalData.ThreatLevel + (DeltaTime * 10.0f));
        }
    }
    else if (TimeSinceLastSight > 5.0f)
    {
        UpdateThreatLevel(TacticalData.ThreatLevel - (DeltaTime * 5.0f));
    }

    // State-specific behavior updates
    switch (TacticalData.CurrentState)
    {
        case ECombat_TacticalState::Patrol:
            if (CanSeePlayer() && DistanceToTarget < 1200.0f)
            {
                SetTacticalState(ECombat_TacticalState::Investigate);
            }
            break;

        case ECombat_TacticalState::Investigate:
            if (CanSeePlayer() && DistanceToTarget < TacticalData.EngagementDistance)
            {
                SetTacticalState(ECombat_TacticalState::Engage);
            }
            else if (TimeSinceLastSight > 10.0f)
            {
                SetTacticalState(ECombat_TacticalState::Patrol);
            }
            break;

        case ECombat_TacticalState::Engage:
            if (DistanceToTarget > TacticalData.EngagementDistance * 1.5f)
            {
                SetTacticalState(ECombat_TacticalState::Investigate);
            }
            break;
    }
}

void ACombatAIController::CalculateOptimalPosition()
{
    // Implementation for calculating optimal tactical position
    // This would involve analyzing terrain, cover, and pack coordination
}

void ACombatAIController::CommunicateWithPack()
{
    // Implementation for pack communication
    // This would handle sharing information between pack members
}