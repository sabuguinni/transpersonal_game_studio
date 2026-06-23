#include "TRexBehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

ATRexAIController::ATRexAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    CurrentAIState = 0; // Start in patrol

    // Set up AI Perception
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    // Sight config
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = SightRadius;
    SightConfig->LoseSightRadius = SightLoseSightRadius;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*SightConfig);

    // Hearing config
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = HearingRadius;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
}

void ATRexAIController::BeginPlay()
{
    Super::BeginPlay();

    // Store patrol origin at spawn location
    if (GetPawn())
    {
        PatrolOrigin = GetPawn()->GetActorLocation();
    }

    // Bind perception callback
    if (PerceptionComponent)
    {
        PerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ATRexAIController::OnTargetPerceptionUpdated);
    }

    // Start behavior tree if assigned
    if (TRexBehaviorTreeAsset)
    {
        RunBehaviorTree(TRexBehaviorTreeAsset);

        // Set blackboard defaults
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsVector(BlackboardKey_PatrolOrigin, PatrolOrigin);
            BlackboardComp->SetValueAsFloat(BlackboardKey_PatrolRadius, PatrolRadius);
            BlackboardComp->SetValueAsInt(BlackboardKey_AIState, 0);
        }
    }
}

void ATRexAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // State-based behavior fallback (when no BT asset assigned)
    if (!TRexBehaviorTreeAsset)
    {
        APawn* ControlledPawn = GetPawn();
        if (!ControlledPawn) return;

        ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
        if (!PlayerChar) return;

        float DistToPlayer = FVector::Dist(ControlledPawn->GetActorLocation(), PlayerChar->GetActorLocation());

        if (DistToPlayer <= AttackRadius)
        {
            // Attack state
            if (CurrentAIState != 3)
            {
                SetAIState(3);
            }
            // Face player
            FVector Dir = (PlayerChar->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
            FRotator LookRot = Dir.Rotation();
            ControlledPawn->SetActorRotation(FRotator(0, LookRot.Yaw, 0));
        }
        else if (DistToPlayer <= SightRadius)
        {
            // Chase state
            if (CurrentAIState != 2)
            {
                SetAIState(2);
            }
            MoveToActor(PlayerChar, AttackRadius * 0.8f);
        }
        else
        {
            // Patrol state
            if (CurrentAIState != 0)
            {
                SetAIState(0);
            }

            // Wander within patrol radius
            float DistFromOrigin = FVector::Dist(ControlledPawn->GetActorLocation(), PatrolOrigin);
            if (DistFromOrigin > PatrolRadius)
            {
                MoveToLocation(PatrolOrigin);
            }
        }
    }
}

void ATRexAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    PatrolOrigin = InPawn->GetActorLocation();
}

void ATRexAIController::SetAIState(int32 NewState)
{
    CurrentAIState = NewState;
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsInt(BlackboardKey_AIState, NewState);
    }
}

bool ATRexAIController::IsPlayerInAttackRange() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;

    ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerChar) return false;

    return FVector::Dist(ControlledPawn->GetActorLocation(), PlayerChar->GetActorLocation()) <= AttackRadius;
}

bool ATRexAIController::IsPlayerInChaseRange() const
{
    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return false;

    ACharacter* PlayerChar = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerChar) return false;

    return FVector::Dist(ControlledPawn->GetActorLocation(), PlayerChar->GetActorLocation()) <= SightRadius;
}

void ATRexAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;

    ACharacter* PlayerChar = Cast<ACharacter>(Actor);
    if (!PlayerChar) return;

    if (Stimulus.WasSuccessfullySensed())
    {
        UpdateBlackboardFromPerception(PlayerChar);
    }
    else
    {
        // Lost sight — return to patrol after delay
        if (BlackboardComp)
        {
            BlackboardComp->ClearValue(BlackboardKey_TargetActor);
            BlackboardComp->SetValueAsInt(BlackboardKey_AIState, 1); // Alert state
        }
        SetAIState(1);
    }
}

void ATRexAIController::UpdateBlackboardFromPerception(AActor* PlayerActor)
{
    if (!BlackboardComp || !PlayerActor) return;

    BlackboardComp->SetValueAsObject(BlackboardKey_TargetActor, PlayerActor);

    float Dist = 0.0f;
    if (GetPawn())
    {
        Dist = FVector::Dist(GetPawn()->GetActorLocation(), PlayerActor->GetActorLocation());
    }

    if (Dist <= AttackRadius)
    {
        BlackboardComp->SetValueAsInt(BlackboardKey_AIState, 3); // Attack
        SetAIState(3);
    }
    else
    {
        BlackboardComp->SetValueAsInt(BlackboardKey_AIState, 2); // Chase
        SetAIState(2);
    }
}
