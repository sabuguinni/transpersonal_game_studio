#include "Perf_RagdollPerformanceOptimizer.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "../Core_RagdollSystem.h"

UPerf_RagdollPerformanceOptimizer::UPerf_RagdollPerformanceOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default settings
    LODSettings = FPerf_RagdollLODSettings();
    CurrentMetrics = FPerf_RagdollPerformanceMetrics();
    
    bEnableRagdollOptimization = true;
    bEnableDistanceCulling = true;
    bEnableLODSystem = true;
    PerformanceUpdateInterval = 0.1f;
    
    LastPerformanceUpdate = 0.0f;
    AccumulatedFrameTime = 0.0f;
    FrameTimesamples = 0;
}

void UPerf_RagdollPerformanceOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll Performance Optimizer initialized"));
    
    // Find all ragdoll actors in the world
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> FoundActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), FoundActors);
        
        for (AActor* Actor : FoundActors)
        {
            if (Actor && Actor->FindComponentByClass<UCore_RagdollSystem>())
            {
                TrackedRagdolls.Add(Actor);
                UE_LOG(LogTemp, Log, TEXT("Found ragdoll actor: %s"), *Actor->GetName());
            }
        }
        
        UE_LOG(LogTemp, Log, TEXT("Tracking %d ragdoll actors"), TrackedRagdolls.Num());
    }
}

void UPerf_RagdollPerformanceOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnableRagdollOptimization)
        return;
    
    // Accumulate frame time for performance analysis
    AccumulatedFrameTime += DeltaTime * 1000.0f; // Convert to milliseconds
    FrameTimesamples++;
    
    LastPerformanceUpdate += DeltaTime;
    if (LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        OptimizeRagdollPerformance();
        LastPerformanceUpdate = 0.0f;
    }
}

void UPerf_RagdollPerformanceOptimizer::OptimizeRagdollPerformance()
{
    if (!GetWorld())
        return;
    
    // Update performance metrics
    AnalyzeRagdollPerformance();
    
    // Apply optimizations based on current performance
    if (bEnableLODSystem)
    {
        UpdateRagdollLOD();
    }
    
    if (bEnableDistanceCulling)
    {
        CullDistantRagdolls();
    }
    
    // Batch updates if performance is poor
    if (CurrentMetrics.RagdollFrameTime > WARNING_FRAME_TIME)
    {
        BatchRagdollUpdates();
    }
    
    // Emergency culling if performance is critical
    if (CurrentMetrics.RagdollFrameTime > CRITICAL_FRAME_TIME)
    {
        // Aggressively cull ragdolls to maintain performance
        int32 CullCount = FMath::Max(1, CurrentMetrics.ActiveRagdolls / 4);
        for (int32 i = 0; i < CullCount && i < TrackedRagdolls.Num(); i++)
        {
            if (TrackedRagdolls[i].IsValid())
            {
                CullRagdoll(TrackedRagdolls[i].Get());
            }
        }
        
        UE_LOG(LogTemp, Warning, TEXT("Emergency ragdoll culling: removed %d ragdolls"), CullCount);
    }
}

void UPerf_RagdollPerformanceOptimizer::UpdateRagdollLOD()
{
    if (!GetWorld())
        return;
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    CurrentMetrics.HighDetailRagdolls = 0;
    CurrentMetrics.LowDetailRagdolls = 0;
    float TotalDistance = 0.0f;
    int32 ValidRagdolls = 0;
    
    for (TWeakObjectPtr<AActor>& RagdollPtr : TrackedRagdolls)
    {
        if (!RagdollPtr.IsValid())
            continue;
        
        AActor* RagdollActor = RagdollPtr.Get();
        float Distance = FVector::Dist(PlayerLocation, RagdollActor->GetActorLocation());
        
        TotalDistance += Distance;
        ValidRagdolls++;
        
        ApplyRagdollLOD(RagdollActor, Distance);
        
        // Count LOD levels
        if (Distance <= LODSettings.HighDetailDistance)
        {
            CurrentMetrics.HighDetailRagdolls++;
        }
        else if (Distance <= LODSettings.LowDetailDistance)
        {
            CurrentMetrics.LowDetailRagdolls++;
        }
    }
    
    // Update average distance
    if (ValidRagdolls > 0)
    {
        CurrentMetrics.AverageRagdollDistance = TotalDistance / ValidRagdolls;
    }
}

void UPerf_RagdollPerformanceOptimizer::ApplyRagdollLOD(AActor* RagdollActor, float Distance)
{
    if (!RagdollActor)
        return;
    
    UCore_RagdollSystem* RagdollComp = RagdollActor->FindComponentByClass<UCore_RagdollSystem>();
    if (!RagdollComp)
        return;
    
    // Apply different LOD levels based on distance
    if (Distance <= LODSettings.HighDetailDistance)
    {
        // High detail - full ragdoll simulation
        RagdollComp->SetComponentTickInterval(1.0f / LODSettings.UpdateFrequencyHigh);
    }
    else if (Distance <= LODSettings.MediumDetailDistance)
    {
        // Medium detail - reduced update frequency
        RagdollComp->SetComponentTickInterval(1.0f / LODSettings.UpdateFrequencyMedium);
    }
    else if (Distance <= LODSettings.LowDetailDistance)
    {
        // Low detail - minimal updates
        RagdollComp->SetComponentTickInterval(1.0f / LODSettings.UpdateFrequencyLow);
    }
    else if (Distance > LODSettings.CullDistance)
    {
        // Too far - consider for culling
        CullRagdoll(RagdollActor);
    }
}

void UPerf_RagdollPerformanceOptimizer::CullDistantRagdolls()
{
    if (!GetWorld())
        return;
    
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
        return;
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    int32 CulledThisFrame = 0;
    
    for (int32 i = TrackedRagdolls.Num() - 1; i >= 0; i--)
    {
        if (!TrackedRagdolls[i].IsValid())
        {
            TrackedRagdolls.RemoveAt(i);
            continue;
        }
        
        AActor* RagdollActor = TrackedRagdolls[i].Get();
        float Distance = FVector::Dist(PlayerLocation, RagdollActor->GetActorLocation());
        
        if (Distance > LODSettings.CullDistance || CurrentMetrics.ActiveRagdolls > LODSettings.MaxActiveRagdolls)
        {
            CullRagdoll(RagdollActor);
            CulledThisFrame++;
        }
    }
    
    CurrentMetrics.CulledRagdolls += CulledThisFrame;
}

void UPerf_RagdollPerformanceOptimizer::CullRagdoll(AActor* RagdollActor)
{
    if (!RagdollActor)
        return;
    
    UCore_RagdollSystem* RagdollComp = RagdollActor->FindComponentByClass<UCore_RagdollSystem>();
    if (RagdollComp)
    {
        // Disable ragdoll simulation
        RagdollComp->SetComponentTickEnabled(false);
        
        // Add to culled list for potential restoration
        CulledRagdolls.AddUnique(RagdollActor);
        
        // Remove from active tracking
        TrackedRagdolls.Remove(RagdollActor);
        
        UE_LOG(LogTemp, Verbose, TEXT("Culled ragdoll actor: %s"), *RagdollActor->GetName());
    }
}

void UPerf_RagdollPerformanceOptimizer::RestoreRagdoll(AActor* RagdollActor)
{
    if (!RagdollActor)
        return;
    
    UCore_RagdollSystem* RagdollComp = RagdollActor->FindComponentByClass<UCore_RagdollSystem>();
    if (RagdollComp)
    {
        // Re-enable ragdoll simulation
        RagdollComp->SetComponentTickEnabled(true);
        
        // Add back to active tracking
        TrackedRagdolls.AddUnique(RagdollActor);
        
        // Remove from culled list
        CulledRagdolls.Remove(RagdollActor);
        
        UE_LOG(LogTemp, Verbose, TEXT("Restored ragdoll actor: %s"), *RagdollActor->GetName());
    }
}

void UPerf_RagdollPerformanceOptimizer::BatchRagdollUpdates()
{
    // Implement batched updates to reduce per-frame overhead
    const int32 MaxUpdatesPerFrame = 5;
    int32 UpdatesThisFrame = 0;
    
    for (TWeakObjectPtr<AActor>& RagdollPtr : TrackedRagdolls)
    {
        if (UpdatesThisFrame >= MaxUpdatesPerFrame)
            break;
        
        if (!RagdollPtr.IsValid())
            continue;
        
        AActor* RagdollActor = RagdollPtr.Get();
        UCore_RagdollSystem* RagdollComp = RagdollActor->FindComponentByClass<UCore_RagdollSystem>();
        
        if (RagdollComp && RagdollComp->IsComponentTickEnabled())
        {
            // Force an update for this ragdoll
            RagdollComp->TickComponent(GetWorld()->GetDeltaSeconds(), LEVELTICK_All, nullptr);
            UpdatesThisFrame++;
        }
    }
}

void UPerf_RagdollPerformanceOptimizer::AnalyzeRagdollPerformance()
{
    // Update active ragdoll count
    CurrentMetrics.ActiveRagdolls = 0;
    for (const TWeakObjectPtr<AActor>& RagdollPtr : TrackedRagdolls)
    {
        if (RagdollPtr.IsValid())
        {
            CurrentMetrics.ActiveRagdolls++;
        }
    }
    
    // Calculate average frame time
    if (FrameTimesamples > 0)
    {
        CurrentMetrics.RagdollFrameTime = AccumulatedFrameTime / FrameTimesamples;
        
        // Reset for next measurement period
        AccumulatedFrameTime = 0.0f;
        FrameTimesamples = 0;
    }
}

void UPerf_RagdollPerformanceOptimizer::LogRagdollPerformanceStats()
{
    UE_LOG(LogTemp, Log, TEXT("=== Ragdoll Performance Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Ragdolls: %d"), CurrentMetrics.ActiveRagdolls);
    UE_LOG(LogTemp, Log, TEXT("Culled Ragdolls: %d"), CurrentMetrics.CulledRagdolls);
    UE_LOG(LogTemp, Log, TEXT("High Detail: %d, Low Detail: %d"), CurrentMetrics.HighDetailRagdolls, CurrentMetrics.LowDetailRagdolls);
    UE_LOG(LogTemp, Log, TEXT("Average Distance: %.2f"), CurrentMetrics.AverageRagdollDistance);
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.RagdollFrameTime);
    UE_LOG(LogTemp, Log, TEXT("Performance Status: %s"), 
           CurrentMetrics.RagdollFrameTime < TARGET_FRAME_TIME ? TEXT("GOOD") :
           CurrentMetrics.RagdollFrameTime < WARNING_FRAME_TIME ? TEXT("WARNING") : TEXT("CRITICAL"));
}

void UPerf_RagdollPerformanceOptimizer::ResetPerformanceMetrics()
{
    CurrentMetrics = FPerf_RagdollPerformanceMetrics();
    AccumulatedFrameTime = 0.0f;
    FrameTimesamples = 0;
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll performance metrics reset"));
}

void UPerf_RagdollPerformanceOptimizer::TestRagdollOptimization()
{
    UE_LOG(LogTemp, Log, TEXT("Testing ragdoll optimization system..."));
    
    // Force a performance analysis
    AnalyzeRagdollPerformance();
    
    // Test LOD system
    UpdateRagdollLOD();
    
    // Test culling system
    CullDistantRagdolls();
    
    // Log results
    LogRagdollPerformanceStats();
    
    UE_LOG(LogTemp, Log, TEXT("Ragdoll optimization test complete"));
}