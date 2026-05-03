#include "CombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

ACombatAIController::ACombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure sight
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 1600.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    
    // Configure hearing
    HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1200.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    AIPerceptionComponent->ConfigureSense(*HearingConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Blackboard and Behavior Tree components
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

    // Initialize default values
    CurrentTarget = nullptr;
    LastKnownTargetLocation = FVector::ZeroVector;
    LastTargetSightTime = 0.0f;
    bIsInCombat = false;
    LastPackCoordinationTime = 0.0f;
    CurrentAIState = ECombat_AIState::Patrolling;
    TacticalRole = ECombat_TacticalRole::Flanker;
}

void ACombatAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombatAIController::OnPerceptionUpdated);
    }

    // Start behavior tree if available
    if (BlackboardAsset && BehaviorTree)
    {
        UseBlackboard(BlackboardAsset);
        RunBehaviorTree(BehaviorTree);
    }
}

void ACombatAIController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);
    
    if (InPawn)
    {
        UpdateBlackboardValues();
    }
}

void ACombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        // Check if this is a player character
        if (Actor->IsA<APawn>() && Actor->GetClass()->GetName().Contains("Character"))
        {
            FActorPerceptionBlueprintInfo PerceptionInfo;
            if (AIPerceptionComponent->GetActorsPerception(Actor, PerceptionInfo))
            {
                bool bCanSeeActor = false;
                for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
                {
                    if (Stimulus.Type == UAISense::GetSenseID<UAISense_Sight>() && Stimulus.WasSuccessfullySensed())
                    {
                        bCanSeeActor = true;
                        break;
                    }
                }

                if (bCanSeeActor)
                {
                    HandleTargetAcquired(Actor);
                }
                else if (CurrentTarget == Actor)
                {
                    HandleTargetLost();
                }
            }
        }
    }
}

void ACombatAIController::SetAIState(ECombat_AIState NewState)
{
    if (CurrentAIState != NewState)
    {
        CurrentAIState = NewState;
        UpdateBlackboardValues();

        // State-specific logic
        switch (NewState)
        {
        case ECombat_AIState::Investigating:
            SetFocus(nullptr);
            break;
        case ECombat_AIState::Engaging:
            bIsInCombat = true;
            CoordinateWithPack();
            break;
        case ECombat_AIState::Retreating:
            bIsInCombat = false;
            SetFocus(nullptr);
            break;
        default:
            break;
        }
    }
}

void ACombatAIController::SetTacticalRole(ECombat_TacticalRole NewRole)
{
    TacticalRole = NewRole;
    UpdateBlackboardValues();
}

void ACombatAIController::SetTarget(AActor* NewTarget)
{
    if (CurrentTarget != NewTarget)
    {
        CurrentTarget = NewTarget;
        if (NewTarget)
        {
            LastKnownTargetLocation = NewTarget->GetActorLocation();
            LastTargetSightTime = GetWorld()->GetTimeSeconds();
            SetFocus(NewTarget);
        }
        else
        {
            SetFocus(nullptr);
        }
        UpdateBlackboardValues();
    }
}

AActor* ACombatAIController::GetCurrentTarget() const
{
    return CurrentTarget;
}

FVector ACombatAIController::CalculateFlankingPosition(AActor* Target)
{
    if (!Target || !GetPawn()) return FVector::ZeroVector;

    FVector TargetLocation = Target->GetActorLocation();
    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Calculate direction from target to me
    FVector DirectionToMe = (MyLocation - TargetLocation).GetSafeNormal();
    
    // Create perpendicular vector for flanking
    FVector FlankingDirection = FVector::CrossProduct(DirectionToMe, FVector::UpVector).GetSafeNormal();
    
    // Randomly choose left or right flanking
    if (FMath::RandBool())
    {
        FlankingDirection *= -1.0f;
    }
    
    // Calculate flanking position
    FVector FlankingPosition = TargetLocation + (FlankingDirection * FlankingDistance);
    
    return FlankingPosition;
}

bool ACombatAIController::ShouldRetreat() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;

    // Check health (assuming pawn has health component or interface)
    // For now, use a simple distance check and pack status
    TArray<ACombatAIController*> PackMembers = GetNearbyPackMembers();
    
    // Retreat if isolated or outnumbered
    if (PackMembers.Num() <= 1 && CurrentTarget)
    {
        return true;
    }

    return false;
}

bool ACombatAIController::CanAttackTarget() const
{
    if (!CurrentTarget || !GetPawn()) return false;

    float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    return DistanceToTarget <= AttackRange;
}

void ACombatAIController::CoordinateWithPack()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPackCoordinationTime < 1.0f) return; // Throttle coordination

    TArray<ACombatAIController*> NearbyPack = GetNearbyPackMembers();
    
    // Simple pack coordination logic
    if (NearbyPack.Num() >= 2 && CurrentTarget)
    {
        // Assign roles based on position
        for (int32 i = 0; i < NearbyPack.Num(); ++i)
        {
            if (NearbyPack[i] && NearbyPack[i] != this)
            {
                ECombat_TacticalRole AssignedRole = (i % 2 == 0) ? ECombat_TacticalRole::Flanker : ECombat_TacticalRole::Distractor;
                NearbyPack[i]->SetTacticalRole(AssignedRole);
            }
        }
    }

    LastPackCoordinationTime = CurrentTime;
}

TArray<ACombatAIController*> ACombatAIController::GetNearbyPackMembers(float SearchRadius) const
{
    TArray<ACombatAIController*> PackMembers;
    
    if (!GetWorld() || !GetPawn()) return PackMembers;

    FVector MyLocation = GetPawn()->GetActorLocation();
    
    // Find all AI controllers in range
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACombatAIController::StaticClass(), FoundActors);
    
    for (AActor* Actor : FoundActors)
    {
        ACombatAIController* OtherController = Cast<ACombatAIController>(Actor);
        if (OtherController && OtherController != this && OtherController->GetPawn())
        {
            float Distance = FVector::Dist(MyLocation, OtherController->GetPawn()->GetActorLocation());
            if (Distance <= SearchRadius)
            {
                PackMembers.Add(OtherController);
            }
        }
    }
    
    return PackMembers;
}

void ACombatAIController::UpdateBlackboardValues()
{
    if (!BlackboardComponent) return;

    // Update blackboard with current state
    BlackboardComponent->SetValueAsEnum(TEXT("AIState"), static_cast<uint8>(CurrentAIState));
    BlackboardComponent->SetValueAsEnum(TEXT("TacticalRole"), static_cast<uint8>(TacticalRole));
    
    if (CurrentTarget)
    {
        BlackboardComponent->SetValueAsObject(TEXT("Target"), CurrentTarget);
        BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), CurrentTarget->GetActorLocation());
    }
    else
    {
        BlackboardComponent->SetValueAsObject(TEXT("Target"), nullptr);
        BlackboardComponent->SetValueAsVector(TEXT("LastKnownTargetLocation"), LastKnownTargetLocation);
    }
    
    BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), bIsInCombat);
    BlackboardComponent->SetValueAsBool(TEXT("CanAttack"), CanAttackTarget());
    BlackboardComponent->SetValueAsBool(TEXT("ShouldRetreat"), ShouldRetreat());
}

void ACombatAIController::HandleTargetLost()
{
    if (CurrentTarget)
    {
        LastKnownTargetLocation = CurrentTarget->GetActorLocation();
    }
    
    SetTarget(nullptr);
    SetAIState(ECombat_AIState::Investigating);
}

void ACombatAIController::HandleTargetAcquired(AActor* NewTarget)
{
    SetTarget(NewTarget);
    SetAIState(ECombat_AIState::Engaging);
}

bool ACombatAIController::IsTargetInRange(AActor* Target, float Range) const
{
    if (!Target || !GetPawn()) return false;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= Range;
}

FVector ACombatAIController::GetRandomFlankingPosition(AActor* Target) const
{
    if (!Target || !GetPawn()) return FVector::ZeroVector;

    FVector TargetLocation = Target->GetActorLocation();
    
    // Generate random angle around target
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    FVector RandomDirection = FVector(FMath::Cos(FMath::DegreesToRadians(RandomAngle)), 
                                     FMath::Sin(FMath::DegreesToRadians(RandomAngle)), 
                                     0.0f);
    
    return TargetLocation + (RandomDirection * FlankingDistance);
}