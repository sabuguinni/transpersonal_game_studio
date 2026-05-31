#include "DinosaurCombatAIController.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

ADinosaurCombatAIController::ADinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize AI Perception
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 5000.0f;
    SightConfig->LoseSightRadius = 6000.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Bind perception events
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAIController::OnTargetPerceptionUpdated);
    
    // Initialize variables
    CurrentTarget = nullptr;
    AttackRange = 300.0f;
    ChaseSpeed = 600.0f;
    PatrolSpeed = 200.0f;
    bIsChasing = false;
    bIsAttacking = false;
    LastKnownTargetLocation = FVector::ZeroVector;
}

void ADinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Start behavior tree if available
    if (BehaviorTreeAsset)
    {
        RunBehaviorTree(BehaviorTreeAsset);
    }
    
    // Set initial patrol point
    if (GetPawn())
    {
        PatrolStartLocation = GetPawn()->GetActorLocation();
    }
}

void ADinosaurCombatAIController::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    // Update combat logic
    UpdateCombatBehavior();
}

void ADinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        PatrolStartLocation = InPawn->GetActorLocation();
        
        // Start behavior tree
        if (BehaviorTreeAsset)
        {
            RunBehaviorTree(BehaviorTreeAsset);
        }
        
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController possessed pawn: %s"), *InPawn->GetName());
    }
}

void ADinosaurCombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (!Actor)
    {
        return;
    }
    
    // Check if this is a player character
    if (Actor->IsA<ACharacter>() && Actor->GetName().Contains("Character"))
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            // Target spotted
            CurrentTarget = Actor;
            LastKnownTargetLocation = Actor->GetActorLocation();
            bIsChasing = true;
            
            // Update blackboard
            if (GetBlackboardComponent())
            {
                GetBlackboardComponent()->SetValueAsObject("TargetActor", CurrentTarget);
                GetBlackboardComponent()->SetValueAsVector("TargetLocation", LastKnownTargetLocation);
                GetBlackboardComponent()->SetValueAsBool("HasTarget", true);
            }
            
            UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI: Target acquired - %s"), *Actor->GetName());
        }
        else
        {
            // Target lost
            if (CurrentTarget == Actor)
            {
                CurrentTarget = nullptr;
                bIsChasing = false;
                
                // Update blackboard
                if (GetBlackboardComponent())
                {
                    GetBlackboardComponent()->SetValueAsObject("TargetActor", nullptr);
                    GetBlackboardComponent()->SetValueAsBool("HasTarget", false);
                }
                
                UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI: Target lost"));
            }
        }
    }
}

void ADinosaurCombatAIController::UpdateCombatBehavior()
{
    if (!GetPawn())
    {
        return;
    }
    
    if (CurrentTarget && IsValid(CurrentTarget))
    {
        float DistanceToTarget = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        if (DistanceToTarget <= AttackRange)
        {
            // In attack range - stop and attack
            StopMovement();
            PerformAttack();
        }
        else if (bIsChasing)
        {
            // Chase target
            MoveToActor(CurrentTarget, AttackRange - 50.0f);
        }
    }
    else if (!bIsChasing)
    {
        // No target - patrol
        PerformPatrol();
    }
}

void ADinosaurCombatAIController::PerformAttack()
{
    if (bIsAttacking)
    {
        return;
    }
    
    bIsAttacking = true;
    
    // Face target
    if (CurrentTarget && GetPawn())
    {
        FVector DirectionToTarget = (CurrentTarget->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
        FRotator TargetRotation = DirectionToTarget.Rotation();
        GetPawn()->SetActorRotation(TargetRotation);
        
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI: Attacking target!"));
    }
    
    // Reset attack flag after delay
    GetWorld()->GetTimerManager().SetTimer(AttackCooldownTimer, [this]()
    {
        bIsAttacking = false;
    }, 2.0f, false);
}

void ADinosaurCombatAIController::PerformPatrol()
{
    if (!GetPawn())
    {
        return;
    }
    
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    float DistanceFromStart = FVector::Dist(CurrentLocation, PatrolStartLocation);
    
    if (DistanceFromStart > 2000.0f)
    {
        // Return to start location
        MoveToLocation(PatrolStartLocation);
    }
    else
    {
        // Random patrol within area
        FVector RandomDirection = FMath::VRand();
        RandomDirection.Z = 0.0f;
        RandomDirection.Normalize();
        
        FVector PatrolTarget = PatrolStartLocation + (RandomDirection * FMath::RandRange(500.0f, 1500.0f));
        MoveToLocation(PatrolTarget);
    }
}

bool ADinosaurCombatAIController::DetectPlayer()
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Use sphere trace to detect player
    FVector StartLocation = GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector;
    FVector EndLocation = StartLocation + FVector(0, 0, 100); // Small trace up
    
    TArray<FHitResult> HitResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetPawn());
    
    bool bHit = GetWorld()->SweepMultiByChannel(
        HitResults,
        StartLocation,
        EndLocation,
        FQuat::Identity,
        ECollisionChannel::ECC_Pawn,
        FCollisionShape::MakeSphere(5000.0f),
        QueryParams
    );
    
    if (bHit)
    {
        for (const FHitResult& Hit : HitResults)
        {
            if (Hit.GetActor() && Hit.GetActor()->IsA<ACharacter>())
            {
                if (Hit.GetActor()->GetName().Contains("Character"))
                {
                    CurrentTarget = Hit.GetActor();
                    LastKnownTargetLocation = Hit.GetActor()->GetActorLocation();
                    bIsChasing = true;
                    
                    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAI: Player detected via sphere trace"));
                    return true;
                }
            }
        }
    }
    
    return false;
}

void ADinosaurCombatAIController::SetCombatTarget(AActor* NewTarget)
{
    CurrentTarget = NewTarget;
    if (NewTarget)
    {
        LastKnownTargetLocation = NewTarget->GetActorLocation();
        bIsChasing = true;
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsObject("TargetActor", CurrentTarget);
            GetBlackboardComponent()->SetValueAsVector("TargetLocation", LastKnownTargetLocation);
            GetBlackboardComponent()->SetValueAsBool("HasTarget", true);
        }
    }
    else
    {
        bIsChasing = false;
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsObject("TargetActor", nullptr);
            GetBlackboardComponent()->SetValueAsBool("HasTarget", false);
        }
    }
}

AActor* ADinosaurCombatAIController::GetCurrentTarget() const
{
    return CurrentTarget;
}

bool ADinosaurCombatAIController::IsInCombat() const
{
    return bIsChasing || bIsAttacking;
}