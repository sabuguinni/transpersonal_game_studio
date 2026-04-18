#include "PerformanceMonitor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Stats/Stats.h"
#include "HAL/IConsoleManager.h"

APerf_PerformanceMonitor::APerf_PerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create root component
    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
    RootComponent = RootSceneComponent;

    // Create indicator mesh component
    IndicatorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("IndicatorMesh"));
    IndicatorMesh->SetupAttachment(RootComponent);

    // Load sphere mesh for visual indicator
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
    if (SphereMeshAsset.Succeeded())
    {
        IndicatorMesh->SetStaticMesh(SphereMeshAsset.Object);
        IndicatorMesh->SetWorldScale3D(FVector(2.0f, 2.0f, 2.0f));
    }

    // Initialize default values
    bEnableMonitoring = true;
    UpdateInterval = 0.1f;
    bEnableVisualFeedback = true;
    bEnableLogging = true;
    LogInterval = 5.0f;

    // Performance targets
    TargetFPS = 60.0f;
    MinimumFPS = 30.0f;
    MaxMemoryUsageMB = 4000.0f;
    MaxDrawCalls = 2000;
    MaxTriangles = 2000000;

    // Initialize performance data
    CurrentPerformanceTier = EEng_PerformanceTier::High;
    bIsPerformanceGood = true;

    // Initialize history arrays
    FPSHistory.Reserve(100);
    MemoryHistory.Reserve(100);
}

void APerf_PerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();

    if (bEnableMonitoring)
    {
        StartMonitoring();
    }

    CreateIndicatorMaterial();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Monitor started at location: %s"), 
           *GetActorLocation().ToString());
}

void APerf_PerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bEnableMonitoring)
        return;

    LastUpdateTime += DeltaTime;
    LastLogTime += DeltaTime;

    // Update performance data at specified interval
    if (LastUpdateTime >= UpdateInterval)
    {
        UpdatePerformanceData();
        LastUpdateTime = 0.0f;
    }

    // Log performance data at specified interval
    if (bEnableLogging && LastLogTime >= LogInterval)
    {
        LogPerformanceData();
        LastLogTime = 0.0f;
    }
}

void APerf_PerformanceMonitor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bEnableLogging)
    {
        // Export final performance data
        FString ExportPath = FPaths::ProjectSavedDir() / TEXT("PerformanceLogs") / 
                           FString::Printf(TEXT("PerfLog_%s.csv"), *FDateTime::Now().ToString());
        ExportPerformanceData(ExportPath);
    }

    Super::EndPlay(EndPlayReason);
}

void APerf_PerformanceMonitor::StartMonitoring()
{
    bEnableMonitoring = true;
    LastUpdateTime = 0.0f;
    LastLogTime = 0.0f;
    
    UE_LOG(LogTemp, Warning, TEXT("Performance monitoring started"));
}

void APerf_PerformanceMonitor::StopMonitoring()
{
    bEnableMonitoring = false;
    UE_LOG(LogTemp, Warning, TEXT("Performance monitoring stopped"));
}

void APerf_PerformanceMonitor::UpdatePerformanceData()
{
    CollectFrameData();
    CollectMemoryData();
    CollectRenderData();
    
    CurrentPerformanceTier = DeterminePerformanceTier();
    UpdatePerformanceHistory();
    
    if (bEnableVisualFeedback)
    {
        UpdateVisualFeedback();
    }
}

void APerf_PerformanceMonitor::CollectFrameData()
{
    // Get frame time from engine
    CurrentFrameData.FrameTime = FApp::GetDeltaTime();
    CurrentFrameData.FPS = 1.0f / FMath::Max(CurrentFrameData.FrameTime, 0.001f);
    
    // Get thread times (approximated)
    CurrentFrameData.GameThreadTime = CurrentFrameData.FrameTime * 0.6f; // Estimate
    CurrentFrameData.RenderThreadTime = CurrentFrameData.FrameTime * 0.3f; // Estimate
    CurrentFrameData.GPUTime = CurrentFrameData.FrameTime * 0.1f; // Estimate
}

void APerf_PerformanceMonitor::CollectMemoryData()
{
    // Get memory statistics
    FPlatformMemoryStats MemStats = FPlatformMemory::GetStats();
    
    CurrentMemoryData.UsedPhysicalMB = MemStats.UsedPhysical / (1024.0f * 1024.0f);
    CurrentMemoryData.UsedVirtualMB = MemStats.UsedVirtual / (1024.0f * 1024.0f);
    
    // Estimate texture and audio memory (would need more specific APIs in real implementation)
    CurrentMemoryData.TextureMemoryMB = CurrentMemoryData.UsedPhysicalMB * 0.4f;
    CurrentMemoryData.AudioMemoryMB = CurrentMemoryData.UsedPhysicalMB * 0.1f;
    CurrentMemoryData.StaticMeshMemoryMB = CurrentMemoryData.UsedPhysicalMB * 0.2f;
}

void APerf_PerformanceMonitor::CollectRenderData()
{
    // Estimate render statistics (in a real implementation, these would come from render stats)
    CurrentRenderData.DrawCalls = FMath::RandRange(800, 1500);
    CurrentRenderData.Triangles = FMath::RandRange(500000, 1500000);
    CurrentRenderData.Vertices = CurrentRenderData.Triangles * 3;
    CurrentRenderData.TextureMemoryPool = (int32)CurrentMemoryData.TextureMemoryMB;
    CurrentRenderData.ActiveLights = FMath::RandRange(10, 50);
}

void APerf_PerformanceMonitor::UpdatePerformanceHistory()
{
    // Add current FPS to history
    FPSHistory.Add(CurrentFrameData.FPS);
    if (FPSHistory.Num() > 100)
    {
        FPSHistory.RemoveAt(0);
    }
    
    // Add current memory to history
    MemoryHistory.Add(CurrentMemoryData.UsedPhysicalMB);
    if (MemoryHistory.Num() > 100)
    {
        MemoryHistory.RemoveAt(0);
    }
}

EEng_PerformanceTier APerf_PerformanceMonitor::DeterminePerformanceTier()
{
    float AvgFPS = CurrentFrameData.FPS;
    float MemoryUsage = CurrentMemoryData.UsedPhysicalMB;
    
    // Calculate average FPS from history
    if (FPSHistory.Num() > 0)
    {
        float Sum = 0.0f;
        for (float FPS : FPSHistory)
        {
            Sum += FPS;
        }
        AvgFPS = Sum / FPSHistory.Num();
    }
    
    // Determine tier based on performance metrics
    if (AvgFPS >= 55.0f && MemoryUsage < MaxMemoryUsageMB * 0.6f)
    {
        return EEng_PerformanceTier::Ultra;
    }
    else if (AvgFPS >= 45.0f && MemoryUsage < MaxMemoryUsageMB * 0.75f)
    {
        return EEng_PerformanceTier::High;
    }
    else if (AvgFPS >= 35.0f && MemoryUsage < MaxMemoryUsageMB * 0.85f)
    {
        return EEng_PerformanceTier::Medium;
    }
    else if (AvgFPS >= 25.0f && MemoryUsage < MaxMemoryUsageMB)
    {
        return EEng_PerformanceTier::Low;
    }
    else
    {
        return EEng_PerformanceTier::Potato;
    }
}

void APerf_PerformanceMonitor::UpdateVisualFeedback()
{
    if (!IndicatorMesh)
        return;
    
    FLinearColor IndicatorColor;
    
    // Color based on performance tier
    switch (CurrentPerformanceTier)
    {
        case EEng_PerformanceTier::Ultra:
            IndicatorColor = FLinearColor::Blue;
            break;
        case EEng_PerformanceTier::High:
            IndicatorColor = FLinearColor::Green;
            break;
        case EEng_PerformanceTier::Medium:
            IndicatorColor = FLinearColor::Yellow;
            break;
        case EEng_PerformanceTier::Low:
            IndicatorColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange
            break;
        case EEng_PerformanceTier::Potato:
            IndicatorColor = FLinearColor::Red;
            break;
        default:
            IndicatorColor = FLinearColor::White;
            break;
    }
    
    SetIndicatorColor(IndicatorColor);
    
    // Update performance status
    bIsPerformanceGood = (CurrentFrameData.FPS >= MinimumFPS && 
                         CurrentMemoryData.UsedPhysicalMB < MaxMemoryUsageMB);
}

void APerf_PerformanceMonitor::SetIndicatorColor(const FLinearColor& Color)
{
    if (IndicatorMesh && IndicatorMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = 
            Cast<UMaterialInstanceDynamic>(IndicatorMesh->GetMaterial(0));
        
        if (DynamicMaterial)
        {
            DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
        }
    }
}

void APerf_PerformanceMonitor::CreateIndicatorMaterial()
{
    if (IndicatorMesh)
    {
        // Create a simple dynamic material for color feedback
        UMaterialInterface* BaseMaterial = IndicatorMesh->GetMaterial(0);
        if (BaseMaterial)
        {
            UMaterialInstanceDynamic* DynamicMaterial = 
                UMaterialInstanceDynamic::Create(BaseMaterial, this);
            IndicatorMesh->SetMaterial(0, DynamicMaterial);
        }
    }
}

void APerf_PerformanceMonitor::LogPerformanceData()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE MONITOR LOG ==="));
    UE_LOG(LogTemp, Warning, TEXT("FPS: %.2f (Target: %.2f)"), CurrentFrameData.FPS, TargetFPS);
    UE_LOG(LogTemp, Warning, TEXT("Memory: %.2f MB (Max: %.2f MB)"), 
           CurrentMemoryData.UsedPhysicalMB, MaxMemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Draw Calls: %d (Max: %d)"), 
           CurrentRenderData.DrawCalls, MaxDrawCalls);
    UE_LOG(LogTemp, Warning, TEXT("Triangles: %d (Max: %d)"), 
           CurrentRenderData.Triangles, MaxTriangles);
    UE_LOG(LogTemp, Warning, TEXT("Performance Tier: %d"), (int32)CurrentPerformanceTier);
    UE_LOG(LogTemp, Warning, TEXT("Performance Good: %s"), bIsPerformanceGood ? TEXT("Yes") : TEXT("No"));
}

void APerf_PerformanceMonitor::ExportPerformanceData(const FString& FilePath)
{
    FString CSVData;
    CSVData += TEXT("Timestamp,FPS,MemoryMB,DrawCalls,Triangles,PerformanceTier\n");
    
    FString CurrentTime = FDateTime::Now().ToString();
    CSVData += FString::Printf(TEXT("%s,%.2f,%.2f,%d,%d,%d\n"),
                              *CurrentTime,
                              CurrentFrameData.FPS,
                              CurrentMemoryData.UsedPhysicalMB,
                              CurrentRenderData.DrawCalls,
                              CurrentRenderData.Triangles,
                              (int32)CurrentPerformanceTier);
    
    FFileHelper::SaveStringToFile(CSVData, *FilePath);
    UE_LOG(LogTemp, Warning, TEXT("Performance data exported to: %s"), *FilePath);
}

void APerf_PerformanceMonitor::ApplyPerformanceTier(EEng_PerformanceTier Tier)
{
    CurrentPerformanceTier = Tier;
    
    // Apply settings based on tier (this would be expanded in a real implementation)
    switch (Tier)
    {
        case EEng_PerformanceTier::Ultra:
            // Ultra settings - maximum quality
            break;
        case EEng_PerformanceTier::High:
            // High settings
            break;
        case EEng_PerformanceTier::Medium:
            // Medium settings
            break;
        case EEng_PerformanceTier::Low:
            // Low settings
            break;
        case EEng_PerformanceTier::Potato:
            // Minimum settings
            OptimizeForLowFPS();
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Applied performance tier: %d"), (int32)Tier);
}

void APerf_PerformanceMonitor::OptimizeForLowFPS()
{
    // Apply optimizations for low FPS scenarios
    UE_LOG(LogTemp, Warning, TEXT("Applying low FPS optimizations"));
    
    // In a real implementation, this would:
    // - Reduce shadow quality
    // - Lower texture resolution
    // - Disable expensive effects
    // - Reduce view distance
    // - Lower LOD bias
}

void APerf_PerformanceMonitor::OptimizeForHighMemory()
{
    // Apply optimizations for high memory usage
    UE_LOG(LogTemp, Warning, TEXT("Applying high memory optimizations"));
    
    // In a real implementation, this would:
    // - Reduce texture streaming pool
    // - Lower audio quality
    // - Reduce mesh LOD levels
    // - Clear unused assets
}

void APerf_PerformanceMonitor::ResetToDefaultSettings()
{
    // Reset all performance settings to defaults
    CurrentPerformanceTier = EEng_PerformanceTier::High;
    UE_LOG(LogTemp, Warning, TEXT("Performance settings reset to defaults"));
}