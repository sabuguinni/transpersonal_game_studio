#include "BTTask_CircleTarget.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_CircleTarget::UBTTask_CircleTarget()
{
    NodeName = TEXT("Circle Target");
    bNotifyTick = true;
    bNotifyTaskFinished = true;
    bCreateNodeInstance = true;
    
    // Initialize default values
    CircleRadius = 500.0f;
    CircleSpeed = 45.0f;
    bClockwise = true;
    RadiusTolerance = 100.0f;
    MaxCircleTime = 10.0f;
    bFaceTarget = true;
    
    // Setup blackboard key
    TargetKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_CircleTarget, TargetKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_CircleTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    FCircleTargetMemory* Memory = reinterpret_cast<FCircleTargetMemory*>(NodeMemory);
    if (!Memory)
    {
        return EBTNodeResult::Failed;
    }

    // Initialize memory
    Memory->CurrentAngle = 0.0f;
    Memory->ElapsedTime = 0.0f;
    Memory->TargetLocation = FVector::ZeroVector;
    Memory->bInitialized = false;

    // Get the target actor
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_CircleTarget: No blackboard component"));
        return EBTNodeResult::Failed;
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!TargetActor)
    {
        UE_LOG(LogTemp, Warning, TEXT("BTTask_CircleTarget: No target actor found"));
        return EBTNodeResult::Failed;
    }

    // Get AI controller and pawn
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return EBTNodeResult::Failed;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    Memory->TargetLocation = TargetActor->GetActorLocation();
    
    // Calculate initial angle based on current position relative to target
    FVector ToAI = ControlledPawn->GetActorLocation() - Memory->TargetLocation;
    ToAI.Z = 0.0f; // Keep on horizontal plane
    Memory->CurrentAngle = FMath::Atan2(ToAI.Y, ToAI.X);
    Memory->bInitialized = true;

    UE_LOG(LogTemp, Log, TEXT("BTTask_CircleTarget: Starting circle behavior around %s"), *TargetActor->GetName());
    
    return EBTNodeResult::InProgress;
}

void UBTTask_CircleTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    FCircleTargetMemory* Memory = reinterpret_cast<FCircleTargetMemory*>(NodeMemory);
    if (!Memory || !Memory->bInitialized)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Update elapsed time
    Memory->ElapsedTime += DeltaSeconds;
    
    // Check if we've exceeded max circle time
    if (Memory->ElapsedTime >= MaxCircleTime)
    {
        UE_LOG(LogTemp, Log, TEXT("BTTask_CircleTarget: Max circle time reached"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // Get components
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    
    // Update target location
    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetKey.SelectedKeyName));
    if (!TargetActor)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }
    
    Memory->TargetLocation = TargetActor->GetActorLocation();

    // Update angle based on circle speed and direction
    float AngleDelta = FMath::DegreesToRadians(CircleSpeed) * DeltaSeconds;
    if (!bClockwise)
    {
        AngleDelta = -AngleDelta;
    }
    Memory->CurrentAngle += AngleDelta;

    // Calculate desired position
    FVector DesiredPosition = CalculateCirclePosition(Memory->TargetLocation, Memory->CurrentAngle, CircleRadius);
    
    // Move to desired position
    FVector CurrentPosition = ControlledPawn->GetActorLocation();
    float DistanceToDesired = FVector::Dist2D(CurrentPosition, DesiredPosition);
    
    // If we're close enough to desired position, continue circling
    if (DistanceToDesired > RadiusTolerance)
    {
        // Move towards desired position
        AIController->MoveToLocation(DesiredPosition, 50.0f, false, true, false, true);
    }

    // Handle facing the target
    if (bFaceTarget)
    {
        UpdateFacing(OwnerComp, Memory->TargetLocation);
    }

    // Debug drawing
    if (CVarAIDebugDraw.GetValueOnGameThread())
    {
        UWorld* World = ControlledPawn->GetWorld();
        if (World)
        {
            // Draw circle path
            DrawDebugCircle(World, Memory->TargetLocation, CircleRadius, 32, FColor::Yellow, false, 0.1f, 0, 2.0f, FVector(0, 0, 1));
            
            // Draw desired position
            DrawDebugSphere(World, DesiredPosition, 25.0f, 8, FColor::Green, false, 0.1f);
            
            // Draw line to target
            if (bFaceTarget)
            {
                DrawDebugLine(World, CurrentPosition, Memory->TargetLocation, FColor::Red, false, 0.1f, 0, 2.0f);
            }
        }
    }
}

uint16 UBTTask_CircleTarget::GetInstanceMemorySize() const
{
    return sizeof(FCircleTargetMemory);
}

FVector UBTTask_CircleTarget::CalculateCirclePosition(const FVector& TargetLocation, float Angle, float Radius) const
{
    FVector Position;
    Position.X = TargetLocation.X + Radius * FMath::Cos(Angle);
    Position.Y = TargetLocation.Y + Radius * FMath::Sin(Angle);
    Position.Z = TargetLocation.Z; // Keep at target's Z level
    
    return Position;
}

void UBTTask_CircleTarget::UpdateFacing(UBehaviorTreeComponent& OwnerComp, const FVector& TargetLocation) const
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController || !AIController->GetPawn())
    {
        return;
    }

    APawn* ControlledPawn = AIController->GetPawn();
    FVector CurrentLocation = ControlledPawn->GetActorLocation();
    FVector DirectionToTarget = (TargetLocation - CurrentLocation).GetSafeNormal();
    
    if (!DirectionToTarget.IsZero())
    {
        FRotator DesiredRotation = DirectionToTarget.Rotation();
        DesiredRotation.Pitch = 0.0f; // Keep level
        DesiredRotation.Roll = 0.0f;
        
        // Smoothly rotate towards target
        FRotator CurrentRotation = ControlledPawn->GetActorRotation();
        FRotator NewRotation = FMath::RInterpTo(CurrentRotation, DesiredRotation, GetWorld()->GetDeltaSeconds(), 2.0f);
        
        ControlledPawn->SetActorRotation(NewRotation);
    }
}