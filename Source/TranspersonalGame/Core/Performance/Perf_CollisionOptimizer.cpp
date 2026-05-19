#include "Perf_CollisionOptimizer.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

UPerf_CollisionOptimizer::UPerf_CollisionOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Default performance settings
    CollisionBudgetMS = 2.0f; // 2ms budget for collision per frame
    MaxTracesPerFrame = 50;
    AdaptiveLODThreshold = 1.5f; // Switch LOD if frame time exceeds 1.5ms
    bEnableAdaptiveLOD = true;
    
    // Initialize default profile
    ActiveProfileName = TEXT("Default");
    
    // Initialize tracking variables
    CurrentFrameTraces = 0;
    LastFrameTime = 0.0f;
    CollisionBudgetUsed = 0.0f;
    
    InitializeDefaultProfiles();
}

void UPerf_CollisionOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(60); // Store last 60 frames (1 second at 60fps)
    
    UE_LOG(LogTemp, Warning, TEXT("Collision Optimizer initialized with budget: %.2fms"), CollisionBudgetMS);
}

void UPerf_CollisionOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateFrameStats(DeltaTime);
    UpdatePerformanceStats();
    
    if (bEnableAdaptiveLOD)
    {
        AdaptLODBasedOnPerformance();
    }
    
    // Reset frame counters
    CurrentFrameTraces = 0;
    CollisionBudgetUsed = 0.0f;
}

void UPerf_CollisionOptimizer::InitializeDefaultProfiles()
{
    CollisionProfiles.Empty();
    
    // High performance profile
    FPerf_CollisionProfile HighPerf;
    HighPerf.ProfileName = TEXT("HighPerformance");
    HighPerf.MaxTraceDistance = 5000.0f;
    HighPerf.MaxSimultaneousTraces = 25;
    HighPerf.LODDistance = 2500.0f;
    HighPerf.CollisionLOD = EPerf_CollisionLOD::Simplified;
    CollisionProfiles.Add(HighPerf);
    
    // Balanced profile
    FPerf_CollisionProfile Balanced;
    Balanced.ProfileName = TEXT("Balanced");
    Balanced.MaxTraceDistance = 10000.0f;
    Balanced.MaxSimultaneousTraces = 50;
    Balanced.LODDistance = 5000.0f;
    Balanced.CollisionLOD = EPerf_CollisionLOD::Full;
    CollisionProfiles.Add(Balanced);
    
    // High quality profile
    FPerf_CollisionProfile HighQuality;
    HighQuality.ProfileName = TEXT("HighQuality");
    HighQuality.MaxTraceDistance = 20000.0f;
    HighQuality.MaxSimultaneousTraces = 100;
    HighQuality.LODDistance = 10000.0f;
    HighQuality.CollisionLOD = EPerf_CollisionLOD::Full;
    CollisionProfiles.Add(HighQuality);
    
    // Default profile
    FPerf_CollisionProfile Default;
    Default.ProfileName = TEXT("Default");
    CollisionProfiles.Add(Default);
}

void UPerf_CollisionOptimizer::SetCollisionProfile(const FString& ProfileName)
{
    FPerf_CollisionProfile* Profile = FindProfile(ProfileName);
    if (Profile)
    {
        ActiveProfileName = ProfileName;
        UE_LOG(LogTemp, Warning, TEXT("Collision profile changed to: %s"), *ProfileName);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Collision profile not found: %s"), *ProfileName);
    }
}

FPerf_CollisionProfile UPerf_CollisionOptimizer::GetActiveProfile() const
{
    const FPerf_CollisionProfile* Profile = const_cast<UPerf_CollisionOptimizer*>(this)->FindProfile(ActiveProfileName);
    return Profile ? *Profile : FPerf_CollisionProfile();
}

void UPerf_CollisionOptimizer::OptimizeCollisionForActor(AActor* Actor, float Distance)
{
    if (!Actor)
    {
        return;
    }
    
    FPerf_CollisionProfile ActiveProfile = GetActiveProfile();
    EPerf_CollisionLOD OptimalLOD = CalculateOptimalLOD(Distance, LastFrameTime);
    
    // Get all collision components
    TArray<UPrimitiveComponent*> CollisionComponents;
    Actor->GetComponents<UPrimitiveComponent>(CollisionComponents);
    
    for (UPrimitiveComponent* Component : CollisionComponents)
    {
        if (!Component)
        {
            continue;
        }
        
        switch (OptimalLOD)
        {
            case EPerf_CollisionLOD::Full:
                Component->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
                Component->SetCollisionResponseToAllChannels(ECR_Block);
                break;
                
            case EPerf_CollisionLOD::Simplified:
                Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                Component->SetCollisionResponseToAllChannels(ECR_Block);
                // Use simple collision shapes
                if (UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(Component))
                {
                    StaticMesh->SetCollisionObjectType(ECC_WorldStatic);
                }
                break;
                
            case EPerf_CollisionLOD::Bounds:
                Component->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
                Component->SetCollisionResponseToAllChannels(ECR_Ignore);
                Component->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
                break;
                
            case EPerf_CollisionLOD::Disabled:
                Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
                break;
        }
    }
}

bool UPerf_CollisionOptimizer::ShouldPerformTrace(const FVector& Start, const FVector& End)
{
    // Check frame budget
    if (CollisionBudgetUsed >= CollisionBudgetMS)
    {
        return false;
    }
    
    // Check trace count limit
    if (CurrentFrameTraces >= MaxTracesPerFrame)
    {
        return false;
    }
    
    // Check distance against active profile
    FPerf_CollisionProfile ActiveProfile = GetActiveProfile();
    float TraceDistance = FVector::Dist(Start, End);
    if (TraceDistance > ActiveProfile.MaxTraceDistance)
    {
        return false;
    }
    
    return true;
}

void UPerf_CollisionOptimizer::RegisterTrace(float TraceTime)
{
    TraceTimings.Add(TraceTime);
    CurrentFrameTraces++;
    CollisionBudgetUsed += TraceTime;
    
    // Keep only recent timings
    if (TraceTimings.Num() > 100)
    {
        TraceTimings.RemoveAt(0);
    }
}

void UPerf_CollisionOptimizer::UpdatePerformanceStats()
{
    // Calculate average trace time
    if (TraceTimings.Num() > 0)
    {
        float TotalTime = 0.0f;
        for (float Time : TraceTimings)
        {
            TotalTime += Time;
        }
        CurrentStats.AverageTraceTime = TotalTime / TraceTimings.Num();
    }
    
    CurrentStats.ActiveTraces = CurrentFrameTraces;
    CurrentStats.TracesPerFrame = CurrentFrameTraces;
    CurrentStats.CollisionBudgetUsed = (CollisionBudgetUsed / CollisionBudgetMS) * 100.0f;
}

void UPerf_CollisionOptimizer::AdaptLODBasedOnPerformance()
{
    if (FrameTimeHistory.Num() < 10)
    {
        return; // Need enough data
    }
    
    // Calculate average frame time for last 10 frames
    float AverageFrameTime = 0.0f;
    for (int32 i = FMath::Max(0, FrameTimeHistory.Num() - 10); i < FrameTimeHistory.Num(); i++)
    {
        AverageFrameTime += FrameTimeHistory[i];
    }
    AverageFrameTime /= 10.0f;
    
    // Adapt profile based on performance
    if (AverageFrameTime > AdaptiveLODThreshold)
    {
        // Performance is poor, switch to high performance profile
        if (ActiveProfileName != TEXT("HighPerformance"))
        {
            SetCollisionProfile(TEXT("HighPerformance"));
            UE_LOG(LogTemp, Warning, TEXT("Adaptive LOD: Switching to HighPerformance profile (frame time: %.2fms)"), AverageFrameTime);
        }
    }
    else if (AverageFrameTime < AdaptiveLODThreshold * 0.5f)
    {
        // Performance is good, can use higher quality
        if (ActiveProfileName == TEXT("HighPerformance"))
        {
            SetCollisionProfile(TEXT("Balanced"));
            UE_LOG(LogTemp, Warning, TEXT("Adaptive LOD: Switching to Balanced profile (frame time: %.2fms)"), AverageFrameTime);
        }
    }
}

void UPerf_CollisionOptimizer::DrawCollisionDebugInfo()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    FVector ActorLocation = GetOwner()->GetActorLocation();
    FPerf_CollisionProfile ActiveProfile = GetActiveProfile();
    
    // Draw LOD distance sphere
    DrawDebugSphere(World, ActorLocation, ActiveProfile.LODDistance, 32, FColor::Yellow, false, 0.1f);
    
    // Draw max trace distance sphere
    DrawDebugSphere(World, ActorLocation, ActiveProfile.MaxTraceDistance, 32, FColor::Red, false, 0.1f);
    
    // Draw performance info
    FString DebugText = FString::Printf(
        TEXT("Collision Optimizer\nProfile: %s\nTraces/Frame: %d/%d\nBudget Used: %.1f%%\nAvg Trace Time: %.3fms"),
        *ActiveProfileName,
        CurrentStats.TracesPerFrame,
        MaxTracesPerFrame,
        CurrentStats.CollisionBudgetUsed,
        CurrentStats.AverageTraceTime
    );
    
    DrawDebugString(World, ActorLocation + FVector(0, 0, 200), DebugText, nullptr, FColor::White, 0.1f);
}

FString UPerf_CollisionOptimizer::GetPerformanceReport() const
{
    FString Report = FString::Printf(
        TEXT("=== Collision Performance Report ===\n")
        TEXT("Active Profile: %s\n")
        TEXT("Traces Per Frame: %d / %d\n")
        TEXT("Average Trace Time: %.3fms\n")
        TEXT("Budget Used: %.1f%% (%.2f / %.2fms)\n")
        TEXT("Adaptive LOD: %s\n")
        TEXT("Frame Time History: %d samples\n"),
        *ActiveProfileName,
        CurrentStats.TracesPerFrame,
        MaxTracesPerFrame,
        CurrentStats.AverageTraceTime,
        CurrentStats.CollisionBudgetUsed,
        CollisionBudgetUsed,
        CollisionBudgetMS,
        bEnableAdaptiveLOD ? TEXT("Enabled") : TEXT("Disabled"),
        FrameTimeHistory.Num()
    );
    
    return Report;
}

void UPerf_CollisionOptimizer::RunCollisionBenchmark()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot run benchmark: No world context"));
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Starting collision benchmark..."));
    
    FVector StartLocation = GetOwner()->GetActorLocation();
    TArray<float> BenchmarkTimes;
    
    // Perform 100 traces in different directions
    for (int32 i = 0; i < 100; i++)
    {
        float Angle = (i / 100.0f) * 2.0f * PI;
        FVector Direction = FVector(FMath::Cos(Angle), FMath::Sin(Angle), -1.0f).GetSafeNormal();
        FVector EndLocation = StartLocation + Direction * 1000.0f;
        
        float StartTime = FPlatformTime::Seconds();
        
        FHitResult HitResult;
        World->LineTraceSingleByChannel(
            HitResult,
            StartLocation,
            EndLocation,
            ECC_WorldStatic
        );
        
        float EndTime = FPlatformTime::Seconds();
        float TraceTime = (EndTime - StartTime) * 1000.0f; // Convert to ms
        BenchmarkTimes.Add(TraceTime);
    }
    
    // Calculate statistics
    float TotalTime = 0.0f;
    float MinTime = BenchmarkTimes[0];
    float MaxTime = BenchmarkTimes[0];
    
    for (float Time : BenchmarkTimes)
    {
        TotalTime += Time;
        MinTime = FMath::Min(MinTime, Time);
        MaxTime = FMath::Max(MaxTime, Time);
    }
    
    float AverageTime = TotalTime / BenchmarkTimes.Num();
    
    UE_LOG(LogTemp, Warning, TEXT("Collision Benchmark Results:"));
    UE_LOG(LogTemp, Warning, TEXT("- Traces performed: %d"), BenchmarkTimes.Num());
    UE_LOG(LogTemp, Warning, TEXT("- Average time: %.3fms"), AverageTime);
    UE_LOG(LogTemp, Warning, TEXT("- Min time: %.3fms"), MinTime);
    UE_LOG(LogTemp, Warning, TEXT("- Max time: %.3fms"), MaxTime);
    UE_LOG(LogTemp, Warning, TEXT("- Total time: %.3fms"), TotalTime);
}

void UPerf_CollisionOptimizer::UpdateFrameStats(float DeltaTime)
{
    LastFrameTime = DeltaTime * 1000.0f; // Convert to ms
    FrameTimeHistory.Add(LastFrameTime);
    
    // Keep only last 60 frames
    if (FrameTimeHistory.Num() > 60)
    {
        FrameTimeHistory.RemoveAt(0);
    }
}

FPerf_CollisionProfile* UPerf_CollisionOptimizer::FindProfile(const FString& ProfileName)
{
    for (FPerf_CollisionProfile& Profile : CollisionProfiles)
    {
        if (Profile.ProfileName == ProfileName)
        {
            return &Profile;
        }
    }
    return nullptr;
}

EPerf_CollisionLOD UPerf_CollisionOptimizer::CalculateOptimalLOD(float Distance, float CurrentFrameTime)
{
    FPerf_CollisionProfile ActiveProfile = GetActiveProfile();
    
    // If performance is poor, use aggressive LOD
    if (CurrentFrameTime > AdaptiveLODThreshold)
    {
        if (Distance > ActiveProfile.LODDistance * 2.0f)
        {
            return EPerf_CollisionLOD::Disabled;
        }
        else if (Distance > ActiveProfile.LODDistance)
        {
            return EPerf_CollisionLOD::Bounds;
        }
        else
        {
            return EPerf_CollisionLOD::Simplified;
        }
    }
    
    // Normal LOD calculation based on distance
    if (Distance > ActiveProfile.LODDistance * 1.5f)
    {
        return EPerf_CollisionLOD::Bounds;
    }
    else if (Distance > ActiveProfile.LODDistance)
    {
        return EPerf_CollisionLOD::Simplified;
    }
    else
    {
        return ActiveProfile.CollisionLOD;
    }
}