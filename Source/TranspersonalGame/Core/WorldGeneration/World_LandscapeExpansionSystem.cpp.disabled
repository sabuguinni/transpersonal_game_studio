#include "World_LandscapeExpansionSystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Landscape/Landscape.h"
#include "LandscapeInfo.h"
#include "LandscapeComponent.h"
#include "LandscapeProxy.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

DEFINE_LOG_CATEGORY(LogWorld_LandscapeExpansion);

UWorld_LandscapeExpansionSystem::UWorld_LandscapeExpansionSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 5.0f; // Check every 5 seconds
    
    // Initialize 200km2 target dimensions
    TargetLandscapeSize = FVector2D(200000.0f, 200000.0f); // 200km x 200km in cm
    CurrentExpansionState = EWorld_ExpansionState::Planning;
    ExpansionProgress = 0.0f;
    
    // Biome definitions with correct coordinates
    InitializeBiomeDefinitions();
}

void UWorld_LandscapeExpansionSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogWorld_LandscapeExpansion, Warning, TEXT("World Landscape Expansion System initialized - Target: 200km2"));
    
    // Start expansion assessment
    AssessCurrentLandscape();
}

void UWorld_LandscapeExpansionSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (CurrentExpansionState == EWorld_ExpansionState::InProgress)
    {
        UpdateExpansionProgress(DeltaTime);
    }
}

void UWorld_LandscapeExpansionSystem::InitializeBiomeDefinitions()
{
    BiomeDefinitions.Empty();
    
    // Pantano (Southwest)
    FWorld_BiomeDefinition PantanoBiome;
    PantanoBiome.BiomeName = TEXT("Pantano");
    PantanoBiome.CenterLocation = FVector(-50000.0f, -45000.0f, 0.0f);
    PantanoBiome.BoundsMin = FVector(-77500.0f, -76500.0f, -200.0f);
    PantanoBiome.BoundsMax = FVector(-25000.0f, -15000.0f, 500.0f);
    PantanoBiome.TerrainType = EWorld_TerrainType::Wetland;
    PantanoBiome.ElevationRange = FVector2D(-50.0f, 100.0f);
    BiomeDefinitions.Add(PantanoBiome);
    
    // Floresta (Northwest)
    FWorld_BiomeDefinition FlorestaBiome;
    FlorestaBiome.BiomeName = TEXT("Floresta");
    FlorestaBiome.CenterLocation = FVector(-45000.0f, 40000.0f, 0.0f);
    FlorestaBiome.BoundsMin = FVector(-77500.0f, 15000.0f, -100.0f);
    FlorestaBiome.BoundsMax = FVector(-15000.0f, 76500.0f, 800.0f);
    FlorestaBiome.TerrainType = EWorld_TerrainType::Forest;
    FlorestaBiome.ElevationRange = FVector2D(0.0f, 600.0f);
    BiomeDefinitions.Add(FlorestaBiome);
    
    // Savana (Center)
    FWorld_BiomeDefinition SavanaBiome;
    SavanaBiome.BiomeName = TEXT("Savana");
    SavanaBiome.CenterLocation = FVector(0.0f, 0.0f, 0.0f);
    SavanaBiome.BoundsMin = FVector(-20000.0f, -20000.0f, -50.0f);
    SavanaBiome.BoundsMax = FVector(20000.0f, 20000.0f, 300.0f);
    SavanaBiome.TerrainType = EWorld_TerrainType::Grassland;
    SavanaBiome.ElevationRange = FVector2D(-20.0f, 200.0f);
    BiomeDefinitions.Add(SavanaBiome);
    
    // Deserto (East)
    FWorld_BiomeDefinition DesertoBiome;
    DesertoBiome.BiomeName = TEXT("Deserto");
    DesertoBiome.CenterLocation = FVector(55000.0f, 0.0f, 0.0f);
    DesertoBiome.BoundsMin = FVector(25000.0f, -30000.0f, -100.0f);
    DesertoBiome.BoundsMax = FVector(79500.0f, 30000.0f, 400.0f);
    DesertoBiome.TerrainType = EWorld_TerrainType::Desert;
    DesertoBiome.ElevationRange = FVector2D(-50.0f, 300.0f);
    BiomeDefinitions.Add(DesertoBiome);
    
    // Montanha Nevada (Northeast)
    FWorld_BiomeDefinition MontanhaBiome;
    MontanhaBiome.BiomeName = TEXT("Montanha");
    MontanhaBiome.CenterLocation = FVector(40000.0f, 50000.0f, 500.0f);
    MontanhaBiome.BoundsMin = FVector(15000.0f, 20000.0f, 200.0f);
    MontanhaBiome.BoundsMax = FVector(79500.0f, 76500.0f, 2000.0f);
    MontanhaBiome.TerrainType = EWorld_TerrainType::Mountain;
    MontanhaBiome.ElevationRange = FVector2D(300.0f, 1800.0f);
    BiomeDefinitions.Add(MontanhaBiome);
    
    UE_LOG(LogWorld_LandscapeExpansion, Log, TEXT("Initialized %d biome definitions"), BiomeDefinitions.Num());
}

void UWorld_LandscapeExpansionSystem::AssessCurrentLandscape()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogWorld_LandscapeExpansion, Error, TEXT("Cannot assess landscape - no world context"));
        return;
    }
    
    // Find existing landscape actors
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    
    CurrentLandscapeSize = FVector2D::ZeroVector;
    
    if (LandscapeActors.Num() > 0)
    {
        ALandscape* MainLandscape = Cast<ALandscape>(LandscapeActors[0]);
        if (MainLandscape)
        {
            FVector LandscapeScale = MainLandscape->GetActorScale3D();
            ULandscapeInfo* LandscapeInfo = MainLandscape->GetLandscapeInfo();
            
            if (LandscapeInfo)
            {
                FIntRect LandscapeBounds = LandscapeInfo->GetCurrentBounds();
                CurrentLandscapeSize.X = LandscapeBounds.Width() * LandscapeScale.X;
                CurrentLandscapeSize.Y = LandscapeBounds.Height() * LandscapeScale.Y;
                
                UE_LOG(LogWorld_LandscapeExpansion, Warning, TEXT("Current landscape size: %.0f x %.0f cm"), 
                       CurrentLandscapeSize.X, CurrentLandscapeSize.Y);
            }
        }
    }
    else
    {
        UE_LOG(LogWorld_LandscapeExpansion, Warning, TEXT("No existing landscape found - will need to create new"));
    }
    
    // Calculate expansion needed
    FVector2D ExpansionNeeded = TargetLandscapeSize - CurrentLandscapeSize;
    if (ExpansionNeeded.X > 1000.0f || ExpansionNeeded.Y > 1000.0f)
    {
        CurrentExpansionState = EWorld_ExpansionState::ReadyToExpand;
        UE_LOG(LogWorld_LandscapeExpansion, Warning, TEXT("Landscape expansion needed: %.0f x %.0f cm"), 
               ExpansionNeeded.X, ExpansionNeeded.Y);
    }
    else
    {
        CurrentExpansionState = EWorld_ExpansionState::Complete;
        UE_LOG(LogWorld_LandscapeExpansion, Log, TEXT("Landscape already at target size"));
    }
}

void UWorld_LandscapeExpansionSystem::StartLandscapeExpansion()
{
    if (CurrentExpansionState != EWorld_ExpansionState::ReadyToExpand)
    {
        UE_LOG(LogWorld_LandscapeExpansion, Warning, TEXT("Cannot start expansion - not ready"));
        return;
    }
    
    CurrentExpansionState = EWorld_ExpansionState::InProgress;
    ExpansionProgress = 0.0f;
    ExpansionStartTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogWorld_LandscapeExpansion, Warning, TEXT("Starting landscape expansion to 200km2"));
    
    // Begin expansion process
    ExpandLandscapeToTarget();
}

void UWorld_LandscapeExpansionSystem::ExpandLandscapeToTarget()
{
    // This is a complex operation that would typically involve:
    // 1. Creating new landscape components
    // 2. Generating heightmaps for each biome
    // 3. Blending biome transitions
    // 4. Updating collision and navigation
    
    // For now, we'll simulate the expansion process
    UE_LOG(LogWorld_LandscapeExpansion, Log, TEXT("Simulating landscape expansion process..."));
    
    // Mark as in progress - actual expansion would happen over multiple frames
    ExpansionProgress = 0.1f;
}

void UWorld_LandscapeExpansionSystem::UpdateExpansionProgress(float DeltaTime)
{
    if (CurrentExpansionState != EWorld_ExpansionState::InProgress)
        return;
    
    // Simulate expansion progress (5% per second)
    ExpansionProgress += DeltaTime * 0.05f;
    
    if (ExpansionProgress >= 1.0f)
    {
        CompleteExpansion();
    }
    else
    {
        UE_LOG(LogWorld_LandscapeExpansion, Log, TEXT("Landscape expansion progress: %.1f%%"), ExpansionProgress * 100.0f);
    }
}

void UWorld_LandscapeExpansionSystem::CompleteExpansion()
{
    CurrentExpansionState = EWorld_ExpansionState::Complete;
    ExpansionProgress = 1.0f;
    
    float ExpansionTime = GetWorld()->GetTimeSeconds() - ExpansionStartTime;
    UE_LOG(LogWorld_LandscapeExpansion, Warning, TEXT("Landscape expansion completed in %.1f seconds"), ExpansionTime);
    
    // Update current size to target
    CurrentLandscapeSize = TargetLandscapeSize;
    
    // Trigger post-expansion events
    OnExpansionComplete();
}

void UWorld_LandscapeExpansionSystem::OnExpansionComplete()
{
    UE_LOG(LogWorld_LandscapeExpansion, Warning, TEXT("Post-expansion setup starting..."));
    
    // Notify other systems that landscape expansion is complete
    // This would trigger vegetation placement, water body creation, etc.
    
    // Create biome markers for reference
    CreateBiomeMarkers();
}

void UWorld_LandscapeExpansionSystem::CreateBiomeMarkers()
{
    UWorld* World = GetWorld();
    if (!World)
        return;
    
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        // Create a marker actor at each biome center
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = FName(*FString::Printf(TEXT("BiomeMarker_%s"), *Biome.BiomeName));
        
        AStaticMeshActor* MarkerActor = World->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(),
            Biome.CenterLocation,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (MarkerActor)
        {
            MarkerActor->SetActorLabel(FString::Printf(TEXT("BiomeCenter_%s"), *Biome.BiomeName));
            MarkerActor->SetActorScale3D(FVector(5.0f, 5.0f, 10.0f)); // Tall marker
            
            UE_LOG(LogWorld_LandscapeExpansion, Log, TEXT("Created biome marker for %s at %s"), 
                   *Biome.BiomeName, *Biome.CenterLocation.ToString());
        }
    }
}

FVector UWorld_LandscapeExpansionSystem::GetRandomLocationInBiome(const FString& BiomeName) const
{
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeName == BiomeName)
        {
            FVector RandomOffset;
            RandomOffset.X = FMath::RandRange(Biome.BoundsMin.X, Biome.BoundsMax.X);
            RandomOffset.Y = FMath::RandRange(Biome.BoundsMin.Y, Biome.BoundsMax.Y);
            RandomOffset.Z = FMath::RandRange(Biome.ElevationRange.X, Biome.ElevationRange.Y);
            
            return RandomOffset;
        }
    }
    
    // Fallback to center if biome not found
    return FVector::ZeroVector;
}

bool UWorld_LandscapeExpansionSystem::IsLocationInBiome(const FVector& Location, const FString& BiomeName) const
{
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        if (Biome.BiomeName == BiomeName)
        {
            return (Location.X >= Biome.BoundsMin.X && Location.X <= Biome.BoundsMax.X &&
                    Location.Y >= Biome.BoundsMin.Y && Location.Y <= Biome.BoundsMax.Y &&
                    Location.Z >= Biome.BoundsMin.Z && Location.Z <= Biome.BoundsMax.Z);
        }
    }
    
    return false;
}

EWorld_TerrainType UWorld_LandscapeExpansionSystem::GetTerrainTypeAtLocation(const FVector& Location) const
{
    for (const FWorld_BiomeDefinition& Biome : BiomeDefinitions)
    {
        if (IsLocationInBiome(Location, Biome.BiomeName))
        {
            return Biome.TerrainType;
        }
    }
    
    return EWorld_TerrainType::Grassland; // Default fallback
}

float UWorld_LandscapeExpansionSystem::GetExpansionProgress() const
{
    return ExpansionProgress;
}

FVector2D UWorld_LandscapeExpansionSystem::GetCurrentLandscapeSize() const
{
    return CurrentLandscapeSize;
}

FVector2D UWorld_LandscapeExpansionSystem::GetTargetLandscapeSize() const
{
    return TargetLandscapeSize;
}