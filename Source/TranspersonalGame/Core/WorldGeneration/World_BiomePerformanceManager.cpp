#include "World_BiomePerformanceManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

AWorld_BiomePerformanceManager::AWorld_BiomePerformanceManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 1.0f; // Tick every second

    // Create visualization mesh component
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;

    // Set default performance settings based on biome type
    PerformanceSettings.TargetTriangleCount = 500000;
    PerformanceSettings.MaxDrawDistance = 15000.0f;
    PerformanceSettings.LODDistanceMultiplier = 1.0f;
    PerformanceSettings.bEnableDistanceCulling = true;
    PerformanceSettings.MaxActorsPerBiome = 1000;

    // Initialize stats
    CurrentStats.CurrentActorCount = 0;
    CurrentStats.CurrentTriangleCount = 0;
    CurrentStats.AverageFrameTime = 0.0f;
    CurrentStats.bPerformanceTarget = true;
    CurrentStats.LastUpdateTime = 0.0f;

    // Set update interval
    UpdateInterval = 5.0f;
    BiomeRadius = 25000.0f;
    LastStatsUpdate = 0.0f;
}

void AWorld_BiomePerformanceManager::BeginPlay()
{
    Super::BeginPlay();

    // Configure performance settings based on biome type
    SetBiomeType(BiomeType);

    // Load and set visualization mesh
    if (VisualizationMesh)
    {
        UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
        if (CubeMesh)
        {
            VisualizationMesh->SetStaticMesh(CubeMesh);
            VisualizationMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 0.5f));
        }
    }

    // Initial performance update
    UpdateBiomePerformance();

    UE_LOG(LogTemp, Warning, TEXT("BiomePerformanceManager: Started for biome %d at location %s"), 
           (int32)BiomeType, *GetActorLocation().ToString());
}

void AWorld_BiomePerformanceManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update stats periodically
    if (CurrentTime - LastStatsUpdate >= UpdateInterval)
    {
        UpdateBiomePerformance();
        LastStatsUpdate = CurrentTime;
    }
}

void AWorld_BiomePerformanceManager::UpdateBiomePerformance()
{
    // Calculate current stats
    CalculateStats();

    // Apply optimizations if needed
    if (!CurrentStats.bPerformanceTarget)
    {
        ApplyPerformanceOptimizations();
    }

    // Update visualization
    UpdateVisualization();

    // Log performance status
    UE_LOG(LogTemp, Log, TEXT("BiomePerf[%d]: Actors=%d, Triangles=%d, Target=%s, FPS=%.1f"), 
           (int32)BiomeType,
           CurrentStats.CurrentActorCount,
           CurrentStats.CurrentTriangleCount,
           CurrentStats.bPerformanceTarget ? TEXT("MET") : TEXT("EXCEEDED"),
           1.0f / FMath::Max(CurrentStats.AverageFrameTime, 0.001f));
}

void AWorld_BiomePerformanceManager::RegisterActor(AActor* Actor)
{
    if (Actor && !ManagedActors.Contains(Actor))
    {
        ManagedActors.Add(Actor);
        
        // Apply LOD settings to new actor
        if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
        {
            // Set LOD distance based on biome settings
            float LODDistance = PerformanceSettings.MaxDrawDistance * PerformanceSettings.LODDistanceMultiplier;
            MeshComp->SetCullDistance(LODDistance);
        }
    }
}

void AWorld_BiomePerformanceManager::UnregisterActor(AActor* Actor)
{
    if (Actor)
    {
        ManagedActors.Remove(Actor);
    }
}

void AWorld_BiomePerformanceManager::OptimizeBiomeActors()
{
    int32 OptimizedCount = 0;
    FVector BiomeCenter = GetActorLocation();

    // Get all actors in biome radius
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this) continue;

        float Distance = FVector::Dist(Actor->GetActorLocation(), BiomeCenter);
        if (Distance <= BiomeRadius)
        {
            // Apply distance-based optimizations
            if (UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                // Set LOD based on distance
                float LODLevel = FMath::Clamp(Distance / (BiomeRadius * 0.25f), 0.0f, 3.0f);
                MeshComp->SetForcedLodModel(FMath::FloorToInt(LODLevel) + 1);
                
                // Enable distance culling
                if (PerformanceSettings.bEnableDistanceCulling)
                {
                    MeshComp->SetCullDistance(PerformanceSettings.MaxDrawDistance);
                }
                
                OptimizedCount++;
            }

            // Optimize skeletal meshes (dinosaurs)
            if (USkeletalMeshComponent* SkelMeshComp = Actor->FindComponentByClass<USkeletalMeshComponent>())
            {
                // Reduce animation update rate for distant actors
                if (Distance > BiomeRadius * 0.5f)
                {
                    SkelMeshComp->SetComponentTickEnabled(false);
                }
                else
                {
                    SkelMeshComp->SetComponentTickEnabled(true);
                }
                
                OptimizedCount++;
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("BiomePerformanceManager: Optimized %d actors in biome %d"), 
           OptimizedCount, (int32)BiomeType);
}

void AWorld_BiomePerformanceManager::SetBiomeType(EBiomeType NewBiomeType)
{
    BiomeType = NewBiomeType;

    // Configure performance settings based on biome type
    switch (BiomeType)
    {
        case EBiomeType::Savana:
            PerformanceSettings.TargetTriangleCount = 500000;
            PerformanceSettings.MaxDrawDistance = 20000.0f;
            PerformanceSettings.LODDistanceMultiplier = 1.0f;
            break;
            
        case EBiomeType::Floresta:
            PerformanceSettings.TargetTriangleCount = 300000; // Dense vegetation
            PerformanceSettings.MaxDrawDistance = 12000.0f;
            PerformanceSettings.LODDistanceMultiplier = 0.8f;
            break;
            
        case EBiomeType::Deserto:
            PerformanceSettings.TargetTriangleCount = 600000; // Minimal vegetation
            PerformanceSettings.MaxDrawDistance = 25000.0f;
            PerformanceSettings.LODDistanceMultiplier = 1.2f;
            break;
            
        case EBiomeType::Pantano:
            PerformanceSettings.TargetTriangleCount = 350000; // Medium density
            PerformanceSettings.MaxDrawDistance = 15000.0f;
            PerformanceSettings.LODDistanceMultiplier = 0.9f;
            break;
            
        case EBiomeType::Montanha:
            PerformanceSettings.TargetTriangleCount = 400000; // Rocky terrain
            PerformanceSettings.MaxDrawDistance = 18000.0f;
            PerformanceSettings.LODDistanceMultiplier = 1.1f;
            break;
    }
}

void AWorld_BiomePerformanceManager::ApplyLODSettings()
{
    OptimizeBiomeActors();
}

void AWorld_BiomePerformanceManager::CalculateStats()
{
    FVector BiomeCenter = GetActorLocation();
    CurrentStats.CurrentActorCount = 0;
    CurrentStats.CurrentTriangleCount = CountTrianglesInBiome();

    // Count actors in biome radius
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this) continue;

        float Distance = FVector::Dist(Actor->GetActorLocation(), BiomeCenter);
        if (Distance <= BiomeRadius)
        {
            CurrentStats.CurrentActorCount++;
        }
    }

    // Calculate average frame time (simplified)
    CurrentStats.AverageFrameTime = GetWorld()->GetDeltaSeconds();
    
    // Check if performance target is met
    CurrentStats.bPerformanceTarget = (CurrentStats.CurrentTriangleCount <= PerformanceSettings.TargetTriangleCount) &&
                                     (CurrentStats.CurrentActorCount <= PerformanceSettings.MaxActorsPerBiome);

    CurrentStats.LastUpdateTime = GetWorld()->GetTimeSeconds();
}

void AWorld_BiomePerformanceManager::ApplyPerformanceOptimizations()
{
    // If performance target is not met, apply aggressive optimizations
    if (CurrentStats.CurrentTriangleCount > PerformanceSettings.TargetTriangleCount)
    {
        // Reduce LOD distance multiplier
        PerformanceSettings.LODDistanceMultiplier *= 0.9f;
        PerformanceSettings.LODDistanceMultiplier = FMath::Max(PerformanceSettings.LODDistanceMultiplier, 0.5f);
        
        // Apply new settings
        OptimizeBiomeActors();
        
        UE_LOG(LogTemp, Warning, TEXT("BiomePerformanceManager: Applied emergency optimizations, new LOD multiplier: %.2f"), 
               PerformanceSettings.LODDistanceMultiplier);
    }
}

int32 AWorld_BiomePerformanceManager::CountTrianglesInBiome()
{
    // Simplified triangle counting - estimate based on actor count and type
    int32 EstimatedTriangles = 0;
    FVector BiomeCenter = GetActorLocation();

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

    for (AActor* Actor : AllActors)
    {
        if (!Actor || Actor == this) continue;

        float Distance = FVector::Dist(Actor->GetActorLocation(), BiomeCenter);
        if (Distance <= BiomeRadius)
        {
            // Estimate triangles based on actor type
            if (Actor->FindComponentByClass<USkeletalMeshComponent>())
            {
                EstimatedTriangles += 5000; // Dinosaur estimate
            }
            else if (Actor->FindComponentByClass<UStaticMeshComponent>())
            {
                EstimatedTriangles += 500; // Static mesh estimate
            }
        }
    }

    return EstimatedTriangles;
}

void AWorld_BiomePerformanceManager::UpdateVisualization()
{
    if (!VisualizationMesh) return;

    // Change color based on performance status
    if (CurrentStats.bPerformanceTarget)
    {
        // Green for good performance
        VisualizationMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(0.0f, 1.0f, 0.0f));
    }
    else
    {
        // Red for poor performance
        VisualizationMesh->SetVectorParameterValueOnMaterials(TEXT("Color"), FVector(1.0f, 0.0f, 0.0f));
    }
}