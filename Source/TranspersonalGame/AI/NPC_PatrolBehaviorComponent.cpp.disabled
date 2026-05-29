#include "NPC_PatrolBehaviorComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

UNPC_PatrolBehaviorComponent::UNPC_PatrolBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second

    // Default patrol settings
    PatrolRadius = 2000.0f;
    MovementSpeed = 300.0f;
    bRandomPatrol = false;
    bReverseDirection = false;

    // Default detection settings
    DetectionRadius = 1500.0f;
    AlertRadius = 3000.0f;

    // Initialize state
    CurrentState = ENPC_PatrolState::Idle;
    CurrentPatrolIndex = 0;
    HomeLocation = FVector::ZeroVector;
    DetectedTarget = nullptr;

    // Movement tracking
    LastPosition = FVector::ZeroVector;
    StuckTimer = 0.0f;
    MaxStuckTime = 5.0f;

    // Wait state
    CurrentWaitTime = 0.0f;
    bIsWaiting = false;
    bPatrolActive = false;
}

void UNPC_PatrolBehaviorComponent::BeginPlay()
{
    Super::BeginPlay();

    // Set home location to current position
    if (AActor* Owner = GetOwner())
    {
        HomeLocation = Owner->GetActorLocation();
        LastPosition = HomeLocation;
    }

    // Generate default patrol points if none exist
    if (PatrolPoints.Num() == 0)
    {
        GenerateRandomPatrolPoints(4);
    }

    // Start patrol automatically
    StartPatrol();
}

void UNPC_PatrolBehaviorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!bPatrolActive)
        return;

    // Check for targets
    AActor* NewTarget = ScanForTargets();
    if (NewTarget && NewTarget != DetectedTarget)
    {
        DetectedTarget = NewTarget;
        CurrentState = ENPC_PatrolState::Alerted;
        OnTargetDetected(NewTarget);
        HandleDetectedTarget();
        return;
    }

    // Update patrol behavior
    UpdatePatrolBehavior(DeltaTime);

    // Check if stuck
    if (AActor* Owner = GetOwner())
    {
        FVector CurrentPosition = Owner->GetActorLocation();
        float DistanceMoved = FVector::Dist(CurrentPosition, LastPosition);
        
        if (DistanceMoved < 10.0f && CurrentState == ENPC_PatrolState::MovingToPoint)
        {
            StuckTimer += DeltaTime;
            if (StuckTimer > MaxStuckTime)
            {
                // Try next patrol point
                MoveToNextPatrolPoint();
                StuckTimer = 0.0f;
            }
        }
        else
        {
            StuckTimer = 0.0f;
        }
        
        LastPosition = CurrentPosition;
    }
}

void UNPC_PatrolBehaviorComponent::UpdatePatrolBehavior(float DeltaTime)
{
    switch (CurrentState)
    {
        case ENPC_PatrolState::Idle:
            if (PatrolPoints.Num() > 0)
            {
                CurrentState = ENPC_PatrolState::MovingToPoint;
            }
            break;

        case ENPC_PatrolState::MovingToPoint:
            if (PatrolPoints.IsValidIndex(CurrentPatrolIndex))
            {
                FVector TargetLocation = PatrolPoints[CurrentPatrolIndex].Location;
                
                if (IsAtLocation(TargetLocation))
                {
                    // Reached patrol point
                    OnPatrolPointReached(CurrentPatrolIndex);
                    
                    float WaitTime = PatrolPoints[CurrentPatrolIndex].WaitTime;
                    if (WaitTime > 0.0f)
                    {
                        bIsWaiting = true;
                        CurrentWaitTime = WaitTime;
                        
                        // Look around if specified
                        if (PatrolPoints[CurrentPatrolIndex].bLookAround)
                        {
                            if (AActor* Owner = GetOwner())
                            {
                                FRotator RandomRotation = FRotator(0, FMath::RandRange(0.0f, 360.0f), 0);
                                Owner->SetActorRotation(RandomRotation);
                            }
                        }
                    }
                    else
                    {
                        MoveToNextPatrolPoint();
                    }
                }
                else
                {
                    // Move towards patrol point
                    if (AActor* Owner = GetOwner())
                    {
                        FVector CurrentLocation = Owner->GetActorLocation();
                        FVector Direction = (TargetLocation - CurrentLocation).GetSafeNormal();
                        
                        // Rotate to face movement direction
                        FRotator TargetRotation = Direction.Rotation();
                        Owner->SetActorRotation(FMath::RInterpTo(Owner->GetActorRotation(), TargetRotation, DeltaTime, 2.0f));
                        
                        // Move towards target
                        FVector NewLocation = CurrentLocation + (Direction * MovementSpeed * DeltaTime);
                        Owner->SetActorLocation(NewLocation);
                    }
                }
            }
            break;

        case ENPC_PatrolState::Investigating:
            // Handle investigation behavior
            if (bIsWaiting)
            {
                CurrentWaitTime -= DeltaTime;
                if (CurrentWaitTime <= 0.0f)
                {
                    bIsWaiting = false;
                    ReturnToPatrol();
                }
            }
            break;

        case ENPC_PatrolState::Returning:
            if (IsAtLocation(HomeLocation))
            {
                CurrentState = ENPC_PatrolState::Idle;
                OnReturnedHome();
            }
            break;

        case ENPC_PatrolState::Alerted:
            // Handle alert behavior - investigate for a while then return
            if (!bIsWaiting)
            {
                bIsWaiting = true;
                CurrentWaitTime = 5.0f; // Alert for 5 seconds
            }
            else
            {
                CurrentWaitTime -= DeltaTime;
                if (CurrentWaitTime <= 0.0f)
                {
                    bIsWaiting = false;
                    DetectedTarget = nullptr;
                    ReturnToPatrol();
                }
            }
            break;
    }

    // Handle waiting state
    if (bIsWaiting && CurrentState == ENPC_PatrolState::MovingToPoint)
    {
        CurrentWaitTime -= DeltaTime;
        if (CurrentWaitTime <= 0.0f)
        {
            bIsWaiting = false;
            MoveToNextPatrolPoint();
        }
    }
}

void UNPC_PatrolBehaviorComponent::MoveToNextPatrolPoint()
{
    if (PatrolPoints.Num() == 0)
        return;

    if (bRandomPatrol)
    {
        CurrentPatrolIndex = FMath::RandRange(0, PatrolPoints.Num() - 1);
    }
    else
    {
        if (bReverseDirection)
        {
            CurrentPatrolIndex--;
            if (CurrentPatrolIndex < 0)
            {
                CurrentPatrolIndex = PatrolPoints.Num() - 1;
            }
        }
        else
        {
            CurrentPatrolIndex++;
            if (CurrentPatrolIndex >= PatrolPoints.Num())
            {
                CurrentPatrolIndex = 0;
            }
        }
    }

    CurrentState = ENPC_PatrolState::MovingToPoint;
}

void UNPC_PatrolBehaviorComponent::HandleDetectedTarget()
{
    if (!DetectedTarget)
        return;

    // Simple alert behavior - look at target and wait
    if (AActor* Owner = GetOwner())
    {
        FVector TargetLocation = DetectedTarget->GetActorLocation();
        FVector OwnerLocation = Owner->GetActorLocation();
        FVector Direction = (TargetLocation - OwnerLocation).GetSafeNormal();
        
        FRotator TargetRotation = Direction.Rotation();
        Owner->SetActorRotation(TargetRotation);
    }
}

void UNPC_PatrolBehaviorComponent::ReturnToPatrol()
{
    CurrentState = ENPC_PatrolState::MovingToPoint;
    DetectedTarget = nullptr;
}

AActor* UNPC_PatrolBehaviorComponent::ScanForTargets()
{
    if (!GetOwner())
        return nullptr;

    UWorld* World = GetWorld();
    if (!World)
        return nullptr;

    FVector OwnerLocation = GetOwner()->GetActorLocation();

    // Scan for target classes
    for (TSubclassOf<AActor> TargetClass : TargetClasses)
    {
        if (!TargetClass)
            continue;

        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, TargetClass, FoundActors);

        for (AActor* Actor : FoundActors)
        {
            if (!Actor || Actor == GetOwner())
                continue;

            float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
            if (Distance <= DetectionRadius)
            {
                return Actor;
            }
        }
    }

    return nullptr;
}

void UNPC_PatrolBehaviorComponent::StartPatrol()
{
    bPatrolActive = true;
    CurrentState = ENPC_PatrolState::Idle;
}

void UNPC_PatrolBehaviorComponent::StopPatrol()
{
    bPatrolActive = false;
    CurrentState = ENPC_PatrolState::Idle;
    DetectedTarget = nullptr;
}

void UNPC_PatrolBehaviorComponent::SetPatrolRadius(float NewRadius)
{
    PatrolRadius = FMath::Max(NewRadius, 100.0f);
    
    // Regenerate patrol points with new radius
    if (PatrolPoints.Num() > 0)
    {
        int32 NumPoints = PatrolPoints.Num();
        PatrolPoints.Empty();
        GenerateRandomPatrolPoints(NumPoints);
    }
}

void UNPC_PatrolBehaviorComponent::AddPatrolPoint(FVector Location, float WaitTime)
{
    FNPC_PatrolPoint NewPoint;
    NewPoint.Location = Location;
    NewPoint.WaitTime = WaitTime;
    NewPoint.bLookAround = true;
    
    PatrolPoints.Add(NewPoint);
}

void UNPC_PatrolBehaviorComponent::GenerateRandomPatrolPoints(int32 NumPoints)
{
    PatrolPoints.Empty();
    
    if (NumPoints <= 0)
        return;

    for (int32 i = 0; i < NumPoints; i++)
    {
        FVector PatrolPoint = GetRandomPointInRadius(HomeLocation, PatrolRadius);
        AddPatrolPoint(PatrolPoint, FMath::RandRange(2.0f, 8.0f));
    }
}

FVector UNPC_PatrolBehaviorComponent::GetRandomPointInRadius(FVector Center, float Radius)
{
    float RandomAngle = FMath::RandRange(0.0f, 360.0f);
    float RandomDistance = FMath::RandRange(Radius * 0.3f, Radius);
    
    FVector RandomDirection = FVector(
        FMath::Cos(FMath::DegreesToRadians(RandomAngle)),
        FMath::Sin(FMath::DegreesToRadians(RandomAngle)),
        0.0f
    );
    
    return Center + (RandomDirection * RandomDistance);
}

bool UNPC_PatrolBehaviorComponent::IsAtLocation(FVector TargetLocation, float Tolerance)
{
    if (!GetOwner())
        return false;

    FVector CurrentLocation = GetOwner()->GetActorLocation();
    float Distance = FVector::Dist2D(CurrentLocation, TargetLocation);
    
    return Distance <= Tolerance;
}