#include "DinosaurCombatAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/Character/TranspersonalCharacter.h"

ADinosaurCombatAIController::ADinosaurCombatAIController()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Create AI Perception Component
    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
    
    // Configure sight sense
    UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
    SightConfig->SightRadius = 5000.0f;
    SightConfig->LoseSightRadius = 5500.0f;
    SightConfig->PeripheralVisionAngleDegrees = 90.0f;
    SightConfig->SetMaxAge(5.0f);
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    
    AIPerceptionComponent->ConfigureSense(*SightConfig);
    AIPerceptionComponent->SetDominantSense(SightConfig->GetSenseImplementation());
    
    // Bind perception events
    AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &ADinosaurCombatAIController::OnTargetPerceptionUpdated);
    
    // Default values
    DetectionRadius = 5000.0f;
    AttackRange = 300.0f;
    MovementSpeed = 600.0f;
    bIsPlayerDetected = false;
    CurrentTarget = nullptr;
    CombatState = ECombat_DinosaurCombatState::Idle;
}

void ADinosaurCombatAIController::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController: BeginPlay started"));
    
    // Start detection timer
    GetWorld()->GetTimerManager().SetTimer(DetectionTimerHandle, this, &ADinosaurCombatAIController::DetectPlayer, 1.0f, true);
}

void ADinosaurCombatAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
    
    if (InPawn)
    {
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController: Possessing pawn %s"), *InPawn->GetName());
        
        // Start behavior tree if available
        if (BehaviorTreeAsset)
        {
            RunBehaviorTree(BehaviorTreeAsset);
            UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController: Behavior tree started"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController: No behavior tree assigned"));
        }
        
        // Set initial blackboard values
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(ECombat_DinosaurCombatState::Idle));
            GetBlackboardComponent()->SetValueAsBool(TEXT("PlayerDetected"), false);
        }
    }
}

void ADinosaurCombatAIController::DetectPlayer()
{
    if (!GetPawn())
        return;
    
    // Get player character
    ATranspersonalCharacter* PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));
    if (!PlayerCharacter)
        return;
    
    FVector DinosaurLocation = GetPawn()->GetActorLocation();
    FVector PlayerLocation = PlayerCharacter->GetActorLocation();
    float DistanceToPlayer = FVector::Dist(DinosaurLocation, PlayerLocation);
    
    // Check if player is within detection radius
    if (DistanceToPlayer <= DetectionRadius)
    {
        // Perform line trace to check line of sight
        FHitResult HitResult;
        FVector Start = DinosaurLocation + FVector(0, 0, 100); // Offset for eye level
        FVector End = PlayerLocation + FVector(0, 0, 100);
        
        FCollisionQueryParams QueryParams;
        QueryParams.AddIgnoredActor(GetPawn());
        QueryParams.AddIgnoredActor(PlayerCharacter);
        
        bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams);
        
        if (!bHit || HitResult.GetActor() == PlayerCharacter)
        {
            // Player detected - start combat behavior
            if (!bIsPlayerDetected)
            {
                bIsPlayerDetected = true;
                CurrentTarget = PlayerCharacter;
                CombatState = ECombat_DinosaurCombatState::Hunting;
                
                UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController: Player detected at distance %.2f"), DistanceToPlayer);
                
                // Update blackboard
                if (GetBlackboardComponent())
                {
                    GetBlackboardComponent()->SetValueAsBool(TEXT("PlayerDetected"), true);
                    GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), PlayerCharacter);
                    GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(ECombat_DinosaurCombatState::Hunting));
                }
                
                // Move towards player
                MoveToPlayer();
            }
            else
            {
                // Continue tracking player
                if (DistanceToPlayer <= AttackRange)
                {
                    CombatState = ECombat_DinosaurCombatState::Attacking;
                    AttackPlayer();
                }
                else
                {
                    MoveToPlayer();
                }
            }
        }
    }
    else if (bIsPlayerDetected && DistanceToPlayer > DetectionRadius * 1.2f)
    {
        // Player lost - return to idle
        bIsPlayerDetected = false;
        CurrentTarget = nullptr;
        CombatState = ECombat_DinosaurCombatState::Idle;
        
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController: Player lost"));
        
        // Update blackboard
        if (GetBlackboardComponent())
        {
            GetBlackboardComponent()->SetValueAsBool(TEXT("PlayerDetected"), false);
            GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), nullptr);
            GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(ECombat_DinosaurCombatState::Idle));
        }
        
        // Stop movement
        StopMovement();
    }
}

void ADinosaurCombatAIController::MoveToPlayer()
{
    if (CurrentTarget && GetPawn())
    {
        // Use AI movement to move towards player
        MoveToActor(CurrentTarget, AttackRange * 0.8f, true, true, false);
        
        UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAIController: Moving to player"));
    }
}

void ADinosaurCombatAIController::AttackPlayer()
{
    if (CurrentTarget && GetPawn())
    {
        // Stop movement for attack
        StopMovement();
        
        // Face the target
        FVector DirectionToTarget = (CurrentTarget->GetActorLocation() - GetPawn()->GetActorLocation()).GetSafeNormal();
        FRotator TargetRotation = DirectionToTarget.Rotation();
        GetPawn()->SetActorRotation(TargetRotation);
        
        // Trigger attack animation/damage
        UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController: Attacking player!"));
        
        // Apply damage to player
        if (ATranspersonalCharacter* Player = Cast<ATranspersonalCharacter>(CurrentTarget))
        {
            // Apply damage through the damage system
            float DamageAmount = 25.0f;
            UGameplayStatics::ApplyPointDamage(Player, DamageAmount, GetPawn()->GetActorLocation(), FHitResult(), this, GetPawn(), nullptr);
        }
        
        // Set cooldown before next attack
        GetWorld()->GetTimerManager().SetTimer(AttackCooldownHandle, [this]()
        {
            CombatState = ECombat_DinosaurCombatState::Hunting;
        }, 2.0f, false);
    }
}

void ADinosaurCombatAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
    if (ATranspersonalCharacter* PlayerCharacter = Cast<ATranspersonalCharacter>(Actor))
    {
        if (Stimulus.WasSuccessfullySensed())
        {
            UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController: Player sensed through AI Perception"));
            
            if (!bIsPlayerDetected)
            {
                bIsPlayerDetected = true;
                CurrentTarget = PlayerCharacter;
                CombatState = ECombat_DinosaurCombatState::Hunting;
                
                // Update blackboard
                if (GetBlackboardComponent())
                {
                    GetBlackboardComponent()->SetValueAsBool(TEXT("PlayerDetected"), true);
                    GetBlackboardComponent()->SetValueAsObject(TEXT("TargetActor"), PlayerCharacter);
                    GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(ECombat_DinosaurCombatState::Hunting));
                }
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("DinosaurCombatAIController: Player lost through AI Perception"));
        }
    }
}

void ADinosaurCombatAIController::SetCombatState(ECombat_DinosaurCombatState NewState)
{
    CombatState = NewState;
    
    if (GetBlackboardComponent())
    {
        GetBlackboardComponent()->SetValueAsEnum(TEXT("CombatState"), static_cast<uint8>(NewState));
    }
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurCombatAIController: Combat state changed to %d"), static_cast<int32>(NewState));
}

ECombat_DinosaurCombatState ADinosaurCombatAIController::GetCombatState() const
{
    return CombatState;
}

bool ADinosaurCombatAIController::IsPlayerDetected() const
{
    return bIsPlayerDetected;
}

AActor* ADinosaurCombatAIController::GetCurrentTarget() const
{
    return CurrentTarget;
}