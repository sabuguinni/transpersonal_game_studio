#include "Perf_VehiclePerformanceOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/StaticMeshActor.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"

UPerf_VehiclePerformanceOptimizer::UPerf_VehiclePerformanceOptimizer()
{
    PerformanceConfig = FPerf_VehiclePerformanceConfig();
    CurrentMetrics = FPerf_VehicleMetrics();
    bIsMonitoring = false;
    LastUpdateTime = 0.0f;
    UpdateInterval = 0.1f;
    TotalPhysicsTime = 0.0f;
    TotalRenderTime = 0.0f;
    FrameCounter = 0;
}

void UPerf_VehiclePerformanceOptimizer::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Log, TEXT("VehiclePerformanceOptimizer: Initializing vehicle performance optimization system"));
    
    // Initialize default configuration
    PerformanceConfig = FPerf_VehiclePerformanceConfig();
    CurrentMetrics = FPerf_VehicleMetrics();
    
    // Start monitoring by default
    StartPerformanceMonitoring();
}

void UPerf_VehiclePerformanceOptimizer::Deinitialize()
{
    StopPerformanceMonitoring();
    RegisteredVehicles.Empty();
    
    UE_LOG(LogTemp, Log, TEXT("VehiclePerformanceOptimizer: Deinitializing vehicle performance optimization system"));
    
    Super::Deinitialize();
}

void UPerf_VehiclePerformanceOptimizer::OptimizeVehiclePerformance()
{
    if (!GetWorld())
    {
        return;
    }

    UpdateVehicleList();
    
    // Get player location for distance calculations
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    for (auto& VehiclePtr : RegisteredVehicles)
    {
        if (AActor* Vehicle = VehiclePtr.Get())
        {
            float DistanceToPlayer = FVector::Dist(Vehicle->GetActorLocation(), PlayerLocation);
            
            // Update LOD based on distance
            UpdateVehicleLOD(Vehicle, DistanceToPlayer);
            
            // Optimize physics based on proximity
            bool bIsNearPlayer = DistanceToPlayer < PerformanceConfig.HighDetailDistance;
            OptimizeVehiclePhysics(Vehicle, bIsNearPlayer);
            
            // Apply culling if too far
            if (DistanceToPlayer > PerformanceConfig.CullingDistance)
            {
                Vehicle->SetActorHiddenInGame(true);
            }
            else
            {
                Vehicle->SetActorHiddenInGame(false);
            }
        }
    }
    
    // Update performance metrics
    UpdatePerformanceMetrics();
}

void UPerf_VehiclePerformanceOptimizer::UpdateVehicleLOD(AActor* VehicleActor, float DistanceToPlayer)
{
    if (!VehicleActor)
    {
        return;
    }

    int32 LODLevel = 0;
    
    if (DistanceToPlayer > PerformanceConfig.LowDetailDistance)
    {
        LODLevel = 3; // Lowest detail
    }
    else if (DistanceToPlayer > PerformanceConfig.MediumDetailDistance)
    {
        LODLevel = 2; // Medium detail
    }
    else if (DistanceToPlayer > PerformanceConfig.HighDetailDistance)
    {
        LODLevel = 1; // High detail
    }
    else
    {
        LODLevel = 0; // Highest detail
    }
    
    ApplyLODSettings(VehicleActor, LODLevel);
}

void UPerf_VehiclePerformanceOptimizer::OptimizeVehiclePhysics(AActor* VehicleActor, bool bIsNearPlayer)
{
    if (!VehicleActor || !PerformanceConfig.bEnablePhysicsOptimization)
    {
        return;
    }

    // Get all primitive components
    TArray<UPrimitiveComponent*> PrimitiveComponents;
    VehicleActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            if (bIsNearPlayer)
            {
                // High detail physics
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetSimulatePhysics(true);
            }
            else
            {
                // Simplified physics or kinematic
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                PrimComp->SetSimulatePhysics(false);
            }
        }
    }
    
    UpdatePhysicsSettings(VehicleActor, bIsNearPlayer);
}

void UPerf_VehiclePerformanceOptimizer::SetVehiclePerformanceConfig(const FPerf_VehiclePerformanceConfig& NewConfig)
{
    PerformanceConfig = NewConfig;
    
    UE_LOG(LogTemp, Log, TEXT("VehiclePerformanceOptimizer: Updated performance configuration"));
    
    // Apply new settings immediately
    OptimizeVehiclePerformance();
}

void UPerf_VehiclePerformanceOptimizer::StartPerformanceMonitoring()
{
    bIsMonitoring = true;
    LastUpdateTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UE_LOG(LogTemp, Log, TEXT("VehiclePerformanceOptimizer: Started performance monitoring"));
}

void UPerf_VehiclePerformanceOptimizer::StopPerformanceMonitoring()
{
    bIsMonitoring = false;
    
    UE_LOG(LogTemp, Log, TEXT("VehiclePerformanceOptimizer: Stopped performance monitoring"));
}

void UPerf_VehiclePerformanceOptimizer::UpdatePerformanceMetrics()
{
    if (!bIsMonitoring || !GetWorld())
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime < UpdateInterval)
    {
        return;
    }
    
    LastUpdateTime = CurrentTime;
    FrameCounter++;
    
    CalculatePerformanceMetrics();
}

void UPerf_VehiclePerformanceOptimizer::RegisterVehicle(AActor* VehicleActor)
{
    if (VehicleActor && !RegisteredVehicles.Contains(VehicleActor))
    {
        RegisteredVehicles.Add(VehicleActor);
        
        UE_LOG(LogTemp, Log, TEXT("VehiclePerformanceOptimizer: Registered vehicle %s"), *VehicleActor->GetName());
    }
}

void UPerf_VehiclePerformanceOptimizer::UnregisterVehicle(AActor* VehicleActor)
{
    if (VehicleActor)
    {
        RegisteredVehicles.Remove(VehicleActor);
        VehiclePhysicsTimes.Remove(VehicleActor);
        VehicleRenderTimes.Remove(VehicleActor);
        
        UE_LOG(LogTemp, Log, TEXT("VehiclePerformanceOptimizer: Unregistered vehicle %s"), *VehicleActor->GetName());
    }
}

void UPerf_VehiclePerformanceOptimizer::RunVehiclePerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("VehiclePerformanceOptimizer: Running vehicle performance test"));
    
    if (!GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("VehiclePerformanceOptimizer: No world available for performance test"));
        return;
    }

    // Test vehicle optimization
    float StartTime = FPlatformTime::Seconds();
    
    OptimizeVehiclePerformance();
    
    float EndTime = FPlatformTime::Seconds();
    float OptimizationTime = (EndTime - StartTime) * 1000.0f; // Convert to ms
    
    UE_LOG(LogTemp, Warning, TEXT("VehiclePerformanceOptimizer: Performance test completed in %.2f ms"), OptimizationTime);
    UE_LOG(LogTemp, Warning, TEXT("VehiclePerformanceOptimizer: Registered vehicles: %d"), RegisteredVehicles.Num());
    UE_LOG(LogTemp, Warning, TEXT("VehiclePerformanceOptimizer: Active vehicles: %d"), CurrentMetrics.ActiveVehicles);
    UE_LOG(LogTemp, Warning, TEXT("VehiclePerformanceOptimizer: Physics-enabled vehicles: %d"), CurrentMetrics.PhysicsEnabledVehicles);
}

void UPerf_VehiclePerformanceOptimizer::GeneratePerformanceReport()
{
    FString ReportContent;
    ReportContent += TEXT("=== VEHICLE PERFORMANCE REPORT ===\n");
    ReportContent += FString::Printf(TEXT("Generated: %s\n"), *FDateTime::Now().ToString());
    ReportContent += TEXT("\n");
    
    ReportContent += TEXT("CONFIGURATION:\n");
    ReportContent += FString::Printf(TEXT("High Detail Distance: %.1f\n"), PerformanceConfig.HighDetailDistance);
    ReportContent += FString::Printf(TEXT("Medium Detail Distance: %.1f\n"), PerformanceConfig.MediumDetailDistance);
    ReportContent += FString::Printf(TEXT("Low Detail Distance: %.1f\n"), PerformanceConfig.LowDetailDistance);
    ReportContent += FString::Printf(TEXT("Physics Optimization: %s\n"), PerformanceConfig.bEnablePhysicsOptimization ? TEXT("Enabled") : TEXT("Disabled"));
    ReportContent += FString::Printf(TEXT("Physics Update Rate: %.1f Hz\n"), PerformanceConfig.PhysicsUpdateRate);
    ReportContent += TEXT("\n");
    
    ReportContent += TEXT("CURRENT METRICS:\n");
    ReportContent += FString::Printf(TEXT("Total Vehicles: %d\n"), CurrentMetrics.TotalVehicles);
    ReportContent += FString::Printf(TEXT("Active Vehicles: %d\n"), CurrentMetrics.ActiveVehicles);
    ReportContent += FString::Printf(TEXT("Physics-Enabled Vehicles: %d\n"), CurrentMetrics.PhysicsEnabledVehicles);
    ReportContent += FString::Printf(TEXT("Average Physics Time: %.2f ms\n"), CurrentMetrics.AveragePhysicsTime);
    ReportContent += FString::Printf(TEXT("Average Render Time: %.2f ms\n"), CurrentMetrics.AverageRenderTime);
    ReportContent += FString::Printf(TEXT("Memory Usage: %.2f MB\n"), CurrentMetrics.MemoryUsageMB);
    ReportContent += FString::Printf(TEXT("Frame Impact: %.2f%%\n"), CurrentMetrics.FrameImpactPercent);
    
    // Save report to file
    FString FilePath = FPaths::ProjectLogDir() / TEXT("VehiclePerformanceReport.txt");
    FFileHelper::SaveStringToFile(ReportContent, *FilePath);
    
    UE_LOG(LogTemp, Warning, TEXT("VehiclePerformanceOptimizer: Performance report saved to %s"), *FilePath);
}

void UPerf_VehiclePerformanceOptimizer::UpdateVehicleList()
{
    // Remove invalid references
    RegisteredVehicles.RemoveAll([](const TWeakObjectPtr<AActor>& VehiclePtr)
    {
        return !VehiclePtr.IsValid();
    });
    
    CurrentMetrics.TotalVehicles = RegisteredVehicles.Num();
}

void UPerf_VehiclePerformanceOptimizer::CalculatePerformanceMetrics()
{
    CurrentMetrics.ActiveVehicles = 0;
    CurrentMetrics.PhysicsEnabledVehicles = 0;
    TotalPhysicsTime = 0.0f;
    TotalRenderTime = 0.0f;
    
    for (auto& VehiclePtr : RegisteredVehicles)
    {
        if (AActor* Vehicle = VehiclePtr.Get())
        {
            if (!Vehicle->IsHidden())
            {
                CurrentMetrics.ActiveVehicles++;
            }
            
            // Check for physics-enabled components
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Vehicle->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            bool bHasPhysics = false;
            for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
            {
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    bHasPhysics = true;
                    break;
                }
            }
            
            if (bHasPhysics)
            {
                CurrentMetrics.PhysicsEnabledVehicles++;
            }
        }
    }
    
    // Calculate averages
    if (CurrentMetrics.ActiveVehicles > 0)
    {
        CurrentMetrics.AveragePhysicsTime = TotalPhysicsTime / CurrentMetrics.ActiveVehicles;
        CurrentMetrics.AverageRenderTime = TotalRenderTime / CurrentMetrics.ActiveVehicles;
    }
    
    // Estimate memory usage (rough calculation)
    CurrentMetrics.MemoryUsageMB = CurrentMetrics.TotalVehicles * 2.5f; // Estimate 2.5MB per vehicle
    
    // Calculate frame impact
    float TotalFrameTime = CurrentMetrics.AveragePhysicsTime + CurrentMetrics.AverageRenderTime;
    float TargetFrameTime = 1000.0f / 60.0f; // 60 FPS target
    CurrentMetrics.FrameImpactPercent = (TotalFrameTime / TargetFrameTime) * 100.0f;
}

float UPerf_VehiclePerformanceOptimizer::CalculateDistanceToPlayer(AActor* VehicleActor)
{
    if (!VehicleActor || !GetWorld())
    {
        return 0.0f;
    }

    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
    if (!PlayerPawn)
    {
        return 0.0f;
    }
    
    return FVector::Dist(VehicleActor->GetActorLocation(), PlayerPawn->GetActorLocation());
}

void UPerf_VehiclePerformanceOptimizer::ApplyLODSettings(AActor* VehicleActor, int32 LODLevel)
{
    if (!VehicleActor)
    {
        return;
    }

    // Apply LOD to static mesh components
    TArray<UStaticMeshComponent*> StaticMeshComponents;
    VehicleActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
    
    for (UStaticMeshComponent* MeshComp : StaticMeshComponents)
    {
        if (MeshComp)
        {
            MeshComp->SetForcedLodModel(LODLevel + 1); // LOD levels are 1-based in UE5
        }
    }
    
    // Apply LOD to skeletal mesh components
    TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
    VehicleActor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
    
    for (USkeletalMeshComponent* SkelMeshComp : SkeletalMeshComponents)
    {
        if (SkelMeshComp)
        {
            SkelMeshComp->SetForcedLOD(LODLevel + 1);
        }
    }
}

void UPerf_VehiclePerformanceOptimizer::OptimizeVehicleComponents(AActor* VehicleActor)
{
    if (!VehicleActor)
    {
        return;
    }

    // Optimize component settings based on distance and performance requirements
    TArray<UActorComponent*> AllComponents = VehicleActor->GetRootComponent()->GetAttachChildren();
    
    for (UActorComponent* Component : AllComponents)
    {
        if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Component))
        {
            // Adjust collision complexity
            if (CalculateDistanceToPlayer(VehicleActor) > PerformanceConfig.MediumDetailDistance)
            {
                PrimComp->SetCollisionResponseToAllChannels(ECR_Ignore);
            }
            else
            {
                PrimComp->SetCollisionResponseToAllChannels(ECR_Block);
            }
        }
    }
}

void UPerf_VehiclePerformanceOptimizer::UpdatePhysicsSettings(AActor* VehicleActor, bool bHighDetail)
{
    if (!VehicleActor)
    {
        return;
    }

    TArray<UPrimitiveComponent*> PrimitiveComponents;
    VehicleActor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
    
    for (UPrimitiveComponent* PrimComp : PrimitiveComponents)
    {
        if (PrimComp)
        {
            if (bHighDetail)
            {
                // High detail physics settings
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                PrimComp->SetNotifyRigidBodyCollision(true);
            }
            else
            {
                // Low detail physics settings
                PrimComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                PrimComp->SetNotifyRigidBodyCollision(false);
            }
        }
    }
}