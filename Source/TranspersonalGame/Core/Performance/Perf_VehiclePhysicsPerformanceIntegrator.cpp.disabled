#include "Perf_VehiclePhysicsPerformanceIntegrator.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Pawn.h"

APerf_VehiclePhysicsPerformanceIntegrator::APerf_VehiclePhysicsPerformanceIntegrator()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;

    PerformanceLevel = EPerf_VehiclePerformanceLevel::High;
    PerformanceUpdateInterval = 1.0f;
    bEnableAutomaticOptimization = true;
    TargetFrameRate = 60.0f;
    LastPerformanceUpdate = 0.0f;
    AccumulatedPhysicsTime = 0.0f;
    PhysicsUpdateCount = 0;

    // Initialize optimization settings based on performance level
    OptimizationSettings.MaxActiveVehicles = 20;
    OptimizationSettings.PhysicsUpdateRate = 60.0f;
    OptimizationSettings.bEnableComplexCollision = true;
    OptimizationSettings.CullingDistance = 10000.0f;
    OptimizationSettings.bEnableLODSystem = true;
}

void APerf_VehiclePhysicsPerformanceIntegrator::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Vehicle Physics Performance Integrator initialized"));
    
    // Apply initial performance settings
    ApplyPerformanceLevelSettings();
    
    // Start performance monitoring
    UpdatePerformanceMetrics();
}

void APerf_VehiclePhysicsPerformanceIntegrator::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    LastPerformanceUpdate += DeltaTime;
    AccumulatedPhysicsTime += DeltaTime;
    PhysicsUpdateCount++;
    
    // Update performance metrics at specified intervals
    if (LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        UpdatePerformanceMetrics();
        
        if (bEnableAutomaticOptimization)
        {
            OptimizeVehiclePhysics();
        }
        
        LastPerformanceUpdate = 0.0f;
    }
    
    // Continuous optimizations
    CullDistantVehicles();
    ManageActiveVehicleCount();
}

void APerf_VehiclePhysicsPerformanceIntegrator::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Calculate physics update time
    if (PhysicsUpdateCount > 0)
    {
        CurrentMetrics.PhysicsUpdateTime = AccumulatedPhysicsTime / PhysicsUpdateCount;
        AccumulatedPhysicsTime = 0.0f;
        PhysicsUpdateCount = 0;
    }
    
    // Count active vehicles
    CurrentMetrics.ActiveVehicleCount = TrackedVehicles.Num();
    
    // Calculate average velocity
    CurrentMetrics.AverageVelocity = CalculateAverageVelocity();
    
    // Calculate collision complexity
    CurrentMetrics.CollisionComplexity = CalculateCollisionComplexity();
    
    // Estimate memory usage
    CurrentMetrics.MemoryUsageMB = EstimateMemoryUsage();
    
    UE_LOG(LogTemp, Log, TEXT("Vehicle Physics Metrics - Active: %d, AvgVel: %.2f, PhysTime: %.4f, Memory: %.2f MB"),
        CurrentMetrics.ActiveVehicleCount,
        CurrentMetrics.AverageVelocity,
        CurrentMetrics.PhysicsUpdateTime,
        CurrentMetrics.MemoryUsageMB);
}

void APerf_VehiclePhysicsPerformanceIntegrator::OptimizeVehiclePhysics()
{
    if (!IsPerformanceTargetMet())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance target not met - applying optimizations"));
        
        // Adjust physics update rate
        AdjustPhysicsUpdateRate();
        
        // Optimize physics settings
        OptimizePhysicsSettings();
        
        // Apply LOD optimizations
        if (OptimizationSettings.bEnableLODSystem)
        {
            for (AActor* Vehicle : TrackedVehicles)
            {
                if (Vehicle)
                {
                    float Distance = FVector::Dist(Vehicle->GetActorLocation(), GetActorLocation());
                    bool bUseLOD = Distance > OptimizationSettings.CullingDistance * 0.5f;
                    EnableVehicleLOD(Vehicle, bUseLOD);
                }
            }
        }
    }
}

void APerf_VehiclePhysicsPerformanceIntegrator::SetPerformanceLevel(EPerf_VehiclePerformanceLevel NewLevel)
{
    PerformanceLevel = NewLevel;
    ApplyPerformanceLevelSettings();
    UE_LOG(LogTemp, Log, TEXT("Vehicle physics performance level set to: %d"), (int32)NewLevel);
}

FPerf_VehiclePhysicsMetrics APerf_VehiclePhysicsPerformanceIntegrator::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_VehiclePhysicsPerformanceIntegrator::RegisterVehicle(AActor* Vehicle)
{
    if (Vehicle && !TrackedVehicles.Contains(Vehicle))
    {
        TrackedVehicles.Add(Vehicle);
        UE_LOG(LogTemp, Log, TEXT("Registered vehicle for performance tracking: %s"), *Vehicle->GetName());
        
        // Apply current optimization settings to new vehicle
        ApplyOptimizationSettings();
    }
}

void APerf_VehiclePhysicsPerformanceIntegrator::UnregisterVehicle(AActor* Vehicle)
{
    if (Vehicle)
    {
        TrackedVehicles.Remove(Vehicle);
        UE_LOG(LogTemp, Log, TEXT("Unregistered vehicle from performance tracking: %s"), *Vehicle->GetName());
    }
}

void APerf_VehiclePhysicsPerformanceIntegrator::ApplyOptimizationSettings()
{
    for (AActor* Vehicle : TrackedVehicles)
    {
        if (Vehicle)
        {
            // Apply complex collision settings
            UpdateVehiclePhysicsComplexity(Vehicle, OptimizationSettings.bEnableComplexCollision);
            
            // Apply LOD settings
            if (OptimizationSettings.bEnableLODSystem)
            {
                float Distance = FVector::Dist(Vehicle->GetActorLocation(), GetActorLocation());
                bool bUseLOD = Distance > OptimizationSettings.CullingDistance * 0.5f;
                EnableVehicleLOD(Vehicle, bUseLOD);
            }
        }
    }
}

bool APerf_VehiclePhysicsPerformanceIntegrator::IsPerformanceTargetMet() const
{
    if (GEngine && GEngine->GetGameViewport())
    {
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        return CurrentFPS >= TargetFrameRate * 0.9f; // 90% of target
    }
    return true; // Assume target met if we can't measure
}

void APerf_VehiclePhysicsPerformanceIntegrator::EnableVehicleLOD(AActor* Vehicle, bool bEnable)
{
    if (!Vehicle)
    {
        return;
    }
    
    // Find static mesh components and apply LOD settings
    TArray<UStaticMeshComponent*> MeshComponents;
    Vehicle->GetComponents<UStaticMeshComponent>(MeshComponents);
    
    for (UStaticMeshComponent* MeshComp : MeshComponents)
    {
        if (MeshComp)
        {
            if (bEnable)
            {
                // Enable LOD - reduce detail at distance
                MeshComp->SetForcedLodModel(2); // Use LOD level 2
            }
            else
            {
                // Disable LOD - use full detail
                MeshComp->SetForcedLodModel(0); // Use highest LOD
            }
        }
    }
}

void APerf_VehiclePhysicsPerformanceIntegrator::UpdateVehiclePhysicsComplexity(AActor* Vehicle, bool bUseComplexCollision)
{
    if (!Vehicle)
    {
        return;
    }
    
    // Find primitive components and update collision complexity
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    Vehicle->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp && PrimComp->GetBodyInstance())
        {
            if (bUseComplexCollision)
            {
                PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            }
            else
            {
                // Simplified collision for performance
                PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
                PrimComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
                PrimComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
            }
        }
    }
}

void APerf_VehiclePhysicsPerformanceIntegrator::AnalyzeVehiclePerformance()
{
    // Analyze current performance state and log findings
    UE_LOG(LogTemp, Log, TEXT("Vehicle Performance Analysis:"));
    UE_LOG(LogTemp, Log, TEXT("- Active Vehicles: %d/%d"), CurrentMetrics.ActiveVehicleCount, OptimizationSettings.MaxActiveVehicles);
    UE_LOG(LogTemp, Log, TEXT("- Physics Update Time: %.4f ms"), CurrentMetrics.PhysicsUpdateTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("- Average Velocity: %.2f units/s"), CurrentMetrics.AverageVelocity);
    UE_LOG(LogTemp, Log, TEXT("- Collision Complexity: %.2f"), CurrentMetrics.CollisionComplexity);
    UE_LOG(LogTemp, Log, TEXT("- Memory Usage: %.2f MB"), CurrentMetrics.MemoryUsageMB);
}

void APerf_VehiclePhysicsPerformanceIntegrator::ApplyPerformanceLevelSettings()
{
    switch (PerformanceLevel)
    {
        case EPerf_VehiclePerformanceLevel::Low:
            OptimizationSettings.MaxActiveVehicles = 10;
            OptimizationSettings.PhysicsUpdateRate = 30.0f;
            OptimizationSettings.bEnableComplexCollision = false;
            OptimizationSettings.CullingDistance = 5000.0f;
            TargetFrameRate = 30.0f;
            break;
            
        case EPerf_VehiclePerformanceLevel::Medium:
            OptimizationSettings.MaxActiveVehicles = 15;
            OptimizationSettings.PhysicsUpdateRate = 45.0f;
            OptimizationSettings.bEnableComplexCollision = true;
            OptimizationSettings.CullingDistance = 7500.0f;
            TargetFrameRate = 45.0f;
            break;
            
        case EPerf_VehiclePerformanceLevel::High:
            OptimizationSettings.MaxActiveVehicles = 20;
            OptimizationSettings.PhysicsUpdateRate = 60.0f;
            OptimizationSettings.bEnableComplexCollision = true;
            OptimizationSettings.CullingDistance = 10000.0f;
            TargetFrameRate = 60.0f;
            break;
            
        case EPerf_VehiclePerformanceLevel::Ultra:
            OptimizationSettings.MaxActiveVehicles = 30;
            OptimizationSettings.PhysicsUpdateRate = 120.0f;
            OptimizationSettings.bEnableComplexCollision = true;
            OptimizationSettings.CullingDistance = 15000.0f;
            TargetFrameRate = 60.0f;
            break;
    }
    
    ApplyOptimizationSettings();
}

void APerf_VehiclePhysicsPerformanceIntegrator::CullDistantVehicles()
{
    if (!GetWorld() || !GetWorld()->GetFirstPlayerController())
    {
        return;
    }
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (int32 i = TrackedVehicles.Num() - 1; i >= 0; --i)
    {
        AActor* Vehicle = TrackedVehicles[i];
        if (!Vehicle)
        {
            TrackedVehicles.RemoveAt(i);
            continue;
        }
        
        float Distance = FVector::Dist(Vehicle->GetActorLocation(), PlayerLocation);
        if (Distance > OptimizationSettings.CullingDistance)
        {
            // Disable physics for distant vehicles
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Vehicle->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp)
                {
                    PrimComp->SetSimulatePhysics(false);
                }
            }
        }
        else
        {
            // Re-enable physics for nearby vehicles
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Vehicle->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp)
                {
                    PrimComp->SetSimulatePhysics(true);
                }
            }
        }
    }
}

void APerf_VehiclePhysicsPerformanceIntegrator::OptimizePhysicsSettings()
{
    // Apply global physics optimizations based on current performance
    if (GetWorld())
    {
        UWorld* World = GetWorld();
        if (World->GetPhysicsScene())
        {
            // Adjust physics substep settings for performance
            if (!IsPerformanceTargetMet())
            {
                // Reduce physics accuracy for better performance
                OptimizationSettings.PhysicsUpdateRate = FMath::Max(30.0f, OptimizationSettings.PhysicsUpdateRate * 0.8f);
            }
            else
            {
                // Gradually increase accuracy when performance allows
                OptimizationSettings.PhysicsUpdateRate = FMath::Min(120.0f, OptimizationSettings.PhysicsUpdateRate * 1.1f);
            }
        }
    }
}

float APerf_VehiclePhysicsPerformanceIntegrator::CalculateAverageVelocity() const
{
    if (TrackedVehicles.Num() == 0)
    {
        return 0.0f;
    }
    
    float TotalVelocity = 0.0f;
    int32 ValidVehicles = 0;
    
    for (AActor* Vehicle : TrackedVehicles)
    {
        if (Vehicle)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Vehicle->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    FVector Velocity = PrimComp->GetPhysicsLinearVelocity();
                    TotalVelocity += Velocity.Size();
                    ValidVehicles++;
                    break; // Only count first physics component per vehicle
                }
            }
        }
    }
    
    return ValidVehicles > 0 ? TotalVelocity / ValidVehicles : 0.0f;
}

float APerf_VehiclePhysicsPerformanceIntegrator::CalculateCollisionComplexity() const
{
    float TotalComplexity = 0.0f;
    
    for (AActor* Vehicle : TrackedVehicles)
    {
        if (Vehicle)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Vehicle->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp)
                {
                    // Estimate complexity based on collision responses
                    int32 ActiveChannels = 0;
                    for (int32 i = 0; i < 32; ++i)
                    {
                        if (PrimComp->GetCollisionResponseToChannel((ECollisionChannel)i) != ECR_Ignore)
                        {
                            ActiveChannels++;
                        }
                    }
                    TotalComplexity += ActiveChannels / 32.0f; // Normalize to 0-1 range
                }
            }
        }
    }
    
    return TrackedVehicles.Num() > 0 ? TotalComplexity / TrackedVehicles.Num() : 0.0f;
}

float APerf_VehiclePhysicsPerformanceIntegrator::EstimateMemoryUsage() const
{
    // Rough estimation of memory usage
    float EstimatedMB = TrackedVehicles.Num() * 2.5f; // ~2.5MB per vehicle (rough estimate)
    return EstimatedMB;
}

void APerf_VehiclePhysicsPerformanceIntegrator::AdjustPhysicsUpdateRate()
{
    if (GetWorld())
    {
        // Apply physics update rate adjustments
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        if (CurrentFPS < TargetFrameRate * 0.8f)
        {
            // Reduce physics update rate for better performance
            OptimizationSettings.PhysicsUpdateRate = FMath::Max(20.0f, OptimizationSettings.PhysicsUpdateRate * 0.9f);
        }
        else if (CurrentFPS > TargetFrameRate * 1.1f)
        {
            // Increase physics update rate when performance allows
            OptimizationSettings.PhysicsUpdateRate = FMath::Min(120.0f, OptimizationSettings.PhysicsUpdateRate * 1.05f);
        }
    }
}

void APerf_VehiclePhysicsPerformanceIntegrator::ManageActiveVehicleCount()
{
    // Ensure we don't exceed maximum active vehicle count
    if (TrackedVehicles.Num() > OptimizationSettings.MaxActiveVehicles)
    {
        // Disable physics for excess vehicles (furthest from player)
        if (GetWorld() && GetWorld()->GetFirstPlayerController())
        {
            APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
            if (PlayerPawn)
            {
                FVector PlayerLocation = PlayerPawn->GetActorLocation();
                
                // Sort by distance and disable furthest vehicles
                TrackedVehicles.Sort([PlayerLocation](const AActor& A, const AActor& B)
                {
                    float DistA = FVector::Dist(A.GetActorLocation(), PlayerLocation);
                    float DistB = FVector::Dist(B.GetActorLocation(), PlayerLocation);
                    return DistA < DistB;
                });
                
                // Disable physics for vehicles beyond the limit
                for (int32 i = OptimizationSettings.MaxActiveVehicles; i < TrackedVehicles.Num(); ++i)
                {
                    AActor* Vehicle = TrackedVehicles[i];
                    if (Vehicle)
                    {
                        TArray<UPrimitiveComponent*> PrimitiveComponents;
                        Vehicle->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
                        
                        for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
                        {
                            if (PrimComp)
                            {
                                PrimComp->SetSimulatePhysics(false);
                            }
                        }
                    }
                }
            }
        }
    }
}