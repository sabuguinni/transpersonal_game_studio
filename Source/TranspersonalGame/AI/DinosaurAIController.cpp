// DinosaurAIController.cpp
// Combat & Enemy AI Agent #12 — Transpersonal Game Studio
// Dinosaur AI state machine: Idle → Patrol → Alert → Chase → Attack → Flee

#include "DinosaurAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

ADinosaurAIController::ADinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Perception setup
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
    SetPerceptionComponent(*PerceptionComponent);

    // Sight config
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 1500.0f;
    SightConfig->LoseSightRadius = 2000.0f;
    SightConfig->PeripheralVisionAngleDegrees = 60.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    PerceptionComponent->ConfigureSense(*SightConfig);

    // Hearing config
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 800.0f;
    HearingConfig->SetMaxAge(3.0f);
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    PerceptionComponent->ConfigureSense(*HearingConfig);

    PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComp"));
}

void ADinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception delegate
    PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ADinosaurAIController::HandlePerceptionUpdated);

    ApplySpeciesDefaults();
    GeneratePatrolPoints();

    // Start behavior tree if assigned
    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }

    SetDinoState(ECombat_DinoState::Patrolling);
}

void ADinosaurAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    switch (CurrentState)
    {
    case ECombat_DinoState::Idle:
        UpdateIdleState(DeltaTime);
        break;
    case ECombat_DinoState::Patrolling:
        UpdatePatrolState(DeltaTime);
        break;
    case ECombat_DinoState::Alerted:
        UpdateAlertedState(DeltaTime);
        break;
    case ECombat_DinoState::Chasing:
        UpdateChaseState(DeltaTime);
        break;
    case ECombat_DinoState::Attacking:
        UpdateAttackState(DeltaTime);
        break;
    case ECombat_DinoState::Fleeing:
        UpdateFleeState(DeltaTime);
        break;
    default:
        break;
    }
}

void ADinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardComp && BehaviorTreeAsset && BehaviorTreeAsset->BlackboardAsset)
    {
        BlackboardComp->InitializeBlackboard(*BehaviorTreeAsset->BlackboardAsset);
    }
}

void ADinosaurAIController::OnUnPossess()
{
    Super::OnUnPossess();
    CurrentTarget = nullptr;
    SetDinoState(ECombat_DinoState::Idle);
}

void ADinosaurAIController::SetDinoState(ECombat_DinoState NewState)
{
    if (CurrentState == NewState) return;

    CurrentState = NewState;
    StateEnteredTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

    APawn* ControlledPawn = GetPawn();
    if (!ControlledPawn) return;

    ACharacter* DinoChar = Cast<ACharacter>(ControlledPawn);
    if (!DinoChar) return;

    UCharacterMovementComponent* MoveComp = DinoChar->GetCharacterMovement();
    if (!MoveComp) return;

    switch (NewState)
    {
    case ECombat_DinoState::Patrolling:
        MoveComp->MaxWalkSpeed = DinoStats.PatrolSpeed;
        break;
    case ECombat_DinoState::Chasing:
        MoveComp->MaxWalkSpeed = DinoStats.ChaseSpeed;
        break;
    case ECombat_DinoState::Fleeing:
        MoveComp->MaxWalkSpeed = DinoStats.ChaseSpeed * 1.2f;
        break;
    case ECombat_DinoState::Idle:
    case ECombat_DinoState::Alerted:
    case ECombat_DinoState::Attacking:
    case ECombat_DinoState::Feeding:
        MoveComp->MaxWalkSpeed = 0.0f;
        break;
    default:
        break;
    }
}

void ADinosaurAIController::AlertNearbyPackMembers(AActor* Threat)
{
    if (!DinoStats.bIsPackHunter || !Threat) return;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), APawn::StaticClass(), NearbyActors);

    for (AActor* Actor : NearbyActors)
    {
        if (Actor == MyPawn) continue;

        float Dist = FVector::Dist(MyPawn->GetActorLocation(), Actor->GetActorLocation());
        if (Dist > DinoStats.PackAlertRadius) continue;

        ADinosaurAIController* PackMember = Cast<ADinosaurAIController>(
            Cast<APawn>(Actor)->GetController()
        );
        if (PackMember && PackMember->Species == Species)
        {
            PackMember->CurrentTarget = Threat;
            PackMember->SetDinoState(ECombat_DinoState::Chasing);
        }
    }
}

float ADinosaurAIController::GetDistanceToTarget() const
{
    if (!CurrentTarget || !GetPawn()) return TNumericLimits<float>::Max();
    return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
}

bool ADinosaurAIController::CanAttack() const
{
    if (!GetWorld()) return false;
    float Now = GetWorld()->GetTimeSeconds();
    return (Now - LastAttackTime) >= DinoStats.AttackCooldown;
}

void ADinosaurAIController::ExecuteAttack()
{
    if (!CanAttack() || !CurrentTarget) return;

    LastAttackTime = GetWorld()->GetTimeSeconds();

    // Apply damage to target
    UGameplayStatics::ApplyDamage(
        CurrentTarget,
        DinoStats.AttackDamage,
        this,
        GetPawn(),
        UDamageType::StaticClass()
    );
}

// ---- Private state update methods ----

void ADinosaurAIController::UpdateIdleState(float DeltaTime)
{
    float TimeInState = GetWorld()->GetTimeSeconds() - StateEnteredTime;
    if (TimeInState > 3.0f)
    {
        SetDinoState(ECombat_DinoState::Patrolling);
    }
}

void ADinosaurAIController::UpdatePatrolState(float DeltaTime)
{
    if (PatrolPoints.Num() == 0) return;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    FVector Target = PatrolPoints[CurrentPatrolIndex];
    float Dist = FVector::Dist2D(MyPawn->GetActorLocation(), Target);

    if (Dist < 150.0f)
    {
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    }
    else
    {
        MoveToLocation(Target, 100.0f, true, true, false, true);
    }
}

void ADinosaurAIController::UpdateAlertedState(float DeltaTime)
{
    if (!CurrentTarget) return;

    float TimeInState = GetWorld()->GetTimeSeconds() - StateEnteredTime;
    if (TimeInState > 2.0f)
    {
        SetDinoState(ECombat_DinoState::Chasing);
        if (DinoStats.bIsPackHunter)
        {
            AlertNearbyPackMembers(CurrentTarget);
        }
    }
}

void ADinosaurAIController::UpdateChaseState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinoState(ECombat_DinoState::Patrolling);
        return;
    }

    float Dist = GetDistanceToTarget();

    if (Dist > DinoStats.DetectionRadius * 1.5f)
    {
        // Lost the target
        CurrentTarget = nullptr;
        SetDinoState(ECombat_DinoState::Patrolling);
        return;
    }

    if (Dist <= DinoStats.AttackRadius)
    {
        SetDinoState(ECombat_DinoState::Attacking);
        return;
    }

    MoveToActor(CurrentTarget, DinoStats.AttackRadius * 0.8f, true, true, false);
}

void ADinosaurAIController::UpdateAttackState(float DeltaTime)
{
    if (!CurrentTarget)
    {
        SetDinoState(ECombat_DinoState::Patrolling);
        return;
    }

    float Dist = GetDistanceToTarget();

    if (Dist > DinoStats.AttackRadius * 1.5f)
    {
        SetDinoState(ECombat_DinoState::Chasing);
        return;
    }

    if (CanAttack())
    {
        ExecuteAttack();
    }
}

void ADinosaurAIController::UpdateFleeState(float DeltaTime)
{
    if (!CurrentTarget) return;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;

    // Move away from threat
    FVector AwayDir = (MyPawn->GetActorLocation() - CurrentTarget->GetActorLocation()).GetSafeNormal();
    FVector FleeTarget = MyPawn->GetActorLocation() + AwayDir * 2000.0f;
    MoveToLocation(FleeTarget, 100.0f);

    float TimeInState = GetWorld()->GetTimeSeconds() - StateEnteredTime;
    if (timeInState > 10.0f)
    {
        CurrentTarget = nullptr;
        SetDinoState(ECombat_DinoState::Patrolling);
    }
}

void ADinosaurAIController::GeneratePatrolPoints()
{
    APawn* MyPawn = GetPawn();
    FVector Origin = MyPawn ? MyPawn->GetActorLocation() : FVector::ZeroVector;

    PatrolPoints.Empty();
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSys) return;

    for (int32 i = 0; i < 4; i++)
    {
        float Angle = (i / 4.0f) * 2.0f * PI;
        float Radius = 800.0f;
        FVector Candidate = Origin + FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.0f);

        FNavLocation NavLoc;
        if (NavSys->GetRandomReachablePointInRadius(Candidate, 300.0f, NavLoc))
        {
            PatrolPoints.Add(NavLoc.Location);
        }
        else
        {
            PatrolPoints.Add(Candidate);
        }
    }
}

void ADinosaurAIController::HandlePerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (!Actor) continue;

        // Only react to player characters
        ACharacter* PlayerChar = Cast<ACharacter>(Actor);
        if (!PlayerChar) continue;

        FActorPerceptionBlueprintInfo Info;
        PerceptionComponent->GetActorsPerception(Actor, Info);

        bool bCurrentlySensed = false;
        for (const FAIStimulus& Stimulus : Info.LastSensedStimuli)
        {
            if (Stimulus.WasSuccessfullySensed())
            {
                bCurrentlySensed = true;
                break;
            }
        }

        if (bCurrentlySensed)
        {
            CurrentTarget = Actor;
            if (CurrentState == ECombat_DinoState::Patrolling || CurrentState == ECombat_DinoState::Idle)
            {
                SetDinoState(ECombat_DinoState::Alerted);
            }
        }
        else if (CurrentTarget == Actor)
        {
            // Lost perception of current target
            CurrentTarget = nullptr;
            if (CurrentState == ECombat_DinoState::Chasing || CurrentState == ECombat_DinoState::Attacking)
            {
                SetDinoState(ECombat_DinoState::Patrolling);
            }
        }
    }
}

void ADinosaurAIController::ApplySpeciesDefaults()
{
    switch (Species)
    {
    case ECombat_DinoSpecies::TRex:
        DinoStats.DetectionRadius = 2500.0f;
        DinoStats.AttackRadius = 350.0f;
        DinoStats.AttackDamage = 120.0f;
        DinoStats.AttackCooldown = 3.0f;
        DinoStats.ChaseSpeed = 700.0f;
        DinoStats.PatrolSpeed = 180.0f;
        DinoStats.bIsPackHunter = false;
        break;

    case ECombat_DinoSpecies::Velociraptor:
        DinoStats.DetectionRadius = 1200.0f;
        DinoStats.AttackRadius = 150.0f;
        DinoStats.AttackDamage = 35.0f;
        DinoStats.AttackCooldown = 0.8f;
        DinoStats.ChaseSpeed = 900.0f;
        DinoStats.PatrolSpeed = 300.0f;
        DinoStats.bIsPackHunter = true;
        DinoStats.PackAlertRadius = 1000.0f;
        break;

    case ECombat_DinoSpecies::Triceratops:
        DinoStats.DetectionRadius = 800.0f;
        DinoStats.AttackRadius = 250.0f;
        DinoStats.AttackDamage = 80.0f;
        DinoStats.AttackCooldown = 2.5f;
        DinoStats.ChaseSpeed = 500.0f;
        DinoStats.PatrolSpeed = 150.0f;
        DinoStats.bIsPackHunter = false;
        break;

    case ECombat_DinoSpecies::Brachiosaurus:
        DinoStats.DetectionRadius = 600.0f;
        DinoStats.AttackRadius = 400.0f;
        DinoStats.AttackDamage = 60.0f;
        DinoStats.AttackCooldown = 4.0f;
        DinoStats.ChaseSpeed = 300.0f;
        DinoStats.PatrolSpeed = 100.0f;
        DinoStats.bIsPackHunter = false;
        break;

    case ECombat_DinoSpecies::Ankylosaurus:
        DinoStats.DetectionRadius = 700.0f;
        DinoStats.AttackRadius = 200.0f;
        DinoStats.AttackDamage = 90.0f;
        DinoStats.AttackCooldown = 3.5f;
        DinoStats.ChaseSpeed = 400.0f;
        DinoStats.PatrolSpeed = 120.0f;
        DinoStats.bIsPackHunter = false;
        break;

    default:
        break;
    }
}
