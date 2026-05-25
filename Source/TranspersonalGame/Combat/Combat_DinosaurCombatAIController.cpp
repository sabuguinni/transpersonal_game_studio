#include "Combat_DinosaurCombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "AIModule/Classes/AIController.h"

// Blackboard key names
const FName ACombat_DinosaurCombatAIController::TargetActorKey = TEXT("TargetActor");
const FName ACombat_DinosaurCombatAIController::InCombatKey = TEXT("InCombat");
const FName ACombat_DinosaurCombatAIController::LastKnownLocationKey = TEXT("LastKnownLocation");
const FName ACombat_DinosaurCombatAIController::DistanceToTargetKey = TEXT("DistanceToTarget");

ACombat_DinosaurCombatAIController::ACombat_DinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create AI components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    // Initialize default values
    CurrentTarget = nullptr;
    bInCombat = false;
    LastAttackTime = 0.0f;
}

void ACombat_DinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    InitializePerception();
    SetupBlackboardKeys();
}

void ACombat_DinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (InPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI: Possessing pawn %s"), *InPawn->GetName());
        
        // Start behavior tree if available
        if (CombatBehaviorTree && BlackboardComponent)
        {
            if (UseBlackboard(CombatBehaviorTree->BlackboardAsset))
            {
                RunBehaviorTree(CombatBehaviorTree);
                UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI: Behavior tree started"));
            }
        }
        else
        {
            // Fallback: basic AI behavior without behavior tree
            StartCombatBehavior();
        }
    }
}

void ACombat_DinosaurCombatAIController::OnUnPossess()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
    
    Super::OnUnPossess();
}

void ACombat_DinosaurCombatAIController::InitializePerception()
{
    if (PerceptionComponent)
    {
        // Configure sight sense
        UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
        if (SightConfig)
        {
            SightConfig->SightRadius = DetectionRadius;
            SightConfig->LoseSightRadius = DetectionRadius + 500.0f;
            SightConfig->PeripheralVisionAngleDegrees = 90.0f;
            SightConfig->SetMaxAge(5.0f);
            SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
            SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
            SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
            SightConfig->DetectionByAffiliation.bDetectEnemies = true;

            PerceptionComponent->ConfigureSense(*SightConfig);
            PerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
            PerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatAIController::OnPerceptionUpdated);
        }
    }
}

void ACombat_DinosaurCombatAIController::SetupBlackboardKeys()
{
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(InCombatKey, false);
        BlackboardComponent->SetValueAsFloat(DistanceToTargetKey, 0.0f);
    }
}

bool ACombat_DinosaurCombatAIController::DetectPlayer()
{
    if (!GetPawn())
        return false;

    // Use sphere trace to detect player
    FVector StartLocation = GetPawn()->GetActorLocation();
    FVector EndLocation = StartLocation; // Same location for sphere trace
    
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    
    TArray<FHitResult> HitResults;
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        StartLocation,
        EndLocation,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(DetectionRadius),
        QueryParams
    );

    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (Hit.GetActor() && Hit.GetActor()->IsA<ACharacter>())
            {
                // Found a character (likely the player)
                SetCombatTarget(Hit.GetActor());
                return true;
            }
        }
    }

    return false;
}

void ACombat_DinosaurCombatAIController::StartCombatBehavior()
{
    bInCombat = true;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(InCombatKey, true);
    }

    // Start basic AI behavior: detect and chase player
    GetWorldTimerManager().SetTimer(
        FTimerHandle(),
        [this]()
        {
            if (DetectPlayer())
            {
                if (CurrentTarget && GetPawn())
                {
                    // Move towards the target
                    MoveToActor(CurrentTarget, AttackRange);
                    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI: Moving to target %s"), *CurrentTarget->GetName());
                }
            }
            UpdateCombatState();
        },
        1.0f, // Update every second
        true  // Loop
    );
}

void ACombat_DinosaurCombatAIController::StopCombatBehavior()
{
    bInCombat = false;
    CurrentTarget = nullptr;
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(InCombatKey, false);
        BlackboardComponent->SetValueAsObject(TargetActorKey, nullptr);
    }
    
    StopMovement();
}

void ACombat_DinosaurCombatAIController::SetCombatTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    
    if (BlackboardComponent && NewTarget)
    {
        BlackboardComponent->SetValueAsObject(TargetActorKey, NewTarget);
        BlackboardComponent->SetValueAsVector(LastKnownLocationKey, NewTarget->GetActorLocation());
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI: Target set to %s"), *NewTarget->GetName());
    }
}

AActor* ACombat_DinosaurCombatAIController::GetCombatTarget() const
{
    return CurrentTarget;
}

float ACombat_DinosaurCombatAIController::GetDistanceToTarget() const
{
    if (CurrentTarget && GetPawn())
    {
        return FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    }
    return 0.0f;
}

bool ACombat_DinosaurCombatAIController::IsInAttackRange() const
{
    return GetDistanceToTarget() <= AttackRange;
}

void ACombat_DinosaurCombatAIController::ExecuteAttack()
{
    if (!CurrentTarget || !GetPawn())
        return;

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastAttackTime < AttackCooldown)
        return;

    if (IsInAttackRange())
    {
        LastAttackTime = CurrentTime;
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI: Executing attack on %s"), *CurrentTarget->GetName());
        
        // TODO: Implement actual attack logic (damage, animation, etc.)
        // For now, just log the attack
    }
}

void ACombat_DinosaurCombatAIController::UpdateCombatState()
{
    if (!bInCombat || !CurrentTarget || !GetPawn())
        return;

    float DistanceToTarget = GetDistanceToTarget();
    
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(DistanceToTargetKey, DistanceToTarget);
    }

    // Check if target is still valid and in range
    if (DistanceToTarget > DetectionRadius * 1.5f)
    {
        // Target too far, stop combat
        StopCombatBehavior();
        return;
    }

    // Execute attack if in range
    if (IsInAttackRange())
    {
        ExecuteAttack();
    }
}

void ACombat_DinosaurCombatAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            SetCombatTarget(Actor);
            if (!bInCombat)
            {
                StartCombatBehavior();
            }
            break;
        }
    }
}