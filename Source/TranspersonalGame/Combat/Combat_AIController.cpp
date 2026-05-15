#include "Combat_AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Engine/Engine.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ACombat_AIController::ACombat_AIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize AI Components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure AI Sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 2000.0f;
    SightConfig->LoseSightRadius = 2200.0f;
    SightConfig->PeripheralVisionAngleDegrees = 120.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*SightConfig);

    // Configure AI Hearing
    UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
    HearingConfig->HearingRange = 1500.0f;
    HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
    HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
    HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
    AIPerceptionComponent->ConfigureSense(*HearingConfig);

    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());

    // Initialize Combat Parameters
    AttackRange = 300.0f;
    DetectionRadius = 2000.0f;
    FleeThreshold = 0.3f;
    AggressionLevel = 0.7f;
    CurrentCombatState = ECombat_AIState::Patrol;
    CurrentTarget = nullptr;
    LastAttackTime = 0.0f;
    AttackCooldown = 2.0f;
    bIsInCombat = false;
    bIsPackLeader = false;
}

void ACombat_AIController::BeginPlay()
{
    Super::BeginPlay();

    // Bind perception events
    if (AIPerceptionComponent)
    {
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_AIController::OnPerceptionUpdated);
    }

    // Start behavior tree after a short delay
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
    {
        SelectBehaviorTree();
    }, 0.5f, false);
}

void ACombat_AIController::Possess(APawn* InPawn)
{
    Super::Possess(InPawn);

    if (BlackboardComponent)
    {
        UseBlackboard(BlackboardComponent->GetBlackboardAsset());
    }
}

void ACombat_AIController::SetCombatState(ECombat_AIState NewState)
{
    if (CurrentCombatState != NewState)
    {
        CurrentCombatState = NewState;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(TEXT("CombatState"), (uint8)NewState);
        }

        // Log state change for debugging
        FString StateName = UEnum::GetValueAsString(NewState);
        UE_LOG(LogTemp, Warning, TEXT("Combat AI State Changed to: %s"), *StateName);

        UpdateCombatBehavior();
    }
}

void ACombat_AIController::SetTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), NewTarget);
    }

    if (NewTarget)
    {
        SetCombatState(ECombat_AIState::Engaging);
        bIsInCombat = true;
    }
    else
    {
        SetCombatState(ECombat_AIState::Patrol);
        bIsInCombat = false;
    }
}

void ACombat_AIController::AddPackMember(AActor* PackMember)
{
    if (PackMember && !PackMembers.Contains(PackMember))
    {
        PackMembers.Add(PackMember);
        
        // Update blackboard with pack size
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsInt(TEXT("PackSize"), PackMembers.Num());
        }
    }
}

void ACombat_AIController::RemovePackMember(AActor* PackMember)
{
    if (PackMembers.Contains(PackMember))
    {
        PackMembers.Remove(PackMember);
        
        // Update blackboard with pack size
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsInt(TEXT("PackSize"), PackMembers.Num());
        }
    }
}

bool ACombat_AIController::IsInAttackRange(AActor* Target)
{
    if (!Target || !GetPawn())
    {
        return false;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

void ACombat_AIController::ExecuteAttack()
{
    if (!CurrentTarget || !GetPawn())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
    {
        return;
    }

    if (IsInAttackRange(CurrentTarget))
    {
        // Execute attack logic
        LastAttackTime = CurrentTime;
        
        // Broadcast pack attack signal if this is a pack leader
        if (bIsPackLeader && PackMembers.Num() > 0)
        {
            BroadcastPackSignal(ECombat_PackSignal::Attack);
        }

        UE_LOG(LogTemp, Warning, TEXT("Combat AI executing attack on target"));
    }
}

void ACombat_AIController::ExecuteFleeResponse()
{
    SetCombatState(ECombat_AIState::Fleeing);
    CurrentTarget = nullptr;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
    }

    // Broadcast flee signal to pack
    if (PackMembers.Num() > 0)
    {
        BroadcastPackSignal(ECombat_PackSignal::Flee);
    }

    UE_LOG(LogTemp, Warning, TEXT("Combat AI executing flee response"));
}

void ACombat_AIController::CoordinatePackAttack()
{
    if (PackMembers.Num() == 0 || !CurrentTarget)
    {
        return;
    }

    // Coordinate pack attack patterns
    BroadcastPackSignal(ECombat_PackSignal::Coordinate);
    
    UE_LOG(LogTemp, Warning, TEXT("Combat AI coordinating pack attack with %d members"), PackMembers.Num());
}

void ACombat_AIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor != GetPawn())
        {
            EvaluateThreat(Actor);
        }
    }
}

void ACombat_AIController::EvaluateThreat(AActor* ThreatActor)
{
    if (!ThreatActor || !GetPawn())
    {
        return;
    }

    // Check if this is a player character
    if (ThreatActor->IsA<ACharacter>())
    {
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), ThreatActor->GetActorLocation());
        
        if (Distance <= DetectionRadius)
        {
            // Evaluate threat level based on distance and aggression
            float ThreatLevel = 1.0f - (Distance / DetectionRadius);
            
            if (ThreatLevel > FleeThreshold && AggressionLevel > 0.5f)
            {
                // Engage target
                SetTarget(ThreatActor);
            }
            else if (ThreatLevel > 0.8f && AggressionLevel <= 0.5f)
            {
                // Flee from overwhelming threat
                ExecuteFleeResponse();
            }
        }
    }
}

void ACombat_AIController::UpdateCombatBehavior()
{
    switch (CurrentCombatState)
    {
        case ECombat_AIState::Patrol:
            // Standard patrol behavior
            break;
            
        case ECombat_AIState::Investigating:
            // Investigation behavior
            break;
            
        case ECombat_AIState::Engaging:
            if (CurrentTarget && IsInAttackRange(CurrentTarget))
            {
                ExecuteAttack();
            }
            break;
            
        case ECombat_AIState::Fleeing:
            // Flee behavior
            break;
            
        case ECombat_AIState::Hunting:
            // Hunting behavior
            break;
    }
}

void ACombat_AIController::SelectBehaviorTree()
{
    UBehaviorTree* SelectedTree = nullptr;

    // Select appropriate behavior tree based on pawn type
    if (GetPawn())
    {
        FString PawnName = GetPawn()->GetName().ToLower();
        
        if (PawnName.Contains("rex") || PawnName.Contains("apex"))
        {
            SelectedTree = ApexPredatorBehaviorTree;
            bIsPackLeader = true;
            AggressionLevel = 0.9f;
        }
        else if (PawnName.Contains("raptor") || PawnName.Contains("carnivore"))
        {
            SelectedTree = CarnivoreBehaviorTree;
            bIsPackLeader = (FMath::RandRange(0, 3) == 0); // 25% chance to be pack leader
            AggressionLevel = 0.8f;
        }
        else
        {
            SelectedTree = HerbivoreBehaviorTree;
            bIsPackLeader = false;
            AggressionLevel = 0.2f;
        }
    }

    if (SelectedTree && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*SelectedTree);
        UE_LOG(LogTemp, Warning, TEXT("Combat AI started behavior tree for %s"), *GetPawn()->GetName());
    }
}

void ACombat_AIController::BroadcastPackSignal(ECombat_PackSignal Signal)
{
    for (AActor* PackMember : PackMembers)
    {
        if (PackMember)
        {
            // Send signal to pack member's AI controller
            if (APawn* PackPawn = Cast<APawn>(PackMember))
            {
                if (ACombat_AIController* PackAI = Cast<ACombat_AIController>(PackPawn->GetController()))
                {
                    PackAI->RespondToPackSignal(Signal, GetPawn());
                }
            }
        }
    }
}

void ACombat_AIController::RespondToPackSignal(ECombat_PackSignal Signal, AActor* Sender)
{
    switch (Signal)
    {
        case ECombat_PackSignal::Attack:
            if (CurrentTarget)
            {
                ExecuteAttack();
            }
            break;
            
        case ECombat_PackSignal::Flee:
            ExecuteFleeResponse();
            break;
            
        case ECombat_PackSignal::Coordinate:
            // Respond to coordination signal
            break;
            
        case ECombat_PackSignal::Regroup:
            SetCombatState(ECombat_AIState::Patrol);
            break;
    }
}