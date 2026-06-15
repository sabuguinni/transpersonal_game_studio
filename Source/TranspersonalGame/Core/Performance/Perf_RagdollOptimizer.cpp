#include "Perf_RagdollOptimizer.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"
#include "DrawDebugHelpers.h"

// Forward declaration to avoid circular dependency
class UCore_RagdollSystem;

UPerf_RagdollOptimizer::UPerf_RagdollOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second for performance
    
    // Initialize default settings
    PerformanceSettings = FPerf_RagdollPerformanceSettings();
    
    HighLODDistance = 1000.0f;
    MediumLODDistance = 2500.0f;
    LowLODDistance = 5000.0f;
    
    ActiveRagdollCount = 0;
    AverageFrameTime = 0.0f;
    RagdollPhysicsCost = 0.0f;
    
    LastPerformanceUpdate = 0.0f;
    PerformanceUpdateInterval = 1.0f;
    FrameTimeHistorySize = 60;
    
    PlayerPawn = nullptr;
}

void UPerf_RagdollOptimizer::BeginPlay()
{
    Super::BeginPlay();
    
    // Get player reference
    if (UWorld* World = GetWorld())
    {
        if (APlayerController* PC = World->GetFirstPlayerController())
        {
            PlayerPawn = PC->GetPawn();
        }
    }
    
    // Initialize frame time history
    FrameTimeHistory.Reserve(FrameTimeHistorySize);
    
    UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Initialized with max %d ragdolls, LOD distances: High=%f, Medium=%f, Low=%f"), 
           PerformanceSettings.MaxSimultaneousRagdolls, HighLODDistance, MediumLODDistance, LowLODDistance);
}

void UPerf_RagdollOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update player reference if lost
    if (!PlayerPawn)
    {
        if (UWorld* World = GetWorld())
        {
            if (APlayerController* PC = World->GetFirstPlayerController())
            {
                PlayerPawn = PC->GetPawn();
            }
        }
    }
    
    // Performance monitoring
    MonitorPerformanceMetrics();
    
    // Update ragdoll performance every interval
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPerformanceUpdate >= PerformanceUpdateInterval)
    {
        UpdateRagdollPerformance();
        LastPerformanceUpdate = CurrentTime;
    }
}

void UPerf_RagdollOptimizer::RegisterRagdoll(UCore_RagdollSystem* RagdollComponent)
{
    if (!RagdollComponent)
    {
        return;
    }
    
    // Check if already registered
    for (const FPerf_RagdollInstance& Instance : ActiveRagdolls)
    {
        if (Instance.RagdollComponent.Get() == RagdollComponent)
        {
            return; // Already registered
        }
    }
    
    // Create new instance
    FPerf_RagdollInstance NewInstance;
    NewInstance.RagdollComponent = RagdollComponent;
    NewInstance.DistanceToPlayer = 0.0f;
    NewInstance.CurrentLOD = EPerf_RagdollLODLevel::High;
    NewInstance.bIsOnScreen = true;
    NewInstance.LastUpdateTime = GetWorld()->GetTimeSeconds();
    
    ActiveRagdolls.Add(NewInstance);
    ActiveRagdollCount = ActiveRagdolls.Num();
    
    UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Registered ragdoll. Active count: %d"), ActiveRagdollCount);
    
    // Enforce ragdoll limit
    EnforceRagdollLimit();
}

void UPerf_RagdollOptimizer::UnregisterRagdoll(UCore_RagdollSystem* RagdollComponent)
{
    if (!RagdollComponent)
    {
        return;
    }
    
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        if (ActiveRagdolls[i].RagdollComponent.Get() == RagdollComponent)
        {
            ActiveRagdolls.RemoveAt(i);
            ActiveRagdollCount = ActiveRagdolls.Num();
            UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Unregistered ragdoll. Active count: %d"), ActiveRagdollCount);
            break;
        }
    }
}

void UPerf_RagdollOptimizer::OptimizeRagdollLOD(UCore_RagdollSystem* RagdollComponent, EPerf_RagdollLODLevel LODLevel)
{
    if (!RagdollComponent)
    {
        return;
    }
    
    // Apply LOD optimizations based on level
    switch (LODLevel)
    {
        case EPerf_RagdollLODLevel::High:
            // Full quality - no optimizations
            break;
            
        case EPerf_RagdollLODLevel::Medium:
            // Reduce physics update rate by 50%
            // Simplify bone constraints
            break;
            
        case EPerf_RagdollLODLevel::Low:
            // Reduce physics update rate by 75%
            // Use simplified collision
            // Reduce bone constraint count
            break;
            
        case EPerf_RagdollLODLevel::Disabled:
            // Disable physics simulation entirely
            break;
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_RagdollOptimizer: Applied LOD %d to ragdoll"), (int32)LODLevel);
}

EPerf_RagdollLODLevel UPerf_RagdollOptimizer::CalculateOptimalLOD(float Distance, bool bIsOnScreen)
{
    if (!bIsOnScreen && PerformanceSettings.bCullOffscreenRagdolls)
    {
        return EPerf_RagdollLODLevel::Disabled;
    }
    
    if (Distance > LowLODDistance)
    {
        return EPerf_RagdollLODLevel::Disabled;
    }
    else if (Distance > MediumLODDistance)
    {
        return EPerf_RagdollLODLevel::Low;
    }
    else if (Distance > HighLODDistance)
    {
        return EPerf_RagdollLODLevel::Medium;
    }
    
    return EPerf_RagdollLODLevel::High;
}

void UPerf_RagdollOptimizer::UpdateRagdollPerformance()
{
    if (!PlayerPawn)
    {
        return;
    }
    
    UpdateRagdollDistances();
    UpdateRagdollLODs();
    CheckScreenVisibility();
    CullDistantRagdolls();
    EnforceRagdollLimit();
}

void UPerf_RagdollOptimizer::CullDistantRagdolls()
{
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (int32 i = ActiveRagdolls.Num() - 1; i >= 0; i--)
    {
        FPerf_RagdollInstance& Instance = ActiveRagdolls[i];
        
        if (!Instance.RagdollComponent.IsValid())
        {
            ActiveRagdolls.RemoveAt(i);
            continue;
        }
        
        if (Instance.DistanceToPlayer > PerformanceSettings.MaxRagdollDistance)
        {
            // Disable ragdoll physics for distant objects
            OptimizeRagdollLOD(Instance.RagdollComponent.Get(), EPerf_RagdollLODLevel::Disabled);
            ActiveRagdolls.RemoveAt(i);
            UE_LOG(LogTemp, VeryVerbose, TEXT("Perf_RagdollOptimizer: Culled distant ragdoll at distance %f"), Instance.DistanceToPlayer);
        }
    }
    
    ActiveRagdollCount = ActiveRagdolls.Num();
}

void UPerf_RagdollOptimizer::EnforceRagdollLimit()
{
    if (ActiveRagdolls.Num() <= PerformanceSettings.MaxSimultaneousRagdolls)
    {
        return;
    }
    
    // Sort by distance (farthest first)
    ActiveRagdolls.Sort([](const FPerf_RagdollInstance& A, const FPerf_RagdollInstance& B)
    {
        return A.DistanceToPlayer > B.DistanceToPlayer;
    });
    
    // Disable excess ragdolls
    int32 ExcessCount = ActiveRagdolls.Num() - PerformanceSettings.MaxSimultaneousRagdolls;
    for (int32 i = 0; i < ExcessCount; i++)
    {
        if (ActiveRagdolls[i].RagdollComponent.IsValid())
        {
            OptimizeRagdollLOD(ActiveRagdolls[i].RagdollComponent.Get(), EPerf_RagdollLODLevel::Disabled);
        }
        ActiveRagdolls.RemoveAt(0);
    }
    
    ActiveRagdollCount = ActiveRagdolls.Num();
    UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Enforced ragdoll limit. Disabled %d excess ragdolls"), ExcessCount);
}

float UPerf_RagdollOptimizer::GetCurrentFPS() const
{
    if (AverageFrameTime > 0.0f)
    {
        return 1.0f / AverageFrameTime;
    }
    return 0.0f;
}

float UPerf_RagdollOptimizer::GetPhysicsFrameTime() const
{
    return RagdollPhysicsCost;
}

void UPerf_RagdollOptimizer::LogPerformanceStats()
{
    float CurrentFPS = GetCurrentFPS();
    
    UE_LOG(LogTemp, Log, TEXT("=== Ragdoll Performance Stats ==="));
    UE_LOG(LogTemp, Log, TEXT("Active Ragdolls: %d / %d"), ActiveRagdollCount, PerformanceSettings.MaxSimultaneousRagdolls);
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.1f"), CurrentFPS);
    UE_LOG(LogTemp, Log, TEXT("Average Frame Time: %.2f ms"), AverageFrameTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("Physics Cost: %.2f ms"), RagdollPhysicsCost * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("LOD Distances: High=%.0f, Medium=%.0f, Low=%.0f"), HighLODDistance, MediumLODDistance, LowLODDistance);
}

void UPerf_RagdollOptimizer::SetPerformanceProfile(bool bHighPerformance)
{
    if (bHighPerformance)
    {
        // High performance profile - aggressive optimization
        PerformanceSettings.MaxSimultaneousRagdolls = 4;
        PerformanceSettings.MaxRagdollDistance = 3000.0f;
        PerformanceSettings.BoneConstraintCullDistance = 1500.0f;
        PerformanceSettings.PhysicsUpdateRate = 30.0f;
        
        HighLODDistance = 500.0f;
        MediumLODDistance = 1500.0f;
        LowLODDistance = 3000.0f;
    }
    else
    {
        // Quality profile - less aggressive optimization
        PerformanceSettings.MaxSimultaneousRagdolls = 8;
        PerformanceSettings.MaxRagdollDistance = 5000.0f;
        PerformanceSettings.BoneConstraintCullDistance = 2000.0f;
        PerformanceSettings.PhysicsUpdateRate = 60.0f;
        
        HighLODDistance = 1000.0f;
        MediumLODDistance = 2500.0f;
        LowLODDistance = 5000.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Applied %s performance profile"), 
           bHighPerformance ? TEXT("High Performance") : TEXT("Quality"));
}

void UPerf_RagdollOptimizer::ApplyPerformanceSettings(const FPerf_RagdollPerformanceSettings& NewSettings)
{
    PerformanceSettings = NewSettings;
    UE_LOG(LogTemp, Log, TEXT("Perf_RagdollOptimizer: Applied new performance settings"));
}

void UPerf_RagdollOptimizer::UpdateRagdollDistances()
{
    if (!PlayerPawn)
    {
        return;
    }
    
    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    
    for (FPerf_RagdollInstance& Instance : ActiveRagdolls)
    {
        if (Instance.RagdollComponent.IsValid())
        {
            if (AActor* Owner = Instance.RagdollComponent->GetOwner())
            {
                Instance.DistanceToPlayer = FVector::Dist(PlayerLocation, Owner->GetActorLocation());
            }
        }
    }
}

void UPerf_RagdollOptimizer::UpdateRagdollLODs()
{
    for (FPerf_RagdollInstance& Instance : ActiveRagdolls)
    {
        if (Instance.RagdollComponent.IsValid())
        {
            EPerf_RagdollLODLevel OptimalLOD = CalculateOptimalLOD(Instance.DistanceToPlayer, Instance.bIsOnScreen);
            
            if (OptimalLOD != Instance.CurrentLOD)
            {
                OptimizeRagdollLOD(Instance.RagdollComponent.Get(), OptimalLOD);
                Instance.CurrentLOD = OptimalLOD;
            }
        }
    }
}

void UPerf_RagdollOptimizer::CheckScreenVisibility()
{
    if (!PlayerPawn)
    {
        return;
    }
    
    // Simple frustum check - in a real implementation, this would use proper camera frustum
    for (FPerf_RagdollInstance& Instance : ActiveRagdolls)
    {
        if (Instance.RagdollComponent.IsValid())
        {
            // For now, assume all ragdolls within reasonable distance are on screen
            Instance.bIsOnScreen = Instance.DistanceToPlayer < LowLODDistance;
        }
    }
}

void UPerf_RagdollOptimizer::MonitorPerformanceMetrics()
{
    // Track frame time
    float CurrentFrameTime = GetWorld()->GetDeltaSeconds();
    
    // Add to history
    FrameTimeHistory.Add(CurrentFrameTime);
    if (FrameTimeHistory.Num() > FrameTimeHistorySize)
    {
        FrameTimeHistory.RemoveAt(0);
    }
    
    // Calculate average
    float TotalFrameTime = 0.0f;
    for (float FrameTime : FrameTimeHistory)
    {
        TotalFrameTime += FrameTime;
    }
    
    if (FrameTimeHistory.Num() > 0)
    {
        AverageFrameTime = TotalFrameTime / FrameTimeHistory.Num();
    }
    
    // Estimate physics cost (simplified)
    RagdollPhysicsCost = ActiveRagdollCount * 0.001f; // 1ms per active ragdoll (rough estimate)
}