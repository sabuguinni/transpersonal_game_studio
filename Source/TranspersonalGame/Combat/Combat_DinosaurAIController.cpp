#include "Combat_DinosaurAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"

ACombat_DinosaurAIController::ACombat_DinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize components
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));

    // Set default values
    DetectionRadius = 2000.0f;
    AttackRange = 300.0f;
    ChaseSpeed = 600.0f;
    PatrolSpeed = 200.0f;
    PatrolRadius = 1000.0f;
    
    CurrentTarget = nullptr;
    bIsHunting = false;

    // Configure AI sight
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = DetectionRadius;
        SightConfig->LoseSightRadius = DetectionRadius + 500.0f;
        SightConfig->PeripheralVisionAngleDegrees = 90.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;

        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    }

    // Bind perception events
    AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurAIController::OnPerceptionUpdated);
}

void ACombat_DinosaurAIController::BeginPlay()
{
    Super::BeginPlay();

    if (GetPawn())
    {
        PatrolCenter = GetPawn()->GetActorLocation();
    }
}

void ACombat_DinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (BlackboardAsset && BlackboardComponent)
    {
        UseBlackboard(BlackboardAsset);
    }

    if (BehaviorTree && BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StartTree(*BehaviorTree);
    }

    UE_LOG(LogTemp, Warning, TEXT("DinosaurAIController possessed pawn: %s"), InPawn ? *InPawn->GetName() : TEXT("None"));
}

void ACombat_DinosaurAIController::StartHunting(AActor* Target)
{
    if (!Target) return;

    CurrentTarget = Target;
    bIsHunting = true;

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Target);
        BlackboardComponent->SetValueAsBool(TEXT("IsHunting"), true);
    }

    // Move towards target
    MoveToActor(Target, AttackRange);

    UE_LOG(LogTemp, Warning, TEXT("T-Rex started hunting: %s"), *Target->GetName());
}

void ACombat_DinosaurAIController::StopHunting()
{
    CurrentTarget = nullptr;
    bIsHunting = false;

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), nullptr);
        BlackboardComponent->SetValueAsBool(TEXT("IsHunting"), false);
    }

    StopMovement();
    PatrolArea();

    UE_LOG(LogTemp, Warning, TEXT("T-Rex stopped hunting"));
}

bool ACombat_DinosaurAIController::DetectPlayer()
{
    if (!GetWorld()) return false;

    // Find player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter) return false;

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return false;

    // Check distance
    float Distance = FVector::Dist(MyPawn->GetActorLocation(), PlayerCharacter->GetActorLocation());
    
    if (Distance <= DetectionRadius)
    {
        // Line trace to check visibility
        FHitResult HitResult;
        FVector Start = MyPawn->GetActorLocation();
        FVector End = PlayerCharacter->GetActorLocation();
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(MyPawn);
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECollisionChannel::ECC_Visibility,
            QueryParams
        );

        // If no obstacle or hit the player
        if (!bHit || HitResult.GetActor() == PlayerCharacter)
        {
            StartHunting(PlayerCharacter);
            return true;
        }
    }

    return false;
}

void ACombat_DinosaurAIController::AttackTarget()
{
    if (!CurrentTarget || !GetPawn()) return;

    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (Distance <= AttackRange)
    {
        // Stop moving and attack
        StopMovement();
        
        // Face the target
        FVector Direction = (CurrentTarget->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
        FRotator NewRotation = Direction.Rotation();
        GetPawn()->SetActorRotation(NewRotation);

        UE_LOG(LogTemp, Warning, TEXT("T-Rex attacking target!"));
        
        // TODO: Trigger attack animation and damage
    }
    else
    {
        // Continue chasing
        MoveToActor(CurrentTarget, AttackRange);
    }
}

void ACombat_DinosaurAIController::PatrolArea()
{
    if (!GetPawn()) return;

    // Generate random patrol point around center
    FVector RandomDirection = FMath::VRand();
    RandomDirection.Z = 0; // Keep on ground level
    RandomDirection.Normalize();
    
    FVector PatrolPoint = PatrolCenter + (RandomDirection * FMath::RandRange(200.0f, PatrolRadius));
    
    MoveToLocation(PatrolPoint, 100.0f);

    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsVector(TEXT("PatrolPoint"), PatrolPoint);
    }

    UE_LOG(LogTemp, Log, TEXT("T-Rex patrolling to: %s"), *PatrolPoint.ToString());
}

void ACombat_DinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (ACharacter* Character = Cast<ACharacter>(Actor))
        {
            // Found player character
            if (Character == UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))
            {
                StartHunting(Character);
                break;
            }
        }
    }
}