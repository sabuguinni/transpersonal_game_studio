#include "Perf_TerrainPerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Landscape/LandscapeProxy.h"
#include "Landscape/LandscapeComponent.h"
#include "Materials/MaterialInterface.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPerf_TerrainPerformanceOptimizer::UPerf_TerrainPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Check every 100ms
    
    // Initialize default LOD settings for high performance
    LODSettings.MaxLODLevel = 7;
    LODSettings.LODDistanceScale = 1.0f;
    LODSettings.ComponentScreenSizeToUseSubSections = 0.25f;
    LODSettings.LOD0ScreenSize = 0.5f;
    LODSettings.LOD0Distribution = 1.75f;
    LODSettings.LODBias = 0.0f;

    // Initialize default culling settings
    CullingSettings.MaxDrawDistance = 50000.0f;
    CullingSettings.bEnableFrustumCulling = true;
    CullingSettings.bEnableOcclusionCulling = true;
    CullingSettings.OcclusionCullingThreshold = 0.1f;
}

void UPerf_TerrainPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    FindAllLandscapeActors();
    ApplyQualitySettings(TerrainQuality);
    
    UE_LOG(LogTemp, Log, TEXT("TerrainPerformanceOptimizer: Initialized with %d landscape actors"), CachedLandscapeActors.Num());
}

void UPerf_TerrainPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    LastOptimizationTime += DeltaTime;
    
    if (LastOptimizationTime >= OptimizationInterval)
    {
        UpdateTerrainPerformanceMetrics();
        
        if (bDynamicLODEnabled)
        {
            UpdateLODBasedOnPerformance();
        }
        
        MonitorTerrainMemoryUsage();
        LastOptimizationTime = 0.0f;
    }
}

void UPerf_TerrainPerformanceOptimizer::OptimizeTerrainForTarget(float TargetFPS)
{
    TargetFrameTime = 1000.0f / TargetFPS; // Convert to milliseconds
    
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f;
    
    if (CurrentFrameTime > TargetFrameTime * 1.2f) // 20% tolerance
    {
        // Performance is poor, reduce quality
        if (TerrainQuality == EPerf_TerrainQuality::Ultra)
        {
            SetTerrainQuality(EPerf_TerrainQuality::High);
        }
        else if (TerrainQuality == EPerf_TerrainQuality::High)
        {
            SetTerrainQuality(EPerf_TerrainQuality::Medium);
        }
        else if (TerrainQuality == EPerf_TerrainQuality::Medium)
        {
            SetTerrainQuality(EPerf_TerrainQuality::Low);
        }
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.8f) // 20% headroom
    {
        // Performance is good, can increase quality
        if (TerrainQuality == EPerf_TerrainQuality::Low)
        {
            SetTerrainQuality(EPerf_TerrainQuality::Medium);
        }
        else if (TerrainQuality == EPerf_TerrainQuality::Medium)
        {
            SetTerrainQuality(EPerf_TerrainQuality::High);
        }
        else if (TerrainQuality == EPerf_TerrainQuality::High)
        {
            SetTerrainQuality(EPerf_TerrainQuality::Ultra);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("TerrainOptimizer: Target FPS %.1f, Current Frame Time %.2fms"), TargetFPS, CurrentFrameTime);
}

void UPerf_TerrainPerformanceOptimizer::SetTerrainQuality(EPerf_TerrainQuality NewQuality)
{
    if (TerrainQuality != NewQuality)
    {
        TerrainQuality = NewQuality;
        ApplyQualitySettings(NewQuality);
        UE_LOG(LogTemp, Log, TEXT("TerrainOptimizer: Quality changed to %d"), (int32)NewQuality);
    }
}

void UPerf_TerrainPerformanceOptimizer::ApplyLODSettings(const FPerf_TerrainLODSettings& NewLODSettings)
{
    LODSettings = NewLODSettings;
    
    for (ALandscapeProxy* Landscape : CachedLandscapeActors)
    {
        if (IsValid(Landscape))
        {
            Landscape->LODDistanceFactor = LODSettings.LODDistanceScale;
            Landscape->ComponentScreenSizeToUseSubSections = LODSettings.ComponentScreenSizeToUseSubSections;
            Landscape->LOD0ScreenSize = LODSettings.LOD0ScreenSize;
            Landscape->LOD0DistributionSetting = LODSettings.LOD0Distribution;
            Landscape->LODBias = LODSettings.LODBias;
            
            // Force LOD update
            Landscape->MarkComponentsRenderStateDirty();
        }
    }
}

void UPerf_TerrainPerformanceOptimizer::ApplyCullingSettings(const FPerf_TerrainCullingSettings& NewCullingSettings)
{
    CullingSettings = NewCullingSettings;
    
    for (ALandscapeProxy* Landscape : CachedLandscapeActors)
    {
        if (IsValid(Landscape))
        {
            // Apply draw distance
            for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
            {
                if (IsValid(Component))
                {
                    Component->SetCachedMaxDrawDistance(CullingSettings.MaxDrawDistance);
                }
            }
        }
    }
}

void UPerf_TerrainPerformanceOptimizer::UpdateTerrainPerformanceMetrics()
{
    VisibleTerrainComponents = 0;
    TerrainRenderTime = 0.0f;
    
    for (ALandscapeProxy* Landscape : CachedLandscapeActors)
    {
        if (IsValid(Landscape))
        {
            for (ULandscapeComponent* Component : Landscape->LandscapeComponents)
            {
                if (IsValid(Component) && Component->IsVisible())
                {
                    VisibleTerrainComponents++;
                }
            }
        }
    }
    
    // Estimate render time based on visible components
    TerrainRenderTime = VisibleTerrainComponents * 0.05f; // Rough estimate: 0.05ms per component
}

float UPerf_TerrainPerformanceOptimizer::GetTerrainRenderCost() const
{
    return TerrainRenderTime;
}

void UPerf_TerrainPerformanceOptimizer::OptimizeTerrainMaterials()
{
    for (ALandscapeProxy* Landscape : CachedLandscapeActors)
    {
        if (IsValid(Landscape))
        {
            UMaterialInterface* LandscapeMaterial = Landscape->GetLandscapeMaterial();
            if (IsValid(LandscapeMaterial))
            {
                // Apply material optimizations based on quality level
                switch (TerrainQuality)
                {
                    case EPerf_TerrainQuality::Ultra:
                        // Keep full material complexity
                        break;
                    case EPerf_TerrainQuality::High:
                        // Reduce some expensive features
                        break;
                    case EPerf_TerrainQuality::Medium:
                        // Further reduce material complexity
                        break;
                    case EPerf_TerrainQuality::Low:
                        // Minimal material features
                        break;
                    case EPerf_TerrainQuality::Potato:
                        // Extremely simplified materials
                        break;
                }
            }
        }
    }
}

void UPerf_TerrainPerformanceOptimizer::EnableDynamicLODScaling(bool bEnable)
{
    bDynamicLODEnabled = bEnable;
    UE_LOG(LogTemp, Log, TEXT("TerrainOptimizer: Dynamic LOD scaling %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_TerrainPerformanceOptimizer::FindAllLandscapeActors()
{
    CachedLandscapeActors.Empty();
    
    UWorld* World = GetWorld();
    if (IsValid(World))
    {
        for (TActorIterator<ALandscapeProxy> ActorItr(World); ActorItr; ++ActorItr)
        {
            ALandscapeProxy* Landscape = *ActorItr;
            if (IsValid(Landscape))
            {
                CachedLandscapeActors.Add(Landscape);
            }
        }
    }
}

void UPerf_TerrainPerformanceOptimizer::ApplyQualitySettings(EPerf_TerrainQuality Quality)
{
    switch (Quality)
    {
        case EPerf_TerrainQuality::Ultra:
            LODSettings.MaxLODLevel = 8;
            LODSettings.LODDistanceScale = 1.0f;
            LODSettings.LOD0ScreenSize = 0.75f;
            CullingSettings.MaxDrawDistance = 100000.0f;
            break;
            
        case EPerf_TerrainQuality::High:
            LODSettings.MaxLODLevel = 7;
            LODSettings.LODDistanceScale = 1.2f;
            LODSettings.LOD0ScreenSize = 0.5f;
            CullingSettings.MaxDrawDistance = 75000.0f;
            break;
            
        case EPerf_TerrainQuality::Medium:
            LODSettings.MaxLODLevel = 6;
            LODSettings.LODDistanceScale = 1.5f;
            LODSettings.LOD0ScreenSize = 0.35f;
            CullingSettings.MaxDrawDistance = 50000.0f;
            break;
            
        case EPerf_TerrainQuality::Low:
            LODSettings.MaxLODLevel = 5;
            LODSettings.LODDistanceScale = 2.0f;
            LODSettings.LOD0ScreenSize = 0.25f;
            CullingSettings.MaxDrawDistance = 30000.0f;
            break;
            
        case EPerf_TerrainQuality::Potato:
            LODSettings.MaxLODLevel = 4;
            LODSettings.LODDistanceScale = 3.0f;
            LODSettings.LOD0ScreenSize = 0.15f;
            CullingSettings.MaxDrawDistance = 15000.0f;
            break;
    }
    
    ApplyLODSettings(LODSettings);
    ApplyCullingSettings(CullingSettings);
}

void UPerf_TerrainPerformanceOptimizer::UpdateLODBasedOnPerformance()
{
    float CurrentFrameTime = FApp::GetDeltaTime() * 1000.0f;
    
    if (CurrentFrameTime > TargetFrameTime * 1.3f)
    {
        // Increase LOD distance scale to reduce detail
        LODSettings.LODDistanceScale = FMath::Min(LODSettings.LODDistanceScale * 1.1f, 3.0f);
        ApplyLODSettings(LODSettings);
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.7f)
    {
        // Decrease LOD distance scale to increase detail
        LODSettings.LODDistanceScale = FMath::Max(LODSettings.LODDistanceScale * 0.95f, 0.5f);
        ApplyLODSettings(LODSettings);
    }
}

void UPerf_TerrainPerformanceOptimizer::MonitorTerrainMemoryUsage()
{
    // Estimate terrain memory usage
    float EstimatedMemoryMB = 0.0f;
    
    for (ALandscapeProxy* Landscape : CachedLandscapeActors)
    {
        if (IsValid(Landscape))
        {
            int32 ComponentCount = Landscape->LandscapeComponents.Num();
            // Rough estimate: 2MB per landscape component
            EstimatedMemoryMB += ComponentCount * 2.0f;
        }
    }
    
    TerrainMemoryUsageMB = EstimatedMemoryMB;
    
    // Log memory usage periodically
    static float LastMemoryLogTime = 0.0f;
    LastMemoryLogTime += GetWorld()->GetDeltaSeconds();
    
    if (LastMemoryLogTime >= 10.0f) // Log every 10 seconds
    {
        UE_LOG(LogTemp, Log, TEXT("TerrainOptimizer: Estimated terrain memory usage: %.1f MB"), TerrainMemoryUsageMB);
        LastMemoryLogTime = 0.0f;
    }
}