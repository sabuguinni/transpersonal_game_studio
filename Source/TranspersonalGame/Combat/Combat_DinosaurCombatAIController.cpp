#include "Combat_DinosaurCombatAIController.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "TranspersonalGame/TranspersonalCharacter.h"

ACombat_DinosaurCombatAIController::ACombat_DinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize Blackboard and Behavior Tree
    BlackboardComp = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoard"));
    
    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    
    // Configure sight sense
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = 5000.0f;
        SightConfig->LoseSightRadius = 5500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
        AIPerceptionComponent->ConfigureSense(*SightConfig);
    }
    
    // Bind perception events
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatAIController::OnPerceptionUpdated);
    
    // Combat state initialization
    CombatState = ECombat_DinosaurCombatState::Idle;
    AggressionLevel = 0.5f;
    DetectionRadius = 5000.0f;
    AttackRange = 300.0f;
    LastPlayerLocation = FVector::ZeroVector;
    TimeSinceLastPlayerSeen = 0.0f;
    bIsPlayerInSight = false;
}

void ACombat_DinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Start in idle state
    SetCombatState(ECombat_DinosaurCombatState::Idle);
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController initialized for %s"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("NoPawn"));
}

void ACombat_DinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController possessed %s"), *InPawn->GetName());
        
        // Start behavior tree if available
        if (BehaviorTreeAsset)
        {
            RunBehaviorTree(BehaviorTreeAsset);
        }
        
        // Initialize blackboard values
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsVector(TEXT("PlayerLocation"), FVector::ZeroVector);
            BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
            BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CombatState));
        }
    }
}

void ACombat_DinosaurCombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update time since last player seen
    if (!bIsPlayerInSight)
    {
        TimeSinceLastPlayerSeen += DeltaTime;
    }
    
    // Perform combat logic based on current state
    UpdateCombatBehavior(DeltaTime);
}

void ACombat_DinosaurCombatAIController::OnPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor) return;
    
    // Check if the detected actor is the player
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(Actor);
    if (Player)
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // Player detected
            bIsPlayerInSight = true;
            TimeSinceLastPlayerSeen = 0.0f;
            LastPlayerLocation = Player->GetActorLocation();
            
            // Update blackboard
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsVector(TEXT("PlayerLocation"), LastPlayerLocation);
                BlackboardComp->SetValueAsBool(TEXT("PlayerInSight"), true);
            }
            
            // Transition to hunting state if idle
            if (CombatState == ECombat_DinosaurCombatState::Idle)
            {
                SetCombatState(ECombat_DinosaurCombatState::Hunting);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Player detected by %s at distance %f"), 
                   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
                   FVector::Dist(GetPawn()->GetActorLocation(), LastPlayerLocation));
        }
        else
        {
            // Player lost
            bIsPlayerInSight = false;
            
            if (BlackboardComp)
            {
                BlackboardComp->SetValueAsBool(TEXT("PlayerInSight"), false);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Player lost by %s"), 
                   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
        }
    }
}

void ACombat_DinosaurCombatAIController::UpdateCombatBehavior(float DeltaTime)
{
    if (!GetPawn()) return;
    
    switch (CombatState)
    {
        case ECombat_DinosaurCombatState::Idle:
            HandleIdleState(DeltaTime);
            break;
            
        case ECombat_DinosaurCombatState::Hunting:
            HandleHuntingState(DeltaTime);
            break;
            
        case ECombat_DinosaurCombatState::Attacking:
            HandleAttackingState(DeltaTime);
            break;
            
        case ECombat_DinosaurCombatState::Retreating:
            HandleRetreatingState(DeltaTime);
            break;
    }
}

void ACombat_DinosaurCombatAIController::HandleIdleState(float DeltaTime)
{
    // In idle state, just patrol or stand guard
    // This could be expanded with patrol points
}

void ACombat_DinosaurCombatAIController::HandleHuntingState(float DeltaTime)
{
    if (!GetPawn()) return;
    
    if (bIsPlayerInSight && LastPlayerLocation != FVector::ZeroVector)
    {
        float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), LastPlayerLocation);
        
        if (DistanceToPlayer <= AttackRange)
        {
            // Close enough to attack
            SetCombatState(ECombat_DinosaurCombatState::Attacking);
        }
        else
        {
            // Move towards player
            MoveToLocation(LastPlayerLocation);
        }
    }
    else if (TimeSinceLastPlayerSeen > 10.0f)
    {
        // Lost player for too long, return to idle
        SetCombatState(ECombat_DinosaurCombatState::Idle);
    }
}

void ACombat_DinosaurCombatAIController::HandleAttackingState(float DeltaTime)
{
    if (!GetPawn()) return;
    
    if (bIsPlayerInSight && LastPlayerLocation != FVector::ZeroVector)
    {
        float DistanceToPlayer = FVector::Dist(GetPawn()->GetActorLocation(), LastPlayerLocation);
        
        if (DistanceToPlayer > AttackRange * 1.5f)
        {
            // Player moved away, return to hunting
            SetCombatState(ECombat_DinosaurCombatState::Hunting);
        }
        else
        {
            // Perform attack
            PerformAttack();
        }
    }
    else
    {
        // Lost sight of player
        SetCombatState(ECombat_DinosaurCombatState::Hunting);
    }
}

void ACombat_DinosaurCombatAIController::HandleRetreatingState(float DeltaTime)
{
    // Move away from player
    if (LastPlayerLocation != FVector::ZeroVector && GetPawn())
    {
        FVector RetreatDirection = GetPawn()->GetActorLocation() - LastPlayerLocation;
        RetreatDirection.Normalize();
        FVector RetreatLocation = GetPawn()->GetActorLocation() + (RetreatDirection * 2000.0f);
        
        MoveToLocation(RetreatLocation);
        
        // After retreating for a while, return to idle
        if (TimeSinceLastPlayerSeen > 5.0f)
        {
            SetCombatState(ECombat_DinosaurCombatState::Idle);
        }
    }
}

void ACombat_DinosaurCombatAIController::SetCombatState(ECombat_DinosaurCombatState NewState)
{
    if (CombatState != NewState)
    {
        CombatState = NewState;
        
        // Update blackboard
        if (BlackboardComp)
        {
            BlackboardComp->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(CombatState));
        }
        
        UE_LOG(LogTemp, Warning, TEXT("%s combat state changed to %d"), 
               GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), 
               static_cast<int32>(CombatState));
    }
}

void ACombat_DinosaurCombatAIController::PerformAttack()
{
    // Basic attack implementation
    UE_LOG(LogTemp, Warning, TEXT("%s performing attack!"), 
           GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
    
    // Here we would trigger attack animations, deal damage, etc.
    // For now, just log the attack
}

bool ACombat_DinosaurCombatAIController::DetectPlayer()
{
    if (!GetWorld()) return false;
    
    // Get player character
    ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!Player || !GetPawn()) return false;
    
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Player->GetActorLocation());
    
    if (Distance <= DetectionRadius)
    {
        // Perform line trace to check line of sight
        FHitResult HitResult;
        FVector Start = GetPawn()->GetActorLocation();
        FVector End = Player->GetActorLocation();
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetPawn());
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECollisionChannel::ECC_Visibility,
            QueryParams
        );
        
        if (!bHit || HitResult.GetActor() == Player)
        {
            // Clear line of sight to player
            LastPlayerLocation = Player->GetActorLocation();
            bIsPlayerInSight = true;
            TimeSinceLastPlayerSeen = 0.0f;
            return true;
        }
    }
    
    return false;
}

void ACombat_DinosaurCombatAIController::SetAggressionLevel(float NewAggression)
{
    AggressionLevel = FMath::Clamp(NewAggression, 0.0f, 1.0f);
    
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
    }
}