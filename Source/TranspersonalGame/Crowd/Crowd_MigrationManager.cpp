#include "Crowd_MigrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"

ACrowd_MigrationManager::ACrowd_MigrationManager()
{
    PrimaryActorTick.bCanEverTick = true;
    
    // Initialize default settings
    RouteUpdateInterval = 2.0f;
    GroupFormationRadius = 1000.0f;
    MaxSimultaneousGroups = 8;
    bEnableSeasonalMigration = true;
    SeasonalMigrationChance = 0.3f;
    
    LastRouteUpdateTime = 0.0f;
    LastSeasonalCheckTime = 0.0f;
    
    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
}

void ACrowd_MigrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeMigrationRoutes();
    
    if (bEnableSeasonalMigration)
    {
        // Start seasonal migration timer
        GetWorld()->GetTimerManager().SetTimer(
            FTimerHandle(),
            this,
            &ACrowd_MigrationManager::HandleSeasonalMigration,
            30.0f,
            true
        );
    }
}

void ACrowd_MigrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastRouteUpdateTime += DeltaTime;
    
    if (LastRouteUpdateTime >= RouteUpdateInterval)
    {
        UpdateMigrationGroups(DeltaTime);
        CleanupCompletedMigrations();
        LastRouteUpdateTime = 0.0f;
    }
}

void ACrowd_MigrationManager::InitializeMigrationRoutes()
{
    MigrationRoutes.Empty();
    
    // Create herbivore migration routes (grazing patterns)
    FCrowd_MigrationRoute HerbivoreRoute1;
    HerbivoreRoute1.StartLocation = FVector(-5000, -3000, 100);
    HerbivoreRoute1.EndLocation = FVector(8000, 4000, 200);
    HerbivoreRoute1.RouteDistance = FVector::Dist(HerbivoreRoute1.StartLocation, HerbivoreRoute1.EndLocation);
    HerbivoreRoute1.SpeciesType = TEXT("Herbivore");
    HerbivoreRoute1.MaxGroupSize = 25;
    HerbivoreRoute1.MigrationSpeed = 250.0f;
    MigrationRoutes.Add(HerbivoreRoute1);
    
    FCrowd_MigrationRoute HerbivoreRoute2;
    HerbivoreRoute2.StartLocation = FVector(6000, -2000, 150);
    HerbivoreRoute2.EndLocation = FVector(-4000, 5000, 180);
    HerbivoreRoute2.RouteDistance = FVector::Dist(HerbivoreRoute2.StartLocation, HerbivoreRoute2.EndLocation);
    HerbivoreRoute2.SpeciesType = TEXT("Herbivore");
    HerbivoreRoute2.MaxGroupSize = 30;
    HerbivoreRoute2.MigrationSpeed = 200.0f;
    MigrationRoutes.Add(HerbivoreRoute2);
    
    // Create carnivore hunting migration routes
    FCrowd_MigrationRoute CarnivoreRoute;
    CarnivoreRoute.StartLocation = FVector(-2000, 1000, 120);
    CarnivoreRoute.EndLocation = FVector(3000, -4000, 160);
    CarnivoreRoute.RouteDistance = FVector::Dist(CarnivoreRoute.StartLocation, CarnivoreRoute.EndLocation);
    CarnivoreRoute.SpeciesType = TEXT("Carnivore");
    CarnivoreRoute.MaxGroupSize = 8;
    CarnivoreRoute.MigrationSpeed = 400.0f;
    MigrationRoutes.Add(CarnivoreRoute);
    
    UE_LOG(LogTemp, Warning, TEXT("Migration Manager: Initialized %d migration routes"), MigrationRoutes.Num());
}

void ACrowd_MigrationManager::StartMigration(const FString& SpeciesType, int32 GroupSize)
{
    if (ActiveMigrationGroups.Num() >= MaxSimultaneousGroups)
    {
        UE_LOG(LogTemp, Warning, TEXT("Migration Manager: Max groups reached, cannot start new migration"));
        return;
    }
    
    // Find suitable route for species
    FCrowd_MigrationRoute* SelectedRoute = nullptr;
    for (FCrowd_MigrationRoute& Route : MigrationRoutes)
    {
        if (Route.SpeciesType == SpeciesType && Route.bIsActive)
        {
            SelectedRoute = &Route;
            break;
        }
    }
    
    if (SelectedRoute)
    {
        int32 ActualGroupSize = FMath::Min(GroupSize, SelectedRoute->MaxGroupSize);
        CreateMigrationGroup(*SelectedRoute, ActualGroupSize);
        UE_LOG(LogTemp, Log, TEXT("Migration Manager: Started %s migration with %d members"), *SpeciesType, ActualGroupSize);
    }
}

void ACrowd_MigrationManager::UpdateMigrationGroups(float DeltaTime)
{
    for (FCrowd_MigrationGroup& Group : ActiveMigrationGroups)
    {
        if (Group.GroupMembers.Num() > 0 && !Group.bReachedDestination)
        {
            MoveMigrationGroup(Group, DeltaTime);
            
            if (IsGroupAtDestination(Group))
            {
                AdvanceGroupToNextWaypoint(Group);
            }
        }
    }
}

void ACrowd_MigrationManager::CreateMigrationGroup(const FCrowd_MigrationRoute& Route, int32 GroupSize)
{
    FCrowd_MigrationGroup NewGroup;
    NewGroup.CurrentDestination = Route.EndLocation;
    NewGroup.RouteIndex = 0;
    NewGroup.GroupCohesion = FMath::RandRange(0.6f, 0.9f);
    
    // Spawn group members around start location
    for (int32 i = 0; i < GroupSize; i++)
    {
        FVector SpawnLocation = Route.StartLocation + FVector(
            FMath::RandRange(-GroupFormationRadius, GroupFormationRadius),
            FMath::RandRange(-GroupFormationRadius, GroupFormationRadius),
            0
        );
        
        FRotator SpawnRotation = FRotator::ZeroRotator;
        
        // Create basic crowd member (placeholder for now)
        AActor* CrowdMember = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, SpawnRotation);
        if (CrowdMember)
        {
            CrowdMember->SetActorLabel(FString::Printf(TEXT("MigrationMember_%s_%d"), *Route.SpeciesType, i));
            NewGroup.GroupMembers.Add(CrowdMember);
        }
    }
    
    ActiveMigrationGroups.Add(NewGroup);
    UE_LOG(LogTemp, Log, TEXT("Migration Manager: Created group with %d members"), NewGroup.GroupMembers.Num());
}

void ACrowd_MigrationManager::MoveMigrationGroup(FCrowd_MigrationGroup& Group, float DeltaTime)
{
    if (Group.GroupMembers.Num() == 0) return;
    
    // Calculate group center
    FVector GroupCenter = FVector::ZeroVector;
    int32 ValidMembers = 0;
    
    for (AActor* Member : Group.GroupMembers)
    {
        if (IsValid(Member))
        {
            GroupCenter += Member->GetActorLocation();
            ValidMembers++;
        }
    }
    
    if (ValidMembers == 0) return;
    
    GroupCenter /= ValidMembers;
    
    // Move each member toward destination with cohesion
    FVector DirectionToDestination = (Group.CurrentDestination - GroupCenter).GetSafeNormal();
    
    for (AActor* Member : Group.GroupMembers)
    {
        if (IsValid(Member))
        {
            FVector MemberLocation = Member->GetActorLocation();
            FVector ToCenter = (GroupCenter - MemberLocation).GetSafeNormal();
            
            // Combine destination movement with group cohesion
            FVector MovementDirection = (DirectionToDestination * (1.0f - Group.GroupCohesion)) + 
                                      (ToCenter * Group.GroupCohesion);
            
            FVector NewLocation = MemberLocation + (MovementDirection * 300.0f * DeltaTime);
            Member->SetActorLocation(NewLocation);
        }
    }
}

bool ACrowd_MigrationManager::IsGroupAtDestination(const FCrowd_MigrationGroup& Group, float Tolerance)
{
    if (Group.GroupMembers.Num() == 0) return true;
    
    // Check if majority of group is within tolerance of destination
    int32 MembersAtDestination = 0;
    
    for (AActor* Member : Group.GroupMembers)
    {
        if (IsValid(Member))
        {
            float DistanceToDestination = FVector::Dist(Member->GetActorLocation(), Group.CurrentDestination);
            if (DistanceToDestination <= Tolerance)
            {
                MembersAtDestination++;
            }
        }
    }
    
    return (MembersAtDestination >= Group.GroupMembers.Num() * 0.7f); // 70% threshold
}

void ACrowd_MigrationManager::AdvanceGroupToNextWaypoint(FCrowd_MigrationGroup& Group)
{
    Group.RouteIndex++;
    
    // For now, mark as completed after reaching first destination
    // In full implementation, would have multiple waypoints
    Group.bReachedDestination = true;
    
    UE_LOG(LogTemp, Log, TEXT("Migration Manager: Group reached destination, migration complete"));
}

void ACrowd_MigrationManager::HandleSeasonalMigration()
{
    LastSeasonalCheckTime += 30.0f;
    
    if (FMath::RandRange(0.0f, 1.0f) < SeasonalMigrationChance)
    {
        // Trigger random seasonal migration
        TArray<FString> SpeciesTypes = {TEXT("Herbivore"), TEXT("Carnivore")};
        FString RandomSpecies = SpeciesTypes[FMath::RandRange(0, SpeciesTypes.Num() - 1)];
        int32 RandomGroupSize = FMath::RandRange(5, 20);
        
        StartMigration(RandomSpecies, RandomGroupSize);
        UE_LOG(LogTemp, Log, TEXT("Migration Manager: Seasonal migration triggered for %s"), *RandomSpecies);
    }
}

TArray<FVector> ACrowd_MigrationManager::GenerateWaypointsForRoute(const FCrowd_MigrationRoute& Route)
{
    TArray<FVector> Waypoints;
    
    // Generate intermediate waypoints between start and end
    int32 NumWaypoints = FMath::RandRange(3, 6);
    
    for (int32 i = 0; i <= NumWaypoints; i++)
    {
        float Alpha = static_cast<float>(i) / NumWaypoints;
        FVector Waypoint = FMath::Lerp(Route.StartLocation, Route.EndLocation, Alpha);
        
        // Add some randomness to avoid straight lines
        Waypoint += FVector(
            FMath::RandRange(-1000, 1000),
            FMath::RandRange(-1000, 1000),
            0
        );
        
        Waypoints.Add(Waypoint);
    }
    
    return Waypoints;
}

void ACrowd_MigrationManager::CleanupCompletedMigrations()
{
    for (int32 i = ActiveMigrationGroups.Num() - 1; i >= 0; i--)
    {
        FCrowd_MigrationGroup& Group = ActiveMigrationGroups[i];
        
        if (Group.bReachedDestination)
        {
            // Clean up group members
            for (AActor* Member : Group.GroupMembers)
            {
                if (IsValid(Member))
                {
                    Member->Destroy();
                }
            }
            
            ActiveMigrationGroups.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("Migration Manager: Cleaned up completed migration group"));
        }
    }
}

void ACrowd_MigrationManager::OnMigrationGroupReachedDestination(FCrowd_MigrationGroup& Group)
{
    Group.bReachedDestination = true;
    UE_LOG(LogTemp, Log, TEXT("Migration Manager: Migration group reached final destination"));
}