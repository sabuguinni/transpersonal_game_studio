#include "Combat_DinosaurAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Perception/AISense_Sight.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "NavigationSystem.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"

ACombat_DinosaurAIController::ACombat_DinosaurAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize AI components
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Combat defaults
    bIsInCombat = false;
    CombatRange = 300.0f;
    DetectionRange = 2000.0f;
    AttackCooldown = 2.0f;
    LastAttackTime = 0.0f;
    CurrentTarget = nullptr;
    
    SetupPerception();
}

void ACombat_DinosaurAIController::BeginPlay()
{
    Super::BeginPlay();
    
    // Start combat checking timer
    if (GetWorld())
    {
        GetWorld()->GetTimerManager().SetTimer(
            CombatCheckTimer,
            this,
            &ACombat_DinosaurAIController::CombatTick,
            0.5f,
            true
        );
    }
}

void ACombat_DinosaurAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (BehaviorTreeAsset && BlackboardComponent)
    {
        // Initialize blackboard
        UseBlackboard(BehaviorTreeAsset->BlackboardAsset);
        
        // Start behavior tree
        RunBehaviorTree(BehaviorTreeAsset);
        
        UE_LOG(LogTemp, Warning, TEXT("Combat AI Controller possessed pawn: %s"), 
               InPawn ? *InPawn->GetName() : TEXT("NULL"));
    }
}

void ACombat_DinosaurAIController::SetupPerception()
{
    if (!AIPerceptionComponent)
        return;
        
    // Create sight configuration
    SightConfig = CreateDefaultSubobject<UAISightConfig>(TEXT("SightConfig"));
    if (SightConfig)
    {
        SightConfig->SightRadius = DetectionRange;
        SightConfig->LoseSightRadius = DetectionRange * 1.2f;
        SightConfig->PeripheralVisionAngleDegrees = 120.0f;
        SightConfig->SetMaxAge(5.0f);
        SightConfig->AutoSuccessRangeFromLastSeenLocation = 500.0f;
        
        // Configure what we can see
        SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
        SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
        SightConfig->DetectionByAffiliation.bDetectEnemies = true;
        
        // Add to perception component
        AIPerceptionComponent->ConfigureSense(*SightConfig);
        AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
        
        // Bind perception events
        AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &ACombat_DinosaurAIController::OnPerceptionUpdated);
    }
}

bool ACombat_DinosaurAIController::DetectPlayer()
{
    if (!GetWorld())
        return false;
        
    // Find player character
    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
    if (!PlayerCharacter || !GetPawn())
        return false;
        
    // Check distance
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), PlayerCharacter->GetActorLocation());
    
    if (Distance <= DetectionRange)
    {
        // Update blackboard
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), PlayerCharacter);
            BlackboardComponent->SetValueAsVector(TEXT("TargetLocation"), PlayerCharacter->GetActorLocation());
            BlackboardComponent->SetValueAsBool(TEXT("HasTarget"), true);
        }
        
        // Start combat if close enough
        if (Distance <= CombatRange && !bIsInCombat)
        {
            StartCombat(PlayerCharacter);
        }
        
        return true;
    }
    
    return false;
}

void ACombat_DinosaurAIController::StartCombat(AActor* Target)
{
    if (!Target)
        return;
        
    bIsInCombat = true;
    CurrentTarget = Target;
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InCombat"), true);
        BlackboardComponent->SetValueAsObject(TEXT("CombatTarget"), Target);
    }
    
    // Move towards target
    MoveToActor(Target, 200.0f);
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur AI starting combat with: %s"), 
           Target ? *Target->GetName() : TEXT("NULL"));
}

void ACombat_DinosaurAIController::EndCombat()
{
    bIsInCombat = false;
    CurrentTarget = nullptr;
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsBool(TEXT("InCombat"), false);
        BlackboardComponent->SetValueAsObject(TEXT("CombatTarget"), nullptr);
    }
    
    // Stop movement
    StopMovement();
    
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur AI ending combat"));
}

bool ACombat_DinosaurAIController::CanAttack() const
{
    if (!bIsInCombat || !CurrentTarget)
        return false;
        
    float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    return (CurrentTime - LastAttackTime) >= AttackCooldown;
}

void ACombat_DinosaurAIController::PerformAttack()
{
    if (!CanAttack() || !CurrentTarget || !GetPawn())
        return;
        
    // Check if target is in range
    float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
    if (Distance > CombatRange)
        return;
        
    // Update attack time
    LastAttackTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Perform attack logic here
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur AI performing attack on: %s"), 
           CurrentTarget ? *CurrentTarget->GetName() : TEXT("NULL"));
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(TEXT("LastAttackTime"), LastAttackTime);
    }
}

void ACombat_DinosaurAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    for (AActor* Actor : UpdatedActors)
    {
        if (Actor && Actor->IsA<ACharacter>())
        {
            // Found a character - likely the player
            if (!bIsInCombat)
            {
                StartCombat(Actor);
            }
            break;
        }
    }
}

void ACombat_DinosaurAIController::CombatTick()
{
    if (bIsInCombat && CurrentTarget)
    {
        // Check if target is still valid and in range
        if (!IsValid(CurrentTarget))
        {
            EndCombat();
            return;
        }
        
        float Distance = FVector::Dist(GetPawn()->GetActorLocation(), CurrentTarget->GetActorLocation());
        
        // End combat if target too far
        if (Distance > DetectionRange * 1.5f)
        {
            EndCombat();
        }
        // Attack if in range and ready
        else if (Distance <= CombatRange && CanAttack())
        {
            PerformAttack();
        }
        // Move towards target if not in attack range
        else if (Distance > CombatRange)
        {
            MoveToActor(CurrentTarget, 200.0f);
        }
    }
    else
    {
        // Not in combat - check for targets
        DetectPlayer();
    }
}