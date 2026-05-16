#include "Perf_DestructionOptimizer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UPerf_DestructionOptimizer::UPerf_DestructionOptimizer()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10 FPS for optimization checks

    // Initialize LOD settings
    HighQualityLOD.DistanceThreshold = 2000.0f;
    HighQualityLOD.MaxDebrisCount = 50;
    HighQualityLOD.DebrisLifetime = 60.0f;
    HighQualityLOD.bEnablePhysicsSimulation = true;

    MediumQualityLOD.DistanceThreshold = 5000.0f;
    MediumQualityLOD.MaxDebrisCount = 25;
    MediumQualityLOD.DebrisLifetime = 30.0f;
    MediumQualityLOD.bEnablePhysicsSimulation = true;

    LowQualityLOD.DistanceThreshold = 10000.0f;
    LowQualityLOD.MaxDebrisCount = 10;
    LowQualityLOD.DebrisLifetime = 15.0f;
    LowQualityLOD.bEnablePhysicsSimulation = false;

    CurrentLODLevel = 1; // Start with medium quality
    TargetFrameTime = 16.67f; // 60 FPS
    MaxDestructionBudget = 5.0f; // 5ms per frame
    MaxSimultaneousDestructions = 3;
}

void UPerf_DestructionOptimizer::BeginPlay()
{
    Super::BeginPlay();

    // Start optimization timers
    GetWorld()->GetTimerManager().SetTimer(
        OptimizationTimerHandle,
        this,
        &UPerf_DestructionOptimizer::OptimizeDestructionSystem,
        1.0f,
        true
    );

    GetWorld()->GetTimerManager().SetTimer(
        CleanupTimerHandle,
        this,
        &UPerf_DestructionOptimizer::CleanupOldDebris,
        5.0f,
        true
    );

    GetWorld()->GetTimerManager().SetTimer(
        MetricsTimerHandle,
        this,
        &UPerf_DestructionOptimizer::UpdatePerformanceMetrics,
        0.5f,
        true
    );

    UE_LOG(LogTemp, Log, TEXT("Destruction Performance Optimizer initialized"));
}

void UPerf_DestructionOptimizer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Track frame time for performance monitoring
    LastFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
    AccumulatedFrameTime += LastFrameTime;
    FrameCounter++;

    // Update LOD based on player distance
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (PlayerPawn)
    {
        UpdateLODBasedOnDistance(PlayerPawn->GetActorLocation());
    }

    // Batch update debris if needed
    if (TrackedDebrisActors.Num() > 20)
    {
        BatchUpdateDebris();
    }
}

void UPerf_DestructionOptimizer::OptimizeDestructionSystem()
{
    if (!GetWorld())
    {
        return;
    }

    // Check current performance
    float CurrentFrameTime = GetAverageFrameTime();
    
    if (CurrentFrameTime > TargetFrameTime * 1.2f) // 20% over target
    {
        // Performance is poor, reduce quality
        if (CurrentLODLevel < 2)
        {
            SetLODLevel(CurrentLODLevel + 1);
            UE_LOG(LogTemp, Warning, TEXT("Destruction performance poor (%.2fms), reducing to LOD %d"), 
                CurrentFrameTime, CurrentLODLevel);
        }
    }
    else if (CurrentFrameTime < TargetFrameTime * 0.8f) // 20% under target
    {
        // Performance is good, can increase quality
        if (CurrentLODLevel > 0)
        {
            SetLODLevel(CurrentLODLevel - 1);
            UE_LOG(LogTemp, Log, TEXT("Destruction performance good (%.2fms), increasing to LOD %d"), 
                CurrentFrameTime, CurrentLODLevel);
        }
    }

    // Cull distant debris
    CullDistantDebris();
}

void UPerf_DestructionOptimizer::SetLODLevel(int32 LODLevel)
{
    CurrentLODLevel = FMath::Clamp(LODLevel, 0, 2);
    
    FPerf_DestructionLOD* SelectedLOD = nullptr;
    switch (CurrentLODLevel)
    {
        case 0: SelectedLOD = &HighQualityLOD; break;
        case 1: SelectedLOD = &MediumQualityLOD; break;
        case 2: SelectedLOD = &LowQualityLOD; break;
    }

    if (SelectedLOD)
    {
        SetDebrisLODSettings(*SelectedLOD);
    }
}

void UPerf_DestructionOptimizer::CullDistantDebris()
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn)
    {
        return;
    }

    FVector PlayerLocation = PlayerPawn->GetActorLocation();
    FPerf_DestructionLOD CurrentLOD = GetCurrentLODSettings();

    // Remove distant or invalid debris
    for (int32 i = TrackedDebrisActors.Num() - 1; i >= 0; i--)
    {
        if (!TrackedDebrisActors[i].IsValid())
        {
            TrackedDebrisActors.RemoveAt(i);
            continue;
        }

        AActor* DebrisActor = TrackedDebrisActors[i].Get();
        float Distance = FVector::Dist(PlayerLocation, DebrisActor->GetActorLocation());

        if (Distance > CurrentLOD.DistanceThreshold)
        {
            DebrisActor->Destroy();
            TrackedDebrisActors.RemoveAt(i);
        }
    }
}

void UPerf_DestructionOptimizer::BatchUpdateDebris()
{
    // Process debris in batches to avoid frame spikes
    const int32 BatchSize = 5;
    int32 ProcessedCount = 0;

    for (int32 i = TrackedDebrisActors.Num() - 1; i >= 0 && ProcessedCount < BatchSize; i--)
    {
        if (!TrackedDebrisActors[i].IsValid())
        {
            TrackedDebrisActors.RemoveAt(i);
            ProcessedCount++;
            continue;
        }

        AActor* DebrisActor = TrackedDebrisActors[i].Get();
        UStaticMeshComponent* MeshComp = DebrisActor->FindComponentByClass<UStaticMeshComponent>();
        
        if (MeshComp)
        {
            // Optimize physics settings based on current LOD
            FPerf_DestructionLOD CurrentLOD = GetCurrentLODSettings();
            MeshComp->SetSimulatePhysics(CurrentLOD.bEnablePhysicsSimulation);
            
            // Reduce collision complexity for distant objects
            if (CurrentLODLevel > 0)
            {
                MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            }
        }
        
        ProcessedCount++;
    }
}

void UPerf_DestructionOptimizer::CleanupOldDebris()
{
    FPerf_DestructionLOD CurrentLOD = GetCurrentLODSettings();
    float CurrentTime = GetWorld()->GetTimeSeconds();

    for (int32 i = TrackedDebrisActors.Num() - 1; i >= 0; i--)
    {
        if (!TrackedDebrisActors[i].IsValid())
        {
            TrackedDebrisActors.RemoveAt(i);
            continue;
        }

        AActor* DebrisActor = TrackedDebrisActors[i].Get();
        float ActorAge = CurrentTime - DebrisActor->GetGameTimeSinceCreation();

        if (ActorAge > CurrentLOD.DebrisLifetime)
        {
            DebrisActor->Destroy();
            TrackedDebrisActors.RemoveAt(i);
        }
    }

    // Enforce maximum debris count
    while (TrackedDebrisActors.Num() > CurrentLOD.MaxDebrisCount)
    {
        // Remove oldest debris first
        if (TrackedDebrisActors[0].IsValid())
        {
            TrackedDebrisActors[0]->Destroy();
        }
        TrackedDebrisActors.RemoveAt(0);
    }
}

FPerf_DestructionMetrics UPerf_DestructionOptimizer::GetPerformanceMetrics() const
{
    return CurrentMetrics;
}

float UPerf_DestructionOptimizer::GetDestructionFrameImpact() const
{
    // Estimate frame impact based on active debris and physics simulation
    float BaseImpact = TrackedDebrisActors.Num() * 0.1f; // 0.1ms per debris
    float PhysicsImpact = 0.0f;

    for (const TWeakObjectPtr<AActor>& DebrisPtr : TrackedDebrisActors)
    {
        if (DebrisPtr.IsValid())
        {
            UStaticMeshComponent* MeshComp = DebrisPtr->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp && MeshComp->IsSimulatingPhysics())
            {
                PhysicsImpact += 0.2f; // Additional cost for physics simulation
            }
        }
    }

    return BaseImpact + PhysicsImpact;
}

bool UPerf_DestructionOptimizer::IsPerformanceOptimal() const
{
    float CurrentFrameTime = GetAverageFrameTime();
    float DestructionImpact = GetDestructionFrameImpact();
    
    return (CurrentFrameTime <= TargetFrameTime) && (DestructionImpact <= MaxDestructionBudget);
}

void UPerf_DestructionOptimizer::UpdateLODBasedOnDistance(const FVector& PlayerLocation)
{
    // Find closest destruction system to determine appropriate LOD
    float ClosestDistance = FLT_MAX;
    
    for (const TWeakObjectPtr<AActor>& DebrisPtr : TrackedDebrisActors)
    {
        if (DebrisPtr.IsValid())
        {
            float Distance = FVector::Dist(PlayerLocation, DebrisPtr->GetActorLocation());
            ClosestDistance = FMath::Min(ClosestDistance, Distance);
        }
    }

    // Adjust LOD based on distance
    int32 TargetLOD = 1; // Default to medium
    if (ClosestDistance < 2000.0f)
    {
        TargetLOD = 0; // High quality
    }
    else if (ClosestDistance > 5000.0f)
    {
        TargetLOD = 2; // Low quality
    }

    if (TargetLOD != CurrentLODLevel)
    {
        SetLODLevel(TargetLOD);
    }
}

void UPerf_DestructionOptimizer::SetDebrisLODSettings(const FPerf_DestructionLOD& LODSettings)
{
    // Apply LOD settings to all tracked debris
    for (const TWeakObjectPtr<AActor>& DebrisPtr : TrackedDebrisActors)
    {
        if (DebrisPtr.IsValid())
        {
            UStaticMeshComponent* MeshComp = DebrisPtr->FindComponentByClass<UStaticMeshComponent>();
            if (MeshComp)
            {
                MeshComp->SetSimulatePhysics(LODSettings.bEnablePhysicsSimulation);
            }
        }
    }
}

void UPerf_DestructionOptimizer::UpdatePerformanceMetrics()
{
    // Update current metrics
    CurrentMetrics.ActiveDebrisCount = TrackedDebrisActors.Num();
    CurrentMetrics.AverageFrameTime = GetAverageFrameTime();
    CurrentMetrics.PhysicsSimulationTime = GetDestructionFrameImpact();
    CurrentMetrics.DestructionEventsThisFrame = 0; // Reset each update
}

float UPerf_DestructionOptimizer::GetAverageFrameTime() const
{
    if (FrameCounter > 0)
    {
        return AccumulatedFrameTime / FrameCounter;
    }
    return 16.67f; // Default to 60 FPS
}

FPerf_DestructionLOD UPerf_DestructionOptimizer::GetCurrentLODSettings() const
{
    switch (CurrentLODLevel)
    {
        case 0: return HighQualityLOD;
        case 1: return MediumQualityLOD;
        case 2: return LowQualityLOD;
        default: return MediumQualityLOD;
    }
}