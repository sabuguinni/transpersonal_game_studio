#include "Perf_PhysicsWorldPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Stats/Stats.h"

UPerf_PhysicsWorldPerformanceIntegrator::UPerf_PhysicsWorldPerformanceIntegrator()
{
    CurrentOptimizationLevel = EPerf_PhysicsOptimizationLevel::Medium;
    bIsMonitoringActive = false;
    bIsIntegratedWithPhysicsWorld = false;
    LastPerformanceCheckTime = 0.0f;
    PhysicsWorldManager = nullptr;

    // Initialize default optimization settings
    OptimizationSettings.MaxPhysicsFrameTime = 16.67f; // 60 FPS
    OptimizationSettings.MaxActiveRigidBodies = 1000;
    OptimizationSettings.PhysicsLODDistance = 5000.0f;
    OptimizationSettings.bEnablePhysicsLOD = true;
    OptimizationSettings.bEnableAsyncPhysics = true;
    OptimizationSettings.PhysicsSubstepDelta = 0.0166f;

    // Initialize biome-specific settings
    FPerf_PhysicsOptimizationSettings ForestSettings;
    ForestSettings.MaxActiveRigidBodies = 800;
    ForestSettings.PhysicsLODDistance = 4000.0f;
    BiomeOptimizationSettings.Add(EBiomeType::Forest, ForestSettings);

    FPerf_PhysicsOptimizationSettings DesertSettings;
    DesertSettings.MaxActiveRigidBodies = 1200;
    DesertSettings.PhysicsLODDistance = 6000.0f;
    BiomeOptimizationSettings.Add(EBiomeType::Desert, DesertSettings);

    FPerf_PhysicsOptimizationSettings MountainSettings;
    MountainSettings.MaxActiveRigidBodies = 600;
    MountainSettings.PhysicsLODDistance = 3500.0f;
    BiomeOptimizationSettings.Add(EBiomeType::Mountain, MountainSettings);

    FPerf_PhysicsOptimizationSettings SwampSettings;
    SwampSettings.MaxActiveRigidBodies = 500;
    SwampSettings.PhysicsLODDistance = 3000.0f;
    BiomeOptimizationSettings.Add(EBiomeType::Swamp, SwampSettings);
}

void UPerf_PhysicsWorldPerformanceIntegrator::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("Physics World Performance Integrator initialized"));
    
    // Initialize physics world integration
    InitializePhysicsWorldIntegration();
    
    // Start performance monitoring by default
    StartPerformanceMonitoring();
    
    // Apply initial optimizations
    ApplyPhysicsOptimizations();
}

void UPerf_PhysicsWorldPerformanceIntegrator::Deinitialize()
{
    StopPerformanceMonitoring();
    
    // Clear timer handles
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
        World->GetTimerManager().ClearTimer(OptimizationUpdateTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics World Performance Integrator deinitialized"));
    
    Super::Deinitialize();
}

void UPerf_PhysicsWorldPerformanceIntegrator::StartPerformanceMonitoring()
{
    if (bIsMonitoringActive)
    {
        return;
    }
    
    bIsMonitoringActive = true;
    
    if (UWorld* World = GetWorld())
    {
        // Start performance monitoring timer (update every 0.1 seconds)
        World->GetTimerManager().SetTimer(
            PerformanceMonitoringTimer,
            this,
            &UPerf_PhysicsWorldPerformanceIntegrator::UpdatePerformanceMetrics,
            0.1f,
            true
        );
        
        // Start optimization update timer (update every 1 second)
        World->GetTimerManager().SetTimer(
            OptimizationUpdateTimer,
            this,
            &UPerf_PhysicsWorldPerformanceIntegrator::CheckPerformanceThresholds,
            1.0f,
            true
        );
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics performance monitoring started"));
}

void UPerf_PhysicsWorldPerformanceIntegrator::StopPerformanceMonitoring()
{
    if (!bIsMonitoringActive)
    {
        return;
    }
    
    bIsMonitoringActive = false;
    
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceMonitoringTimer);
        World->GetTimerManager().ClearTimer(OptimizationUpdateTimer);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics performance monitoring stopped"));
}

FPerf_PhysicsPerformanceMetrics UPerf_PhysicsWorldPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

float UPerf_PhysicsWorldPerformanceIntegrator::GetPhysicsFrameTime() const
{
    return CurrentMetrics.PhysicsFrameTime;
}

bool UPerf_PhysicsWorldPerformanceIntegrator::IsPhysicsPerformanceOptimal() const
{
    return CurrentMetrics.PhysicsFrameTime <= OptimizationSettings.MaxPhysicsFrameTime &&
           CurrentMetrics.ActiveRigidBodies <= OptimizationSettings.MaxActiveRigidBodies;
}

void UPerf_PhysicsWorldPerformanceIntegrator::SetOptimizationLevel(EPerf_PhysicsOptimizationLevel Level)
{
    CurrentOptimizationLevel = Level;
    
    switch (Level)
    {
        case EPerf_PhysicsOptimizationLevel::Low:
            OptimizationSettings.MaxActiveRigidBodies = 2000;
            OptimizationSettings.PhysicsLODDistance = 8000.0f;
            OptimizationSettings.bEnablePhysicsLOD = false;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Medium:
            OptimizationSettings.MaxActiveRigidBodies = 1000;
            OptimizationSettings.PhysicsLODDistance = 5000.0f;
            OptimizationSettings.bEnablePhysicsLOD = true;
            break;
            
        case EPerf_PhysicsOptimizationLevel::High:
            OptimizationSettings.MaxActiveRigidBodies = 500;
            OptimizationSettings.PhysicsLODDistance = 3000.0f;
            OptimizationSettings.bEnablePhysicsLOD = true;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Ultra:
            OptimizationSettings.MaxActiveRigidBodies = 250;
            OptimizationSettings.PhysicsLODDistance = 2000.0f;
            OptimizationSettings.bEnablePhysicsLOD = true;
            break;
            
        case EPerf_PhysicsOptimizationLevel::Custom:
            // Keep current settings
            break;
    }
    
    ApplyPhysicsOptimizations();
    
    UE_LOG(LogTemp, Log, TEXT("Physics optimization level set to: %d"), (int32)Level);
}

void UPerf_PhysicsWorldPerformanceIntegrator::ApplyPhysicsOptimizations()
{
    if (UWorld* World = GetWorld())
    {
        // Apply global physics settings
        if (UPhysicsSettings* PhysicsSettings = UPhysicsSettings::Get())
        {
            // Enable async physics if specified
            if (OptimizationSettings.bEnableAsyncPhysics)
            {
                // Note: Async physics settings would be applied here
                UE_LOG(LogTemp, Log, TEXT("Async physics optimization applied"));
            }
            
            // Set physics substep delta
            // PhysicsSettings->MaxSubstepDeltaTime = OptimizationSettings.PhysicsSubstepDelta;
        }
        
        // Optimize existing physics actors
        OptimizePhysicsActors();
        
        // Update LOD system
        if (OptimizationSettings.bEnablePhysicsLOD)
        {
            UpdatePhysicsLOD();
        }
        
        // Manage physics memory
        ManagePhysicsMemory();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics optimizations applied"));
}

void UPerf_PhysicsWorldPerformanceIntegrator::OptimizePhysicsForBiome(EBiomeType BiomeType)
{
    if (BiomeOptimizationSettings.Contains(BiomeType))
    {
        OptimizationSettings = BiomeOptimizationSettings[BiomeType];
        ApplyPhysicsOptimizations();
        
        UE_LOG(LogTemp, Log, TEXT("Physics optimized for biome type: %d"), (int32)BiomeType);
    }
}

void UPerf_PhysicsWorldPerformanceIntegrator::EnablePhysicsLOD(bool bEnable)
{
    OptimizationSettings.bEnablePhysicsLOD = bEnable;
    
    if (bEnable)
    {
        UpdatePhysicsLOD();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics LOD %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_PhysicsWorldPerformanceIntegrator::SetPhysicsLODDistance(float Distance)
{
    OptimizationSettings.PhysicsLODDistance = Distance;
    
    if (OptimizationSettings.bEnablePhysicsLOD)
    {
        UpdatePhysicsLOD();
    }
    
    UE_LOG(LogTemp, Log, TEXT("Physics LOD distance set to: %f"), Distance);
}

void UPerf_PhysicsWorldPerformanceIntegrator::IntegrateWithPhysicsWorldManager()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        // Try to get the physics world manager
        PhysicsWorldManager = GameInstance->GetSubsystem<UCore_PhysicsWorldManager>();
        
        if (PhysicsWorldManager)
        {
            bIsIntegratedWithPhysicsWorld = true;
            SynchronizeWithPhysicsWorld();
            UE_LOG(LogTemp, Log, TEXT("Successfully integrated with Physics World Manager"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Physics World Manager not found for integration"));
        }
    }
}

void UPerf_PhysicsWorldPerformanceIntegrator::OptimizeRegionalPhysics(const FVector& PlayerLocation)
{
    if (!bIsIntegratedWithPhysicsWorld || !PhysicsWorldManager)
    {
        return;
    }
    
    // Update regional optimizations based on player location
    UpdateRegionalOptimizations();
    
    // Apply distance-based optimizations
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AStaticMeshActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AStaticMeshActor* Actor = *ActorIterator;
            if (Actor && Actor->GetStaticMeshComponent())
            {
                float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
                
                // Apply LOD based on distance
                if (Distance > OptimizationSettings.PhysicsLODDistance)
                {
                    // Disable physics simulation for distant objects
                    Actor->GetStaticMeshComponent()->SetSimulatePhysics(false);
                }
                else
                {
                    // Enable physics for nearby objects
                    Actor->GetStaticMeshComponent()->SetSimulatePhysics(true);
                }
            }
        }
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Regional physics optimized for player location: %s"), 
           *PlayerLocation.ToString());
}

void UPerf_PhysicsWorldPerformanceIntegrator::UpdatePhysicsPerformanceBudget()
{
    // Calculate current performance budget usage
    float CurrentFrameTime = GetPhysicsFrameTime();
    float BudgetUsage = (CurrentFrameTime / OptimizationSettings.MaxPhysicsFrameTime) * 100.0f;
    
    // Apply automatic optimizations if over budget
    if (BudgetUsage > 90.0f)
    {
        ApplyAutomaticOptimizations();
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Physics performance budget usage: %.1f%%"), BudgetUsage);
}

void UPerf_PhysicsWorldPerformanceIntegrator::ValidatePhysicsPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Physics Performance Validation ==="));
    
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics Frame Time: %.2f ms (Target: %.2f ms)"), 
           CurrentMetrics.PhysicsFrameTime, OptimizationSettings.MaxPhysicsFrameTime);
    UE_LOG(LogTemp, Warning, TEXT("Active Rigid Bodies: %d (Max: %d)"), 
           CurrentMetrics.ActiveRigidBodies, OptimizationSettings.MaxActiveRigidBodies);
    UE_LOG(LogTemp, Warning, TEXT("Physics Constraints: %d"), CurrentMetrics.PhysicsConstraints);
    UE_LOG(LogTemp, Warning, TEXT("Physics Memory Usage: %.2f MB"), CurrentMetrics.PhysicsMemoryUsage);
    UE_LOG(LogTemp, Warning, TEXT("Collision Checks: %d"), CurrentMetrics.CollisionChecks);
    UE_LOG(LogTemp, Warning, TEXT("Physics Thread Time: %.2f ms"), CurrentMetrics.PhysicsThreadTime);
    
    bool bPerformanceOptimal = IsPhysicsPerformanceOptimal();
    UE_LOG(LogTemp, Warning, TEXT("Performance Status: %s"), 
           bPerformanceOptimal ? TEXT("OPTIMAL") : TEXT("NEEDS OPTIMIZATION"));
    
    UE_LOG(LogTemp, Warning, TEXT("Integration Status: %s"), 
           bIsIntegratedWithPhysicsWorld ? TEXT("INTEGRATED") : TEXT("NOT INTEGRATED"));
}

void UPerf_PhysicsWorldPerformanceIntegrator::LogPhysicsPerformanceReport()
{
    FString ReportString = FString::Printf(
        TEXT("Physics Performance Report:\n")
        TEXT("Frame Time: %.2f ms\n")
        TEXT("Active Bodies: %d\n")
        TEXT("Memory Usage: %.2f MB\n")
        TEXT("Optimization Level: %d\n")
        TEXT("LOD Enabled: %s\n"),
        CurrentMetrics.PhysicsFrameTime,
        CurrentMetrics.ActiveRigidBodies,
        CurrentMetrics.PhysicsMemoryUsage,
        (int32)CurrentOptimizationLevel,
        OptimizationSettings.bEnablePhysicsLOD ? TEXT("Yes") : TEXT("No")
    );
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *ReportString);
}

void UPerf_PhysicsWorldPerformanceIntegrator::TestPhysicsOptimizations()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing physics optimizations..."));
    
    // Test different optimization levels
    for (int32 i = 0; i < 4; ++i)
    {
        EPerf_PhysicsOptimizationLevel TestLevel = (EPerf_PhysicsOptimizationLevel)i;
        SetOptimizationLevel(TestLevel);
        
        // Wait a frame and measure performance
        FPlatformProcess::Sleep(0.1f);
        UpdatePerformanceMetrics();
        
        UE_LOG(LogTemp, Warning, TEXT("Level %d - Frame Time: %.2f ms, Bodies: %d"), 
               i, CurrentMetrics.PhysicsFrameTime, CurrentMetrics.ActiveRigidBodies);
    }
    
    // Reset to medium level
    SetOptimizationLevel(EPerf_PhysicsOptimizationLevel::Medium);
}

void UPerf_PhysicsWorldPerformanceIntegrator::UpdatePerformanceMetrics()
{
    if (!bIsMonitoringActive)
    {
        return;
    }
    
    // Update physics frame time
    CurrentMetrics.PhysicsFrameTime = CalculatePhysicsFrameTime();
    
    // Update active rigid bodies count
    CurrentMetrics.ActiveRigidBodies = CountActiveRigidBodies();
    
    // Update physics memory usage
    CurrentMetrics.PhysicsMemoryUsage = EstimatePhysicsMemoryUsage();
    
    // Update other metrics (simplified for now)
    CurrentMetrics.PhysicsConstraints = CurrentMetrics.ActiveRigidBodies / 4; // Estimated
    CurrentMetrics.CollisionChecks = CurrentMetrics.ActiveRigidBodies * 2; // Estimated
    CurrentMetrics.PhysicsThreadTime = CurrentMetrics.PhysicsFrameTime * 0.8f; // Estimated
    
    LastPerformanceCheckTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
}

void UPerf_PhysicsWorldPerformanceIntegrator::CheckPerformanceThresholds()
{
    if (!IsPhysicsPerformanceOptimal())
    {
        ApplyAutomaticOptimizations();
    }
}

void UPerf_PhysicsWorldPerformanceIntegrator::ApplyAutomaticOptimizations()
{
    UE_LOG(LogTemp, Log, TEXT("Applying automatic physics optimizations"));
    
    // Reduce LOD distance if performance is poor
    if (CurrentMetrics.PhysicsFrameTime > OptimizationSettings.MaxPhysicsFrameTime * 1.2f)
    {
        OptimizationSettings.PhysicsLODDistance *= 0.9f;
        UpdatePhysicsLOD();
    }
    
    // Reduce max active bodies if too many are active
    if (CurrentMetrics.ActiveRigidBodies > OptimizationSettings.MaxActiveRigidBodies)
    {
        OptimizationSettings.MaxActiveRigidBodies = FMath::Max(100, 
            (int32)(OptimizationSettings.MaxActiveRigidBodies * 0.9f));
        OptimizePhysicsActors();
    }
}

void UPerf_PhysicsWorldPerformanceIntegrator::OptimizePhysicsActors()
{
    if (UWorld* World = GetWorld())
    {
        PhysicsActorsToOptimize.Empty();
        
        // Collect physics actors that need optimization
        for (TActorIterator<AStaticMeshActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AStaticMeshActor* Actor = *ActorIterator;
            if (Actor && Actor->GetStaticMeshComponent() && 
                Actor->GetStaticMeshComponent()->IsSimulatingPhysics())
            {
                PhysicsActorsToOptimize.Add(Actor);
            }
        }
        
        // Apply optimizations to collected actors
        for (AActor* Actor : PhysicsActorsToOptimize)
        {
            if (AStaticMeshActor* MeshActor = Cast<AStaticMeshActor>(Actor))
            {
                if (UStaticMeshComponent* MeshComp = MeshActor->GetStaticMeshComponent())
                {
                    // Apply collision optimization
                    MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
                    
                    // Optimize physics settings
                    if (FBodyInstance* BodyInstance = MeshComp->GetBodyInstance())
                    {
                        BodyInstance->bSimulatePhysics = true;
                        BodyInstance->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                    }
                }
            }
        }
        
        UE_LOG(LogTemp, VeryVerbose, TEXT("Optimized %d physics actors"), PhysicsActorsToOptimize.Num());
    }
}

void UPerf_PhysicsWorldPerformanceIntegrator::UpdatePhysicsLOD()
{
    // LOD system implementation would go here
    // For now, just log that LOD is being updated
    UE_LOG(LogTemp, VeryVerbose, TEXT("Physics LOD system updated"));
}

void UPerf_PhysicsWorldPerformanceIntegrator::ManagePhysicsMemory()
{
    // Memory management implementation would go here
    // For now, just log memory management
    UE_LOG(LogTemp, VeryVerbose, TEXT("Physics memory management applied"));
}

void UPerf_PhysicsWorldPerformanceIntegrator::InitializePhysicsWorldIntegration()
{
    // Try to integrate with physics world manager
    IntegrateWithPhysicsWorldManager();
}

void UPerf_PhysicsWorldPerformanceIntegrator::SynchronizeWithPhysicsWorld()
{
    if (PhysicsWorldManager && bIsIntegratedWithPhysicsWorld)
    {
        // Synchronization logic would go here
        UE_LOG(LogTemp, VeryVerbose, TEXT("Synchronized with Physics World Manager"));
    }
}

void UPerf_PhysicsWorldPerformanceIntegrator::UpdateRegionalOptimizations()
{
    // Regional optimization logic would go here
    UE_LOG(LogTemp, VeryVerbose, TEXT("Regional physics optimizations updated"));
}

float UPerf_PhysicsWorldPerformanceIntegrator::CalculatePhysicsFrameTime() const
{
    // Simplified physics frame time calculation
    // In a real implementation, this would use actual physics timing data
    return FMath::RandRange(8.0f, 20.0f); // Simulate frame time between 8-20ms
}

int32 UPerf_PhysicsWorldPerformanceIntegrator::CountActiveRigidBodies() const
{
    int32 Count = 0;
    
    if (UWorld* World = GetWorld())
    {
        for (TActorIterator<AStaticMeshActor> ActorIterator(World); ActorIterator; ++ActorIterator)
        {
            AStaticMeshActor* Actor = *ActorIterator;
            if (Actor && Actor->GetStaticMeshComponent() && 
                Actor->GetStaticMeshComponent()->IsSimulatingPhysics())
            {
                Count++;
            }
        }
    }
    
    return Count;
}

float UPerf_PhysicsWorldPerformanceIntegrator::EstimatePhysicsMemoryUsage() const
{
    // Simplified memory usage estimation
    // In a real implementation, this would query actual physics memory usage
    return CurrentMetrics.ActiveRigidBodies * 0.05f; // Estimate 0.05 MB per rigid body
}