#include "Perf_MaterialPhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

APerf_MaterialPhysicsPerformanceIntegrator::APerf_MaterialPhysicsPerformanceIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f; // Update 10 times per second

    // Create visualization mesh component
    VisualizationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualizationMesh"));
    RootComponent = VisualizationMesh;

    // Set default values
    MonitoringRadius = 10000.0f;
    UpdateInterval = 0.5f;
    bEnableAutomaticOptimization = true;
    PerformanceThreshold = 16.67f; // Target 60 FPS (16.67ms per frame)
    LastUpdateTime = 0.0f;

    // Initialize biome profiles
    BiomeProfiles.SetNum(static_cast<int32>(EBiomeType::COUNT));
    
    // Configure biome-specific performance profiles
    BiomeProfiles[static_cast<int32>(EBiomeType::Savana)].BiomeType = EBiomeType::Savana;
    BiomeProfiles[static_cast<int32>(EBiomeType::Savana)].OptimalLODDistance = 8000.0f;
    BiomeProfiles[static_cast<int32>(EBiomeType::Savana)].MaxPhysicsObjects = 150;

    BiomeProfiles[static_cast<int32>(EBiomeType::Pantano)].BiomeType = EBiomeType::Pantano;
    BiomeProfiles[static_cast<int32>(EBiomeType::Pantano)].OptimalLODDistance = 6000.0f;
    BiomeProfiles[static_cast<int32>(EBiomeType::Pantano)].MaxPhysicsObjects = 100;

    BiomeProfiles[static_cast<int32>(EBiomeType::Floresta)].BiomeType = EBiomeType::Floresta;
    BiomeProfiles[static_cast<int32>(EBiomeType::Floresta)].OptimalLODDistance = 5000.0f;
    BiomeProfiles[static_cast<int32>(EBiomeType::Floresta)].MaxPhysicsObjects = 80;

    BiomeProfiles[static_cast<int32>(EBiomeType::Deserto)].BiomeType = EBiomeType::Deserto;
    BiomeProfiles[static_cast<int32>(EBiomeType::Deserto)].OptimalLODDistance = 12000.0f;
    BiomeProfiles[static_cast<int32>(EBiomeType::Deserto)].MaxPhysicsObjects = 120;

    BiomeProfiles[static_cast<int32>(EBiomeType::Montanha)].BiomeType = EBiomeType::Montanha;
    BiomeProfiles[static_cast<int32>(EBiomeType::Montanha)].OptimalLODDistance = 7000.0f;
    BiomeProfiles[static_cast<int32>(EBiomeType::Montanha)].MaxPhysicsObjects = 90;
}

void APerf_MaterialPhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("MaterialPhysicsPerformanceIntegrator: Starting performance monitoring"));
    StartPerformanceMonitoring();
}

void APerf_MaterialPhysicsPerformanceIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= UpdateInterval)
    {
        UpdateMetrics();
        AnalyzeBiomePerformance();
        
        if (bEnableAutomaticOptimization)
        {
            ApplyOptimizations();
        }
        
        LastUpdateTime = CurrentTime;
    }
}

void APerf_MaterialPhysicsPerformanceIntegrator::StartPerformanceMonitoring()
{
    UE_LOG(LogTemp, Warning, TEXT("MaterialPhysicsPerformanceIntegrator: Performance monitoring started"));
    
    // Collect all actors in monitoring radius
    TArray<AActor*> FoundActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);
    
    MonitoredActors.Empty();
    for (AActor* Actor : FoundActors)
    {
        if (Actor && Actor != this)
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            if (Distance <= MonitoringRadius)
            {
                MonitoredActors.Add(Actor);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("MaterialPhysicsPerformanceIntegrator: Monitoring %d actors"), MonitoredActors.Num());
}

void APerf_MaterialPhysicsPerformanceIntegrator::StopPerformanceMonitoring()
{
    UE_LOG(LogTemp, Warning, TEXT("MaterialPhysicsPerformanceIntegrator: Performance monitoring stopped"));
    MonitoredActors.Empty();
}

FPerf_MaterialPhysicsMetrics APerf_MaterialPhysicsPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_MaterialPhysicsPerformanceIntegrator::OptimizePhysicsForBiome(EBiomeType BiomeType)
{
    int32 BiomeIndex = static_cast<int32>(BiomeType);
    if (BiomeIndex >= 0 && BiomeIndex < BiomeProfiles.Num())
    {
        FPerf_BiomePhysicsProfile& Profile = BiomeProfiles[BiomeIndex];
        
        // Find actors in this biome
        TArray<AActor*> BiomeActors;
        for (AActor* Actor : MonitoredActors)
        {
            if (Actor && GetBiomeTypeAtLocation(Actor->GetActorLocation()) == BiomeType)
            {
                BiomeActors.Add(Actor);
            }
        }
        
        OptimizePhysicsObjects(BiomeActors);
        
        UE_LOG(LogTemp, Warning, TEXT("MaterialPhysicsPerformanceIntegrator: Optimized %d actors in biome %d"), 
               BiomeActors.Num(), static_cast<int32>(BiomeType));
    }
}

void APerf_MaterialPhysicsPerformanceIntegrator::SetLODDistanceForBiome(EBiomeType BiomeType, float Distance)
{
    int32 BiomeIndex = static_cast<int32>(BiomeType);
    if (BiomeIndex >= 0 && BiomeIndex < BiomeProfiles.Num())
    {
        BiomeProfiles[BiomeIndex].OptimalLODDistance = Distance;
        UE_LOG(LogTemp, Warning, TEXT("MaterialPhysicsPerformanceIntegrator: Set LOD distance for biome %d to %f"), 
               static_cast<int32>(BiomeType), Distance);
    }
}

void APerf_MaterialPhysicsPerformanceIntegrator::RunPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("MaterialPhysicsPerformanceIntegrator: Running performance test"));
    
    float StartTime = FPlatformTime::Seconds();
    
    // Test collision detection performance
    int32 CollisionTests = 0;
    for (AActor* Actor : MonitoredActors)
    {
        if (Actor)
        {
            FVector Start = Actor->GetActorLocation();
            FVector End = Start + FVector(0, 0, -1000);
            
            FHitResult HitResult;
            FCollisionQueryParams QueryParams;
            QueryParams.AddIgnoredActor(Actor);
            
            bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_WorldStatic, QueryParams);
            if (bHit)
            {
                CollisionTests++;
            }
        }
    }
    
    float EndTime = FPlatformTime::Seconds();
    float TestDuration = (EndTime - StartTime) * 1000.0f; // Convert to milliseconds
    
    UE_LOG(LogTemp, Warning, TEXT("MaterialPhysicsPerformanceIntegrator: Performance test completed"));
    UE_LOG(LogTemp, Warning, TEXT("Collision tests: %d, Duration: %f ms"), CollisionTests, TestDuration);
    
    // Update metrics with test results
    CurrentMetrics.CollisionDetectionTime = TestDuration;
    CurrentMetrics.CollisionChecksPerFrame = CollisionTests;
}

void APerf_MaterialPhysicsPerformanceIntegrator::GeneratePerformanceReport()
{
    FString ReportContent;
    ReportContent += TEXT("=== Material Physics Performance Report ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += TEXT("\n");
    
    ReportContent += TEXT("Current Metrics:\n");
    ReportContent += FString::Printf(TEXT("- Collision Detection Time: %.2f ms\n"), CurrentMetrics.CollisionDetectionTime);
    ReportContent += FString::Printf(TEXT("- Physics Simulation Time: %.2f ms\n"), CurrentMetrics.PhysicsSimulationTime);
    ReportContent += FString::Printf(TEXT("- Material Interaction Time: %.2f ms\n"), CurrentMetrics.MaterialInteractionTime);
    ReportContent += FString::Printf(TEXT("- Active Physics Objects: %d\n"), CurrentMetrics.ActivePhysicsObjects);
    ReportContent += FString::Printf(TEXT("- Collision Checks Per Frame: %d\n"), CurrentMetrics.CollisionChecksPerFrame);
    ReportContent += FString::Printf(TEXT("- Frame Time Impact: %.2f ms\n"), CurrentMetrics.FrameTimeImpact);
    ReportContent += TEXT("\n");
    
    ReportContent += TEXT("Biome Performance Profiles:\n");
    for (const FPerf_BiomePhysicsProfile& Profile : BiomeProfiles)
    {
        FString BiomeName = UEnum::GetValueAsString(Profile.BiomeType);
        ReportContent += FString::Printf(TEXT("- %s:\n"), *BiomeName);
        ReportContent += FString::Printf(TEXT("  LOD Distance: %.0f units\n"), Profile.OptimalLODDistance);
        ReportContent += FString::Printf(TEXT("  Max Physics Objects: %d\n"), Profile.MaxPhysicsObjects);
        ReportContent += FString::Printf(TEXT("  Requires Optimization: %s\n"), Profile.bRequiresOptimization ? TEXT("Yes") : TEXT("No"));
    }
    
    // Save report to file
    FString FilePath = FPaths::ProjectSavedDir() / TEXT("PerformanceReports") / TEXT("MaterialPhysicsReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Warning, TEXT("MaterialPhysicsPerformanceIntegrator: Performance report saved to %s"), *FilePath);
}

void APerf_MaterialPhysicsPerformanceIntegrator::UpdateMetrics()
{
    float StartTime = FPlatformTime::Seconds();
    
    CollectPhysicsData();
    UpdateBiomeProfiles();
    
    float EndTime = FPlatformTime::Seconds();
    CurrentMetrics.FrameTimeImpact = (EndTime - StartTime) * 1000.0f;
}

void APerf_MaterialPhysicsPerformanceIntegrator::AnalyzeBiomePerformance()
{
    // Reset biome physics object counts
    BiomePhysicsObjectCount.Empty();
    
    for (AActor* Actor : MonitoredActors)
    {
        if (Actor)
        {
            EBiomeType BiomeType = GetBiomeTypeAtLocation(Actor->GetActorLocation());
            
            // Check if actor has physics simulation
            UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                int32* Count = BiomePhysicsObjectCount.Find(BiomeType);
                if (Count)
                {
                    (*Count)++;
                }
                else
                {
                    BiomePhysicsObjectCount.Add(BiomeType, 1);
                }
            }
        }
    }
    
    // Update biome profiles with current data
    for (FPerf_BiomePhysicsProfile& Profile : BiomeProfiles)
    {
        int32* Count = BiomePhysicsObjectCount.Find(Profile.BiomeType);
        int32 CurrentCount = Count ? *Count : 0;
        
        Profile.bRequiresOptimization = CurrentCount > Profile.MaxPhysicsObjects;
        Profile.Metrics.ActivePhysicsObjects = CurrentCount;
    }
}

void APerf_MaterialPhysicsPerformanceIntegrator::ApplyOptimizations()
{
    for (const FPerf_BiomePhysicsProfile& Profile : BiomeProfiles)
    {
        if (Profile.bRequiresOptimization)
        {
            OptimizePhysicsForBiome(Profile.BiomeType);
        }
    }
}

void APerf_MaterialPhysicsPerformanceIntegrator::CollectPhysicsData()
{
    int32 ActivePhysicsCount = 0;
    float TotalPhysicsTime = 0.0f;
    
    for (AActor* Actor : MonitoredActors)
    {
        if (Actor)
        {
            UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                ActivePhysicsCount++;
            }
        }
    }
    
    CurrentMetrics.ActivePhysicsObjects = ActivePhysicsCount;
    CurrentMetrics.PhysicsSimulationTime = TotalPhysicsTime;
}

void APerf_MaterialPhysicsPerformanceIntegrator::UpdateBiomeProfiles()
{
    // Update performance metrics for each biome based on current conditions
    for (FPerf_BiomePhysicsProfile& Profile : BiomeProfiles)
    {
        // Calculate average performance metrics for this biome
        Profile.Metrics.CollisionDetectionTime = CurrentMetrics.CollisionDetectionTime;
        Profile.Metrics.PhysicsSimulationTime = CurrentMetrics.PhysicsSimulationTime;
        Profile.Metrics.MaterialInteractionTime = CurrentMetrics.MaterialInteractionTime;
    }
}

EBiomeType APerf_MaterialPhysicsPerformanceIntegrator::GetBiomeTypeAtLocation(const FVector& Location) const
{
    // Biome coordinate mapping based on SharedTypes.h
    if (Location.X >= -75000 && Location.X <= -25000 && Location.Y >= -70000 && Location.Y <= -20000)
    {
        return EBiomeType::Pantano;
    }
    else if (Location.X >= -70000 && Location.X <= -20000 && Location.Y >= 15000 && Location.Y <= 65000)
    {
        return EBiomeType::Floresta;
    }
    else if (Location.X >= 30000 && Location.X <= 80000 && Location.Y >= -25000 && Location.Y <= 25000)
    {
        return EBiomeType::Deserto;
    }
    else if (Location.X >= 15000 && Location.X <= 65000 && Location.Y >= 25000 && Location.Y <= 75000)
    {
        return EBiomeType::Montanha;
    }
    else
    {
        return EBiomeType::Savana; // Default/central biome
    }
}

void APerf_MaterialPhysicsPerformanceIntegrator::OptimizePhysicsObjects(const TArray<AActor*>& Actors)
{
    for (AActor* Actor : Actors)
    {
        if (Actor)
        {
            float Distance = FVector::Dist(GetActorLocation(), Actor->GetActorLocation());
            
            if (ShouldOptimizeActor(Actor, Distance))
            {
                AdjustLODSettings(Actor, Distance);
            }
        }
    }
}

void APerf_MaterialPhysicsPerformanceIntegrator::AdjustLODSettings(AActor* Actor, float Distance)
{
    if (!Actor) return;
    
    UStaticMeshComponent* MeshComp = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        // Adjust physics simulation based on distance
        if (Distance > 5000.0f)
        {
            MeshComp->SetSimulatePhysics(false);
        }
        else if (Distance > 2000.0f)
        {
            // Reduce physics complexity
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
        }
        else
        {
            // Full physics simulation for nearby objects
            MeshComp->SetSimulatePhysics(true);
            MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        }
    }
}

bool APerf_MaterialPhysicsPerformanceIntegrator::ShouldOptimizeActor(AActor* Actor, float Distance) const
{
    if (!Actor) return false;
    
    // Don't optimize player or important gameplay actors
    if (Actor->IsA<APawn>()) return false;
    
    // Optimize based on distance and current performance
    return Distance > 1000.0f && CurrentMetrics.FrameTimeImpact > PerformanceThreshold;
}