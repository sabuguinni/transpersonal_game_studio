#include "CrowdMigrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/AssetManager.h"

ACrowdMigrationManager::ACrowdMigrationManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Initialize default values
    SeasonalMigrationTimer = 0.0f;
    MigrationInterval = 300.0f; // 5 minutes between migrations
    MaxActiveHerds = 10;
    
    // Performance settings
    LODDistance1 = 5000.0f;  // High detail
    LODDistance2 = 15000.0f; // Medium detail
    MaxVisibleHerdMembers = 100;

    // Initialize species data
    InitializeSpeciesData();
}

void ACrowdMigrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeDefaultRoutes();
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdMigrationManager: Initialized with %d migration routes"), MigrationRoutes.Num());
    
    // Start first migration after 30 seconds
    GetWorldTimerManager().SetTimer(FTimerHandle(), this, &ACrowdMigrationManager::StartSeasonalMigration, 30.0f, false);
}

void ACrowdMigrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    SeasonalMigrationTimer += DeltaTime;

    // Update all active herds
    for (int32 i = ActiveHerds.Num() - 1; i >= 0; i--)
    {
        if (ActiveHerds[i].bIsActive && ActiveHerds[i].HerdMembers.Num() > 0)
        {
            // Find matching route for this herd
            for (const FCrowd_MigrationRoute& Route : MigrationRoutes)
            {
                if (Route.WayPoints.Num() > 0)
                {
                    UpdateHerdMovement(ActiveHerds[i], Route, DeltaTime);
                    break;
                }
            }
        }
        else
        {
            // Remove inactive herds
            ActiveHerds.RemoveAt(i);
        }
    }

    // Performance optimization - cull distant herds
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        CullDistantHerds(PlayerPawn->GetActorLocation());
    }

    // Trigger new migration if enough time has passed
    if (SeasonalMigrationTimer >= MigrationInterval)
    {
        StartSeasonalMigration();
        SeasonalMigrationTimer = 0.0f;
    }
}

void ACrowdMigrationManager::InitializeDefaultRoutes()
{
    // Savana to Forest migration route
    FCrowd_MigrationRoute SavanaToForest;
    SavanaToForest.StartBiome = EBiomeType::Savana;
    SavanaToForest.EndBiome = EBiomeType::Forest;
    SavanaToForest.RouteSpeed = 400.0f;
    SavanaToForest.MaxHerdSize = 75;
    SavanaToForest.WayPoints.Add(FVector(0, 0, 100));        // Savana start
    SavanaToForest.WayPoints.Add(FVector(-15000, 15000, 200)); // Mid point
    SavanaToForest.WayPoints.Add(FVector(-30000, 30000, 150)); // Near forest
    SavanaToForest.WayPoints.Add(FVector(-45000, 40000, 100)); // Forest destination

    // Forest to Desert migration route
    FCrowd_MigrationRoute ForestToDesert;
    ForestToDesert.StartBiome = EBiomeType::Forest;
    ForestToDesert.EndBiome = EBiomeType::Desert;
    ForestToDesert.RouteSpeed = 350.0f;
    ForestToDesert.MaxHerdSize = 60;
    ForestToDesert.WayPoints.Add(FVector(-45000, 40000, 100)); // Forest start
    ForestToDesert.WayPoints.Add(FVector(-20000, 20000, 150)); // Mid point
    ForestToDesert.WayPoints.Add(FVector(20000, -10000, 200)); // Near desert
    ForestToDesert.WayPoints.Add(FVector(55000, 0, 100));      // Desert destination

    // Desert to Mountain migration route
    FCrowd_MigrationRoute DesertToMountain;
    DesertToMountain.StartBiome = EBiomeType::Desert;
    DesertToMountain.EndBiome = EBiomeType::Mountain;
    DesertToMountain.RouteSpeed = 300.0f;
    DesertToMountain.MaxHerdSize = 40;
    DesertToMountain.WayPoints.Add(FVector(55000, 0, 100));    // Desert start
    DesertToMountain.WayPoints.Add(FVector(47500, 25000, 300)); // Mid point
    DesertToMountain.WayPoints.Add(FVector(40000, 50000, 500)); // Mountain destination

    MigrationRoutes.Add(SavanaToForest);
    MigrationRoutes.Add(ForestToDesert);
    MigrationRoutes.Add(DesertToMountain);
}

void ACrowdMigrationManager::InitializeSpeciesData()
{
    MigratorySpecies.Add("Triceratops");
    MigratorySpecies.Add("Parasaurolophus");
    MigratorySpecies.Add("Brachiosaurus");
    MigratorySpecies.Add("Ankylosaurus");

    SpeciesMovementSpeed.Add("Triceratops", 350.0f);
    SpeciesMovementSpeed.Add("Parasaurolophus", 450.0f);
    SpeciesMovementSpeed.Add("Brachiosaurus", 250.0f);
    SpeciesMovementSpeed.Add("Ankylosaurus", 200.0f);
}

void ACrowdMigrationManager::StartSeasonalMigration()
{
    if (ActiveHerds.Num() >= MaxActiveHerds)
    {
        UE_LOG(LogTemp, Warning, TEXT("CrowdMigrationManager: Maximum active herds reached (%d)"), MaxActiveHerds);
        return;
    }

    // Select random migration route
    if (MigrationRoutes.Num() > 0)
    {
        int32 RandomRouteIndex = FMath::RandRange(0, MigrationRoutes.Num() - 1);
        const FCrowd_MigrationRoute& SelectedRoute = MigrationRoutes[RandomRouteIndex];

        // Select random species
        if (MigratorySpecies.Num() > 0)
        {
            int32 RandomSpeciesIndex = FMath::RandRange(0, MigratorySpecies.Num() - 1);
            const FString& SelectedSpecies = MigratorySpecies[RandomSpeciesIndex];

            // Start migration
            FVector StartLocation = SelectedRoute.WayPoints[0];
            CreateMigrationHerd(SelectedSpecies, StartLocation, SelectedRoute);

            UE_LOG(LogTemp, Warning, TEXT("CrowdMigrationManager: Started %s migration from %s to %s"), 
                   *SelectedSpecies, 
                   *UEnum::GetValueAsString(SelectedRoute.StartBiome), 
                   *UEnum::GetValueAsString(SelectedRoute.EndBiome));
        }
    }
}

void ACrowdMigrationManager::CreateMigrationHerd(const FString& Species, const FVector& StartLocation, const FCrowd_MigrationRoute& Route)
{
    FCrowd_HerdData NewHerd;
    NewHerd.HerdCenter = StartLocation;
    NewHerd.HerdRadius = 1500.0f;
    NewHerd.TargetDestination = Route.WayPoints.Num() > 1 ? Route.WayPoints[1] : StartLocation;
    NewHerd.CurrentWayPointIndex = 0;
    NewHerd.bIsActive = true;

    // Spawn herd members in formation
    int32 HerdSize = FMath::RandRange(Route.MaxHerdSize / 2, Route.MaxHerdSize);
    
    for (int32 i = 0; i < HerdSize; i++)
    {
        // Calculate spawn position in circular formation around herd center
        float Angle = (2.0f * PI * i) / HerdSize;
        float Distance = FMath::RandRange(100.0f, NewHerd.HerdRadius);
        
        FVector SpawnOffset = FVector(
            FMath::Cos(Angle) * Distance,
            FMath::Sin(Angle) * Distance,
            FMath::RandRange(-50.0f, 50.0f)
        );
        
        FVector SpawnLocation = StartLocation + SpawnOffset;
        SpawnHerdMember(Species, SpawnLocation, NewHerd);
    }

    ActiveHerds.Add(NewHerd);
    
    UE_LOG(LogTemp, Warning, TEXT("CrowdMigrationManager: Created herd of %d %s at location %s"), 
           HerdSize, *Species, *StartLocation.ToString());
}

void ACrowdMigrationManager::SpawnHerdMember(const FString& Species, const FVector& Location, FCrowd_HerdData& HerdData)
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Map species to asset paths
    FString AssetPath;
    if (Species == "Triceratops")
    {
        AssetPath = "/Game/Dinosaur_Pack/Triceratops/Mesh/SKM_Triceratops";
    }
    else if (Species == "Parasaurolophus")
    {
        AssetPath = "/Game/Dinosaur_Pack/Parasaurolophus/Mesh/SKM_Parasaurolophus_Mesh";
    }
    else if (Species == "Brachiosaurus")
    {
        AssetPath = "/Game/Dinosaur_Pack/Brachiosaurus/Mesh/SKM_Brachiosaurus";
    }
    else if (Species == "Ankylosaurus")
    {
        AssetPath = "/Game/Dinosaur_Pack/Ankylosaurus/Mesh/SKM_Ankylo_Mesh";
    }

    if (!AssetPath.IsEmpty())
    {
        // Create a simple static mesh actor for now
        AActor* NewActor = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator);
        if (NewActor)
        {
            NewActor->SetActorLabel(FString::Printf(TEXT("%s_Migration_%d"), *Species, HerdData.HerdMembers.Num()));
            HerdData.HerdMembers.Add(NewActor);
        }
    }
}

void ACrowdMigrationManager::UpdateHerdMovement(FCrowd_HerdData& HerdData, const FCrowd_MigrationRoute& Route, float DeltaTime)
{
    if (Route.WayPoints.Num() <= HerdData.CurrentWayPointIndex + 1)
    {
        // Reached final destination
        HerdData.bIsActive = false;
        return;
    }

    FVector CurrentTarget = Route.WayPoints[HerdData.CurrentWayPointIndex + 1];
    FVector Direction = (CurrentTarget - HerdData.HerdCenter).GetSafeNormal();
    float MoveDistance = Route.RouteSpeed * DeltaTime;

    // Move herd center
    HerdData.HerdCenter += Direction * MoveDistance;

    // Move all herd members towards the new center with some randomization
    for (AActor* Member : HerdData.HerdMembers)
    {
        if (Member && IsValid(Member))
        {
            FVector MemberDirection = (HerdData.HerdCenter - Member->GetActorLocation()).GetSafeNormal();
            FVector RandomOffset = FVector(
                FMath::RandRange(-100.0f, 100.0f),
                FMath::RandRange(-100.0f, 100.0f),
                0.0f
            );
            
            FVector NewLocation = Member->GetActorLocation() + (MemberDirection * MoveDistance * 0.8f) + (RandomOffset * DeltaTime);
            Member->SetActorLocation(NewLocation);
        }
    }

    // Check if reached current waypoint
    if (IsHerdAtDestination(HerdData, CurrentTarget))
    {
        HerdData.CurrentWayPointIndex++;
        UE_LOG(LogTemp, Log, TEXT("CrowdMigrationManager: Herd reached waypoint %d"), HerdData.CurrentWayPointIndex);
    }
}

bool ACrowdMigrationManager::IsHerdAtDestination(const FCrowd_HerdData& HerdData, const FVector& Destination)
{
    float Distance = FVector::Dist(HerdData.HerdCenter, Destination);
    return Distance < 1000.0f; // Within 10 meters
}

void ACrowdMigrationManager::UpdateHerdLOD(FCrowd_HerdData& HerdData, const FVector& PlayerLocation)
{
    float DistanceToPlayer = FVector::Dist(HerdData.HerdCenter, PlayerLocation);
    
    int32 VisibleMembers = MaxVisibleHerdMembers;
    
    if (DistanceToPlayer > LODDistance2)
    {
        VisibleMembers = FMath::Max(1, MaxVisibleHerdMembers / 4); // Very low detail
    }
    else if (DistanceToPlayer > LODDistance1)
    {
        VisibleMembers = MaxVisibleHerdMembers / 2; // Medium detail
    }

    // Hide/show herd members based on LOD
    for (int32 i = 0; i < HerdData.HerdMembers.Num(); i++)
    {
        if (HerdData.HerdMembers[i] && IsValid(HerdData.HerdMembers[i]))
        {
            bool bShouldBeVisible = i < VisibleMembers;
            HerdData.HerdMembers[i]->SetActorHiddenInGame(!bShouldBeVisible);
        }
    }
}

void ACrowdMigrationManager::CullDistantHerds(const FVector& PlayerLocation)
{
    const float MaxDistance = 50000.0f; // 500 meters

    for (int32 i = ActiveHerds.Num() - 1; i >= 0; i--)
    {
        float Distance = FVector::Dist(ActiveHerds[i].HerdCenter, PlayerLocation);
        
        if (Distance > MaxDistance)
        {
            // Destroy distant herd members to save performance
            for (AActor* Member : ActiveHerds[i].HerdMembers)
            {
                if (Member && IsValid(Member))
                {
                    Member->Destroy();
                }
            }
            ActiveHerds.RemoveAt(i);
        }
        else
        {
            // Update LOD for remaining herds
            UpdateHerdLOD(ActiveHerds[i], PlayerLocation);
        }
    }
}

void ACrowdMigrationManager::AddMigrationRoute(const FCrowd_MigrationRoute& NewRoute)
{
    MigrationRoutes.Add(NewRoute);
}

FCrowd_MigrationRoute ACrowdMigrationManager::GetRouteForBiomes(EBiomeType StartBiome, EBiomeType EndBiome)
{
    for (const FCrowd_MigrationRoute& Route : MigrationRoutes)
    {
        if (Route.StartBiome == StartBiome && Route.EndBiome == EndBiome)
        {
            return Route;
        }
    }
    
    return FCrowd_MigrationRoute(); // Return default route if not found
}

FVector ACrowdMigrationManager::CalculateHerdCenter(const FCrowd_HerdData& HerdData)
{
    if (HerdData.HerdMembers.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector Sum = FVector::ZeroVector;
    int32 ValidMembers = 0;

    for (AActor* Member : HerdData.HerdMembers)
    {
        if (Member && IsValid(Member))
        {
            Sum += Member->GetActorLocation();
            ValidMembers++;
        }
    }

    return ValidMembers > 0 ? Sum / ValidMembers : FVector::ZeroVector;
}