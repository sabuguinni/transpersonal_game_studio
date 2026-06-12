#include "Perf_TerrainPhysicsProfiler.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Landscape/Landscape.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"
#include "Components/SceneComponent.h"

APerf_TerrainPhysicsProfiler::APerf_TerrainPhysicsProfiler()
{
    PrimaryActorTick.bCanEverTick = true;
    
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;
    
    bEnableProfiling = true;
    ProfilingInterval = 1.0f;
    MaxSamplesPerFrame = 10;
    FrameTimeImpact = 0.0f;
    MemoryFootprint = 0.0f;
    OptimizationLevel = 1;
    LastProfilingTime = 0.0f;
    
    InitializeBiomeProfiles();
}

void APerf_TerrainPhysicsProfiler::BeginPlay()
{
    Super::BeginPlay();
    
    // Find all landscape actors in the world
    UWorld* World = GetWorld();
    if (World)
    {
        for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
        {
            ALandscape* Landscape = *ActorItr;
            if (Landscape)
            {
                TrackedLandscapes.Add(Landscape);
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsProfiler: Found %d landscape actors"), TrackedLandscapes.Num());
    }
    
    if (bEnableProfiling)
    {
        StartProfiling();
    }
}

void APerf_TerrainPhysicsProfiler::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bEnableProfiling)
        return;
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastProfilingTime >= ProfilingInterval)
    {
        UpdateTerrainPhysicsMetrics();
        LastProfilingTime = CurrentTime;
    }
    
    // Track frame time impact
    FrameTimeSamples.Add(DeltaTime);
    if (FrameTimeSamples.Num() > 60) // Keep last 60 frames
    {
        FrameTimeSamples.RemoveAt(0);
    }
    
    // Calculate average frame time impact
    if (FrameTimeSamples.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float Sample : FrameTimeSamples)
        {
            TotalTime += Sample;
        }
        FrameTimeImpact = TotalTime / FrameTimeSamples.Num();
    }
}

void APerf_TerrainPhysicsProfiler::InitializeBiomeProfiles()
{
    BiomeProfiles.Empty();
    
    // Plains biome profile
    FPerf_BiomePhysicsProfile PlainsProfile;
    PlainsProfile.BiomeName = TEXT("Plains");
    PlainsProfile.SurfaceFriction = 0.8f;
    PlainsProfile.CollisionCost = 1.0f;
    PlainsProfile.MaxDeformationPoints = 64;
    PlainsProfile.OptimalLODDistance = 15000.0f;
    BiomeProfiles.Add(PlainsProfile);
    
    // Forest biome profile
    FPerf_BiomePhysicsProfile ForestProfile;
    ForestProfile.BiomeName = TEXT("Forest");
    ForestProfile.SurfaceFriction = 0.9f;
    ForestProfile.CollisionCost = 1.3f;
    ForestProfile.MaxDeformationPoints = 48;
    ForestProfile.OptimalLODDistance = 12000.0f;
    BiomeProfiles.Add(ForestProfile);
    
    // Desert biome profile
    FPerf_BiomePhysicsProfile DesertProfile;
    DesertProfile.BiomeName = TEXT("Desert");
    DesertProfile.SurfaceFriction = 0.4f;
    DesertProfile.CollisionCost = 0.8f;
    DesertProfile.MaxDeformationPoints = 96;
    DesertProfile.OptimalLODDistance = 18000.0f;
    BiomeProfiles.Add(DesertProfile);
    
    // Mountain biome profile
    FPerf_BiomePhysicsProfile MountainProfile;
    MountainProfile.BiomeName = TEXT("Mountain");
    MountainProfile.SurfaceFriction = 1.2f;
    MountainProfile.CollisionCost = 1.8f;
    MountainProfile.MaxDeformationPoints = 32;
    MountainProfile.OptimalLODDistance = 8000.0f;
    BiomeProfiles.Add(MountainProfile);
}

void APerf_TerrainPhysicsProfiler::UpdateTerrainPhysicsMetrics()
{
    AnalyzeCollisionComplexity();
    MonitorDeformationPerformance();
    
    // Update physics update time
    CurrentMetrics.PhysicsUpdateTime = FrameTimeImpact * 1000.0f; // Convert to milliseconds
    
    // Estimate memory usage (simplified calculation)
    CurrentMetrics.DeformationMemoryUsage = CurrentMetrics.ActiveDeformationPoints * 32.0f; // 32 bytes per point
    
    // Count collision queries (approximation based on active objects)
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentMetrics.CollisionQueries = World->GetNumPawns() * 4; // Estimate 4 queries per pawn per frame
    }
    
    // Calculate surface interaction cost
    CurrentMetrics.SurfaceInteractionCost = CurrentMetrics.CollisionComplexity * 0.1f + 
                                           CurrentMetrics.ActiveDeformationPoints * 0.05f;
}

void APerf_TerrainPhysicsProfiler::AnalyzeCollisionComplexity()
{
    float TotalComplexity = 0.0f;
    int32 LandscapeCount = 0;
    
    for (ALandscape* Landscape : TrackedLandscapes)
    {
        if (Landscape && IsValid(Landscape))
        {
            // Simplified complexity calculation based on landscape size
            FVector LandscapeScale = Landscape->GetActorScale3D();
            float ComplexityFactor = (LandscapeScale.X + LandscapeScale.Y) / 200.0f;
            TotalComplexity += ComplexityFactor;
            LandscapeCount++;
        }
    }
    
    CurrentMetrics.CollisionComplexity = LandscapeCount > 0 ? TotalComplexity / LandscapeCount : 0.0f;
}

void APerf_TerrainPhysicsProfiler::MonitorDeformationPerformance()
{
    // Count active deformation points (simplified - would need integration with actual deformation system)
    UWorld* World = GetWorld();
    if (World)
    {
        int32 DeformationCount = 0;
        
        // Look for terrain physics actors with active deformation
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetName().Contains(TEXT("TerrainPhysics")))
            {
                DeformationCount += 8; // Estimate 8 deformation points per terrain physics actor
            }
        }
        
        CurrentMetrics.ActiveDeformationPoints = DeformationCount;
    }
}

void APerf_TerrainPhysicsProfiler::OptimizePhysicsSettings()
{
    float TargetFrameTime = 1.0f / 60.0f; // 60 FPS target
    
    if (FrameTimeImpact > TargetFrameTime * 1.2f) // 20% over target
    {
        // Reduce collision complexity
        if (OptimizationLevel < 3)
        {
            OptimizationLevel++;
            UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsProfiler: Increasing optimization level to %d"), OptimizationLevel);
        }
    }
    else if (FrameTimeImpact < TargetFrameTime * 0.8f) // 20% under target
    {
        // Can afford higher quality
        if (OptimizationLevel > 1)
        {
            OptimizationLevel--;
            UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsProfiler: Decreasing optimization level to %d"), OptimizationLevel);
        }
    }
}

void APerf_TerrainPhysicsProfiler::StartProfiling()
{
    bEnableProfiling = true;
    LastProfilingTime = GetWorld()->GetTimeSeconds();
    UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsProfiler: Started profiling"));
}

void APerf_TerrainPhysicsProfiler::StopProfiling()
{
    bEnableProfiling = false;
    UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsProfiler: Stopped profiling"));
}

FPerf_TerrainPhysicsMetrics APerf_TerrainPhysicsProfiler::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_TerrainPhysicsProfiler::SetOptimizationLevel(int32 Level)
{
    OptimizationLevel = FMath::Clamp(Level, 1, 5);
    UE_LOG(LogTemp, Warning, TEXT("TerrainPhysicsProfiler: Set optimization level to %d"), OptimizationLevel);
}

float APerf_TerrainPhysicsProfiler::GetBiomePerformanceScore(const FString& BiomeName) const
{
    for (const FPerf_BiomePhysicsProfile& Profile : BiomeProfiles)
    {
        if (Profile.BiomeName == BiomeName)
        {
            // Calculate performance score based on collision cost and current metrics
            float BaseScore = 100.0f;
            float CostPenalty = Profile.CollisionCost * 10.0f;
            float ComplexityPenalty = CurrentMetrics.CollisionComplexity * 5.0f;
            
            return FMath::Max(0.0f, BaseScore - CostPenalty - ComplexityPenalty);
        }
    }
    
    return 50.0f; // Default score for unknown biomes
}

void APerf_TerrainPhysicsProfiler::OptimizeForTargetFrameRate(float TargetFPS)
{
    float TargetFrameTime = 1.0f / TargetFPS;
    
    if (FrameTimeImpact > TargetFrameTime)
    {
        // Need to optimize
        int32 NewOptimizationLevel = FMath::CeilToInt((FrameTimeImpact / TargetFrameTime) * OptimizationLevel);
        SetOptimizationLevel(NewOptimizationLevel);
        
        OptimizePhysicsSettings();
    }
}

void APerf_TerrainPhysicsProfiler::GeneratePerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("=== TERRAIN PHYSICS PERFORMANCE REPORT ==="));
    UE_LOG(LogTemp, Warning, TEXT("Frame Time Impact: %.2f ms"), FrameTimeImpact * 1000.0f);
    UE_LOG(LogTemp, Warning, TEXT("Collision Complexity: %.2f"), CurrentMetrics.CollisionComplexity);
    UE_LOG(LogTemp, Warning, TEXT("Active Deformation Points: %d"), CurrentMetrics.ActiveDeformationPoints);
    UE_LOG(LogTemp, Warning, TEXT("Memory Footprint: %.2f KB"), CurrentMetrics.DeformationMemoryUsage / 1024.0f);
    UE_LOG(LogTemp, Warning, TEXT("Optimization Level: %d"), OptimizationLevel);
    UE_LOG(LogTemp, Warning, TEXT("Tracked Landscapes: %d"), TrackedLandscapes.Num());
    
    for (const FPerf_BiomePhysicsProfile& Profile : BiomeProfiles)
    {
        float Score = GetBiomePerformanceScore(Profile.BiomeName);
        UE_LOG(LogTemp, Warning, TEXT("Biome %s Performance Score: %.1f"), *Profile.BiomeName, Score);
    }
}

void APerf_TerrainPhysicsProfiler::DebugTerrainPhysicsPerformance()
{
    GeneratePerformanceReport();
    
    // Visual debug in viewport
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("Terrain Physics FPS Impact: %.1f ms"), FrameTimeImpact * 1000.0f));
        
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, 
            FString::Printf(TEXT("Optimization Level: %d/5"), OptimizationLevel));
    }
}

// Component Implementation
UPerf_TerrainPhysicsProfilerComponent::UPerf_TerrainPhysicsProfilerComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    bAutoProfile = true;
    ProfileRadius = 5000.0f;
}

void UPerf_TerrainPhysicsProfilerComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPerf_TerrainPhysicsProfilerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bAutoProfile)
    {
        ProfileLocalTerrainPhysics();
    }
}

void UPerf_TerrainPhysicsProfilerComponent::ProfileLocalTerrainPhysics()
{
    AActor* Owner = GetOwner();
    if (!Owner)
        return;
    
    FVector OwnerLocation = Owner->GetActorLocation();
    
    // Analyze nearby collision
    AnalyzeNearbyCollision();
    
    // Update local metrics based on nearby terrain
    LocalMetrics.CollisionComplexity = 1.0f; // Simplified for local area
    LocalMetrics.ActiveDeformationPoints = 4; // Estimate for local area
    LocalMetrics.DeformationMemoryUsage = LocalMetrics.ActiveDeformationPoints * 32.0f;
    LocalMetrics.PhysicsUpdateTime = 0.5f; // Local area should be faster
    LocalMetrics.CollisionQueries = 2; // Fewer queries for local area
    LocalMetrics.SurfaceInteractionCost = 0.3f; // Lower cost for local area
}

void UPerf_TerrainPhysicsProfilerComponent::AnalyzeNearbyCollision()
{
    // Simplified analysis of collision complexity in the local area
    AActor* Owner = GetOwner();
    if (!Owner)
        return;
    
    UWorld* World = Owner->GetWorld();
    if (!World)
        return;
    
    FVector OwnerLocation = Owner->GetActorLocation();
    
    // Count nearby actors that might affect terrain physics performance
    int32 NearbyActorCount = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor != Owner)
        {
            float Distance = FVector::Dist(Actor->GetActorLocation(), OwnerLocation);
            if (Distance <= ProfileRadius)
            {
                NearbyActorCount++;
            }
        }
    }
    
    // Update local complexity based on nearby actor density
    LocalMetrics.CollisionComplexity = FMath::Min(5.0f, NearbyActorCount * 0.1f);
}

FPerf_TerrainPhysicsMetrics UPerf_TerrainPhysicsProfilerComponent::GetLocalMetrics() const
{
    return LocalMetrics;
}

void UPerf_TerrainPhysicsProfilerComponent::SetProfileRadius(float Radius)
{
    ProfileRadius = FMath::Max(100.0f, Radius);
}