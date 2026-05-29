#include "Combat_DinosaurCombatController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

ACombat_DinosaurCombatController::ACombat_DinosaurCombatController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Configure AI Perception for sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = DetectionRadius;
        SightConfig->LoseSightRadius = DetectionRadius + 500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurCombatController::OnPerceptionUpdated);
}

void ACombat_DinosaurCombatController::BeginPlay()
{
    Super::BeginPlay();

    // Start detection timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            DetectionTimerHandle,
            this,
            &ACombat_DinosaurCombatController::DetectPlayer,
            DetectionInterval,
            true
        );
    }
}

void ACombat_DinosaurCombatController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardComponent && CombatBehaviorTree)
    {
        if (CombatBehaviorTree->BlackboardAsset)
        {
            BlackboardComponent->InitializeBlackboard(*CombatBehaviorTree->BlackboardAsset);
        }
    }

    if (BehaviorTreeComponent && CombatBehaviorTree)
    {
        BehaviorTreeComponent->StartTree(*CombatBehaviorTree);
    }

    UE_LOG(LogTemp, Warning, TEXT("Combat AI Controller possessed pawn: %s"), InPawn ? *InPawn->GetName() : TEXT("None"));
}

void ACombat_DinosaurCombatController::OnUnPossess()
{
    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }

    // Clear timers
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
        GetWorld()->GetTimerManager().ClearTimer(DetectionTimerHandle);
    }

    Super::OnUnPossess();
}

bool ACombat_DinosaurCombatController::DetectPlayer()
{
    if (!GetPawn())
    {
        return false;
    }

    // Find player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter)
    {
        return false;
    }

    FVector MyLocation = GetPawn()->GetActorLocation();
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    float DistanceToPlayer = FVector::Dist(MyLocation, PlayerLocation);

    // Check if player is within detection range
    if (DistanceToPlayer <= DetectionRadius)
    {
        // Line of sight check
        FHitResult HitResult;
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetPawn());
        QueryParams.AddIgnoredActor(PlayerCharacter);

        bool bHasLineOfSight = !GetWorld()->LineTraceSingleByChannel(
            HitResult,
            MyLocation + FVector(0, 0, 100), // Slightly elevated start
            PlayerLocation + FVector(0, 0, 100), // Slightly elevated end
            ECC_Visibility,
            QueryParams
        );

        if (bHasLineOfSight)
        {
            StartCombat(PlayerCharacter);
            return true;
        }
    }

    return false;
}

void ACombat_DinosaurCombatController::StartCombat(AActor* Target)
{
    if (!Target || bIsInCombat)
    {
        return;
    }

    bIsInCombat = true;
    CurrentTarget = Target;

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
        BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Combat AI: Starting combat with target %s"), *Target->GetName());

    // Start chasing the target
    ChaseTarget(Target);
}

void ACombat_DinosaurCombatController::EndCombat()
{
    bIsInCombat = false;
    CurrentTarget = nullptr;

    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
        BlackboardComponent->SetValueAsBool(TEXT("IsInCombat"), false);
    }

    // Clear attack timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().ClearTimer(AttackTimerHandle);
    }

    UE_LOG(LogTemp, Warning, TEXT("Combat AI: Ending combat"));

    // Return to patrol
    PatrolArea();
}

bool ACombat_DinosaurCombatController::IsInAttackRange(AActor* Target) const
{
    if (!Target || !GetPawn())
    {
        return false;
    }

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), Target->GetActorLocation());
    return Distance <= AttackRange;
}

void ACombat_DinosaurCombatController::AttackTarget()
{
    if (!CurrentTarget || !GetPawn())
    {
        return;
    }

    if (IsInAttackRange(CurrentTarget))
    {
        UE_LOG(LogTemp, Warning, TEXT("Combat AI: Attacking target %s"), *CurrentTarget->GetName());

        // Trigger attack animation/damage here
        // For now, just log the attack

        // Set attack cooldown
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().SetTimer(
                AttackTimerHandle,
                this,
                &ACombat_DinosaurCombatController::AttackTarget,
                AttackCooldown,
                false
            );
        }
    }
    else
    {
        // Continue chasing if not in range
        ChaseTarget(CurrentTarget);
    }
}

void ACombat_DinosaurCombatController::ChaseTarget(AActor* Target)
{
    if (!Target)
    {
        return;
    }

    // Move towards target
    MoveToActor(Target, AttackRange - 100.0f); // Stop slightly before attack range

    UE_LOG(LogTemp, Log, TEXT("Combat AI: Chasing target %s"), *Target->GetName());
}

void ACombat_DinosaurCombatController::PatrolArea()
{
    if (!GetPawn())
    {
        return;
    }

    // Simple patrol behavior - move to a random location nearby
    FVector CurrentLocation = GetPawn()->GetActorLocation();
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0; // Keep on ground level
    RandomDirection.Normalize();

    FVector PatrolTarget = CurrentLocation + (RandomDirection * 1000.0f);
    
    MoveToLocation(PatrolTarget);

    UE_LOG(LogTemp, Log, TEXT("Combat AI: Patrolling to location"));
}

void ACombat_DinosaurCombatController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            // Check if this is the player character
            if (Character == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
            {
                StartCombat(Character);
                break;
            }
        }
    }
}