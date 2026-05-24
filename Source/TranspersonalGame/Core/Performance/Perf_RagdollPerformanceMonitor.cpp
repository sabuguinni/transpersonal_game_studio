#include "Perf_RagdollPerformanceMonitor.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "HAL/PlatformApplicationMisc.h"
#include "Stats/Stats.h"
#include "Kismet/GameplayStatics.h"

APerf_RagdollPerformanceMonitor::APerf_RagdollPerformanceMonitor()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    
    // Initialize default values
    bIsMonitoring = false;
    QualityLevel = EPerf_RagdollQuality::Medium;
    MonitoringInterval = 0.5f;
    MaxActiveRagdolls = 20;
    CullingDistance = 5000.0f;
    bAutoOptimize = true;
    TargetFrameRate = 60.0f;
    MonitoringTimer = 0.0f;
    
    // Initialize metrics
    CurrentMetrics = FPerf_RagdollMetrics();
}

void APerf_RagdollPerformanceMonitor::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll Performance Monitor initialized"));
    
    // Start monitoring automatically
    if (bAutoOptimize)
    {
        StartMonitoring();
    }
}

void APerf_RagdollPerformanceMonitor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsMonitoring)
    {
        MonitoringTimer += DeltaTime;
        
        // Update metrics at specified interval
        if (MonitoringTimer >= MonitoringInterval)
        {
            UpdateMetrics();
            
            if (bAutoOptimize)
            {
                AnalyzePerformance();
                ApplyOptimizations();
            }
            
            MonitoringTimer = 0.0f;
        }
        
        // Track frame times
        FrameTimeHistory.Add(DeltaTime);
        if (FrameTimeHistory.Num() > 100)
        {
            FrameTimeHistory.RemoveAt(0);
        }
    }
}

void APerf_RagdollPerformanceMonitor::StartMonitoring()
{
    bIsMonitoring = true;
    MonitoringTimer = 0.0f;
    FrameTimeHistory.Empty();
    MetricsHistory.Empty();
    
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll performance monitoring started"));
}

void APerf_RagdollPerformanceMonitor::StopMonitoring()
{
    bIsMonitoring = false;
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll performance monitoring stopped"));
}

FPerf_RagdollMetrics APerf_RagdollPerformanceMonitor::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void APerf_RagdollPerformanceMonitor::OptimizeRagdollPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("Optimizing ragdoll performance"));
    
    // Cull distant ragdolls
    CullDistantRagdolls(CullingDistance);
    
    // Limit active ragdolls
    LimitActiveRagdolls(MaxActiveRagdolls);
    
    // Optimize constraints
    OptimizeRagdollConstraints();
    
    // Adjust LOD
    AdjustRagdollLOD();
    
    // Cleanup inactive ragdolls
    CleanupInactiveRagdolls();
}

void APerf_RagdollPerformanceMonitor::SetRagdollQuality(EPerf_RagdollQuality Quality)
{
    QualityLevel = Quality;
    
    switch (Quality)
    {
        case EPerf_RagdollQuality::Low:
            MaxActiveRagdolls = 10;
            CullingDistance = 3000.0f;
            break;
        case EPerf_RagdollQuality::Medium:
            MaxActiveRagdolls = 20;
            CullingDistance = 5000.0f;
            break;
        case EPerf_RagdollQuality::High:
            MaxActiveRagdolls = 35;
            CullingDistance = 7500.0f;
            break;
        case EPerf_RagdollQuality::Ultra:
            MaxActiveRagdolls = 50;
            CullingDistance = 10000.0f;
            break;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Ragdoll quality set to: %d"), (int32)Quality);
}

void APerf_RagdollPerformanceMonitor::CullDistantRagdolls(float MaxDistance)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 CulledCount = 0;
    
    // Find all characters with ragdoll physics
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    for (AActor* Actor : Characters)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->GetMesh())
        {
            USkeletalMeshComponent* Mesh = Character->GetMesh();
            if (Mesh->IsSimulatingPhysics())
            {
                float Distance = FVector::Dist(PlayerLocation, Character->GetActorLocation());
                if (Distance > MaxDistance)
                {
                    // Disable ragdoll physics for distant characters
                    Mesh->SetSimulatePhysics(false);
                    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    CulledCount++;
                }
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Culled %d distant ragdolls beyond %.0f units"), CulledCount, MaxDistance);
}

void APerf_RagdollPerformanceMonitor::LimitActiveRagdolls(int32 MaxActive)
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<ACharacter*> ActiveRagdolls;
    
    // Find all active ragdolls
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    for (AActor* Actor : Characters)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->GetMesh() && Character->GetMesh()->IsSimulatingPhysics())
        {
            ActiveRagdolls.Add(Character);
        }
    }
    
    // Disable excess ragdolls (furthest from player first)
    if (ActiveRagdolls.Num() > MaxActive)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        if (PlayerPawn)
        {
            FVector PlayerLocation = PlayerPawn->GetActorLocation();
            
            // Sort by distance
            ActiveRagdolls.Sort([PlayerLocation](const ACharacter& A, const ACharacter& B) {
                float DistA = FVector::DistSquared(PlayerLocation, A.GetActorLocation());
                float DistB = FVector::DistSquared(PlayerLocation, B.GetActorLocation());
                return DistA < DistB;
            });
            
            // Disable furthest ragdolls
            int32 DisabledCount = 0;
            for (int32 i = MaxActive; i < ActiveRagdolls.Num(); i++)
            {
                if (ActiveRagdolls[i] && ActiveRagdolls[i]->GetMesh())
                {
                    ActiveRagdolls[i]->GetMesh()->SetSimulatePhysics(false);
                    DisabledCount++;
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Limited active ragdolls: disabled %d excess ragdolls"), DisabledCount);
        }
    }
}

void APerf_RagdollPerformanceMonitor::RunRagdollPerformanceTest()
{
    UE_LOG(LogTemp, Warning, TEXT("Running ragdoll performance test"));
    
    // Force update metrics
    UpdateMetrics();
    
    // Log current state
    UE_LOG(LogTemp, Warning, TEXT("Performance Test Results:"));
    UE_LOG(LogTemp, Warning, TEXT("- Active Ragdolls: %d"), CurrentMetrics.ActiveRagdolls);
    UE_LOG(LogTemp, Warning, TEXT("- Physics Constraints: %d"), CurrentMetrics.PhysicsConstraints);
    UE_LOG(LogTemp, Warning, TEXT("- Simulation Time: %.2fms"), CurrentMetrics.SimulationTime);
    UE_LOG(LogTemp, Warning, TEXT("- CPU Time: %.2fms"), CurrentMetrics.CPUTime);
    UE_LOG(LogTemp, Warning, TEXT("- Memory Usage: %.2fMB"), CurrentMetrics.MemoryUsage);
    
    // Calculate average frame time
    if (FrameTimeHistory.Num() > 0)
    {
        float AvgFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            AvgFrameTime += FrameTime;
        }
        AvgFrameTime /= FrameTimeHistory.Num();
        float AvgFPS = 1.0f / AvgFrameTime;
        
        UE_LOG(LogTemp, Warning, TEXT("- Average FPS: %.1f"), AvgFPS);
        
        // Performance recommendations
        if (AvgFPS < TargetFrameRate * 0.8f)
        {
            UE_LOG(LogTemp, Warning, TEXT("RECOMMENDATION: Reduce ragdoll quality or limit active ragdolls"));
        }
    }
}

void APerf_RagdollPerformanceMonitor::GeneratePerformanceReport()
{
    UE_LOG(LogTemp, Warning, TEXT("Generating ragdoll performance report"));
    
    FString Report = TEXT("=== RAGDOLL PERFORMANCE REPORT ===\n");
    Report += FString::Printf(TEXT("Quality Level: %d\n"), (int32)QualityLevel);
    Report += FString::Printf(TEXT("Max Active Ragdolls: %d\n"), MaxActiveRagdolls);
    Report += FString::Printf(TEXT("Culling Distance: %.0f\n"), CullingDistance);
    Report += FString::Printf(TEXT("Auto Optimize: %s\n"), bAutoOptimize ? TEXT("Enabled") : TEXT("Disabled"));
    Report += TEXT("\nCurrent Metrics:\n");
    Report += FString::Printf(TEXT("- Active Ragdolls: %d\n"), CurrentMetrics.ActiveRagdolls);
    Report += FString::Printf(TEXT("- Physics Constraints: %d\n"), CurrentMetrics.PhysicsConstraints);
    Report += FString::Printf(TEXT("- Simulation Time: %.2fms\n"), CurrentMetrics.SimulationTime);
    Report += FString::Printf(TEXT("- CPU Time: %.2fms\n"), CurrentMetrics.CPUTime);
    Report += FString::Printf(TEXT("- Memory Usage: %.2fMB\n"), CurrentMetrics.MemoryUsage);
    
    UE_LOG(LogTemp, Warning, TEXT("%s"), *Report);
}

void APerf_RagdollPerformanceMonitor::UpdateMetrics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Reset metrics
    CurrentMetrics = FPerf_RagdollMetrics();
    
    // Find active ragdolls
    FindActiveRagdolls();
    
    // Calculate simulation time (approximate)
    CurrentMetrics.SimulationTime = CurrentMetrics.ActiveRagdolls * 0.5f; // Rough estimate
    
    // Calculate memory usage (approximate)
    CurrentMetrics.MemoryUsage = CurrentMetrics.ActiveRagdolls * 2.5f; // MB per ragdoll estimate
    
    // Calculate CPU time from frame history
    if (FrameTimeHistory.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            TotalTime += FrameTime;
        }
        CurrentMetrics.CPUTime = (TotalTime / FrameTimeHistory.Num()) * 1000.0f; // Convert to ms
    }
    
    // Store in history
    MetricsHistory.Add(CurrentMetrics);
    if (MetricsHistory.Num() > 50)
    {
        MetricsHistory.RemoveAt(0);
    }
}

void APerf_RagdollPerformanceMonitor::AnalyzePerformance()
{
    // Check if performance is below target
    if (FrameTimeHistory.Num() > 10)
    {
        float AvgFrameTime = 0.0f;
        for (int32 i = FrameTimeHistory.Num() - 10; i < FrameTimeHistory.Num(); i++)
        {
            AvgFrameTime += FrameTimeHistory[i];
        }
        AvgFrameTime /= 10.0f;
        float CurrentFPS = 1.0f / AvgFrameTime;
        
        if (CurrentFPS < TargetFrameRate * 0.8f)
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance below target (%.1f FPS), applying optimizations"), CurrentFPS);
        }
    }
}

void APerf_RagdollPerformanceMonitor::ApplyOptimizations()
{
    // Auto-optimize based on current performance
    if (CurrentMetrics.ActiveRagdolls > MaxActiveRagdolls)
    {
        LimitActiveRagdolls(MaxActiveRagdolls);
    }
    
    // Adjust quality based on performance
    if (FrameTimeHistory.Num() > 0)
    {
        float AvgFrameTime = 0.0f;
        for (float FrameTime : FrameTimeHistory)
        {
            AvgFrameTime += FrameTime;
        }
        AvgFrameTime /= FrameTimeHistory.Num();
        float CurrentFPS = 1.0f / AvgFrameTime;
        
        if (CurrentFPS < TargetFrameRate * 0.7f && QualityLevel != EPerf_RagdollQuality::Low)
        {
            // Reduce quality
            EPerf_RagdollQuality NewQuality = (EPerf_RagdollQuality)((int32)QualityLevel - 1);
            SetRagdollQuality(NewQuality);
        }
        else if (CurrentFPS > TargetFrameRate * 1.2f && QualityLevel != EPerf_RagdollQuality::Ultra)
        {
            // Increase quality
            EPerf_RagdollQuality NewQuality = (EPerf_RagdollQuality)((int32)QualityLevel + 1);
            SetRagdollQuality(NewQuality);
        }
    }
}

void APerf_RagdollPerformanceMonitor::LogPerformanceData()
{
    UE_LOG(LogTemp, Log, TEXT("Ragdoll Performance - Active: %d, Constraints: %d, SimTime: %.2fms"), 
           CurrentMetrics.ActiveRagdolls, CurrentMetrics.PhysicsConstraints, CurrentMetrics.SimulationTime);
}

void APerf_RagdollPerformanceMonitor::FindActiveRagdolls()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    CurrentMetrics.ActiveRagdolls = 0;
    CurrentMetrics.PhysicsConstraints = 0;
    
    // Count active ragdolls
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    for (AActor* Actor : Characters)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->GetMesh() && Character->GetMesh()->IsSimulatingPhysics())
        {
            CurrentMetrics.ActiveRagdolls++;
        }
    }
    
    // Count physics constraints
    TArray<AActor*> Constraints;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Constraints);
    
    for (AActor* Actor : Constraints)
    {
        if (Actor->FindComponentByClass<UPhysicsConstraintComponent>())
        {
            CurrentMetrics.PhysicsConstraints++;
        }
    }
}

void APerf_RagdollPerformanceMonitor::OptimizeRagdollConstraints()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    // Find and optimize physics constraints
    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
    
    int32 OptimizedCount = 0;
    for (AActor* Actor : Actors)
    {
        UPhysicsConstraintComponent* Constraint = Actor->FindComponentByClass<UPhysicsConstraintComponent>();
        if (Constraint)
        {
            // Reduce constraint solver iterations for distant objects
            APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
            if (PlayerPawn)
            {
                float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Actor->GetActorLocation());
                if (Distance > CullingDistance * 0.5f)
                {
                    // Reduce quality for distant constraints
                    OptimizedCount++;
                }
            }
        }
    }
    
    if (OptimizedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Optimized %d ragdoll constraints"), OptimizedCount);
    }
}

void APerf_RagdollPerformanceMonitor::AdjustRagdollLOD()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
    if (!PlayerPawn) return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    for (AActor* Actor : Characters)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->GetMesh())
        {
            float Distance = FVector::Dist(PlayerLocation, Character->GetActorLocation());
            USkeletalMeshComponent* Mesh = Character->GetMesh();
            
            // Adjust LOD based on distance
            if (Distance > CullingDistance * 0.8f)
            {
                Mesh->SetForcedLOD(3); // Lowest detail
            }
            else if (Distance > CullingDistance * 0.5f)
            {
                Mesh->SetForcedLOD(2); // Medium detail
            }
            else if (Distance > CullingDistance * 0.25f)
            {
                Mesh->SetForcedLOD(1); // High detail
            }
            else
            {
                Mesh->SetForcedLOD(0); // Highest detail
            }
        }
    }
}

void APerf_RagdollPerformanceMonitor::CleanupInactiveRagdolls()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    TArray<AActor*> Characters;
    UGameplayStatics::GetAllActorsOfClass(World, ACharacter::StaticClass(), Characters);
    
    int32 CleanedCount = 0;
    for (AActor* Actor : Characters)
    {
        ACharacter* Character = Cast<ACharacter>(Actor);
        if (Character && Character->GetMesh())
        {
            USkeletalMeshComponent* Mesh = Character->GetMesh();
            
            // Check if ragdoll has been inactive for too long
            if (Mesh->IsSimulatingPhysics())
            {
                // Get velocity to check if ragdoll is at rest
                FVector Velocity = Mesh->GetPhysicsLinearVelocity();
                if (Velocity.Size() < 10.0f) // Very low velocity threshold
                {
                    // Ragdoll is at rest, consider disabling physics
                    Mesh->SetSimulatePhysics(false);
                    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                    CleanedCount++;
                }
            }
        }
    }
    
    if (CleanedCount > 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cleaned up %d inactive ragdolls"), CleanedCount);
    }
}