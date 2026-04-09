#include "BTTask_PackCoordination.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Navigation/PathFollowingComponent.h"
#include "../DinosaurCombatController.h"

UBTTask_PackCoordination::UBTTask_PackCoordination()
{
    NodeName = TEXT("Pack Coordination");
    bNotifyTick = true;
    bNotifyTaskFinished = true;
    bCreateNodeInstance = true;
    
    // Initialize default values
    PackTactic = EPackTactic::Surround;
    CoordinationDistance = 600.0f;
    TacticDuration = 5.0f;
    bWaitForLeaderSignal = true;
    PositionTolerance = 150.0f;
    
    // Setup blackboard keys
    TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PackCoordination, TargetKey), AActor::StaticClass());
    PackLeaderKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_PackCoordination, PackLeaderKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_PackCoordination::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FPackCoordinationMemory* Memory = reinterpret_cast<FPackCoordinationMemory*>(NodeMemory);
    if (!Memory)
    {
        return EBTNodeResult::Failed;
    }

    // Initialize memory
    Memory->CurrentTactic = PackTactic;
    Memory->TargetPosition = FVector::ZeroVector;
    Memory->AssignedPosition = FVector::ZeroVector;
    Memory->ElapsedTime = 0.0f;
    Memory->bPositionReached = false;
    Memory->bLeaderSignalReceived = !bWaitForLeaderSignal; // If not waiting for signal, consider it received
    Memory->PackMemberIndex = 0;

    // Get components
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_PackCoordination: No blackboard component"));
        return EBTNodeResult::Failed;
    }

    // Get target
    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_PackCoordination: No target actor found"));
        return EBTNodeResult::Failed;
    }

    Memory->TargetPosition = TargetActor->GetActorLocation();

    // Get AI controller
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return EBTNodeResult::Failed;
    }

    // Try to get pack information from combat controller
    ADinosaurCombatController* CombatController = Cast<ADinosaurCombatController>(AIController);
    if (CombatController)
    {
        // Get nearby allies to determine pack size and member index
        TArray<ADinosaurCombatController*> NearbyAllies = CombatController->GetNearbyAllies(1500.0f);
        
        // Find our index in the pack
        for (int32 i = 0; i < NearbyAllies.Num(); i++)
        {
            if (NearbyAllies[i] == CombatController)
            {
                Memory->PackMemberIndex = i;
                break;
            }
        }

        // Calculate tactical position based on pack size
        int32 TotalPackMembers = FMath::Max(1, NearbyAllies.Num());
        Memory->AssignedPosition = CalculateTacticalPosition(
            Memory->TargetPosition, 
            Memory->CurrentTactic, 
            Memory->PackMemberIndex, 
            TotalPackMembers
        );

        UE_LOG(LogTemp, Log, TEXT("BTTask_PackCoordination: Pack member %d of %d executing %s tactic"), 
            Memory->PackMemberIndex, TotalPackMembers, *UEnum::GetValueAsString(Memory->CurrentTactic));
    }
    else
    {
        // Fallback for non-combat controllers
        Memory->AssignedPosition = CalculateTacticalPosition(
            Memory->TargetPosition, 
            Memory->CurrentTactic, 
            0, 
            1
        );
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_PackCoordination::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FPackCoordinationMemory* Memory = reinterpret_cast<FPackCoordinationMemory*>(NodeMemory);
    if (!Memory)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Update elapsed time
    Memory->ElapsedTime += DeltaSeconds;

    // Check if tactic duration exceeded
    if (Memory->ElapsedTime >= TacticDuration)
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_PackCoordination: Tactic duration completed"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // Get AI controller and pawn
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    APawn* ControlledPawn = AIController->GetPawn();

    // Check if we need to wait for leader signal
    if (bWaitForLeaderSignal && !Memory->bLeaderSignalReceived)
    {
        // Check for leader signal (simplified - in real implementation this would be more sophisticated)
        UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
        AActor* PackLeader = Cast<AActor>(BlackboardComp->GetValueAsObject(PackLeaderKey.SelectedKeyName));
        
        if (PackLeader)
        {
            // Simple distance-based signal detection
            float DistanceToLeader = FVector::Dist(ControlledPawn->GetActorLocation(), PackLeader->GetActorLocation());
            if (DistanceToLeader < 800.0f) // Leader is close, assume signal received
            {
                Memory->bLeaderSignalReceived = true;
                UE_LOG(LogTemp, Log, TEXT("BTTask_PackCoordination: Leader signal received"));
            }
        }
        else
        {
            // No leader found, proceed without waiting
            Memory->bLeaderSignalReceived = true;
        }

        if (!Memory->bLeaderSignalReceived)
        {
            // Still waiting for signal, stay in position
            return;
        }
    }

    // Execute the tactic
    ExecuteTactic(OwnerComp, Memory, DeltaSeconds);

    // Debug drawing
    if (CVarAIDebugDraw.GetValueOnGameThread())
    {
        UWorld* World = ControlledPawn->GetWorld();
        if (World)
        {
            // Draw assigned position
            DrawDebugSphere(World, Memory->AssignedPosition, 50.0f, 8, 
                Memory->bPositionReached ? FColor::Green : FColor::Orange, false, 0.1f);
            
            // Draw line to assigned position
            DrawDebugLine(World, ControlledPawn->GetActorLocation(), Memory->AssignedPosition, 
                FColor::Blue, false, 0.1f, 0, 2.0f);
            
            // Draw tactic indicator
            FString TacticName = UEnum::GetValueAsString(Memory->CurrentTactic);
            DrawDebugString(World, ControlledPawn->GetActorLocation() + FVector(0, 0, 100), 
                TacticName, nullptr, FColor::White, 0.1f);
        }
    }
}

uint16 UBTTask_PackCoordination::GetInstanceMemorySize() const
{
    return sizeof(FPackCoordinationMemory);
}

FVector UBTTask_PackCoordination::CalculateTacticalPosition(const FVector& TargetLocation, EPackTactic Tactic, int32 MemberIndex, int32 TotalMembers) const
{
    FVector Position = TargetLocation;
    
    switch (Tactic)
    {
        case EPackTactic::Surround:
        {
            // Arrange pack members in a circle around target
            float AngleStep = 360.0f / FMath::Max(1, TotalMembers);
            float MemberAngle = AngleStep * MemberIndex;
            float AngleRad = FMath::DegreesToRadians(MemberAngle);
            
            Position.X += CoordinationDistance * FMath::Cos(AngleRad);
            Position.Y += CoordinationDistance * FMath::Sin(AngleRad);
            break;
        }
        
        case EPackTactic::FlankLeft:
        {
            // Position to the left side of target
            FVector LeftOffset = FVector(0, -CoordinationDistance, 0);
            Position += LeftOffset + FVector(MemberIndex * 100.0f, 0, 0); // Spread out members
            break;
        }
        
        case EPackTactic::FlankRight:
        {
            // Position to the right side of target
            FVector RightOffset = FVector(0, CoordinationDistance, 0);
            Position += RightOffset + FVector(MemberIndex * 100.0f, 0, 0); // Spread out members
            break;
        }
        
        case EPackTactic::Distract:
        {
            // Position at medium distance for distraction
            float DistractAngle = FMath::DegreesToRadians(45.0f + (MemberIndex * 90.0f));
            Position.X += (CoordinationDistance * 0.7f) * FMath::Cos(DistractAngle);
            Position.Y += (CoordinationDistance * 0.7f) * FMath::Sin(DistractAngle);
            break;
        }
        
        case EPackTactic::DirectAssault:
        {
            // Position directly in front of target for assault
            FVector AssaultOffset = FVector(-CoordinationDistance * 0.5f, MemberIndex * 150.0f - (TotalMembers * 75.0f), 0);
            Position += AssaultOffset;
            break;
        }
        
        case EPackTactic::Retreat:
        {
            // Position behind current location, away from target
            FVector RetreatOffset = FVector(CoordinationDistance * 1.5f, MemberIndex * 200.0f - (TotalMembers * 100.0f), 0);
            Position += RetreatOffset;
            break;
        }
        
        case EPackTactic::Ambush:
        {
            // Position in cover or at distance for ambush
            float AmbushAngle = FMath::DegreesToRadians(180.0f + (MemberIndex * 60.0f));
            Position.X += (CoordinationDistance * 1.2f) * FMath::Cos(AmbushAngle);
            Position.Y += (CoordinationDistance * 1.2f) * FMath::Sin(AmbushAngle);
            break;
        }
    }
    
    return Position;
}

bool UBTTask_PackCoordination::IsPositionReached(const FVector& CurrentPosition, const FVector& TargetPosition, float Tolerance) const
{
    return FVector::Dist2D(CurrentPosition, TargetPosition) <= Tolerance;
}

void UBTTask_PackCoordination::ExecuteTactic(UBehaviorTreeComponent& OwnerComp, FPackCoordinationMemory* Memory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    APawn* ControlledPawn = AIController->GetPawn();
    
    FVector CurrentPosition = ControlledPawn->GetActorLocation();
    
    // Check if we've reached our assigned position
    if (!Memory->bPositionReached)
    {
        Memory->bPositionReached = IsPositionReached(CurrentPosition, Memory->AssignedPosition, PositionTolerance);
        
        if (!Memory->bPositionReached)
        {
            // Move towards assigned position
            AIController->MoveToLocation(Memory->AssignedPosition, PositionTolerance * 0.5f, false, true, false, true);
        }
        else
        {
            UE_LOG(LogTemp, Log, TEXT("BTTask_PackCoordination: Position reached for tactic %s"), 
                *UEnum::GetValueAsString(Memory->CurrentTactic));
        }
    }
    else
    {
        // Position reached, execute tactic-specific behavior
        switch (Memory->CurrentTactic)
        {
            case EPackTactic::Distract:
            {
                // Make noise or movement to distract
                // This could trigger animation or sound events
                break;
            }
            
            case EPackTactic::DirectAssault:
            {
                // Prepare for direct attack
                // This could trigger aggressive animations or abilities
                break;
            }
            
            case EPackTactic::Ambush:
            {
                // Stay hidden and wait
                // This could trigger stealth behavior
                break;
            }
            
            default:
            {
                // For other tactics, maintain position
                break;
            }
        }
    }
}