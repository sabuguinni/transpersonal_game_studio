#include "NPC_DinosaurMigrationSystem.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "AI/Navigation/NavigationSystem.h"
#include "NavigationSystem.h"

UNPC_DinosaurMigrationSystem::UNPC_DinosaurMigrationSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // Migration parameters
    SeasonalMigrationDistance = 50000.0f;
    MigrationSpeed = 300.0f;
    MigrationGroupSize = 15;
    SeasonDuration = 300.0f;
    
    // Current state
    CurrentSeason = ENPC_Season::Spring;
    SeasonTimer = 0.0f;
    bIsMigrating = false;
    MigrationProgress = 0.0f;
    
    // Initialize migration routes
    InitializeMigrationRoutes();
}

void UNPC_DinosaurMigrationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Get world reference
    World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("DinosaurMigrationSystem: No valid world reference"));
        return;
    }
    
    // Initialize navigation system
    NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    
    // Set initial migration target
    UpdateSeasonalTarget();
    
    UE_LOG(LogTemp, Log, TEXT("DinosaurMigrationSystem initialized - Season: %d"), (int32)CurrentSeason);
}

void UNPC_DinosaurMigrationSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!World) return;
    
    // Update season timer
    SeasonTimer += DeltaTime;
    
    // Check for season change
    if (SeasonTimer >= SeasonDuration)
    {
        AdvanceSeason();
    }
    
    // Update migration if active
    if (bIsMigrating)
    {
        UpdateMigration(DeltaTime);
    }
    
    // Check environmental triggers
    CheckEnvironmentalTriggers();
}

void UNPC_DinosaurMigrationSystem::InitializeMigrationRoutes()
{
    // Clear existing routes
    MigrationRoutes.Empty();
    
    // Spring routes - move to fertile valleys
    FNPC_MigrationRoute SpringRoute;
    SpringRoute.Season = ENPC_Season::Spring;
    SpringRoute.TargetLocation = FVector(25000, 15000, 500);
    SpringRoute.RouteWaypoints.Add(FVector(5000, 5000, 200));
    SpringRoute.RouteWaypoints.Add(FVector(15000, 10000, 350));
    SpringRoute.RouteWaypoints.Add(FVector(25000, 15000, 500));
    SpringRoute.PreferredBiome = ENPC_BiomeType::TemperateForest;
    SpringRoute.MigrationReason = TEXT("Seeking fertile feeding grounds");
    MigrationRoutes.Add(SpringRoute);
    
    // Summer routes - move to water sources
    FNPC_MigrationRoute SummerRoute;
    SummerRoute.Season = ENPC_Season::Summer;
    SummerRoute.TargetLocation = FVector(-20000, 30000, 100);
    SummerRoute.RouteWaypoints.Add(FVector(0, 20000, 200));
    SummerRoute.RouteWaypoints.Add(FVector(-10000, 25000, 150));
    SummerRoute.RouteWaypoints.Add(FVector(-20000, 30000, 100));
    SummerRoute.PreferredBiome = ENPC_BiomeType::RiverDelta;
    SummerRoute.MigrationReason = TEXT("Following water sources during dry season");
    MigrationRoutes.Add(SummerRoute);
    
    // Autumn routes - move to sheltered areas
    FNPC_MigrationRoute AutumnRoute;
    AutumnRoute.Season = ENPC_Season::Autumn;
    AutumnRoute.TargetLocation = FVector(10000, -25000, 800);
    AutumnRoute.RouteWaypoints.Add(FVector(15000, -10000, 400));
    AutumnRoute.RouteWaypoints.Add(FVector(12000, -18000, 600));
    AutumnRoute.RouteWaypoints.Add(FVector(10000, -25000, 800));
    AutumnRoute.PreferredBiome = ENPC_BiomeType::Mountains;
    AutumnRoute.MigrationReason = TEXT("Seeking shelter from seasonal storms");
    MigrationRoutes.Add(AutumnRoute);
    
    // Winter routes - move to warmer lowlands
    FNPC_MigrationRoute WinterRoute;
    WinterRoute.Season = ENPC_Season::Winter;
    WinterRoute.TargetLocation = FVector(-15000, -10000, 50);
    WinterRoute.RouteWaypoints.Add(FVector(-5000, -5000, 150));
    WinterRoute.RouteWaypoints.Add(FVector(-10000, -8000, 100));
    WinterRoute.RouteWaypoints.Add(FVector(-15000, -10000, 50));
    WinterRoute.PreferredBiome = ENPC_BiomeType::Coastal;
    WinterRoute.MigrationReason = TEXT("Escaping harsh mountain weather");
    MigrationRoutes.Add(WinterRoute);
}

void UNPC_DinosaurMigrationSystem::AdvanceSeason()
{
    // Reset season timer
    SeasonTimer = 0.0f;
    
    // Advance to next season
    int32 NextSeasonInt = ((int32)CurrentSeason + 1) % 4;
    CurrentSeason = (ENPC_Season)NextSeasonInt;
    
    // Update seasonal target
    UpdateSeasonalTarget();
    
    // Trigger migration
    StartMigration();
    
    UE_LOG(LogTemp, Log, TEXT("Season advanced to: %d"), (int32)CurrentSeason);
}

void UNPC_DinosaurMigrationSystem::UpdateSeasonalTarget()
{
    // Find route for current season
    for (const FNPC_MigrationRoute& Route : MigrationRoutes)
    {
        if (Route.Season == CurrentSeason)
        {
            CurrentMigrationRoute = Route;
            CurrentTargetLocation = Route.TargetLocation;
            CurrentWaypointIndex = 0;
            break;
        }
    }
}

void UNPC_DinosaurMigrationSystem::StartMigration()
{
    if (bIsMigrating) return;
    
    bIsMigrating = true;
    MigrationProgress = 0.0f;
    CurrentWaypointIndex = 0;
    
    // Get owner location
    AActor* Owner = GetOwner();
    if (Owner)
    {
        MigrationStartLocation = Owner->GetActorLocation();
        
        // Calculate total migration distance
        float TotalDistance = 0.0f;
        FVector CurrentPos = MigrationStartLocation;
        
        for (const FVector& Waypoint : CurrentMigrationRoute.RouteWaypoints)
        {
            TotalDistance += FVector::Dist(CurrentPos, Waypoint);
            CurrentPos = Waypoint;
        }
        
        TotalMigrationDistance = TotalDistance;
        
        UE_LOG(LogTemp, Log, TEXT("Migration started - Distance: %.2f, Reason: %s"), 
               TotalMigrationDistance, *CurrentMigrationRoute.MigrationReason);
    }
}

void UNPC_DinosaurMigrationSystem::UpdateMigration(float DeltaTime)
{
    AActor* Owner = GetOwner();
    if (!Owner || CurrentWaypointIndex >= CurrentMigrationRoute.RouteWaypoints.Num())
    {
        CompleteMigration();
        return;
    }
    
    FVector CurrentLocation = Owner->GetActorLocation();
    FVector TargetWaypoint = CurrentMigrationRoute.RouteWaypoints[CurrentWaypointIndex];
    
    // Calculate movement
    FVector Direction = (TargetWaypoint - CurrentLocation).GetSafeNormal();
    float MoveDistance = MigrationSpeed * DeltaTime;
    
    // Check if we've reached the current waypoint
    float DistanceToWaypoint = FVector::Dist(CurrentLocation, TargetWaypoint);
    if (DistanceToWaypoint <= MoveDistance * 1.5f)
    {
        // Move to next waypoint
        CurrentWaypointIndex++;
        if (CurrentWaypointIndex >= CurrentMigrationRoute.RouteWaypoints.Num())
        {
            CompleteMigration();
            return;
        }
    }
    
    // Move towards waypoint
    FVector NewLocation = CurrentLocation + (Direction * MoveDistance);
    
    // Use navigation system if available
    if (NavSystem)
    {
        FNavLocation NavResult;
        if (NavSystem->ProjectPointToNavigation(NewLocation, NavResult, FVector(1000, 1000, 500)))
        {
            NewLocation = NavResult.Location;
        }
    }
    
    Owner->SetActorLocation(NewLocation);
    
    // Update migration progress
    float DistanceTraveled = FVector::Dist(MigrationStartLocation, CurrentLocation);
    MigrationProgress = FMath::Clamp(DistanceTraveled / TotalMigrationDistance, 0.0f, 1.0f);
}

void UNPC_DinosaurMigrationSystem::CompleteMigration()
{
    bIsMigrating = false;
    MigrationProgress = 1.0f;
    CurrentWaypointIndex = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Migration completed - Reached seasonal destination"));
    
    // Broadcast migration completion
    OnMigrationCompleted.Broadcast(CurrentSeason, CurrentTargetLocation);
}

void UNPC_DinosaurMigrationSystem::CheckEnvironmentalTriggers()
{
    // Check for environmental conditions that might trigger emergency migration
    AActor* Owner = GetOwner();
    if (!Owner) return;
    
    FVector OwnerLocation = Owner->GetActorLocation();
    
    // Check for dangerous weather (simplified)
    if (World)
    {
        // Example: Check for volcanic activity, extreme weather, etc.
        // This would integrate with weather/disaster systems
        
        // For now, just check altitude for winter survival
        if (CurrentSeason == ENPC_Season::Winter && OwnerLocation.Z > 1000.0f)
        {
            // Force migration to lower altitude
            if (!bIsMigrating)
            {
                UE_LOG(LogTemp, Warning, TEXT("Emergency migration triggered - High altitude in winter"));
                StartMigration();
            }
        }
    }
}

bool UNPC_DinosaurMigrationSystem::ShouldMigrate() const
{
    // Check various conditions that would trigger migration
    
    // Seasonal migration
    if (SeasonTimer >= SeasonDuration * 0.8f && !bIsMigrating)
    {
        return true;
    }
    
    // Environmental pressure
    AActor* Owner = GetOwner();
    if (Owner)
    {
        FVector OwnerLocation = Owner->GetActorLocation();
        
        // Check if in unsuitable biome for current season
        ENPC_BiomeType CurrentBiome = DetermineBiomeType(OwnerLocation);
        if (CurrentBiome != CurrentMigrationRoute.PreferredBiome)
        {
            return true;
        }
    }
    
    return false;
}

ENPC_BiomeType UNPC_DinosaurMigrationSystem::DetermineBiomeType(const FVector& Location) const
{
    // Simple biome determination based on location and altitude
    float Altitude = Location.Z;
    float DistanceFromOrigin = FVector2D(Location.X, Location.Y).Size();
    
    if (Altitude > 800.0f)
    {
        return ENPC_BiomeType::Mountains;
    }
    else if (Altitude < 100.0f && DistanceFromOrigin > 40000.0f)
    {
        return ENPC_BiomeType::Coastal;
    }
    else if (FMath::Abs(Location.X) < 10000.0f && FMath::Abs(Location.Y) < 10000.0f)
    {
        return ENPC_BiomeType::RiverDelta;
    }
    else
    {
        return ENPC_BiomeType::TemperateForest;
    }
}

FVector UNPC_DinosaurMigrationSystem::GetCurrentMigrationTarget() const
{
    if (bIsMigrating && CurrentWaypointIndex < CurrentMigrationRoute.RouteWaypoints.Num())
    {
        return CurrentMigrationRoute.RouteWaypoints[CurrentWaypointIndex];
    }
    
    return CurrentTargetLocation;
}

float UNPC_DinosaurMigrationSystem::GetMigrationProgress() const
{
    return MigrationProgress;
}

ENPC_Season UNPC_DinosaurMigrationSystem::GetCurrentSeason() const
{
    return CurrentSeason;
}

bool UNPC_DinosaurMigrationSystem::IsMigrating() const
{
    return bIsMigrating;
}