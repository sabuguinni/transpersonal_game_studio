#include "Crowd_MigrationManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Components/PrimitiveComponent.h"

ACrowd_MigrationManager::ACrowd_MigrationManager()
{
    PrimaryActorTick.bCanEverTick = true;

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Initialize migration settings
    SeasonalMigrationInterval = 300.0f; // 5 minutes
    MaxSimultaneousMigrations = 3;
    GroupFormationRadius = 500.0f;
    bEnableSeasonalMigration = true;
    
    LastSeasonalCheck = 0.0f;
    ActiveMigrationCount = 0;

    // Setup default migration routes
    FCrowd_MigrationRoute DefaultRoute;
    DefaultRoute.WayPoints.Add(FVector(0, 0, 100));
    DefaultRoute.WayPoints.Add(FVector(2000, 1000, 150));
    DefaultRoute.WayPoints.Add(FVector(4000, 0, 200));
    DefaultRoute.RouteLength = 5000.0f;
    DefaultRoute.StartBiome = EBiomeType::Savanna;
    DefaultRoute.EndBiome = EBiomeType::Forest;
    DefaultRoute.MaxGroupSize = 30;
    MigrationRoutes.Add(DefaultRoute);
}

void ACrowd_MigrationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Migration Manager initialized with %d routes"), MigrationRoutes.Num());
    
    // Initialize seasonal migration timer
    LastSeasonalCheck = GetWorld()->GetTimeSeconds();
}

void ACrowd_MigrationManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bEnableSeasonalMigration)
    {
        UpdateSeasonalMigrations(DeltaTime);
    }

    // Update all active migration groups
    for (int32 i = ActiveMigrationGroups.Num() - 1; i >= 0; i--)
    {
        if (ActiveMigrationGroups[i].bIsActive)
        {
            UpdateGroupMovement(ActiveMigrationGroups[i], DeltaTime);
            CheckGroupProgress();
        }
        else
        {
            // Remove inactive groups
            ActiveMigrationGroups.RemoveAt(i);
            ActiveMigrationCount--;
        }
    }
}

void ACrowd_MigrationManager::StartMigration(const FCrowd_MigrationRoute& Route, const TArray<AActor*>& Participants)
{
    if (ActiveMigrationCount >= MaxSimultaneousMigrations)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start migration - maximum simultaneous migrations reached"));
        return;
    }

    if (Participants.Num() == 0 || Route.WayPoints.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start migration - invalid participants or route"));
        return;
    }

    FCrowd_MigrationGroup NewGroup;
    NewGroup.GroupMembers = Participants;
    NewGroup.CurrentDestination = Route.WayPoints[0];
    NewGroup.CurrentWayPointIndex = 0;
    NewGroup.MovementSpeed = 250.0f + FMath::RandRange(-50.0f, 50.0f); // Randomize speed slightly
    NewGroup.bIsActive = true;

    ActiveMigrationGroups.Add(NewGroup);
    ActiveMigrationCount++;

    UE_LOG(LogTemp, Log, TEXT("Started migration with %d participants"), Participants.Num());

    // Form the group at rally point
    if (Route.WayPoints.Num() > 0)
    {
        FormMigrationGroup(Participants, Route.WayPoints[0]);
    }
}

void ACrowd_MigrationManager::StopMigration(int32 GroupIndex)
{
    if (GroupIndex >= 0 && GroupIndex < ActiveMigrationGroups.Num())
    {
        ActiveMigrationGroups[GroupIndex].bIsActive = false;
        UE_LOG(LogTemp, Log, TEXT("Stopped migration group %d"), GroupIndex);
    }
}

void ACrowd_MigrationManager::AddMigrationRoute(const FCrowd_MigrationRoute& NewRoute)
{
    MigrationRoutes.Add(NewRoute);
    UE_LOG(LogTemp, Log, TEXT("Added new migration route with %d waypoints"), NewRoute.WayPoints.Num());
}

void ACrowd_MigrationManager::UpdateGroupMovement(FCrowd_MigrationGroup& Group, float DeltaTime)
{
    if (!Group.bIsActive || Group.GroupMembers.Num() == 0)
    {
        return;
    }

    FVector GroupCenter = CalculateGroupCenterOfMass(Group);
    FVector DirectionToDestination = (Group.CurrentDestination - GroupCenter).GetSafeNormal();
    float DistanceToDestination = FVector::Dist(GroupCenter, Group.CurrentDestination);

    // Move group members towards destination
    for (AActor* Member : Group.GroupMembers)
    {
        if (IsValid(Member))
        {
            FVector CurrentLocation = Member->GetActorLocation();
            FVector DesiredLocation = CurrentLocation + (DirectionToDestination * Group.MovementSpeed * DeltaTime);
            
            // Add some randomization to prevent perfect formation
            FVector Randomization = FVector(
                FMath::RandRange(-50.0f, 50.0f),
                FMath::RandRange(-50.0f, 50.0f),
                0.0f
            );
            DesiredLocation += Randomization;

            Member->SetActorLocation(DesiredLocation);
        }
    }

    // Check if group reached destination
    if (DistanceToDestination < 200.0f)
    {
        HandleGroupArrival(Group);
    }

    // Handle collisions and obstacles
    HandleGroupCollisions(Group);
}

bool ACrowd_MigrationManager::IsRouteBlocked(const FCrowd_MigrationRoute& Route)
{
    // Simple implementation - check for obstacles along route
    for (int32 i = 0; i < Route.WayPoints.Num() - 1; i++)
    {
        FVector Start = Route.WayPoints[i];
        FVector End = Route.WayPoints[i + 1];
        
        FHitResult HitResult;
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            Start,
            End,
            ECollisionChannel::ECC_WorldStatic
        );

        if (bHit)
        {
            return true; // Route is blocked
        }
    }
    return false;
}

void ACrowd_MigrationManager::TriggerSeasonalMigration()
{
    if (MigrationRoutes.Num() == 0)
    {
        return;
    }

    // Find actors suitable for migration (basic implementation)
    TArray<AActor*> PotentialMigrants = FindNearbyActors(GetActorLocation(), 2000.0f);
    
    if (PotentialMigrants.Num() > 5) // Minimum group size
    {
        // Select a random route
        int32 RouteIndex = FMath::RandRange(0, MigrationRoutes.Num() - 1);
        FCrowd_MigrationRoute& SelectedRoute = MigrationRoutes[RouteIndex];
        
        // Limit group size
        int32 GroupSize = FMath::Min(PotentialMigrants.Num(), SelectedRoute.MaxGroupSize);
        TArray<AActor*> MigrationGroup;
        
        for (int32 i = 0; i < GroupSize; i++)
        {
            MigrationGroup.Add(PotentialMigrants[i]);
        }

        StartMigration(SelectedRoute, MigrationGroup);
        UE_LOG(LogTemp, Log, TEXT("Triggered seasonal migration with %d participants"), GroupSize);
    }
}

TArray<AActor*> ACrowd_MigrationManager::FindNearbyActors(FVector Location, float Radius)
{
    TArray<AActor*> FoundActors;
    
    // Get all actors in the world
    UWorld* World = GetWorld();
    if (!World) return FoundActors;

    for (TActorIterator<AActor> ActorIterator(World); ActorIterator; ++ActorIterator)
    {
        AActor* Actor = *ActorIterator;
        if (IsValid(Actor) && Actor != this)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), Location);
            if (Distance <= Radius)
            {
                FoundActors.Add(Actor);
            }
        }
    }

    return FoundActors;
}

void ACrowd_MigrationManager::FormMigrationGroup(const TArray<AActor*>& Actors, FVector RallyPoint)
{
    // Move actors to formation around rally point
    float AngleStep = 360.0f / FMath::Max(1, Actors.Num());
    
    for (int32 i = 0; i < Actors.Num(); i++)
    {
        if (IsValid(Actors[i]))
        {
            float Angle = i * AngleStep;
            float RadiusOffset = FMath::RandRange(100.0f, GroupFormationRadius);
            
            FVector FormationPosition = RallyPoint + FVector(
                FMath::Cos(FMath::DegreesToRadians(Angle)) * RadiusOffset,
                FMath::Sin(FMath::DegreesToRadians(Angle)) * RadiusOffset,
                0.0f
            );

            Actors[i]->SetActorLocation(FormationPosition);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Formed migration group of %d actors at rally point"), Actors.Num());
}

void ACrowd_MigrationManager::HandleGroupArrival(FCrowd_MigrationGroup& Group)
{
    // Find the route this group is following
    for (const FCrowd_MigrationRoute& Route : MigrationRoutes)
    {
        if (Group.CurrentWayPointIndex < Route.WayPoints.Num() - 1)
        {
            // Move to next waypoint
            Group.CurrentWayPointIndex++;
            Group.CurrentDestination = Route.WayPoints[Group.CurrentWayPointIndex];
            UE_LOG(LogTemp, Log, TEXT("Group advanced to waypoint %d"), Group.CurrentWayPointIndex);
            return;
        }
    }

    // Group reached final destination
    Group.bIsActive = false;
    UE_LOG(LogTemp, Log, TEXT("Migration group completed journey"));
}

void ACrowd_MigrationManager::OptimizeMigrationPaths()
{
    // Optimize routes by removing unnecessary waypoints
    for (FCrowd_MigrationRoute& Route : MigrationRoutes)
    {
        if (Route.WayPoints.Num() > 2)
        {
            // Simple optimization - remove waypoints that are too close together
            for (int32 i = Route.WayPoints.Num() - 2; i > 0; i--)
            {
                float Distance = FVector::Dist(Route.WayPoints[i], Route.WayPoints[i + 1]);
                if (Distance < 300.0f) // Too close
                {
                    Route.WayPoints.RemoveAt(i);
                }
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("Optimized migration paths"));
}

void ACrowd_MigrationManager::UpdateSeasonalMigrations(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    if (CurrentTime - LastSeasonalCheck >= SeasonalMigrationInterval)
    {
        TriggerSeasonalMigration();
        LastSeasonalCheck = CurrentTime;
    }
}

void ACrowd_MigrationManager::CheckGroupProgress()
{
    // Monitor group health and progress
    for (FCrowd_MigrationGroup& Group : ActiveMigrationGroups)
    {
        if (Group.bIsActive)
        {
            // Remove invalid actors from group
            for (int32 i = Group.GroupMembers.Num() - 1; i >= 0; i--)
            {
                if (!IsValid(Group.GroupMembers[i]))
                {
                    Group.GroupMembers.RemoveAt(i);
                }
            }

            // Deactivate group if too few members remain
            if (Group.GroupMembers.Num() < 3)
            {
                Group.bIsActive = false;
                UE_LOG(LogTemp, Warning, TEXT("Migration group disbanded - insufficient members"));
            }
        }
    }
}

void ACrowd_MigrationManager::HandleGroupCollisions(FCrowd_MigrationGroup& Group)
{
    // Basic collision avoidance
    FVector GroupCenter = CalculateGroupCenterOfMass(Group);
    
    // Check for obstacles ahead
    FHitResult HitResult;
    FVector ForwardDirection = (Group.CurrentDestination - GroupCenter).GetSafeNormal();
    FVector TraceEnd = GroupCenter + (ForwardDirection * 500.0f);
    
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult,
        GroupCenter,
        TraceEnd,
        ECollisionChannel::ECC_WorldStatic
    );

    if (bHit)
    {
        // Adjust destination to avoid obstacle
        FVector AvoidanceDirection = FVector::CrossProduct(ForwardDirection, FVector::UpVector);
        Group.CurrentDestination += AvoidanceDirection * 300.0f;
        
        UE_LOG(LogTemp, Log, TEXT("Group avoiding obstacle, adjusting path"));
    }
}

FVector ACrowd_MigrationManager::CalculateGroupCenterOfMass(const FCrowd_MigrationGroup& Group)
{
    if (Group.GroupMembers.Num() == 0)
    {
        return FVector::ZeroVector;
    }

    FVector CenterOfMass = FVector::ZeroVector;
    int32 ValidMembers = 0;

    for (AActor* Member : Group.GroupMembers)
    {
        if (IsValid(Member))
        {
            CenterOfMass += Member->GetActorLocation();
            ValidMembers++;
        }
    }

    if (ValidMembers > 0)
    {
        CenterOfMass /= ValidMembers;
    }

    return CenterOfMass;
}