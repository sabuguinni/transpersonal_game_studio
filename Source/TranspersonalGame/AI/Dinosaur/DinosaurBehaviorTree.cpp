#include "DinosaurBehaviorTree.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"

UDinosaurBehaviorTree::UDinosaurBehaviorTree()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = BehaviorUpdateInterval;
    
    // Initialize default behavior parameters
    TerritoryRadius = 5000.0f;
    DetectionRange = 3000.0f;
    AttackRange = 300.0f;
    PackCoordinationRange = 1500.0f;
    
    // Initialize behavior states
    bIsHunting = false;
    bIsPatrolling = true;
    bIsInPack = false;
    CurrentTarget = nullptr;
    TerritoryCenter = FVector::ZeroVector;
}

void UDinosaurBehaviorTree::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize territory center to spawn location
    if (AActor* Owner = GetOwner())
    {
        TerritoryCenter = Owner->GetActorLocation();
        GeneratePatrolPoints();
    }
    
    // Check for pack members nearby
    UpdatePackBehavior();
}

void UDinosaurBehaviorTree::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastBehaviorUpdate += DeltaTime;
    
    if (LastBehaviorUpdate >= BehaviorUpdateInterval)
    {
        LastBehaviorUpdate = 0.0f;
        
        // Update behavior based on current state
        if (bIsHunting)
        {
            UpdateHuntingBehavior();
        }
        else if (bIsPatrolling)
        {
            UpdatePatrolBehavior();
        }
        
        // Always update pack coordination if in pack
        if (bIsInPack)
        {
            UpdatePackCoordination();
        }
        
        // Check for player detection
        if (!bIsHunting && IsPlayerInDetectionRange())
        {
            AActor* Player = FindNearestPlayer();
            if (Player)
            {
                StartHuntingBehavior(Player);
            }
        }
    }
}

void UDinosaurBehaviorTree::InitializeDinosaurBehavior(const FString& DinosaurType, const FVector& SpawnLocation)
{
    TerritoryCenter = SpawnLocation;
    
    // Set species-specific parameters
    if (DinosaurType == "TRex")
    {
        TerritoryRadius = 8000.0f;
        DetectionRange = 4000.0f;
        AttackRange = 500.0f;
        bIsInPack = false;
    }
    else if (DinosaurType == "Raptor")
    {
        TerritoryRadius = 3000.0f;
        DetectionRange = 2500.0f;
        AttackRange = 200.0f;
        bIsInPack = true;
        PackCoordinationRange = 1500.0f;
    }
    else if (DinosaurType == "Herbivore")
    {
        TerritoryRadius = 4000.0f;
        DetectionRange = 2000.0f;
        AttackRange = 0.0f; // Herbivores flee instead of attack
        bIsInPack = true;
    }
    
    GeneratePatrolPoints();
    UpdatePackBehavior();
}

void UDinosaurBehaviorTree::SetBehaviorState(const FString& NewState)
{
    if (NewState == "Hunting")
    {
        bIsHunting = true;
        bIsPatrolling = false;
    }
    else if (NewState == "Patrolling")
    {
        bIsHunting = false;
        bIsPatrolling = true;
        CurrentTarget = nullptr;
    }
    else if (NewState == "Idle")
    {
        bIsHunting = false;
        bIsPatrolling = false;
        CurrentTarget = nullptr;
    }
}

void UDinosaurBehaviorTree::UpdateTerritoryCenter(const FVector& NewCenter)
{
    TerritoryCenter = NewCenter;
    GeneratePatrolPoints();
}

bool UDinosaurBehaviorTree::IsPlayerInDetectionRange() const
{
    float DistanceToPlayer = GetDistanceToPlayer();
    return DistanceToPlayer > 0.0f && DistanceToPlayer <= DetectionRange;
}

bool UDinosaurBehaviorTree::IsPlayerInAttackRange() const
{
    float DistanceToPlayer = GetDistanceToPlayer();
    return DistanceToPlayer > 0.0f && DistanceToPlayer <= AttackRange;
}

FVector UDinosaurBehaviorTree::GetRandomPatrolPoint() const
{
    if (PatrolPoints.Num() == 0)
    {
        // Fallback to random point around territory center
        float RandomAngle = FMath::RandRange(0.0f, 360.0f);
        float RandomDistance = FMath::RandRange(TerritoryRadius * 0.3f, TerritoryRadius * 0.8f);
        
        FVector RandomOffset = FVector(
            FMath::Cos(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
            FMath::Sin(FMath::DegreesToRadians(RandomAngle)) * RandomDistance,
            0.0f
        );
        
        return TerritoryCenter + RandomOffset;
    }
    
    return PatrolPoints[FMath::RandRange(0, PatrolPoints.Num() - 1)];
}

void UDinosaurBehaviorTree::StartHuntingBehavior(AActor* Target)
{
    if (Target)
    {
        CurrentTarget = Target;
        bIsHunting = true;
        bIsPatrolling = false;
        
        // Update blackboard if available
        if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
        {
            BlackboardComp->SetValueAsObject("TargetActor", Target);
            BlackboardComp->SetValueAsBool("IsHunting", true);
        }
    }
}

void UDinosaurBehaviorTree::StopHuntingBehavior()
{
    CurrentTarget = nullptr;
    bIsHunting = false;
    bIsPatrolling = true;
    
    // Update blackboard if available
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsObject("TargetActor", nullptr);
        BlackboardComp->SetValueAsBool("IsHunting", false);
    }
}

void UDinosaurBehaviorTree::UpdatePackBehavior()
{
    if (!bIsInPack) return;
    
    // Find nearby pack members
    PackMembers.Empty();
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor != GetOwner())
            {
                // Check if actor has dinosaur behavior component
                if (UDinosaurBehaviorTree* OtherBehavior = Actor->FindComponentByClass<UDinosaurBehaviorTree>())
                {
                    float Distance = FVector::Dist(GetOwner()->GetActorLocation(), Actor->GetActorLocation());
                    if (Distance <= PackCoordinationRange)
                    {
                        PackMembers.Add(Actor);
                    }
                }
            }
        }
    }
    
    // Determine pack leader (closest to territory center)
    if (PackMembers.Num() > 0)
    {
        PackLeader = nullptr;
        float ClosestDistance = FLT_MAX;
        
        for (AActor* Member : PackMembers)
        {
            float Distance = FVector::Dist(TerritoryCenter, Member->GetActorLocation());
            if (Distance < ClosestDistance)
            {
                ClosestDistance = Distance;
                PackLeader = Member;
            }
        }
    }
}

void UDinosaurBehaviorTree::UpdatePatrolBehavior()
{
    if (PatrolPoints.Num() == 0) return;
    
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector TargetPatrolPoint = PatrolPoints[CurrentPatrolIndex];
    
    // Check if reached current patrol point
    float DistanceToTarget = FVector::Dist(CurrentLocation, TargetPatrolPoint);
    if (DistanceToTarget < 500.0f) // Within 5 meters
    {
        // Move to next patrol point
        CurrentPatrolIndex = (CurrentPatrolIndex + 1) % PatrolPoints.Num();
    }
    
    // Update blackboard with current patrol target
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsVector("PatrolTarget", TargetPatrolPoint);
        BlackboardComp->SetValueAsBool("IsPatrolling", true);
    }
}

void UDinosaurBehaviorTree::UpdateHuntingBehavior()
{
    if (!CurrentTarget)
    {
        StopHuntingBehavior();
        return;
    }
    
    // Check if target is still in range
    float DistanceToTarget = FVector::Dist(GetOwner()->GetActorLocation(), CurrentTarget->GetActorLocation());
    
    if (DistanceToTarget > DetectionRange * 1.5f) // Lost target
    {
        StopHuntingBehavior();
        return;
    }
    
    // Update blackboard with target information
    if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
    {
        BlackboardComp->SetValueAsObject("TargetActor", CurrentTarget);
        BlackboardComp->SetValueAsVector("TargetLocation", CurrentTarget->GetActorLocation());
        BlackboardComp->SetValueAsBool("CanAttack", DistanceToTarget <= AttackRange);
    }
}

void UDinosaurBehaviorTree::UpdatePackCoordination()
{
    // Coordinate with pack members for hunting
    if (bIsHunting && PackMembers.Num() > 0)
    {
        // Share target information with pack
        for (AActor* Member : PackMembers)
        {
            if (UDinosaurBehaviorTree* MemberBehavior = Member->FindComponentByClass<UDinosaurBehaviorTree>())
            {
                if (!MemberBehavior->bIsHunting && CurrentTarget)
                {
                    MemberBehavior->StartHuntingBehavior(CurrentTarget);
                }
            }
        }
    }
}

void UDinosaurBehaviorTree::GeneratePatrolPoints()
{
    PatrolPoints.Empty();
    
    // Generate 6-8 patrol points in a rough circle around territory center
    int32 NumPoints = FMath::RandRange(6, 8);
    float AngleStep = 360.0f / NumPoints;
    
    for (int32 i = 0; i < NumPoints; i++)
    {
        float Angle = i * AngleStep + FMath::RandRange(-30.0f, 30.0f); // Add some randomness
        float Distance = FMath::RandRange(TerritoryRadius * 0.4f, TerritoryRadius * 0.9f);
        
        FVector PatrolPoint = TerritoryCenter + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.0f
        );
        
        PatrolPoints.Add(PatrolPoint);
    }
    
    CurrentPatrolIndex = 0;
}

AActor* UDinosaurBehaviorTree::FindNearestPlayer() const
{
    if (UWorld* World = GetWorld())
    {
        APlayerController* PlayerController = World->GetFirstPlayerController();
        if (PlayerController && PlayerController->GetPawn())
        {
            return PlayerController->GetPawn();
        }
    }
    return nullptr;
}

float UDinosaurBehaviorTree::GetDistanceToPlayer() const
{
    AActor* Player = FindNearestPlayer();
    if (Player && GetOwner())
    {
        return FVector::Dist(GetOwner()->GetActorLocation(), Player->GetActorLocation());
    }
    return -1.0f;
}