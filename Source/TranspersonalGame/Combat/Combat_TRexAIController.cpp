#include "Combat_TRexAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "TranspersonalGame/TranspersonalCharacter.h"

ACombat_TRexAIController::ACombat_TRexAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure Sight Sense
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 3000.0f;
        SightConfig->LoseSightRadius = 3500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }

    // Initialize Blackboard and Behavior Tree components
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

    // Initialize state
    CurrentState = ECombat_TRexState::Idle;
    TargetPlayer = nullptr;
    LastDetectionTime = 0.0f;
    LastAttackTime = 0.0f;
    PatrolWaitTime = 0.0f;

    // Initialize stats with default values
    TRexStats.MaxHealth = 1000.0f;
    TRexStats.CurrentHealth = 1000.0f;
    TRexStats.AttackDamage = 150.0f;
    TRexStats.AttackRange = 400.0f;
    TRexStats.SightRange = 3000.0f;
    TRexStats.MovementSpeed = 600.0f;
    TRexStats.TerritoryRadius = 2000.0f;
}

void ACombat_TRexAIController::BeginPlay()
{
    Super::BeginPlay();

    // Set territory center to current location
    if (GetPawn())
    {
        TerritoryCenter = GetPawn()->GetActorLocation();
    }

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_TRexAIController::OnPerceptionUpdated);
    }

    // Start with patrol state
    SetCombatState(ECombat_TRexState::Patrolling);
}

void ACombat_TRexAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // Start behavior tree if available
    if (BehaviorTree && BlackboardComponent)
    {
        RunBehaviorTree(BehaviorTree);
        
        // Set initial blackboard values
        BlackboardComponent->SetValueAsEnum(FName("CombatState"), static_cast<uint8>(CurrentState));
        BlackboardComponent->SetValueAsVector(FName("TerritoryCenter"), TerritoryCenter);
        BlackboardComponent->SetValueAsFloat(FName("TerritoryRadius"), TRexStats.TerritoryRadius);
    }

    // Configure movement speed
    if (ACharacter* Character = Cast<ACharacter>(InPawn))
    {
        if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        {
            Movement->MaxWalkSpeed = TRexStats.MovementSpeed;
        }
    }
}

void ACombat_TRexAIController::OnUnPossess()
{
    Super::OnUnPossess();
    
    // Stop behavior tree
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ACombat_TRexAIController::DetectPlayer()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastDetectionTime < DETECTION_INTERVAL)
    {
        return;
    }
    LastDetectionTime = CurrentTime;

    // Find all players in the world
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATranspersonalCharacter::StaticClass(), FoundActors);

    ATranspersonalCharacter* ClosestPlayer = nullptr;
    float ClosestDistance = TRexStats.SightRange;

    for (AActor* Actor : FoundActors)
    {
        if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(Actor))
        {
            float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                // Line of sight check
                FHitResult HitResult;
                FVector Start = GetPawn()->GetActorLocation();
                FVector End = Player->GetActorLocation();
                
                bool bHit = GetWorld()->LineTraceSingleByChannel(
                    HitResult,
                    Start,
                    End,
                    ECollisionChannel::ECC_Visibility
                );

                if (!bHit || HitResult.GetActor() == Player)
                {
                    ClosestPlayer = Player;
                    ClosestDistance = Distance;
                }
            }
        }
    }

    if (ClosestPlayer && ClosestPlayer != TargetPlayer)
    {
        StartHunting(ClosestPlayer);
    }
    else if (!ClosestPlayer && TargetPlayer)
    {
        // Lost target
        ReturnToPatrol();
    }
}

void ACombat_TRexAIController::StartHunting(ATranspersonalCharacter* Player)
{
    TargetPlayer = Player;
    SetCombatState(ECombat_TRexState::Hunting);

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(FName("TargetPlayer"), TargetPlayer);
    }

    // Move towards target
    if (TargetPlayer)
    {
        MoveToActor(TargetPlayer, TRexStats.AttackRange);
    }
}

void ACombat_TRexAIController::AttackTarget()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < ATTACK_COOLDOWN)
    {
        return;
    }

    if (!TargetPlayer || !IsPlayerInRange(TRexStats.AttackRange))
    {
        return;
    }

    LastAttackTime = CurrentTime;
    SetCombatState(ECombat_TRexState::Attacking);

    // Perform attack logic here
    // For now, just log the attack
    UE_LOG(LogTemp, Warning, TEXT("T-Rex attacking player!"));

    // Return to hunting after attack
    FTimerHandle AttackTimer;
    GetWorld()->GetTimerManager().SetTimer(AttackTimer, [this]()
    {
        if (TargetPlayer)
        {
            SetCombatState(ECombat_TRexState::Hunting);
            MoveToActor(TargetPlayer, TRexStats.AttackRange);
        }
    }, 1.0f, false);
}

void ACombat_TRexAIController::ReturnToPatrol()
{
    TargetPlayer = nullptr;
    SetCombatState(ECombat_TRexState::Patrolling);

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(FName("TargetPlayer"), nullptr);
    }

    // Move to random patrol point
    FVector PatrolPoint = GetRandomPatrolPoint();
    MoveToLocation(PatrolPoint);
}

void ACombat_TRexAIController::SetCombatState(ECombat_TRexState NewState)
{
    if (CurrentState != NewState)
    {
        CurrentState = NewState;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(FName("CombatState"), static_cast<uint8>(CurrentState));
        }
    }
}

void ACombat_TRexAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(Actor))
        {
            FAIStimulus Stimulus;
            if (AIPerceptionComponent->GetActorsPerception(Actor, Stimulus))
            {
                if (Stimulus.WasSuccessfullySensed())
                {
                    StartHunting(Player);
                }
                else
                {
                    // Lost sight of player
                    if (TargetPlayer == Player)
                    {
                        ReturnToPatrol();
                    }
                }
            }
        }
    }
}

bool ACombat_TRexAIController::IsPlayerInRange(float Range) const
{
    if (!TargetPlayer || !GetPawn())
    {
        return false;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), TargetPlayer->GetActorLocation());
    return Distance <= Range;
}

bool ACombat_TRexAIController::IsInTerritory(const FVector& Location) const
{
    float Distance = FVector::Dist(TerritoryCenter, Location);
    return Distance <= TRexStats.TerritoryRadius;
}

FVector ACombat_TRexAIController::GetRandomPatrolPoint() const
{
    // Generate random point within territory
    FVector RandomDirection = FVector(
        FMath::RandRange(-1.0f, 1.0f),
        FMath::RandRange(-1.0f, 1.0f),
        0.0f
    ).GetSafeNormal();

    float RandomDistance = FMath::RandRange(500.0f, TRexStats.TerritoryRadius * 0.8f);
    FVector PatrolPoint = TerritoryCenter + (RandomDirection * RandomDistance);

    // Ensure Z coordinate is reasonable
    PatrolPoint.Z = TerritoryCenter.Z;

    return PatrolPoint;
}