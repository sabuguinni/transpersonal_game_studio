#include "NPC_DinosaurBehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

UNPC_DinosaurBehaviorTree::UNPC_DinosaurBehaviorTree()
{
    CurrentState = ENPC_DinosaurState::Idle;
    OwnerPawn = nullptr;
    OwnerController = nullptr;
    BehaviorTreeAsset = nullptr;
    BehaviorTreeComponent = nullptr;
    BlackboardComponent = nullptr;
    
    InitializeDefaultStats();
}

void UNPC_DinosaurBehaviorTree::InitializeBehaviorTree(APawn* DinosaurPawn, AAIController* AIController)
{
    if (!DinosaurPawn || !AIController)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorTree: Invalid pawn or controller"));
        return;
    }

    OwnerPawn = DinosaurPawn;
    OwnerController = AIController;
    
    // Get behavior tree component from AI controller
    BehaviorTreeComponent = AIController->FindComponentByClass<UBehaviorTreeComponent>();
    if (!BehaviorTreeComponent)
    {
        BehaviorTreeComponent = AIController->GetBrainComponent();
    }
    
    // Get blackboard component
    BlackboardComponent = AIController->FindComponentByClass<UBlackboardComponent>();
    
    // Generate initial patrol points around spawn location
    FVector SpawnLocation = DinosaurPawn->GetActorLocation();
    GeneratePatrolPoints(SpawnLocation, DinosaurStats.PatrolRadius);
    
    // Set initial state
    UpdateBehaviorState(ENPC_DinosaurState::Patrolling);
    
    UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorTree: Initialized for %s"), *DinosaurPawn->GetName());
}

void UNPC_DinosaurBehaviorTree::UpdateBehaviorState(ENPC_DinosaurState NewState)
{
    if (CurrentState != NewState)
    {
        ENPC_DinosaurState PreviousState = CurrentState;
        CurrentState = NewState;
        
        // Update blackboard with new state
        if (BlackboardComponent)
        {
            BlackboardComponent->SetValueAsEnum(FName("DinosaurState"), static_cast<uint8>(NewState));
        }
        
        UE_LOG(LogTemp, Log, TEXT("NPC_DinosaurBehaviorTree: State changed from %d to %d"), 
               static_cast<int32>(PreviousState), static_cast<int32>(NewState));
    }
}

void UNPC_DinosaurBehaviorTree::UpdatePlayerMemory(FVector PlayerLocation, bool bPlayerVisible)
{
    if (bPlayerVisible)
    {
        DinosaurMemory.LastKnownPlayerLocation = PlayerLocation;
        DinosaurMemory.TimeSincePlayerSeen = 0.0f;
        DinosaurMemory.bHasSeenPlayer = true;
        
        // Update blackboard
        if (BlackboardComponent)
        {
            SetBlackboardValue(BlackboardComponent, FName("PlayerLocation"), PlayerLocation);
            SetBlackboardBool(BlackboardComponent, FName("HasSeenPlayer"), true);
        }
    }
    else
    {
        DinosaurMemory.TimeSincePlayerSeen += GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.016f;
        
        // Forget player after 30 seconds
        if (DinosaurMemory.TimeSincePlayerSeen > 30.0f)
        {
            DinosaurMemory.bHasSeenPlayer = false;
            if (BlackboardComponent)
            {
                SetBlackboardBool(BlackboardComponent, FName("HasSeenPlayer"), false);
            }
        }
    }
}

void UNPC_DinosaurBehaviorTree::SetupPatrolPoints(const TArray<FVector>& Points)
{
    DinosaurMemory.PatrolPoints = Points;
    DinosaurMemory.CurrentPatrolIndex = 0;
    
    if (BlackboardComponent && Points.Num() > 0)
    {
        SetBlackboardValue(BlackboardComponent, FName("PatrolTarget"), Points[0]);
    }
}

FVector UNPC_DinosaurBehaviorTree::GetNextPatrolPoint()
{
    if (DinosaurMemory.PatrolPoints.Num() == 0)
    {
        return OwnerPawn ? OwnerPawn->GetActorLocation() : FVector::ZeroVector;
    }
    
    return DinosaurMemory.PatrolPoints[DinosaurMemory.CurrentPatrolIndex];
}

void UNPC_DinosaurBehaviorTree::AdvancePatrolPoint()
{
    if (DinosaurMemory.PatrolPoints.Num() > 0)
    {
        DinosaurMemory.CurrentPatrolIndex = (DinosaurMemory.CurrentPatrolIndex + 1) % DinosaurMemory.PatrolPoints.Num();
        
        if (BlackboardComponent)
        {
            SetBlackboardValue(BlackboardComponent, FName("PatrolTarget"), GetNextPatrolPoint());
        }
    }
}

bool UNPC_DinosaurBehaviorTree::ShouldAttackPlayer(FVector PlayerLocation) const
{
    if (!OwnerPawn || !IsAlive())
    {
        return false;
    }
    
    float DistanceToPlayer = CalculateDistanceToPlayer(PlayerLocation);
    return DistanceToPlayer <= DinosaurStats.AttackRange && DinosaurMemory.bHasSeenPlayer;
}

bool UNPC_DinosaurBehaviorTree::ShouldFleeFromPlayer(FVector PlayerLocation) const
{
    if (!OwnerPawn || !IsAlive())
    {
        return false;
    }
    
    // Flee if health is low or fear level is high
    bool bLowHealth = DinosaurStats.Health < (DinosaurStats.MaxHealth * 0.3f);
    bool bHighFear = DinosaurMemory.FearLevel > 70.0f;
    
    return (bLowHealth || bHighFear) && DinosaurMemory.bHasSeenPlayer;
}

bool UNPC_DinosaurBehaviorTree::ShouldHuntPlayer(FVector PlayerLocation) const
{
    if (!OwnerPawn || !IsAlive())
    {
        return false;
    }
    
    float DistanceToPlayer = CalculateDistanceToPlayer(PlayerLocation);
    bool bInDetectionRange = DistanceToPlayer <= DinosaurStats.DetectionRange;
    bool bHighHunger = DinosaurMemory.HungerLevel > 60.0f;
    
    return bInDetectionRange && (DinosaurMemory.bHasSeenPlayer || bHighHunger);
}

void UNPC_DinosaurBehaviorTree::UpdateBlackboard(UBlackboardComponent* BlackboardComp)
{
    if (!BlackboardComp)
    {
        return;
    }
    
    // Update all relevant blackboard keys
    BlackboardComp->SetValueAsEnum(FName("DinosaurState"), static_cast<uint8>(CurrentState));
    BlackboardComp->SetValueAsVector(FName("PatrolTarget"), GetNextPatrolPoint());
    BlackboardComp->SetValueAsFloat(FName("Health"), DinosaurStats.Health);
    BlackboardComp->SetValueAsFloat(FName("HungerLevel"), DinosaurMemory.HungerLevel);
    BlackboardComp->SetValueAsFloat(FName("FearLevel"), DinosaurMemory.FearLevel);
    BlackboardComp->SetValueAsBool(FName("HasSeenPlayer"), DinosaurMemory.bHasSeenPlayer);
    
    if (DinosaurMemory.bHasSeenPlayer)
    {
        BlackboardComp->SetValueAsVector(FName("PlayerLocation"), DinosaurMemory.LastKnownPlayerLocation);
    }
}

void UNPC_DinosaurBehaviorTree::SetBlackboardValue(UBlackboardComponent* BlackboardComp, FName KeyName, FVector Value)
{
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsVector(KeyName, Value);
    }
}

void UNPC_DinosaurBehaviorTree::SetBlackboardBool(UBlackboardComponent* BlackboardComp, FName KeyName, bool Value)
{
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsBool(KeyName, Value);
    }
}

void UNPC_DinosaurBehaviorTree::TakeDamage(float DamageAmount)
{
    DinosaurStats.Health = FMath::Max(0.0f, DinosaurStats.Health - DamageAmount);
    
    // Increase fear when taking damage
    DinosaurMemory.FearLevel = FMath::Min(100.0f, DinosaurMemory.FearLevel + (DamageAmount * 2.0f));
    
    // Update blackboard
    if (BlackboardComponent)
    {
        BlackboardComponent->SetValueAsFloat(FName("Health"), DinosaurStats.Health);
        BlackboardComponent->SetValueAsFloat(FName("FearLevel"), DinosaurMemory.FearLevel);
    }
    
    // Change behavior based on damage
    if (DinosaurStats.Health <= 0.0f)
    {
        UpdateBehaviorState(ENPC_DinosaurState::Fleeing);
    }
    else if (DinosaurStats.Health < DinosaurStats.MaxHealth * 0.3f)
    {
        UpdateBehaviorState(ENPC_DinosaurState::Fleeing);
    }
    
    UE_LOG(LogTemp, Warning, TEXT("NPC_DinosaurBehaviorTree: Took %f damage, health now %f"), DamageAmount, DinosaurStats.Health);
}

void UNPC_DinosaurBehaviorTree::InitializeDefaultStats()
{
    DinosaurStats.Health = 100.0f;
    DinosaurStats.MaxHealth = 100.0f;
    DinosaurStats.AttackDamage = 25.0f;
    DinosaurStats.MovementSpeed = 300.0f;
    DinosaurStats.DetectionRange = 1500.0f;
    DinosaurStats.AttackRange = 200.0f;
    DinosaurStats.PatrolRadius = 2000.0f;
}

void UNPC_DinosaurBehaviorTree::GeneratePatrolPoints(FVector CenterLocation, float Radius)
{
    DinosaurMemory.PatrolPoints.Empty();
    
    // Generate 4-6 patrol points in a circle around the center
    int32 NumPoints = FMath::RandRange(4, 6);
    float AngleStep = 360.0f / NumPoints;
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = i * AngleStep;
        float RadianAngle = FMath::DegreesToRadians(Angle);
        
        FVector Offset = FVector(
            FMath::Cos(RadianAngle) * Radius,
            FMath::Sin(RadianAngle) * Radius,
            0.0f
        );
        
        FVector PatrolPoint = CenterLocation + Offset;
        DinosaurMemory.PatrolPoints.Add(PatrolPoint);
    }
    
    DinosaurMemory.CurrentPatrolIndex = 0;
}

float UNPC_DinosaurBehaviorTree::CalculateDistanceToPlayer(FVector PlayerLocation) const
{
    if (!OwnerPawn)
    {
        return 99999.0f;
    }
    
    return FVector::Dist(OwnerPawn->GetActorLocation(), PlayerLocation);
}

bool UNPC_DinosaurBehaviorTree::IsPlayerInRange(FVector PlayerLocation, float Range) const
{
    return CalculateDistanceToPlayer(PlayerLocation) <= Range;
}

void UNPC_DinosaurBehaviorTree::UpdateMemoryTimers(float DeltaTime)
{
    DinosaurMemory.TimeSincePlayerSeen += DeltaTime;
    
    // Gradually reduce fear over time
    DinosaurMemory.FearLevel = FMath::Max(0.0f, DinosaurMemory.FearLevel - (DeltaTime * 5.0f));
    
    // Gradually increase hunger over time
    DinosaurMemory.HungerLevel = FMath::Min(100.0f, DinosaurMemory.HungerLevel + (DeltaTime * 2.0f));
}