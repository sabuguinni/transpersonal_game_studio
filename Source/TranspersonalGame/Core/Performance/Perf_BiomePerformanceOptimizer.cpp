#include "Perf_BiomePerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"

UPerf_BiomePerformanceOptimizer::UPerf_BiomePerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    CurrentPerformanceLevel = EPerf_BiomePerformanceLevel::Medium;
    CurrentBiome = EBiomeType::Savanna;
    bAutoOptimizationEnabled = true;
    TargetFrameRate = 60.0f;
    OptimizationThreshold = 0.8f; // Optimize when frame time exceeds 80% of target
    LastOptimizationTime = 0.0f;
    
    InitializeBiomeSettings();
}

void UPerf_BiomePerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Component initialized"));
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(60); // Store 6 seconds of history at 10Hz
    
    // Gather initial performance data
    GatherPerformanceData();
    
    // Set initial optimization based on current biome
    OptimizeForBiome(CurrentBiome);
}

void UPerf_BiomePerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update frame time history
    UpdateFrameTimeHistory(DeltaTime);
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Perform auto optimization if enabled
    if (bAutoOptimizationEnabled)
    {
        PerformAutoOptimization();
    }
}

void UPerf_BiomePerformanceOptimizer::SetPerformanceLevel(EPerf_BiomePerformanceLevel NewLevel)
{
    CurrentPerformanceLevel = NewLevel;
    
    // Apply performance level settings to current biome
    if (BiomeSettings.Contains(CurrentBiome))
    {
        FPerf_BiomePerformanceSettings Settings = BiomeSettings[CurrentBiome];
        
        // Adjust settings based on performance level
        switch (NewLevel)
        {
            case EPerf_BiomePerformanceLevel::Low:
                Settings.MaxPhysicsActors *= 0.5f;
                Settings.CullingDistance *= 0.7f;
                Settings.LODDistance *= 0.6f;
                Settings.UpdateFrequency = 0.2f;
                break;
                
            case EPerf_BiomePerformanceLevel::Medium:
                // Use default settings
                break;
                
            case EPerf_BiomePerformanceLevel::High:
                Settings.MaxPhysicsActors *= 1.5f;
                Settings.CullingDistance *= 1.3f;
                Settings.LODDistance *= 1.2f;
                Settings.UpdateFrequency = 0.05f;
                break;
                
            case EPerf_BiomePerformanceLevel::Ultra:
                Settings.MaxPhysicsActors *= 2.0f;
                Settings.CullingDistance *= 1.5f;
                Settings.LODDistance *= 1.5f;
                Settings.UpdateFrequency = 0.03f;
                break;
        }
        
        ApplyBiomeSettings(Settings);
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Performance level set to %d"), (int32)NewLevel);
}

void UPerf_BiomePerformanceOptimizer::OptimizeForBiome(EBiomeType BiomeType)
{
    CurrentBiome = BiomeType;
    
    if (BiomeSettings.Contains(BiomeType))
    {
        ApplyBiomeSettings(BiomeSettings[BiomeType]);
        UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Optimized for biome %d"), (int32)BiomeType);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("BiomePerformanceOptimizer: No settings found for biome %d"), (int32)BiomeType);
    }
}

void UPerf_BiomePerformanceOptimizer::ApplyBiomeSettings(const FPerf_BiomePerformanceSettings& Settings)
{
    // Update tick interval based on update frequency
    PrimaryComponentTick.TickInterval = Settings.UpdateFrequency;
    
    // Apply optimization settings
    if (Settings.bEnablePhysicsOptimization)
    {
        OptimizePhysicsActors();
    }
    
    if (Settings.bEnableCullingOptimization)
    {
        OptimizeCulling();
    }
    
    OptimizeLOD();
    
    UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Applied settings - MaxPhysics: %.1f, Culling: %.1f, LOD: %.1f"), 
           Settings.MaxPhysicsActors, Settings.CullingDistance, Settings.LODDistance);
}

void UPerf_BiomePerformanceOptimizer::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Update current frame time
    CurrentMetrics.CurrentFrameTime = World->GetDeltaSeconds();
    CurrentMetrics.AverageFrameTime = CalculateAverageFrameTime();
    
    // Count active physics actors
    CurrentMetrics.ActivePhysicsActors = 0;
    CurrentMetrics.CulledActors = 0;
    
    for (AActor* Actor : TrackedActors)
    {
        if (IsValid(Actor))
        {
            UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.ActivePhysicsActors++;
            }
            
            if (!Actor->GetActorEnableCollision())
            {
                CurrentMetrics.CulledActors++;
            }
        }
    }
    
    // Estimate physics and rendering times (simplified)
    CurrentMetrics.PhysicsSimulationTime = CurrentMetrics.CurrentFrameTime * 0.3f; // Rough estimate
    CurrentMetrics.RenderingTime = CurrentMetrics.CurrentFrameTime * 0.5f; // Rough estimate
}

void UPerf_BiomePerformanceOptimizer::OptimizePhysicsActors()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;
    
    FVector OwnerLocation = OwnerActor->GetActorLocation();
    
    // Get current biome settings
    FPerf_BiomePerformanceSettings CurrentSettings;
    if (BiomeSettings.Contains(CurrentBiome))
    {
        CurrentSettings = BiomeSettings[CurrentBiome];
    }
    
    // Find all actors within optimization radius
    TArray<AActor*> NearbyActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), NearbyActors);
    
    int32 PhysicsActorCount = 0;
    
    for (AActor* Actor : NearbyActors)
    {
        if (!IsValid(Actor) || Actor == OwnerActor) continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (Distance <= CurrentSettings.CullingDistance)
        {
            UPrimitiveComponent* PrimComp = Actor->FindComponentByClass<UPrimitiveComponent>();
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                PhysicsActorCount++;
                
                // Disable physics for distant actors if we exceed the limit
                if (PhysicsActorCount > CurrentSettings.MaxPhysicsActors && Distance > CurrentSettings.LODDistance)
                {
                    PrimComp->SetSimulatePhysics(false);
                    UE_LOG(LogTemp, Verbose, TEXT("BiomePerformanceOptimizer: Disabled physics for distant actor %s"), *Actor->GetName());
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Optimized %d physics actors"), PhysicsActorCount);
}

void UPerf_BiomePerformanceOptimizer::OptimizeCulling()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;
    
    FVector OwnerLocation = OwnerActor->GetActorLocation();
    
    // Get current biome settings
    FPerf_BiomePerformanceSettings CurrentSettings;
    if (BiomeSettings.Contains(CurrentBiome))
    {
        CurrentSettings = BiomeSettings[CurrentBiome];
    }
    
    // Update tracked actors list
    TrackedActors.Empty();
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), TrackedActors);
    
    int32 CulledCount = 0;
    
    for (AActor* Actor : TrackedActors)
    {
        if (!IsValid(Actor) || Actor == OwnerActor) continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        
        if (ShouldCullActor(Actor, Distance))
        {
            Actor->SetActorEnableCollision(false);
            Actor->SetActorHiddenInGame(true);
            CulledCount++;
        }
        else
        {
            Actor->SetActorEnableCollision(true);
            Actor->SetActorHiddenInGame(false);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Culled %d actors"), CulledCount);
}

void UPerf_BiomePerformanceOptimizer::OptimizeLOD()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    AActor* OwnerActor = GetOwner();
    if (!OwnerActor) return;
    
    FVector OwnerLocation = OwnerActor->GetActorLocation();
    
    for (AActor* Actor : TrackedActors)
    {
        if (!IsValid(Actor) || Actor == OwnerActor) continue;
        
        float Distance = FVector::Dist(OwnerLocation, Actor->GetActorLocation());
        UpdateActorLOD(Actor, Distance);
    }
}

void UPerf_BiomePerformanceOptimizer::EnableAutoOptimization(bool bEnable)
{
    bAutoOptimizationEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Auto optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_BiomePerformanceOptimizer::InitializeBiomeSettings()
{
    // Savanna settings - open terrain, moderate complexity
    FPerf_BiomePerformanceSettings SavannaSettings;
    SavannaSettings.MaxPhysicsActors = 60.0f;
    SavannaSettings.CullingDistance = 6000.0f;
    SavannaSettings.LODDistance = 2500.0f;
    SavannaSettings.UpdateFrequency = 0.1f;
    BiomeSettings.Add(EBiomeType::Savanna, SavannaSettings);
    
    // Swamp settings - dense vegetation, high complexity
    FPerf_BiomePerformanceSettings SwampSettings;
    SwampSettings.MaxPhysicsActors = 40.0f;
    SwampSettings.CullingDistance = 4000.0f;
    SwampSettings.LODDistance = 1800.0f;
    SwampSettings.UpdateFrequency = 0.15f;
    BiomeSettings.Add(EBiomeType::Swamp, SwampSettings);
    
    // Forest settings - very dense, highest complexity
    FPerf_BiomePerformanceSettings ForestSettings;
    ForestSettings.MaxPhysicsActors = 35.0f;
    ForestSettings.CullingDistance = 3500.0f;
    ForestSettings.LODDistance = 1500.0f;
    ForestSettings.UpdateFrequency = 0.2f;
    BiomeSettings.Add(EBiomeType::Forest, ForestSettings);
    
    // Desert settings - sparse, lowest complexity
    FPerf_BiomePerformanceSettings DesertSettings;
    DesertSettings.MaxPhysicsActors = 80.0f;
    DesertSettings.CullingDistance = 8000.0f;
    DesertSettings.LODDistance = 3500.0f;
    DesertSettings.UpdateFrequency = 0.08f;
    BiomeSettings.Add(EBiomeType::Desert, DesertSettings);
    
    // Mountain settings - varied complexity
    FPerf_BiomePerformanceSettings MountainSettings;
    MountainSettings.MaxPhysicsActors = 50.0f;
    MountainSettings.CullingDistance = 5500.0f;
    MountainSettings.LODDistance = 2200.0f;
    MountainSettings.UpdateFrequency = 0.12f;
    BiomeSettings.Add(EBiomeType::Mountain, MountainSettings);
    
    UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Initialized settings for 5 biomes"));
}

void UPerf_BiomePerformanceOptimizer::UpdateFrameTimeHistory(float DeltaTime)
{
    FrameTimeHistory.Add(DeltaTime);
    
    // Keep only recent history
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

float UPerf_BiomePerformanceOptimizer::CalculateAverageFrameTime() const
{
    if (FrameTimeHistory.Num() == 0) return 0.0f;
    
    float Total = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        Total += FrameTime;
    }
    
    return Total / FrameTimeHistory.Num();
}

void UPerf_BiomePerformanceOptimizer::PerformAutoOptimization()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    float CurrentTime = World->GetTimeSeconds();
    
    // Only optimize every few seconds to avoid constant adjustments
    if (CurrentTime - LastOptimizationTime < 2.0f) return;
    
    float TargetFrameTime = 1.0f / TargetFrameRate;
    float CurrentFrameTime = CalculateAverageFrameTime();
    
    // Check if we need to optimize
    if (CurrentFrameTime > TargetFrameTime * OptimizationThreshold)
    {
        // Performance is below target - reduce quality
        if (CurrentPerformanceLevel != EPerf_BiomePerformanceLevel::Low)
        {
            EPerf_BiomePerformanceLevel NewLevel = static_cast<EPerf_BiomePerformanceLevel>(
                static_cast<int32>(CurrentPerformanceLevel) - 1);
            SetPerformanceLevel(NewLevel);
            
            UE_LOG(LogTemp, Warning, TEXT("BiomePerformanceOptimizer: Auto-reduced performance level due to low FPS"));
        }
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.6f)
    {
        // Performance is well above target - increase quality
        if (CurrentPerformanceLevel != EPerf_BiomePerformanceLevel::Ultra)
        {
            EPerf_BiomePerformanceLevel NewLevel = static_cast<EPerf_BiomePerformanceLevel>(
                static_cast<int32>(CurrentPerformanceLevel) + 1);
            SetPerformanceLevel(NewLevel);
            
            UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Auto-increased performance level due to high FPS"));
        }
    }
    
    LastOptimizationTime = CurrentTime;
}

void UPerf_BiomePerformanceOptimizer::UpdateActorLOD(AActor* Actor, float Distance)
{
    if (!IsValid(Actor)) return;
    
    FPerf_BiomePerformanceSettings CurrentSettings;
    if (BiomeSettings.Contains(CurrentBiome))
    {
        CurrentSettings = BiomeSettings[CurrentBiome];
    }
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        // Simple LOD based on distance
        if (Distance > CurrentSettings.LODDistance * 2.0f)
        {
            // Far LOD - lowest detail
            MeshComp->SetForcedLodModel(3);
        }
        else if (Distance > CurrentSettings.LODDistance)
        {
            // Medium LOD
            MeshComp->SetForcedLodModel(2);
        }
        else if (Distance > CurrentSettings.LODDistance * 0.5f)
        {
            // Near LOD
            MeshComp->SetForcedLodModel(1);
        }
        else
        {
            // Highest LOD
            MeshComp->SetForcedLodModel(0);
        }
    }
}

bool UPerf_BiomePerformanceOptimizer::ShouldCullActor(AActor* Actor, float Distance) const
{
    if (!IsValid(Actor)) return false;
    
    FPerf_BiomePerformanceSettings CurrentSettings;
    if (BiomeSettings.Contains(CurrentBiome))
    {
        CurrentSettings = BiomeSettings[CurrentBiome];
    }
    
    // Cull actors beyond culling distance
    if (Distance > CurrentSettings.CullingDistance)
    {
        return true;
    }
    
    // Don't cull important actors
    if (Actor->IsA<APawn>() || Actor->GetName().Contains(TEXT("Player")))
    {
        return false;
    }
    
    return false;
}

void UPerf_BiomePerformanceOptimizer::GatherPerformanceData()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Initialize tracked actors
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), TrackedActors);
    
    UE_LOG(LogTemp, Log, TEXT("BiomePerformanceOptimizer: Gathered data for %d actors"), TrackedActors.Num());
}