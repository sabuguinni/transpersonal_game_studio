#include "Perf_TerrainPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

APerf_TerrainPerformanceIntegrator::APerf_TerrainPerformanceIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create visualization mesh
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    VisualizationMesh->SetupAttachment(RootComponent);

    // Initialize terrain performance settings
    TerrainLODDistance = 5000.0f;
    MaxTerrainChunks = 100;
    TerrainCullingDistance = 10000.0f;
    bEnableTerrainLOD = true;
    bEnableTerrainCulling = true;

    // Initialize physics integration settings
    PhysicsUpdateRate = 60.0f;
    MaxPhysicsObjects = 500;
    bEnablePhysicsLOD = true;
    PhysicsLODDistance = 3000.0f;

    // Initialize performance metrics
    CurrentTerrainFrameTime = 0.0f;
    ActiveTerrainChunks = 0;
    ActivePhysicsObjects = 0;
    TerrainMemoryUsage = 0.0f;

    // Initialize biome performance settings
    bEnableBiomeSpecificOptimization = true;
    
    // Set biome-specific LOD multipliers
    BiomeLODMultipliers.Add(EBiomeType::Savana, 1.0f);
    BiomeLODMultipliers.Add(EBiomeType::Floresta, 0.8f);  // Dense vegetation needs more optimization
    BiomeLODMultipliers.Add(EBiomeType::Pantano, 0.9f);
    BiomeLODMultipliers.Add(EBiomeType::Deserto, 1.2f);   // Sparse terrain can handle more detail
    BiomeLODMultipliers.Add(EBiomeType::Montanha, 0.7f);  // Complex geometry needs heavy optimization

    // Set biome-specific max objects
    BiomeMaxObjects.Add(EBiomeType::Savana, 1000);
    BiomeMaxObjects.Add(EBiomeType::Floresta, 800);
    BiomeMaxObjects.Add(EBiomeType::Pantano, 600);
    BiomeMaxObjects.Add(EBiomeType::Deserto, 400);
    BiomeMaxObjects.Add(EBiomeType::Montanha, 500);

    // Initialize performance thresholds
    TargetFrameTime = 16.67f; // 60 FPS target
    MaxMemoryUsage = 2048.0f; // 2GB limit
    MaxDrawCalls = 2000;

    // Initialize timing
    LastUpdateTime = 0.0f;
    UpdateInterval = 0.1f; // Update every 100ms
    
    // Initialize performance history
    MaxHistorySize = 100;
    FrameTimeHistory.Reserve(MaxHistorySize);
    MemoryUsageHistory.Reserve(MaxHistorySize);
}

void APerf_TerrainPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();

    // Initialize performance monitoring
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            TEXT("Terrain Performance Integrator: System initialized"));
    }

    // Start performance optimization
    OptimizeTerrainPerformance();
    OptimizePhysicsPerformance();

    if (bEnableBiomeSpecificOptimization)
    {
        // Apply optimizations for all biomes
        for (const auto& BiomePair : BiomeLODMultipliers)
        {
            OptimizeBiomePerformance(BiomePair.Key);
        }
    }
}

void APerf_TerrainPerformanceIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Update performance metrics
    UpdatePerformanceMetrics();

    // Check if we need to apply optimizations
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateInterval)
    {
        LastUpdateTime = CurrentTime;

        // Apply continuous optimizations
        ApplyTerrainOptimizations();
        ApplyPhysicsOptimizations();
        
        if (bEnableBiomeSpecificOptimization)
        {
            ApplyBiomeOptimizations();
        }

        // Monitor frame time and adjust if needed
        MonitorFrameTime();
        ManageMemoryUsage();
    }
}

void APerf_TerrainPerformanceIntegrator::OptimizeTerrainPerformance()
{
    if (!bEnableTerrainLOD)
        return;

    // Get all terrain-related actors
    UWorld* World = GetWorld();
    if (!World)
        return;

    TArray<AActor*> TerrainActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), TerrainActors);

    int32 OptimizedChunks = 0;
    
    for (AActor* Actor : TerrainActors)
    {
        if (!Actor || !Actor->GetName().Contains(TEXT("Terrain")))
            continue;

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        
        // Apply LOD based on distance
        if (Distance > TerrainCullingDistance && bEnableTerrainCulling)
        {
            Actor->SetActorHiddenInGame(true);
        }
        else if (Distance > TerrainLODDistance)
        {
            // Apply low LOD
            SetTerrainLODLevel(2);
            Actor->SetActorHiddenInGame(false);
        }
        else
        {
            // Apply high LOD
            SetTerrainLODLevel(0);
            Actor->SetActorHiddenInGame(false);
        }
        
        OptimizedChunks++;
        
        if (OptimizedChunks >= MaxTerrainChunks)
            break;
    }

    ActiveTerrainChunks = OptimizedChunks;

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Blue, 
            FString::Printf(TEXT("Terrain Performance: Optimized %d chunks"), OptimizedChunks));
    }
}

void APerf_TerrainPerformanceIntegrator::SetTerrainLODLevel(int32 LODLevel)
{
    // Clamp LOD level
    LODLevel = FMath::Clamp(LODLevel, 0, 3);

    // Apply LOD settings based on level
    switch (LODLevel)
    {
        case 0: // High quality
            TerrainLODDistance = 3000.0f;
            MaxTerrainChunks = 150;
            break;
        case 1: // Medium quality
            TerrainLODDistance = 5000.0f;
            MaxTerrainChunks = 100;
            break;
        case 2: // Low quality
            TerrainLODDistance = 7000.0f;
            MaxTerrainChunks = 75;
            break;
        case 3: // Very low quality
            TerrainLODDistance = 10000.0f;
            MaxTerrainChunks = 50;
            break;
    }
}

void APerf_TerrainPerformanceIntegrator::UpdateTerrainCulling()
{
    if (!bEnableTerrainCulling)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor || !Actor->GetName().Contains(TEXT("Terrain")))
            continue;

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        bool bShouldCull = Distance > TerrainCullingDistance;
        
        Actor->SetActorHiddenInGame(bShouldCull);
    }
}

void APerf_TerrainPerformanceIntegrator::OptimizePhysicsPerformance()
{
    if (!bEnablePhysicsLOD)
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    TArray<AActor*> PhysicsActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), PhysicsActors);

    int32 OptimizedObjects = 0;
    
    for (AActor* Actor : PhysicsActors)
    {
        if (!Actor)
            continue;

        // Check if actor has physics components
        UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (!MeshComp || !MeshComp->IsSimulatingPhysics())
            continue;

        float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
        
        // Apply physics LOD based on distance
        if (Distance > PhysicsLODDistance)
        {
            // Disable physics simulation for distant objects
            MeshComp->SetSimulatePhysics(false);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        else
        {
            // Enable full physics for nearby objects
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
        
        OptimizedObjects++;
        
        if (OptimizedObjects >= MaxPhysicsObjects)
            break;
    }

    ActivePhysicsObjects = OptimizedObjects;
}

void APerf_TerrainPerformanceIntegrator::SetPhysicsLODLevel(int32 LODLevel)
{
    // Clamp LOD level
    LODLevel = FMath::Clamp(LODLevel, 0, 3);

    // Apply physics LOD settings
    switch (LODLevel)
    {
        case 0: // High quality
            PhysicsUpdateRate = 60.0f;
            MaxPhysicsObjects = 500;
            PhysicsLODDistance = 2000.0f;
            break;
        case 1: // Medium quality
            PhysicsUpdateRate = 30.0f;
            MaxPhysicsObjects = 300;
            PhysicsLODDistance = 3000.0f;
            break;
        case 2: // Low quality
            PhysicsUpdateRate = 20.0f;
            MaxPhysicsObjects = 200;
            PhysicsLODDistance = 4000.0f;
            break;
        case 3: // Very low quality
            PhysicsUpdateRate = 10.0f;
            MaxPhysicsObjects = 100;
            PhysicsLODDistance = 5000.0f;
            break;
    }
}

float APerf_TerrainPerformanceIntegrator::GetTerrainPerformanceScore()
{
    // Calculate performance score based on multiple factors
    float FrameTimeScore = FMath::Clamp(TargetFrameTime / FMath::Max(CurrentTerrainFrameTime, 1.0f), 0.0f, 1.0f);
    float MemoryScore = FMath::Clamp(1.0f - (TerrainMemoryUsage / MaxMemoryUsage), 0.0f, 1.0f);
    float ChunkScore = FMath::Clamp(1.0f - (float(ActiveTerrainChunks) / float(MaxTerrainChunks)), 0.0f, 1.0f);

    return (FrameTimeScore + MemoryScore + ChunkScore) / 3.0f;
}

void APerf_TerrainPerformanceIntegrator::GeneratePerformanceReport()
{
    FString ReportContent = FString::Printf(TEXT(
        "=== TERRAIN PERFORMANCE REPORT ===\n"
        "Frame Time: %.2f ms (Target: %.2f ms)\n"
        "Active Terrain Chunks: %d / %d\n"
        "Active Physics Objects: %d / %d\n"
        "Memory Usage: %.2f MB / %.2f MB\n"
        "Performance Score: %.2f%%\n"
        "LOD Distance: %.0f units\n"
        "Culling Distance: %.0f units\n"
        "Biome Optimization: %s\n"
        "=== END REPORT ===\n"
    ),
    CurrentTerrainFrameTime,
    TargetFrameTime,
    ActiveTerrainChunks,
    MaxTerrainChunks,
    ActivePhysicsObjects,
    MaxPhysicsObjects,
    TerrainMemoryUsage,
    MaxMemoryUsage,
    GetTerrainPerformanceScore() * 100.0f,
    TerrainLODDistance,
    TerrainCullingDistance,
    bEnableBiomeSpecificOptimization ? TEXT("Enabled") : TEXT("Disabled")
    );

    // Save report to file
    FString FilePath = FPaths::ProjectLogDir() / TEXT("TerrainPerformanceReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            TEXT("Terrain Performance Report generated"));
    }
}

bool APerf_TerrainPerformanceIntegrator::IsPerformanceOptimal()
{
    return GetTerrainPerformanceScore() > 0.8f && 
           CurrentTerrainFrameTime < TargetFrameTime &&
           TerrainMemoryUsage < MaxMemoryUsage * 0.8f;
}

void APerf_TerrainPerformanceIntegrator::OptimizeBiomePerformance(EBiomeType BiomeType)
{
    if (!BiomeLODMultipliers.Contains(BiomeType))
        return;

    float LODMultiplier = BiomeLODMultipliers[BiomeType];
    int32 MaxObjects = BiomeMaxObjects.Contains(BiomeType) ? BiomeMaxObjects[BiomeType] : 500;

    // Apply biome-specific optimizations
    float AdjustedLODDistance = TerrainLODDistance * LODMultiplier;
    int32 AdjustedMaxChunks = FMath::FloorToInt(MaxTerrainChunks * LODMultiplier);

    if (GEngine)
    {
        FString BiomeName = UEnum::GetValueAsString(BiomeType);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, 
            FString::Printf(TEXT("Biome %s: LOD Distance %.0f, Max Chunks %d"), 
                *BiomeName, AdjustedLODDistance, AdjustedMaxChunks));
    }
}

void APerf_TerrainPerformanceIntegrator::SetBiomeLODMultiplier(EBiomeType BiomeType, float Multiplier)
{
    BiomeLODMultipliers.Add(BiomeType, FMath::Clamp(Multiplier, 0.1f, 2.0f));
}

float APerf_TerrainPerformanceIntegrator::GetBiomePerformanceScore(EBiomeType BiomeType)
{
    if (!BiomeLODMultipliers.Contains(BiomeType))
        return 0.5f;

    float LODMultiplier = BiomeLODMultipliers[BiomeType];
    
    // Calculate biome-specific performance score
    float BaseScore = GetTerrainPerformanceScore();
    float BiomeModifier = FMath::Clamp(1.0f / LODMultiplier, 0.5f, 1.5f);
    
    return BaseScore * BiomeModifier;
}

void APerf_TerrainPerformanceIntegrator::UpdatePerformanceMetrics()
{
    // Update frame time
    if (GEngine && GEngine->GetGameViewport())
    {
        CurrentTerrainFrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    }

    // Update memory usage (simplified)
    TerrainMemoryUsage = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f); // Convert to MB

    // Add to history
    if (FrameTimeHistory.Num() >= MaxHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    FrameTimeHistory.Add(CurrentTerrainFrameTime);

    if (MemoryUsageHistory.Num() >= MaxHistorySize)
    {
        MemoryUsageHistory.RemoveAt(0);
    }
    MemoryUsageHistory.Add(TerrainMemoryUsage);
}

void APerf_TerrainPerformanceIntegrator::ApplyTerrainOptimizations()
{
    if (CurrentTerrainFrameTime > TargetFrameTime * 1.2f)
    {
        // Performance is poor, increase optimization
        TerrainLODDistance *= 0.9f;
        MaxTerrainChunks = FMath::Max(MaxTerrainChunks - 5, 25);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, 
                TEXT("Terrain Performance: Increasing optimization"));
        }
    }
    else if (CurrentTerrainFrameTime < TargetFrameTime * 0.8f)
    {
        // Performance is good, can reduce optimization
        TerrainLODDistance *= 1.05f;
        MaxTerrainChunks = FMath::Min(MaxTerrainChunks + 2, 200);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, 
                TEXT("Terrain Performance: Reducing optimization"));
        }
    }
}

void APerf_TerrainPerformanceIntegrator::ApplyPhysicsOptimizations()
{
    if (ActivePhysicsObjects > MaxPhysicsObjects * 0.9f)
    {
        // Too many physics objects, reduce limit
        MaxPhysicsObjects = FMath::Max(MaxPhysicsObjects - 10, 50);
        PhysicsLODDistance *= 0.9f;
    }
    else if (ActivePhysicsObjects < MaxPhysicsObjects * 0.5f)
    {
        // Can handle more physics objects
        MaxPhysicsObjects = FMath::Min(MaxPhysicsObjects + 5, 1000);
        PhysicsLODDistance *= 1.02f;
    }
}

void APerf_TerrainPerformanceIntegrator::ApplyBiomeOptimizations()
{
    // Apply biome-specific optimizations based on current performance
    for (auto& BiomePair : BiomeLODMultipliers)
    {
        EBiomeType BiomeType = BiomePair.Key;
        float& Multiplier = BiomePair.Value;
        
        if (CurrentTerrainFrameTime > TargetFrameTime)
        {
            // Reduce quality for this biome
            Multiplier = FMath::Max(Multiplier * 0.98f, 0.3f);
        }
        else if (CurrentTerrainFrameTime < TargetFrameTime * 0.7f)
        {
            // Can increase quality for this biome
            Multiplier = FMath::Min(Multiplier * 1.01f, 2.0f);
        }
    }
}

void APerf_TerrainPerformanceIntegrator::MonitorFrameTime()
{
    if (FrameTimeHistory.Num() < 10)
        return;

    // Calculate average frame time over last 10 frames
    float AverageFrameTime = 0.0f;
    for (int32 i = FrameTimeHistory.Num() - 10; i < FrameTimeHistory.Num(); i++)
    {
        AverageFrameTime += FrameTimeHistory[i];
    }
    AverageFrameTime /= 10.0f;

    // Check if we need emergency optimization
    if (AverageFrameTime > TargetFrameTime * 2.0f)
    {
        // Emergency optimization
        SetTerrainLODLevel(3); // Lowest quality
        SetPhysicsLODLevel(3);
        
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                TEXT("EMERGENCY: Applying maximum terrain optimization"));
        }
    }
}

void APerf_TerrainPerformanceIntegrator::ManageMemoryUsage()
{
    if (TerrainMemoryUsage > MaxMemoryUsage * 0.9f)
    {
        // High memory usage, force garbage collection and reduce limits
        if (GEngine)
        {
            GEngine->ForceGarbageCollection(true);
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, 
                TEXT("Terrain Performance: High memory usage, forcing cleanup"));
        }
        
        // Reduce terrain chunk limit
        MaxTerrainChunks = FMath::Max(MaxTerrainChunks - 10, 25);
        MaxPhysicsObjects = FMath::Max(MaxPhysicsObjects - 20, 50);
    }
}