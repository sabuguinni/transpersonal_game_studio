#include "Crowd_MigrationManager.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/KismetMathLibrary.h"

UCrowd_MigrationManager::UCrowd_MigrationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f;
    
    UpdateInterval = 0.5f;
    SeasonalMigrationChance = 0.1f;
    LastUpdateTime = 0.0f;
}

void UCrowd_MigrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize default migration routes for different biomes
    FCrowd_MigrationRoute SavanaRoute;
    SavanaRoute.Waypoints.Add(FVector(0, 0, 100));
    SavanaRoute.Waypoints.Add(FVector(5000, 5000, 100));
    SavanaRoute.Waypoints.Add(FVector(-5000, 5000, 100));
    SavanaRoute.Waypoints.Add(FVector(-5000, -5000, 100));
    SavanaRoute.Speed = 300.0f;
    SavanaRoute.MaxGroupSize = 50;
    MigrationRoutes.Add(SavanaRoute);
    
    FCrowd_MigrationRoute ForestRoute;
    ForestRoute.Waypoints.Add(FVector(-45000, 40000, 100));
    ForestRoute.Waypoints.Add(FVector(-43000, 42000, 100));
    ForestRoute.Waypoints.Add(FVector(-47000, 42000, 100));
    ForestRoute.Waypoints.Add(FVector(-47000, 38000, 100));
    ForestRoute.Speed = 200.0f;
    ForestRoute.MaxGroupSize = 30;
    MigrationRoutes.Add(ForestRoute);
}

void UCrowd_MigrationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastUpdateTime += DeltaTime;
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdateMigrationGroups(DeltaTime);
        LastUpdateTime = 0.0f;
    }
}

void UCrowd_MigrationManager::StartMigration(const FCrowd_MigrationRoute& Route, const TArray<AActor*>& Actors)
{
    if (Actors.Num() == 0) return;
    
    FCrowd_MigrationGroup NewGroup;
    NewGroup.Members = Actors;
    NewGroup.CurrentWaypointIndex = 0;
    
    if (Route.Waypoints.Num() > 0)
    {
        NewGroup.CurrentTarget = Route.Waypoints[0];
    }
    
    ActiveGroups.Add(NewGroup);
}

void UCrowd_MigrationManager::StopMigration(int32 GroupIndex)
{
    if (ActiveGroups.IsValidIndex(GroupIndex))
    {
        ActiveGroups.RemoveAt(GroupIndex);
    }
}

void UCrowd_MigrationManager::UpdateMigrationGroups(float DeltaTime)
{
    for (int32 i = ActiveGroups.Num() - 1; i >= 0; i--)
    {
        FCrowd_MigrationGroup& Group = ActiveGroups[i];
        
        // Remove null actors
        Group.Members.RemoveAll([](AActor* Actor) { return !IsValid(Actor); });
        
        if (Group.Members.Num() == 0)
        {
            ActiveGroups.RemoveAt(i);
            continue;
        }
        
        UpdateGroupMovement(Group, DeltaTime);
        ApplyFlockingBehavior(Group, DeltaTime);
    }
}

FVector UCrowd_MigrationManager::CalculateGroupCenter(const FCrowd_MigrationGroup& Group)
{
    if (Group.Members.Num() == 0) return FVector::ZeroVector;
    
    FVector Center = FVector::ZeroVector;
    for (AActor* Actor : Group.Members)
    {
        if (IsValid(Actor))
        {
            Center += Actor->GetActorLocation();
        }
    }
    
    return Center / Group.Members.Num();
}

void UCrowd_MigrationManager::ApplyFlockingBehavior(FCrowd_MigrationGroup& Group, float DeltaTime)
{
    for (AActor* Actor : Group.Members)
    {
        if (!IsValid(Actor)) continue;
        
        FVector Separation = CalculateSeparation(Actor, Group);
        FVector Alignment = CalculateAlignment(Actor, Group);
        FVector Cohesion = CalculateCohesion(Actor, Group);
        
        // Combine flocking forces
        FVector FlockingForce = Separation * 2.0f + Alignment * 1.0f + Cohesion * 1.5f;
        
        // Apply movement
        FVector CurrentLocation = Actor->GetActorLocation();
        FVector NewLocation = CurrentLocation + FlockingForce * DeltaTime;
        
        Actor->SetActorLocation(NewLocation);
        
        // Update rotation to face movement direction
        if (!FlockingForce.IsNearlyZero())
        {
            FRotator NewRotation = UKismetMathLibrary::FindLookAtRotation(CurrentLocation, NewLocation);
            Actor->SetActorRotation(NewRotation);
        }
    }
}

void UCrowd_MigrationManager::UpdateGroupMovement(FCrowd_MigrationGroup& Group, float DeltaTime)
{
    if (MigrationRoutes.Num() == 0) return;
    
    // Use first route for now (can be expanded for multiple routes)
    const FCrowd_MigrationRoute& Route = MigrationRoutes[0];
    
    if (Route.Waypoints.Num() == 0) return;
    
    FVector GroupCenter = CalculateGroupCenter(Group);
    FVector TargetWaypoint = Route.Waypoints[Group.CurrentWaypointIndex];
    
    float DistanceToTarget = FVector::Dist(GroupCenter, TargetWaypoint);
    
    // Check if group reached current waypoint
    if (DistanceToTarget < Route.GroupRadius)
    {
        Group.CurrentWaypointIndex = (Group.CurrentWaypointIndex + 1) % Route.Waypoints.Num();
        Group.CurrentTarget = Route.Waypoints[Group.CurrentWaypointIndex];
    }
    else
    {
        Group.CurrentTarget = TargetWaypoint;
    }
}

FVector UCrowd_MigrationManager::CalculateSeparation(AActor* Actor, const FCrowd_MigrationGroup& Group)
{
    if (!IsValid(Actor)) return FVector::ZeroVector;
    
    FVector SeparationForce = FVector::ZeroVector;
    FVector ActorLocation = Actor->GetActorLocation();
    int32 NeighborCount = 0;
    
    for (AActor* Other : Group.Members)
    {
        if (!IsValid(Other) || Other == Actor) continue;
        
        FVector OtherLocation = Other->GetActorLocation();
        float Distance = FVector::Dist(ActorLocation, OtherLocation);
        
        if (Distance < 500.0f && Distance > 0.0f) // Separation radius
        {
            FVector AwayVector = (ActorLocation - OtherLocation).GetSafeNormal();
            SeparationForce += AwayVector / Distance; // Closer = stronger force
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        SeparationForce /= NeighborCount;
        SeparationForce = SeparationForce.GetSafeNormal() * 100.0f; // Separation strength
    }
    
    return SeparationForce;
}

FVector UCrowd_MigrationManager::CalculateAlignment(AActor* Actor, const FCrowd_MigrationGroup& Group)
{
    if (!IsValid(Actor)) return FVector::ZeroVector;
    
    FVector AverageVelocity = FVector::ZeroVector;
    FVector ActorLocation = Actor->GetActorLocation();
    int32 NeighborCount = 0;
    
    for (AActor* Other : Group.Members)
    {
        if (!IsValid(Other) || Other == Actor) continue;
        
        FVector OtherLocation = Other->GetActorLocation();
        float Distance = FVector::Dist(ActorLocation, OtherLocation);
        
        if (Distance < 1000.0f) // Alignment radius
        {
            FVector OtherForward = Other->GetActorForwardVector();
            AverageVelocity += OtherForward;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        AverageVelocity /= NeighborCount;
        AverageVelocity = AverageVelocity.GetSafeNormal() * 50.0f; // Alignment strength
    }
    
    return AverageVelocity;
}

FVector UCrowd_MigrationManager::CalculateCohesion(AActor* Actor, const FCrowd_MigrationGroup& Group)
{
    if (!IsValid(Actor)) return FVector::ZeroVector;
    
    FVector CenterOfMass = FVector::ZeroVector;
    FVector ActorLocation = Actor->GetActorLocation();
    int32 NeighborCount = 0;
    
    for (AActor* Other : Group.Members)
    {
        if (!IsValid(Other) || Other == Actor) continue;
        
        FVector OtherLocation = Other->GetActorLocation();
        float Distance = FVector::Dist(ActorLocation, OtherLocation);
        
        if (Distance < 1500.0f) // Cohesion radius
        {
            CenterOfMass += OtherLocation;
            NeighborCount++;
        }
    }
    
    if (NeighborCount > 0)
    {
        CenterOfMass /= NeighborCount;
        FVector CohesionForce = (CenterOfMass - ActorLocation).GetSafeNormal() * 75.0f;
        return CohesionForce;
    }
    
    return FVector::ZeroVector;
}