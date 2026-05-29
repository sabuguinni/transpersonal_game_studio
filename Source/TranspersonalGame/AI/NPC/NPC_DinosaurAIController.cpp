#include "NPC_DinosaurAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"

ANPC_DinosaurAIController::ANPC_DinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    CurrentState = ENPC_DinosaurState::Idle;
    CurrentTarget = nullptr;
    LastUpdateTime = 0.0f;
    
    DinosaurStats.Health = 100.0f;
    DinosaurStats.Hunger = 50.0f;
    DinosaurStats.Energy = 100.0f;
    DinosaurStats.Aggression = 30.0f;
    DinosaurStats.Fear = 0.0f;
}

void ANPC_DinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    SetupAIPerception();
    
    if (GetPawn())
    {
        HomeLocation = GetPawn()->GetActorLocation();
    }

    if (BehaviorTree && BlackboardAsset)
    {
        RunBehaviorTree(BehaviorTree);
        GetBlackboardComponent()->InitializeBlackboard(*BlackboardAsset);
        UpdateBlackboard();
    }

    SetDinosaurState(ENPC_DinosaurState::Patrolling);
}

void ANPC_DinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateStats(DeltaTime);
    
    if (GetWorld()->GetTimeSeconds() - LastUpdateTime >= StatUpdateInterval)
    {
        UpdateBlackboard();
        LastUpdateTime = GetWorld()->GetTimeSeconds();
    }
}

void ANPC_DinosaurAIController::SetupAIPerception()
{
    if (AIPerceptionComponent)
    {
        // Setup sight perception
        UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
        if (SightConfig)
        {
            SightConfig->SightRadius = SightRadius;
            SightConfig->LoseSightRadius = SightRadius + 500.0f;
            SightConfig->PeripheralVisionAngleDegrees = 90.0f;
            SightConfig->SetMaxAge(5.0f);
            SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
            SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
            SightConfig->DetectionByAffiliation.bDetectEnemies = true;
            
            AIPerceptionComponent->ConfigureSense(*SightConfig);
        }

        // Setup hearing perception
        UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
        if (HearingConfig)
        {
            HearingConfig->HearingRange = HearingRadius;
            HearingConfig->SetMaxAge(3.0f);
            HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
            HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
            HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
            
            AIPerceptionComponent->ConfigureSense(*HearingConfig);
        }

        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ANPC_DinosaurAIController::OnPerceptionUpdated);
    }
}

void ANPC_DinosaurAIController::SetDinosaurState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        UpdateBlackboard();
    }
}

void ANPC_DinosaurAIController::UpdateStats(float DeltaTime)
{
    // Hunger increases over time
    DinosaurStats.Hunger = FMath::Clamp(DinosaurStats.Hunger + (DeltaTime * 0.5f), 0.0f, 100.0f);
    
    // Energy decreases based on activity
    float EnergyDrain = 0.2f;
    if (CurrentState == ENPC_DinosaurState::Hunting || CurrentState == ENPC_DinosaurState::Fleeing)
    {
        EnergyDrain = 1.0f;
    }
    else if (CurrentState == ENPC_DinosaurState::Patrolling)
    {
        EnergyDrain = 0.5f;
    }
    
    DinosaurStats.Energy = FMath::Clamp(DinosaurStats.Energy - (DeltaTime * EnergyDrain), 0.0f, 100.0f);
    
    // Health decreases if hunger is too high
    if (DinosaurStats.Hunger > 80.0f)
    {
        DinosaurStats.Health = FMath::Clamp(DinosaurStats.Health - (DeltaTime * 2.0f), 0.0f, 100.0f);
    }
    
    // Fear decreases over time
    DinosaurStats.Fear = FMath::Clamp(DinosaurStats.Fear - (DeltaTime * 5.0f), 0.0f, 100.0f);
}

void ANPC_DinosaurAIController::UpdateBlackboard()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsEnum(FName("CurrentState"), static_cast<uint8>(CurrentState));
        BlackboardComponent->SetValueAsFloat(FName("Health"), DinosaurStats.Health);
        BlackboardComponent->SetValueAsFloat(FName("Hunger"), DinosaurStats.Hunger);
        BlackboardComponent->SetValueAsFloat(FName("Energy"), DinosaurStats.Energy);
        BlackboardComponent->SetValueAsFloat(FName("Aggression"), DinosaurStats.Aggression);
        BlackboardComponent->SetValueAsFloat(FName("Fear"), DinosaurStats.Fear);
        BlackboardComponent->SetValueAsVector(FName("HomeLocation"), HomeLocation);
        
        if (CurrentTarget)
        {
            BlackboardComponent->SetValueAsObject(FName("TargetActor"), CurrentTarget);
        }
    }
}

bool ANPC_DinosaurAIController::CanSeeTarget(AActor* Target) const
{
    if (!Target || !AIPerceptionComponent)
    {
        return false;
    }

    FActorPerceptionBlueprintInfo Info;
    return AIPerceptionComponent->GetActorsPerception(Target, Info) && Info.LastSensedStimuli.Num() > 0;
}

void ANPC_DinosaurAIController::StartPatrol()
{
    SetDinosaurState(ENPC_DinosaurState::Patrolling);
    CurrentTarget = nullptr;
    
    FVector PatrolPoint = GetRandomPatrolPoint();
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(FName("PatrolTarget"), PatrolPoint);
    }
}

void ANPC_DinosaurAIController::StartHunting(AActor* Target)
{
    if (Target)
    {
        SetDinosaurState(ENPC_DinosaurState::Hunting);
        CurrentTarget = Target;
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(FName("TargetActor"), Target);
        }
    }
}

void ANPC_DinosaurAIController::StartFleeing(AActor* Threat)
{
    if (Threat)
    {
        SetDinosaurState(ENPC_DinosaurState::Fleeing);
        DinosaurStats.Fear = FMath::Clamp(DinosaurStats.Fear + 30.0f, 0.0f, 100.0f);
        
        // Calculate flee direction away from threat
        FVector FleeDirection = (GetPawn()->GetActorLocation() - Threat->GetActorLocation()).GetSafeNormal();
        FVector FleeTarget = GetPawn()->GetActorLocation() + (FleeDirection * FleeDistance);
        
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsVector(FName("FleeTarget"), FleeTarget);
        }
    }
}

FVector ANPC_DinosaurAIController::GetRandomPatrolPoint() const
{
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0.0f;
    RandomDirection.Normalize();
    
    float RandomDistance = FMath::RandRange(PatrolRadius * 0.3f, PatrolRadius);
    FVector PatrolPoint = HomeLocation + (RandomDirection * RandomDistance);
    
    // Try to find a valid navigation point
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (NavSys)
    {
        FNavLocation NavLocation;
        if (NavSys->ProjectPointToNavigation(PatrolPoint, NavLocation, FVector(500.0f, 500.0f, 1000.0f)))
        {
            return NavLocation.Location;
        }
    }
    
    return PatrolPoint;
}

void ANPC_DinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor)
        {
            continue;
        }

        // Check if this is a player character
        if (Actor->IsA<ACharacter>() && Actor->IsA<APawn>())
        {
            APawn* PlayerPawn = Cast<APawn>(Actor);
            if (PlayerPawn && PlayerPawn->IsPlayerControlled())
            {
                float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), Actor->GetActorLocation());
                
                // Different behavior based on dinosaur type and stats
                if (DinosaurStats.Aggression > 50.0f && DistanceToPlayer < AttackRange * 2.0f)
                {
                    StartHunting(Actor);
                }
                else if (DinosaurStats.Fear > 30.0f || DinosaurStats.Health < 30.0f)
                {
                    StartFleeing(Actor);
                }
                else if (CurrentState == ENPC_DinosaurState::Patrolling)
                {
                    // Just watch the player but don't engage
                    if (BlackboardComponent)
                    {
                        BlackboardComponent->SetValueAsObject(FName("ObservedActor"), Actor);
                    }
                }
            }
        }
    }
}