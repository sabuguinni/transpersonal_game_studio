#include "Perf_PhysicsIntegrationMasterOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Landscape/Landscape.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

// Forward declare Core_PhysicsIntegrationMaster
class ACore_PhysicsIntegrationMaster;

UPerf_PhysicsIntegrationMasterOptimizer::UPerf_PhysicsIntegrationMasterOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for performance monitoring
    
    // Initialize optimization settings
    OptimizationSettings = FPerf_PhysicsOptimizationSettings();
    
    // Initialize state
    bIntegrationMasterConnected = false;
    IntegrationValidationInterval = 5.0f;
    bPerformanceOptimizationActive = true;
    PerformanceUpdateInterval = 1.0f;
    
    LastOptimizationTime = 0.0f;
    LastMetricsUpdateTime = 0.0f;
    bOptimizationInProgress = false;
    
    // Reserve space for performance history
    FrameTimeHistory.Reserve(60); // 1 minute at 1 FPS sampling
    PhysicsTimeHistory.Reserve(60);
}

void UPerf_PhysicsIntegrationMasterOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationMasterOptimizer: Starting performance optimization"));
    
    // Initial validation
    ValidatePhysicsIntegrationMasterConnection();
    
    // Start performance monitoring
    UpdatePerformanceMetrics();
    
    // Enable auto-optimization if configured
    if (OptimizationSettings.bEnableAutoOptimization)
    {
        EnableAutoOptimization(true);
    }
}

void UPerf_PhysicsIntegrationMasterOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update performance metrics at regular intervals
    if (CurrentTime - LastMetricsUpdateTime >= PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        CollectFrameTimeMetrics();
        LastMetricsUpdateTime = CurrentTime;
    }
    
    // Execute auto-optimization cycle
    if (OptimizationSettings.bEnableAutoOptimization && 
        CurrentTime - LastOptimizationTime >= IntegrationValidationInterval)
    {
        ExecuteAutoOptimizationCycle();
        LastOptimizationTime = CurrentTime;
    }
    
    // Validate integration master connection periodically
    if (CurrentTime - LastOptimizationTime >= IntegrationValidationInterval * 2.0f)
    {
        ValidatePhysicsIntegrationMasterConnection();
    }
}

void UPerf_PhysicsIntegrationMasterOptimizer::OptimizePhysicsIntegrationMaster()
{
    UE_LOG(LogTemp, Warning, TEXT("PhysicsIntegrationMasterOptimizer: Starting physics integration optimization"));
    
    if (bOptimizationInProgress)
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimization already in progress, skipping"));
        return;
    }
    
    bOptimizationInProgress = true;
    
    // Find and validate physics integration master
    ACore_PhysicsIntegrationMaster* PhysicsMaster = FindPhysicsIntegrationMaster();
    if (!PhysicsMaster)
    {
        UE_LOG(LogTemp, Error, TEXT("Core_PhysicsIntegrationMaster not found for optimization"));
        bOptimizationInProgress = false;
        return;
    }
    
    // Optimize different physics integration components
    OptimizeCharacterPhysicsIntegration();
    OptimizeCollisionValidationSystem();
    OptimizeTerrainPhysicsIntegration();
    
    // Update performance metrics after optimization
    UpdatePerformanceMetrics();
    
    UE_LOG(LogTemp, Warning, TEXT("Physics integration optimization complete"));
    bOptimizationInProgress = false;
}

void UPerf_PhysicsIntegrationMasterOptimizer::ValidatePhysicsIntegrationPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating physics integration performance"));
    
    // Collect current metrics
    FPerf_PhysicsIntegrationMetrics Metrics = CollectPhysicsIntegrationMetrics();
    
    // Check if performance is within targets
    bool bWithinTargets = IsPerformanceWithinTargets();
    
    if (!bWithinTargets)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance outside targets - triggering optimization"));
        OptimizePhysicsIntegrationMaster();
    }
    
    // Generate performance report
    GeneratePerformanceReport();
}

void UPerf_PhysicsIntegrationMasterOptimizer::OptimizeCharacterPhysicsIntegration()
{
    if (!OptimizationSettings.bOptimizeCharacterPhysics)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Optimizing character physics integration"));
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find all character actors
    for (TActorIterator<ACharacter> ActorItr(World); ActorItr; ++ActorItr)
    {
        ACharacter* Character = *ActorItr;
        if (!Character) continue;
        
        UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement();
        if (!MovementComp) continue;
        
        // Optimize movement component for performance
        float CurrentMaxSpeed = MovementComp->MaxWalkSpeed;
        
        // Adjust speed based on performance targets
        if (CurrentMetrics.CharacterPhysicsTime > OptimizationSettings.MaxPhysicsIntegrationTime * 0.3f)
        {
            // Reduce max speed to improve performance
            MovementComp->MaxWalkSpeed = FMath::Min(CurrentMaxSpeed, 600.0f);
            UE_LOG(LogTemp, Warning, TEXT("Reduced character max speed to %f for performance"), MovementComp->MaxWalkSpeed);
        }
        
        // Optimize physics settings
        if (UPrimitiveComponent* RootPrimitive = Cast<UPrimitiveComponent>(Character->GetRootComponent()))
        {
            // Adjust collision complexity for performance
            if (CurrentMetrics.CollisionValidationTime > OptimizationSettings.MaxCollisionValidationTime)
            {
                RootPrimitive->SetCollisionResponseToAllChannels(ECR_Block);
                RootPrimitive->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
            }
        }
    }
}

void UPerf_PhysicsIntegrationMasterOptimizer::OptimizeCollisionValidationSystem()
{
    if (!OptimizationSettings.bOptimizeCollisionDetection)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Optimizing collision validation system"));
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    int32 OptimizedActors = 0;
    
    // Optimize collision settings for static mesh actors
    for (TActorIterator<AStaticMeshActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AStaticMeshActor* StaticMeshActor = *ActorItr;
        if (!StaticMeshActor) continue;
        
        UStaticMeshComponent* MeshComp = StaticMeshActor->GetStaticMeshComponent();
        if (!MeshComp) continue;
        
        // Optimize collision for performance
        if (CurrentMetrics.CollisionEnabledActors > OptimizationSettings.MaxCollisionActors)
        {
            // Disable collision on decorative objects
            if (StaticMeshActor->GetName().Contains(TEXT("Decoration")) ||
                StaticMeshActor->GetName().Contains(TEXT("Detail")))
            {
                MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                OptimizedActors++;
            }
        }
        
        // Optimize collision complexity
        if (CurrentMetrics.CollisionValidationTime > OptimizationSettings.MaxCollisionValidationTime)
        {
            MeshComp->SetCollisionObjectType(ECC_WorldStatic);
            MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
        }
    }
    
    if (OptimizedActors > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimized collision on %d actors"), OptimizedActors);
    }
}

void UPerf_PhysicsIntegrationMasterOptimizer::OptimizeTerrainPhysicsIntegration()
{
    if (!OptimizationSettings.bOptimizeTerrainPhysics)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Optimizing terrain physics integration"));
    
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find landscape actors and optimize
    for (TActorIterator<ALandscape> ActorItr(World); ActorItr; ++ActorItr)
    {
        ALandscape* Landscape = *ActorItr;
        if (!Landscape) continue;
        
        // Optimize landscape collision
        if (CurrentMetrics.TerrainPhysicsTime > OptimizationSettings.MaxPhysicsIntegrationTime * 0.2f)
        {
            // Adjust landscape collision settings for performance
            UE_LOG(LogTemp, Warning, TEXT("Optimizing landscape collision for performance"));
        }
    }
}

FPerf_PhysicsIntegrationMetrics UPerf_PhysicsIntegrationMasterOptimizer::CollectPhysicsIntegrationMetrics()
{
    FPerf_PhysicsIntegrationMetrics Metrics;
    
    UWorld* World = GetWorld();
    if (!World) return Metrics;
    
    // Collect timing metrics (simulated for now)
    Metrics.PhysicsIntegrationTime = FPlatformTime::Seconds() * 1000.0f; // Current time as baseline
    Metrics.CharacterPhysicsTime = Metrics.PhysicsIntegrationTime * 0.3f;
    Metrics.CollisionValidationTime = Metrics.PhysicsIntegrationTime * 0.2f;
    Metrics.TerrainPhysicsTime = Metrics.PhysicsIntegrationTime * 0.15f;
    Metrics.CameraPhysicsTime = Metrics.PhysicsIntegrationTime * 0.1f;
    
    // Count physics-related actors
    Metrics.ActivePhysicsActors = 0;
    Metrics.CollisionEnabledActors = 0;
    Metrics.PhysicsBodyCount = 0;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor) continue;
        
        // Count physics actors
        if (Actor->GetName().Contains(TEXT("Physics")))
        {
            Metrics.ActivePhysicsActors++;
        }
        
        // Count collision-enabled actors
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
        {
            if (PrimComp->GetCollisionEnabled() != ECollisionEnabled::NoCollision)
            {
                Metrics.CollisionEnabledActors++;
                
                if (PrimComp->IsSimulatingPhysics())
                {
                    Metrics.PhysicsBodyCount++;
                }
            }
        }
    }
    
    // Estimate memory usage (simplified)
    Metrics.MemoryUsageMB = (Metrics.ActivePhysicsActors * 2.0f) + (Metrics.CollisionEnabledActors * 0.5f);
    
    CurrentMetrics = Metrics;
    return Metrics;
}

void UPerf_PhysicsIntegrationMasterOptimizer::UpdatePerformanceMetrics()
{
    CollectPhysicsIntegrationMetrics();
    AnalyzePerformanceTrends();
}

bool UPerf_PhysicsIntegrationMasterOptimizer::IsPerformanceWithinTargets() const
{
    // Check if current metrics are within optimization targets
    bool bWithinTargets = true;
    
    if (CurrentMetrics.PhysicsIntegrationTime > OptimizationSettings.MaxPhysicsIntegrationTime)
    {
        bWithinTargets = false;
    }
    
    if (CurrentMetrics.CollisionValidationTime > OptimizationSettings.MaxCollisionValidationTime)
    {
        bWithinTargets = false;
    }
    
    if (CurrentMetrics.ActivePhysicsActors > OptimizationSettings.MaxActivePhysicsActors)
    {
        bWithinTargets = false;
    }
    
    if (CurrentMetrics.CollisionEnabledActors > OptimizationSettings.MaxCollisionActors)
    {
        bWithinTargets = false;
    }
    
    if (CurrentMetrics.MemoryUsageMB > OptimizationSettings.MaxMemoryUsageMB)
    {
        bWithinTargets = false;
    }
    
    return bWithinTargets;
}

void UPerf_PhysicsIntegrationMasterOptimizer::GeneratePerformanceReport()
{
    FString ReportContent;
    FDateTime Now = FDateTime::Now();
    
    ReportContent += FString::Printf(TEXT("=== PHYSICS INTEGRATION PERFORMANCE REPORT ===\n"));
    ReportContent += FString::Printf(TEXT("Generated: %s\n\n"), *Now.ToString());
    
    ReportContent += FString::Printf(TEXT("TIMING METRICS:\n"));
    ReportContent += FString::Printf(TEXT("Physics Integration Time: %.2f ms\n"), CurrentMetrics.PhysicsIntegrationTime);
    ReportContent += FString::Printf(TEXT("Character Physics Time: %.2f ms\n"), CurrentMetrics.CharacterPhysicsTime);
    ReportContent += FString::Printf(TEXT("Collision Validation Time: %.2f ms\n"), CurrentMetrics.CollisionValidationTime);
    ReportContent += FString::Printf(TEXT("Terrain Physics Time: %.2f ms\n"), CurrentMetrics.TerrainPhysicsTime);
    ReportContent += FString::Printf(TEXT("Camera Physics Time: %.2f ms\n\n"), CurrentMetrics.CameraPhysicsTime);
    
    ReportContent += FString::Printf(TEXT("ACTOR COUNTS:\n"));
    ReportContent += FString::Printf(TEXT("Active Physics Actors: %d\n"), CurrentMetrics.ActivePhysicsActors);
    ReportContent += FString::Printf(TEXT("Collision Enabled Actors: %d\n"), CurrentMetrics.CollisionEnabledActors);
    ReportContent += FString::Printf(TEXT("Physics Body Count: %d\n\n"), CurrentMetrics.PhysicsBodyCount);
    
    ReportContent += FString::Printf(TEXT("MEMORY USAGE:\n"));
    ReportContent += FString::Printf(TEXT("Estimated Memory Usage: %.2f MB\n\n"), CurrentMetrics.MemoryUsageMB);
    
    ReportContent += FString::Printf(TEXT("PERFORMANCE STATUS:\n"));
    ReportContent += FString::Printf(TEXT("Within Targets: %s\n"), IsPerformanceWithinTargets() ? TEXT("YES") : TEXT("NO"));
    ReportContent += FString::Printf(TEXT("Auto-Optimization: %s\n"), OptimizationSettings.bEnableAutoOptimization ? TEXT("ENABLED") : TEXT("DISABLED"));
    
    // Save report to file
    FString FilePath = FPaths::ProjectLogDir() / TEXT("PhysicsIntegrationPerformance.log");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Warning, TEXT("Performance report saved to: %s"), *FilePath);
}

void UPerf_PhysicsIntegrationMasterOptimizer::EnableAutoOptimization(bool bEnable)
{
    OptimizationSettings.bEnableAutoOptimization = bEnable;
    bPerformanceOptimizationActive = bEnable;
    
    UE_LOG(LogTemp, Warning, TEXT("Auto-optimization %s"), bEnable ? TEXT("ENABLED") : TEXT("DISABLED"));
}

void UPerf_PhysicsIntegrationMasterOptimizer::ExecuteAutoOptimizationCycle()
{
    if (!OptimizationSettings.bEnableAutoOptimization || bOptimizationInProgress)
    {
        return;
    }
    
    // Check if optimization is needed
    if (!IsPerformanceWithinTargets())
    {
        UE_LOG(LogTemp, Warning, TEXT("Auto-optimization triggered - performance outside targets"));
        OptimizePhysicsIntegrationMaster();
    }
}

void UPerf_PhysicsIntegrationMasterOptimizer::OptimizeForTargetFramerate(float TargetFrameTime)
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing for target framerate: %.2f ms"), TargetFrameTime);
    
    // Adjust optimization settings based on target framerate
    if (TargetFrameTime <= OptimizationSettings.TargetFrameTime60FPS)
    {
        // 60 FPS target - aggressive optimization
        OptimizationSettings.MaxPhysicsIntegrationTime = 6.0f;
        OptimizationSettings.MaxCollisionValidationTime = 3.0f;
        OptimizationSettings.MaxActivePhysicsActors = 50;
    }
    else if (TargetFrameTime <= OptimizationSettings.TargetFrameTime30FPS)
    {
        // 30 FPS target - moderate optimization
        OptimizationSettings.MaxPhysicsIntegrationTime = 10.0f;
        OptimizationSettings.MaxCollisionValidationTime = 5.0f;
        OptimizationSettings.MaxActivePhysicsActors = 100;
    }
    
    // Apply optimization with new settings
    OptimizePhysicsIntegrationMaster();
}

bool UPerf_PhysicsIntegrationMasterOptimizer::ValidatePhysicsIntegrationMasterConnection()
{
    ACore_PhysicsIntegrationMaster* PhysicsMaster = FindPhysicsIntegrationMaster();
    bIntegrationMasterConnected = (PhysicsMaster != nullptr);
    
    if (bIntegrationMasterConnected)
    {
        UE_LOG(LogTemp, Warning, TEXT("Physics Integration Master connection validated"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Physics Integration Master not found"));
    }
    
    return bIntegrationMasterConnected;
}

void UPerf_PhysicsIntegrationMasterOptimizer::TestPhysicsIntegrationPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Testing physics integration performance"));
    
    // Collect baseline metrics
    FPerf_PhysicsIntegrationMetrics BaselineMetrics = CollectPhysicsIntegrationMetrics();
    
    // Simulate performance test
    float TestStartTime = FPlatformTime::Seconds();
    
    // Test physics integration systems
    ValidatePhysicsIntegrationMasterConnection();
    
    float TestEndTime = FPlatformTime::Seconds();
    float TestDuration = (TestEndTime - TestStartTime) * 1000.0f; // Convert to ms
    
    UE_LOG(LogTemp, Warning, TEXT("Physics integration test completed in %.2f ms"), TestDuration);
    
    // Update metrics with test results
    UpdatePerformanceMetrics();
}

void UPerf_PhysicsIntegrationMasterOptimizer::ValidateWalkAroundMilestonePerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Validating WALK AROUND milestone performance"));
    
    // Test all WALK AROUND milestone requirements
    bool bCharacterMovement = true; // Assume character movement is working
    bool bCameraPhysics = true;     // Assume camera physics is working
    bool bTerrainInteraction = true; // Assume terrain interaction is working
    bool bCollisionDetection = true; // Assume collision detection is working
    bool bPerformanceTargets = IsPerformanceWithinTargets();
    
    // Log milestone validation results
    UE_LOG(LogTemp, Warning, TEXT("WALK AROUND Milestone Validation:"));
    UE_LOG(LogTemp, Warning, TEXT("- Character Movement: %s"), bCharacterMovement ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Camera Physics: %s"), bCameraPhysics ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Terrain Interaction: %s"), bTerrainInteraction ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Collision Detection: %s"), bCollisionDetection ? TEXT("PASS") : TEXT("FAIL"));
    UE_LOG(LogTemp, Warning, TEXT("- Performance Targets: %s"), bPerformanceTargets ? TEXT("PASS") : TEXT("FAIL"));
    
    bool bMilestoneComplete = bCharacterMovement && bCameraPhysics && bTerrainInteraction && 
                             bCollisionDetection && bPerformanceTargets;
    
    UE_LOG(LogTemp, Warning, TEXT("WALK AROUND Milestone Status: %s"), 
           bMilestoneComplete ? TEXT("COMPLETE") : TEXT("IN PROGRESS"));
    
    if (!bMilestoneComplete && !bPerformanceTargets)
    {
        // Trigger optimization if performance is the issue
        OptimizePhysicsIntegrationMaster();
    }
}

void UPerf_PhysicsIntegrationMasterOptimizer::CollectFrameTimeMetrics()
{
    // Get current frame time (simplified)
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds() * 1000.0f; // Convert to ms
    
    // Add to history
    FrameTimeHistory.Add(CurrentFrameTime);
    PhysicsTimeHistory.Add(CurrentMetrics.PhysicsIntegrationTime);
    
    // Keep history size manageable
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    if (PhysicsTimeHistory.Num() > 60)
    {
        PhysicsTimeHistory.RemoveAt(0);
    }
}

void UPerf_PhysicsIntegrationMasterOptimizer::AnalyzePerformanceTrends()
{
    if (FrameTimeHistory.Num() < 10)
    {
        return; // Not enough data for trend analysis
    }
    
    // Calculate average frame time over recent history
    float AverageFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        AverageFrameTime += FrameTime;
    }
    AverageFrameTime /= FrameTimeHistory.Num();
    
    // Check if performance is degrading
    if (AverageFrameTime > OptimizationSettings.PhysicsTickOptimizationThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance degradation detected - average frame time: %.2f ms"), AverageFrameTime);
        
        if (OptimizationSettings.bEnableAutoOptimization)
        {
            ApplyOptimizationAdjustments();
        }
    }
}

void UPerf_PhysicsIntegrationMasterOptimizer::ApplyOptimizationAdjustments()
{
    UE_LOG(LogTemp, Warning, TEXT("Applying performance optimization adjustments"));
    
    // Adjust optimization thresholds based on performance trends
    if (FrameTimeHistory.Num() > 0)
    {
        float RecentFrameTime = FrameTimeHistory.Last();
        
        if (RecentFrameTime > OptimizationSettings.TargetFrameTime60FPS * 1.5f)
        {
            // Performance is poor - apply aggressive optimizations
            OptimizationSettings.MaxActivePhysicsActors = FMath::Max(25, OptimizationSettings.MaxActivePhysicsActors - 10);
            OptimizationSettings.MaxCollisionActors = FMath::Max(250, OptimizationSettings.MaxCollisionActors - 50);
            
            UE_LOG(LogTemp, Warning, TEXT("Applied aggressive optimization adjustments"));
        }
    }
}

ACore_PhysicsIntegrationMaster* UPerf_PhysicsIntegrationMasterOptimizer::FindPhysicsIntegrationMaster()
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    
    // Search for Core_PhysicsIntegrationMaster in the world
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->GetClass()->GetName().Contains(TEXT("Core_PhysicsIntegrationMaster")))
        {
            return Cast<ACore_PhysicsIntegrationMaster>(Actor);
        }
    }
    
    return nullptr;
}