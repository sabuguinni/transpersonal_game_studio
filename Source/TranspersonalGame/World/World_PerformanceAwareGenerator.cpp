#include "World_PerformanceAwareGenerator.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Landscape/Landscape.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/GameViewportClient.h"

UWorld_PerformanceAwareGenerator::UWorld_PerformanceAwareGenerator()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 1.0f; // Update every second
    
    CurrentPerformanceLevel = EWorld_PerformanceLevel::Ultra;
    PerformanceUpdateTimer = 0.0f;
    
    // Initialize default performance settings
    InitializePerformanceSettings();
    InitializeBiomeLocations();
}

void UWorld_PerformanceAwareGenerator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("World_PerformanceAwareGenerator: BeginPlay - Initializing performance monitoring"));
    
    // Start performance monitoring
    UpdatePerformanceMetrics();
    
    // Log initial state
    LogPerformanceMetrics();
}

void UWorld_PerformanceAwareGenerator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    PerformanceUpdateTimer += DeltaTime;
    
    if (PerformanceUpdateTimer >= PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        PerformanceUpdateTimer = 0.0f;
        
        // Check if performance level changed and apply optimizations
        EWorld_PerformanceLevel NewLevel = CalculatePerformanceLevel(PerformanceMetrics.CurrentFPS, PerformanceMetrics.AverageFPS);
        if (NewLevel != CurrentPerformanceLevel)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance level changed from %d to %d"), (int32)CurrentPerformanceLevel, (int32)NewLevel);
            SetPerformanceLevel(NewLevel);
        }
    }
}

void UWorld_PerformanceAwareGenerator::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get current FPS
    if (GEngine && GEngine->GetGameViewport())
    {
        PerformanceMetrics.CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        
        // Clamp to reasonable values
        PerformanceMetrics.CurrentFPS = FMath::Clamp(PerformanceMetrics.CurrentFPS, 1.0f, 300.0f);
        
        // Update FPS history
        FPSHistory.Add(PerformanceMetrics.CurrentFPS);
        if (FPSHistory.Num() > MaxFPSHistorySize)
        {
            FPSHistory.RemoveAt(0);
        }
        
        // Calculate average FPS
        if (FPSHistory.Num() > 0)
        {
            float Total = 0.0f;
            for (float FPS : FPSHistory)
            {
                Total += FPS;
            }
            PerformanceMetrics.AverageFPS = Total / FPSHistory.Num();
        }
    }
    
    // Collect world statistics
    CollectWorldStatistics();
    
    // Update performance level
    PerformanceMetrics.CurrentPerformanceLevel = CalculatePerformanceLevel(PerformanceMetrics.CurrentFPS, PerformanceMetrics.AverageFPS);
}

void UWorld_PerformanceAwareGenerator::GenerateBiomeWithPerformanceConstraints(EBiomeType BiomeType, FVector Location, float Radius)
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Generating biome %d at location (%f, %f, %f) with performance level %d"), 
           (int32)BiomeType, Location.X, Location.Y, Location.Z, (int32)CurrentPerformanceLevel);
    
    FWorld_BiomePerformanceSettings Settings = GetBiomePerformanceSettings(BiomeType);
    
    // Adjust settings based on current performance level
    switch (CurrentPerformanceLevel)
    {
        case EWorld_PerformanceLevel::Ultra:
            // No reduction
            break;
        case EWorld_PerformanceLevel::High:
            Settings.VegetationDensityMultiplier *= 0.8f;
            Settings.MaxVegetationInstances = FMath::RoundToInt(Settings.MaxVegetationInstances * 0.8f);
            break;
        case EWorld_PerformanceLevel::Medium:
            Settings.VegetationDensityMultiplier *= 0.6f;
            Settings.MaxVegetationInstances = FMath::RoundToInt(Settings.MaxVegetationInstances * 0.6f);
            Settings.TerrainDetailLevel *= 0.8f;
            break;
        case EWorld_PerformanceLevel::Low:
            Settings.VegetationDensityMultiplier *= 0.4f;
            Settings.MaxVegetationInstances = FMath::RoundToInt(Settings.MaxVegetationInstances * 0.4f);
            Settings.TerrainDetailLevel *= 0.6f;
            Settings.WaterQualityLevel *= 0.7f;
            break;
        case EWorld_PerformanceLevel::Minimal:
            Settings.VegetationDensityMultiplier *= 0.2f;
            Settings.MaxVegetationInstances = FMath::RoundToInt(Settings.MaxVegetationInstances * 0.2f);
            Settings.TerrainDetailLevel *= 0.4f;
            Settings.WaterQualityLevel *= 0.5f;
            break;
    }
    
    // Generate vegetation with performance constraints
    SpawnPerformanceAwareVegetation(BiomeType, Location, Settings.MaxVegetationInstances);
    
    // Generate dinosaurs with performance limits
    SpawnDinosaursWithPerformanceLimits(BiomeType, Location, Settings.MaxDinosaurInstances);
    
    UE_LOG(LogTemp, Warning, TEXT("Biome generation complete. Vegetation instances: %d, Dinosaur limit: %d"), 
           Settings.MaxVegetationInstances, Settings.MaxDinosaurInstances);
}

void UWorld_PerformanceAwareGenerator::SpawnPerformanceAwareVegetation(EBiomeType BiomeType, FVector Location, int32 MaxInstances)
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawning vegetation for biome %d at (%f, %f, %f), max instances: %d"), 
           (int32)BiomeType, Location.X, Location.Y, Location.Z, MaxInstances);
    
    // Check current vegetation count
    if (PerformanceMetrics.VegetationInstanceCount >= MaxTotalVegetationInstances)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum vegetation instances reached (%d), skipping spawn"), MaxTotalVegetationInstances);
        return;
    }
    
    // Adjust max instances based on current world state
    int32 RemainingInstances = MaxTotalVegetationInstances - PerformanceMetrics.VegetationInstanceCount;
    MaxInstances = FMath::Min(MaxInstances, RemainingInstances);
    
    // Spawn vegetation in a radius around the location
    float SpawnRadius = 15000.0f; // 15km radius
    int32 SpawnedCount = 0;
    
    for (int32 i = 0; i < MaxInstances && SpawnedCount < MaxInstances; i++)
    {
        // Random position within radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0.0f
        );
        
        FVector SpawnLocation = Location + RandomOffset;
        SpawnLocation.Z = 100.0f; // Ground level
        
        // Create a simple vegetation actor (placeholder)
        AActor* VegetationActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (VegetationActor)
        {
            // Add static mesh component
            UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(VegetationActor);
            VegetationActor->SetRootComponent(MeshComponent);
            
            // Set label based on biome type
            FString BiomeName = UEnum::GetValueAsString(BiomeType);
            VegetationActor->SetActorLabel(FString::Printf(TEXT("Vegetation_%s_%d"), *BiomeName, i));
            
            SpawnedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d vegetation instances for biome %d"), SpawnedCount, (int32)BiomeType);
}

void UWorld_PerformanceAwareGenerator::SpawnDinosaursWithPerformanceLimits(EBiomeType BiomeType, FVector Location, int32 MaxCount)
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawning dinosaurs for biome %d at (%f, %f, %f), max count: %d"), 
           (int32)BiomeType, Location.X, Location.Y, Location.Z, MaxCount);
    
    // Check current dinosaur count
    if (PerformanceMetrics.DinosaurCount >= MaxTotalDinosaurs)
    {
        UE_LOG(LogTemp, Warning, TEXT("Maximum dinosaur count reached (%d), skipping spawn"), MaxTotalDinosaurs);
        return;
    }
    
    // Adjust max count based on current world state
    int32 RemainingDinosaurs = MaxTotalDinosaurs - PerformanceMetrics.DinosaurCount;
    MaxCount = FMath::Min(MaxCount, RemainingDinosaurs);
    
    // Spawn dinosaurs in a smaller radius
    float SpawnRadius = 5000.0f; // 5km radius
    int32 SpawnedCount = 0;
    
    for (int32 i = 0; i < MaxCount && SpawnedCount < MaxCount; i++)
    {
        // Random position within radius
        FVector RandomOffset = FVector(
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            FMath::RandRange(-SpawnRadius, SpawnRadius),
            0.0f
        );
        
        FVector SpawnLocation = Location + RandomOffset;
        SpawnLocation.Z = 200.0f; // Above ground
        
        // Create a dinosaur actor (placeholder)
        AActor* DinosaurActor = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), SpawnLocation, FRotator::ZeroRotator);
        if (DinosaurActor)
        {
            // Add static mesh component
            UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(DinosaurActor);
            DinosaurActor->SetRootComponent(MeshComponent);
            
            // Set label based on biome type
            FString BiomeName = UEnum::GetValueAsString(BiomeType);
            DinosaurActor->SetActorLabel(FString::Printf(TEXT("Dinosaur_%s_%d"), *BiomeName, i));
            
            SpawnedCount++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Spawned %d dinosaurs for biome %d"), SpawnedCount, (int32)BiomeType);
}

void UWorld_PerformanceAwareGenerator::SetPerformanceLevel(EWorld_PerformanceLevel NewLevel)
{
    if (CurrentPerformanceLevel != NewLevel)
    {
        UE_LOG(LogTemp, Warning, TEXT("Setting performance level to %d"), (int32)NewLevel);
        CurrentPerformanceLevel = NewLevel;
        ApplyPerformanceOptimizations(NewLevel);
    }
}

void UWorld_PerformanceAwareGenerator::ApplyPerformanceOptimizations(EWorld_PerformanceLevel NewLevel)
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Applying performance optimizations for level %d"), (int32)NewLevel);
    
    // Adjust vegetation culling distance based on performance level
    switch (NewLevel)
    {
        case EWorld_PerformanceLevel::Ultra:
            VegetationCullDistance = 50000.0f;
            break;
        case EWorld_PerformanceLevel::High:
            VegetationCullDistance = 40000.0f;
            break;
        case EWorld_PerformanceLevel::Medium:
            VegetationCullDistance = 30000.0f;
            break;
        case EWorld_PerformanceLevel::Low:
            VegetationCullDistance = 20000.0f;
            break;
        case EWorld_PerformanceLevel::Minimal:
            VegetationCullDistance = 10000.0f;
            break;
    }
    
    // Apply vegetation culling
    CullDistantVegetation(VegetationCullDistance);
    
    // Optimize water quality
    OptimizeWaterQuality(NewLevel);
}

void UWorld_PerformanceAwareGenerator::CullDistantVegetation(float MaxDistance)
{
    if (!GetWorld())
    {
        return;
    }
    
    // Get player location
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 CulledCount = 0;
    
    // Find and cull distant vegetation
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetActorLabel().Contains(TEXT("Vegetation_")))
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance > MaxDistance)
            {
                Actor->SetActorHiddenInGame(true);
                CulledCount++;
            }
            else
            {
                Actor->SetActorHiddenInGame(false);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Culled %d vegetation instances beyond %f units"), CulledCount, MaxDistance);
}

void UWorld_PerformanceAwareGenerator::OptimizeWaterQuality(EWorld_PerformanceLevel PerformanceLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing water quality for performance level %d"), (int32)PerformanceLevel);
    
    // Water quality optimization would be implemented here
    // For now, just log the optimization
}

FWorld_BiomePerformanceSettings UWorld_PerformanceAwareGenerator::GetBiomePerformanceSettings(EBiomeType BiomeType) const
{
    if (const FWorld_BiomePerformanceSettings* Settings = BiomePerformanceSettings.Find(BiomeType))
    {
        return *Settings;
    }
    
    // Return default settings if not found
    return FWorld_BiomePerformanceSettings();
}

void UWorld_PerformanceAwareGenerator::SetBiomePerformanceSettings(EBiomeType BiomeType, const FWorld_BiomePerformanceSettings& Settings)
{
    BiomePerformanceSettings.Add(BiomeType, Settings);
    UE_LOG(LogTemp, Warning, TEXT("Updated performance settings for biome %d"), (int32)BiomeType);
}

void UWorld_PerformanceAwareGenerator::InitializeBiomeLocations()
{
    // Initialize biome locations from brain memory
    BiomeLocations.Add(EBiomeType::Savanna, FVector(0.0f, 0.0f, 0.0f));
    BiomeLocations.Add(EBiomeType::Swamp, FVector(-50000.0f, -45000.0f, 0.0f));
    BiomeLocations.Add(EBiomeType::Forest, FVector(-45000.0f, 40000.0f, 0.0f));
    BiomeLocations.Add(EBiomeType::Desert, FVector(55000.0f, 0.0f, 0.0f));
    BiomeLocations.Add(EBiomeType::Mountain, FVector(40000.0f, 50000.0f, 0.0f));
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized biome locations for 5 biomes"));
}

void UWorld_PerformanceAwareGenerator::InitializePerformanceSettings()
{
    // Initialize default performance settings for each biome
    FWorld_BiomePerformanceSettings DefaultSettings;
    
    // Savanna settings
    DefaultSettings.MaxVegetationInstances = 8000;
    DefaultSettings.MaxRockInstances = 3000;
    DefaultSettings.VegetationDensityMultiplier = 0.8f;
    DefaultSettings.MaxDinosaurInstances = 40;
    BiomePerformanceSettings.Add(EBiomeType::Savanna, DefaultSettings);
    
    // Forest settings (more vegetation)
    DefaultSettings.MaxVegetationInstances = 15000;
    DefaultSettings.MaxRockInstances = 2000;
    DefaultSettings.VegetationDensityMultiplier = 1.5f;
    DefaultSettings.MaxDinosaurInstances = 30;
    BiomePerformanceSettings.Add(EBiomeType::Forest, DefaultSettings);
    
    // Desert settings (less vegetation, more rocks)
    DefaultSettings.MaxVegetationInstances = 2000;
    DefaultSettings.MaxRockInstances = 8000;
    DefaultSettings.VegetationDensityMultiplier = 0.3f;
    DefaultSettings.MaxDinosaurInstances = 25;
    BiomePerformanceSettings.Add(EBiomeType::Desert, DefaultSettings);
    
    // Mountain settings (moderate vegetation, many rocks)
    DefaultSettings.MaxVegetationInstances = 5000;
    DefaultSettings.MaxRockInstances = 10000;
    DefaultSettings.VegetationDensityMultiplier = 0.6f;
    DefaultSettings.MaxDinosaurInstances = 20;
    BiomePerformanceSettings.Add(EBiomeType::Mountain, DefaultSettings);
    
    // Swamp settings (water-heavy)
    DefaultSettings.MaxVegetationInstances = 10000;
    DefaultSettings.MaxRockInstances = 1000;
    DefaultSettings.VegetationDensityMultiplier = 1.2f;
    DefaultSettings.WaterQualityLevel = 1.5f;
    DefaultSettings.MaxDinosaurInstances = 35;
    BiomePerformanceSettings.Add(EBiomeType::Swamp, DefaultSettings);
    
    UE_LOG(LogTemp, Warning, TEXT("Initialized performance settings for all biomes"));
}

EWorld_PerformanceLevel UWorld_PerformanceAwareGenerator::CalculatePerformanceLevel(float CurrentFPS, float AverageFPS) const
{
    float EffectiveFPS = FMath::Min(CurrentFPS, AverageFPS);
    
    if (EffectiveFPS >= 60.0f)
    {
        return EWorld_PerformanceLevel::Ultra;
    }
    else if (EffectiveFPS >= 45.0f)
    {
        return EWorld_PerformanceLevel::High;
    }
    else if (EffectiveFPS >= 30.0f)
    {
        return EWorld_PerformanceLevel::Medium;
    }
    else if (EffectiveFPS >= 20.0f)
    {
        return EWorld_PerformanceLevel::Low;
    }
    else
    {
        return EWorld_PerformanceLevel::Minimal;
    }
}

void UWorld_PerformanceAwareGenerator::CollectWorldStatistics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Count all actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    PerformanceMetrics.TotalActorsInWorld = AllActors.Num();
    
    // Count vegetation instances
    int32 VegCount = 0;
    int32 DinoCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            FString Label = Actor->GetActorLabel();
            if (Label.Contains(TEXT("Vegetation_")))
            {
                VegCount++;
            }
            else if (Label.Contains(TEXT("Dinosaur_")) || Label.Contains(TEXT("TRex_")) || Label.Contains(TEXT("Raptor_")))
            {
                DinoCount++;
            }
        }
    }
    
    PerformanceMetrics.VegetationInstanceCount = VegCount;
    PerformanceMetrics.DinosaurCount = DinoCount;
    
    // Estimate memory usage (simplified)
    PerformanceMetrics.MemoryUsageMB = (PerformanceMetrics.TotalActorsInWorld * 0.1f) + (VegCount * 0.05f) + (DinoCount * 2.0f);
}

FVector UWorld_PerformanceAwareGenerator::GetBiomeLocation(EBiomeType BiomeType) const
{
    if (const FVector* Location = BiomeLocations.Find(BiomeType))
    {
        return *Location;
    }
    
    return FVector::ZeroVector;
}

void UWorld_PerformanceAwareGenerator::LogPerformanceMetrics() const
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE METRICS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f"), PerformanceMetrics.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.1f"), PerformanceMetrics.AverageFPS);
    UE_LOG(LogTemp, Warning, TEXT("Performance Level: %d"), (int32)PerformanceMetrics.CurrentPerformanceLevel);
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), PerformanceMetrics.TotalActorsInWorld);
    UE_LOG(LogTemp, Warning, TEXT("Vegetation Count: %d"), PerformanceMetrics.VegetationInstanceCount);
    UE_LOG(LogTemp, Warning, TEXT("Dinosaur Count: %d"), PerformanceMetrics.DinosaurCount);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), PerformanceMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("==========================="));
}

void UWorld_PerformanceAwareGenerator::AnalyzeWorldPerformance()
{
    UpdatePerformanceMetrics();
    LogPerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("World performance analysis complete"));
}

void UWorld_PerformanceAwareGenerator::GeneratePerformanceReport()
{
    AnalyzeWorldPerformance();
    
    // Generate detailed performance report
    FString Report = FString::Printf(TEXT("PERFORMANCE REPORT - %s\n"), *FDateTime::Now().ToString());
    Report += FString::Printf(TEXT("Current FPS: %.1f\n"), PerformanceMetrics.CurrentFPS);
    Report += FString::Printf(TEXT("Average FPS: %.1f\n"), PerformanceMetrics.AverageFPS);
    Report += FString::Printf(TEXT("Performance Level: %s\n"), *UEnum::GetValueAsString(PerformanceMetrics.CurrentPerformanceLevel));
    Report += FString::Printf(TEXT("Total Actors: %d\n"), PerformanceMetrics.TotalActorsInWorld);
    Report += FString::Printf(TEXT("Vegetation: %d / %d\n"), PerformanceMetrics.VegetationInstanceCount, MaxTotalVegetationInstances);
    Report += FString::Printf(TEXT("Dinosaurs: %d / %d\n"), PerformanceMetrics.DinosaurCount, MaxTotalDinosaurs);
    Report += FString::Printf(TEXT("Memory Usage: %.1f MB\n"), PerformanceMetrics.MemoryUsageMB);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Report Generated:\n%s"), *Report);
}

void UWorld_PerformanceAwareGenerator::OptimizeExistingBiome(EBiomeType BiomeType, FVector Location, float Radius)
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing existing biome %d at location (%f, %f, %f)"), 
           (int32)BiomeType, Location.X, Location.Y, Location.Z);
    
    // Apply current performance optimizations to existing biome
    ApplyPerformanceOptimizations(CurrentPerformanceLevel);
}

void UWorld_PerformanceAwareGenerator::OptimizeTerrainLOD(float PlayerDistance)
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing terrain LOD for player distance: %f"), PlayerDistance);
    
    // Terrain LOD optimization would be implemented here
    // For now, just log the optimization
}

void UWorld_PerformanceAwareGenerator::AdjustTerrainComplexity(EWorld_PerformanceLevel TargetLevel)
{
    UE_LOG(LogTemp, Warning, TEXT("Adjusting terrain complexity for performance level %d"), (int32)TargetLevel);
    
    // Terrain complexity adjustment would be implemented here
    // For now, just log the adjustment
}