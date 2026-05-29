#include "Perf_BiomePhysicsOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/PlatformFilemanager.h"

UPerf_BiomePhysicsOptimizer::UPerf_BiomePhysicsOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default settings
    OptimizationSettings = FPerf_BiomeOptimizationSettings();
    CurrentMetrics = FPerf_BiomePhysicsMetrics();
    
    bIsOptimizationActive = true;
    LastOptimizationTime = 0.0f;
    LastOptimizedBiome = EBiomeType::Savanna;
    
    // Initialize performance tracking
    AccumulatedFrameTime = 0.0f;
    FrameCount = 0;
    FrameTimeHistory.Reserve(60); // Store last 60 frame times
}

void UPerf_BiomePhysicsOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize biome actor counts
    BiomeActorCounts.Empty();
    BiomeActorCounts.Add(EBiomeType::Savanna, 0);
    BiomeActorCounts.Add(EBiomeType::Swamp, 0);
    BiomeActorCounts.Add(EBiomeType::Forest, 0);
    BiomeActorCounts.Add(EBiomeType::Desert, 0);
    BiomeActorCounts.Add(EBiomeType::Mountain, 0);
    
    // Start initial optimization
    if (bIsOptimizationActive)
    {
        OptimizeBiomePhysics(EBiomeType::Savanna);
    }
    
    UE_LOG(LogTemp, Log, TEXT("BiomePhysicsOptimizer initialized"));
}

void UPerf_BiomePhysicsOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsOptimizationActive)
    {
        return;
    }
    
    // Update performance metrics
    UpdateMetrics();
    
    // Track frame time for performance analysis
    AccumulatedFrameTime += DeltaTime * 1000.0f; // Convert to milliseconds
    FrameCount++;
    
    // Update frame time history
    if (FrameTimeHistory.Num() >= 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    FrameTimeHistory.Add(DeltaTime * 1000.0f);
    
    // Check if we need to adjust physics quality
    if (AccumulatedFrameTime > OptimizationSettings.TargetFrameTime)
    {
        AdjustPhysicsQualityForPerformance();
    }
    
    // Apply LOD optimizations every 10 frames
    if (FrameCount % 10 == 0)
    {
        ApplyPhysicsLOD();
    }
    
    // Clean up inactive actors every 60 frames (6 seconds at 10fps tick)
    if (FrameCount % 60 == 0)
    {
        CleanupInactivePhysicsActors();
    }
    
    LastOptimizationTime += DeltaTime;
}

FPerf_BiomePhysicsMetrics UPerf_BiomePhysicsOptimizer::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UPerf_BiomePhysicsOptimizer::OptimizeBiomePhysics(EBiomeType BiomeType)
{
    switch (BiomeType)
    {
        case EBiomeType::Savanna:
            OptimizeSavannaPhysics();
            break;
        case EBiomeType::Swamp:
            OptimizeSwampPhysics();
            break;
        case EBiomeType::Forest:
            OptimizeForestPhysics();
            break;
        case EBiomeType::Desert:
            OptimizeDesertPhysics();
            break;
        case EBiomeType::Mountain:
            OptimizeMountainPhysics();
            break;
    }
    
    LastOptimizedBiome = BiomeType;
    CurrentMetrics.CurrentBiome = BiomeType;
    
    UE_LOG(LogTemp, Log, TEXT("Optimized physics for biome: %d"), (int32)BiomeType);
}

void UPerf_BiomePhysicsOptimizer::SetOptimizationSettings(const FPerf_BiomeOptimizationSettings& NewSettings)
{
    OptimizationSettings = NewSettings;
    
    // Apply new settings immediately
    if (bIsOptimizationActive)
    {
        OptimizeBiomePhysics(CurrentMetrics.CurrentBiome);
    }
}

void UPerf_BiomePhysicsOptimizer::EnablePhysicsLOD(bool bEnable)
{
    OptimizationSettings.bEnableAdaptivePhysics = bEnable;
    
    if (bEnable)
    {
        ApplyPhysicsLOD();
    }
}

void UPerf_BiomePhysicsOptimizer::UpdatePhysicsQuality(float QualityScale)
{
    OptimizationSettings.PhysicsQualityScale = FMath::Clamp(QualityScale, 0.1f, 2.0f);
    AdjustPhysicsQualityForPerformance();
}

void UPerf_BiomePhysicsOptimizer::OptimizeSavannaPhysics()
{
    // Savanna optimization: moderate physics complexity, focus on grass and rock physics
    FVector SavannaCenter(0.0f, 0.0f, 0.0f);
    OptimizePhysicsActorsInRange(SavannaCenter, 10000.0f);
    
    // Reduce physics complexity for distant grass
    OptimizationSettings.PhysicsLODDistance = 3000.0f;
    OptimizationSettings.MaxPhysicsActorsPerBiome = 150;
}

void UPerf_BiomePhysicsOptimizer::OptimizeSwampPhysics()
{
    // Swamp optimization: complex water physics, reduce vegetation physics
    FVector SwampCenter(-50000.0f, -45000.0f, 0.0f);
    OptimizePhysicsActorsInRange(SwampCenter, 8000.0f);
    
    // Prioritize water physics over vegetation
    OptimizationSettings.PhysicsLODDistance = 2500.0f;
    OptimizationSettings.MaxPhysicsActorsPerBiome = 100;
}

void UPerf_BiomePhysicsOptimizer::OptimizeForestPhysics()
{
    // Forest optimization: complex tree physics, reduce undergrowth
    FVector ForestCenter(-45000.0f, 40000.0f, 0.0f);
    OptimizePhysicsActorsInRange(ForestCenter, 12000.0f);
    
    // Focus on tree trunk physics
    OptimizationSettings.PhysicsLODDistance = 4000.0f;
    OptimizationSettings.MaxPhysicsActorsPerBiome = 200;
}

void UPerf_BiomePhysicsOptimizer::OptimizeDesertPhysics()
{
    // Desert optimization: minimal physics, focus on sand and rock
    FVector DesertCenter(55000.0f, 0.0f, 0.0f);
    OptimizePhysicsActorsInRange(DesertCenter, 15000.0f);
    
    // Minimal physics for sand particles
    OptimizationSettings.PhysicsLODDistance = 5000.0f;
    OptimizationSettings.MaxPhysicsActorsPerBiome = 80;
}

void UPerf_BiomePhysicsOptimizer::OptimizeMountainPhysics()
{
    // Mountain optimization: rock physics priority, reduce vegetation
    FVector MountainCenter(40000.0f, 50000.0f, 0.0f);
    OptimizePhysicsActorsInRange(MountainCenter, 10000.0f);
    
    // Focus on large rock physics
    OptimizationSettings.PhysicsLODDistance = 6000.0f;
    OptimizationSettings.MaxPhysicsActorsPerBiome = 120;
}

float UPerf_BiomePhysicsOptimizer::AnalyzeBiomePerformance(EBiomeType BiomeType)
{
    float PerformanceScore = 100.0f;
    
    // Analyze current frame time
    if (FrameTimeHistory.Num() > 0)
    {
        float AverageFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            AverageFrameTime += FrameTime;
        }
        AverageFrameTime /= FrameTimeHistory.Num();
        
        // Penalize performance score based on frame time
        if (AverageFrameTime > OptimizationSettings.TargetFrameTime)
        {
            float Penalty = (AverageFrameTime - OptimizationSettings.TargetFrameTime) / OptimizationSettings.TargetFrameTime;
            PerformanceScore -= Penalty * 50.0f;
        }
    }
    
    // Factor in physics actor count
    int32 ActorCount = BiomeActorCounts.Contains(BiomeType) ? BiomeActorCounts[BiomeType] : 0;
    if (ActorCount > OptimizationSettings.MaxPhysicsActorsPerBiome)
    {
        float Overage = (float)(ActorCount - OptimizationSettings.MaxPhysicsActorsPerBiome) / OptimizationSettings.MaxPhysicsActorsPerBiome;
        PerformanceScore -= Overage * 30.0f;
    }
    
    return FMath::Clamp(PerformanceScore, 0.0f, 100.0f);
}

void UPerf_BiomePhysicsOptimizer::GeneratePerformanceReport()
{
    FString ReportContent;
    ReportContent += TEXT("=== BIOME PHYSICS PERFORMANCE REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Current Biome: %d\n"), (int32)CurrentMetrics.CurrentBiome);
    ReportContent += FString::Printf(TEXT("Total Physics Actors: %d\n"), CurrentMetrics.TotalPhysicsActors);
    ReportContent += FString::Printf(TEXT("Active Physics Actors: %d\n"), CurrentMetrics.ActivePhysicsActors);
    ReportContent += FString::Printf(TEXT("Physics Update Time: %.2fms\n"), CurrentMetrics.PhysicsUpdateTime);
    ReportContent += FString::Printf(TEXT("Memory Usage: %.2fMB\n"), CurrentMetrics.MemoryUsageMB);
    ReportContent += FString::Printf(TEXT("Biome Transition Cost: %.2fms\n"), CurrentMetrics.BiomeTransitionCost);
    
    // Add biome-specific performance scores
    for (auto& BiomePair : BiomeActorCounts)
    {
        float Score = AnalyzeBiomePerformance(BiomePair.Key);
        ReportContent += FString::Printf(TEXT("Biome %d Performance Score: %.1f/100\n"), (int32)BiomePair.Key, Score);
    }
    
    // Save report to file
    FString FilePath = FPaths::ProjectSavedDir() + TEXT("BiomePhysicsReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Log, TEXT("Performance report saved to: %s"), *FilePath);
}

void UPerf_BiomePhysicsOptimizer::UpdateMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Count physics actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    CurrentMetrics.TotalPhysicsActors = 0;
    CurrentMetrics.ActivePhysicsActors = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                CurrentMetrics.TotalPhysicsActors++;
                
                if (PrimComp->IsCollisionEnabled())
                {
                    CurrentMetrics.ActivePhysicsActors++;
                }
            }
        }
    }
    
    // Update physics update time (average of recent frame times)
    if (FrameTimeHistory.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            TotalTime += FrameTime;
        }
        CurrentMetrics.PhysicsUpdateTime = TotalTime / FrameTimeHistory.Num();
    }
    
    // Estimate memory usage (rough calculation)
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.TotalPhysicsActors * 0.001f; // 1KB per actor estimate
}

void UPerf_BiomePhysicsOptimizer::ApplyPhysicsLOD()
{
    if (!OptimizationSettings.bEnableAdaptivePhysics)
    {
        return;
    }
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    APawn* PlayerPawn = World->GetFirstPlayerController() ? World->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    // Apply LOD to physics actors based on distance
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetRootComponent())
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            
            if (PrimComp)
            {
                if (Distance > OptimizationSettings.PhysicsLODDistance)
                {
                    // Disable physics for distant actors
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        PrimComp->SetSimulatePhysics(false);
                    }
                }
                else if (Distance < OptimizationSettings.PhysicsLODDistance * 0.8f)
                {
                    // Re-enable physics for close actors
                    if (!PrimComp->IsSimulatingPhysics() && Actor->GetActorLabel().Contains(TEXT("Physics")))
                    {
                        PrimComp->SetSimulatePhysics(true);
                    }
                }
            }
        }
    }
}

void UPerf_BiomePhysicsOptimizer::OptimizePhysicsActorsInRange(const FVector& Center, float Radius)
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 ActorsInRange = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && FVector::Dist(Actor->GetActorLocation(), Center) <= Radius)
        {
            UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
            if (PrimComp && PrimComp->IsSimulatingPhysics())
            {
                ActorsInRange++;
                
                // Apply quality scaling
                if (OptimizationSettings.PhysicsQualityScale < 1.0f)
                {
                    // Reduce physics complexity
                    PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
                    PrimComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Optimized %d physics actors in range"), ActorsInRange);
}

void UPerf_BiomePhysicsOptimizer::AdjustPhysicsQualityForPerformance()
{
    // Calculate target quality based on performance
    float TargetQuality = 1.0f;
    
    if (FrameTimeHistory.Num() > 10)
    {
        float AverageFrameTime = 0.0f;
        for (int32 i = FrameTimeHistory.Num() - 10; i < FrameTimeHistory.Num(); i++)
        {
            AverageFrameTime += FrameTimeHistory[i];
        }
        AverageFrameTime /= 10.0f;
        
        if (AverageFrameTime > OptimizationSettings.TargetFrameTime * 1.2f)
        {
            TargetQuality = 0.7f; // Reduce quality
        }
        else if (AverageFrameTime < OptimizationSettings.TargetFrameTime * 0.8f)
        {
            TargetQuality = 1.3f; // Increase quality
        }
    }
    
    OptimizationSettings.PhysicsQualityScale = FMath::Lerp(OptimizationSettings.PhysicsQualityScale, TargetQuality, 0.1f);
    OptimizationSettings.PhysicsQualityScale = FMath::Clamp(OptimizationSettings.PhysicsQualityScale, 0.3f, 1.5f);
}

void UPerf_BiomePhysicsOptimizer::CleanupInactivePhysicsActors()
{
    // Remove invalid weak pointers from tracked actors
    TrackedPhysicsActors.RemoveAll([](const TWeakObjectPtr<AActor>& ActorPtr)
    {
        return !ActorPtr.IsValid();
    });
    
    UE_LOG(LogTemp, Log, TEXT("Cleaned up inactive physics actors. Tracking %d actors"), TrackedPhysicsActors.Num());
}