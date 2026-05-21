#include "Perf_LODManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

APerf_LODManager::APerf_LODManager()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    
    // Set default LOD settings
    LODSettings = FPerf_LODSettings();
    CurrentGlobalLODLevel = EPerf_LODLevel::High;
    bEnableAutomaticLOD = true;
    UpdateInterval = 0.1f;
    bEnablePerformanceBasedLOD = true;
    TargetFrameTime = 16.67f; // 60 FPS target
    
    // Initialize stats
    CurrentStats = FPerf_LODStats();
    bIsOptimizing = false;
    
    TimeSinceLastUpdate = 0.0f;
    LastFrameTime = 16.67f;
    PerformanceAdjustmentCounter = 0;
}

void APerf_LODManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_LODManager: Starting LOD management system"));
    
    // Initialize managed actors list
    UpdateManagedActorsList();
    
    // Set initial LOD levels
    UpdateLODForAllActors();
    
    UE_LOG(LogTemp, Warning, TEXT("Perf_LODManager: Initialized with %d managed actors"), ManagedActors.Num());
}

void APerf_LODManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (!bEnableAutomaticLOD)
    {
        return;
    }
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        // Update frame time tracking
        LastFrameTime = DeltaTime * 1000.0f; // Convert to milliseconds
        
        // Performance-based LOD adjustment
        if (bEnablePerformanceBasedLOD)
        {
            OptimizeBasedOnPerformance(LastFrameTime);
        }
        
        // Update LOD for all actors
        UpdateLODForAllActors();
        
        // Update statistics
        UpdateStatistics();
        
        TimeSinceLastUpdate = 0.0f;
    }
}

void APerf_LODManager::SetGlobalLODLevel(EPerf_LODLevel NewLODLevel)
{
    if (CurrentGlobalLODLevel != NewLODLevel)
    {
        EPerf_LODLevel OldLevel = CurrentGlobalLODLevel;
        CurrentGlobalLODLevel = NewLODLevel;
        
        UE_LOG(LogTemp, Warning, TEXT("Perf_LODManager: Global LOD level changed from %d to %d"), 
               (int32)OldLevel, (int32)NewLODLevel);
        
        // Apply new LOD settings based on level
        SetLODSettingsFromPerformanceLevel(NewLODLevel);
        
        // Update all actors immediately
        UpdateLODForAllActors();
        
        // Broadcast event
        OnLODLevelChanged.Broadcast(NewLODLevel);
    }
}

void APerf_LODManager::UpdateLODForAllActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Update managed actors list periodically
    static int32 UpdateCounter = 0;
    UpdateCounter++;
    if (UpdateCounter % 50 == 0) // Every 5 seconds at 0.1s intervals
    {
        UpdateManagedActorsList();
    }
    
    // Update LOD for each managed actor
    for (int32 i = ManagedActors.Num() - 1; i >= 0; i--)
    {
        if (ManagedActors[i].IsValid())
        {
            UpdateLODForActor(ManagedActors[i].Get());
        }
        else
        {
            // Remove invalid actors
            ManagedActors.RemoveAt(i);
        }
    }
}

void APerf_LODManager::UpdateLODForActor(AActor* Actor)
{
    if (!Actor || !GetWorld())
    {
        return;
    }
    
    // Calculate distance to player
    float Distance = CalculateDistanceToPlayer(Actor);
    
    // Check if actor should be culled
    if (ShouldCullActor(Actor, Distance))
    {
        Actor->SetActorHiddenInGame(true);
        Actor->SetActorTickEnabled(false);
        return;
    }
    else
    {
        Actor->SetActorHiddenInGame(false);
        Actor->SetActorTickEnabled(true);
    }
    
    // Calculate appropriate LOD level
    EPerf_LODLevel TargetLODLevel = CalculateLODLevelForDistance(Distance);
    
    // Store current LOD level for this actor
    ActorLODLevels.Add(Actor, TargetLODLevel);
    
    // Apply LOD to mesh components
    TArray<UStaticMeshComponent*> StaticMeshes;
    Actor->GetComponents<UStaticMeshComponent>(StaticMeshes);
    for (UStaticMeshComponent* MeshComp : StaticMeshes)
    {
        ApplyLODToStaticMesh(MeshComp, TargetLODLevel);
    }
    
    TArray<USkeletalMeshComponent*> SkeletalMeshes;
    Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
    for (USkeletalMeshComponent* MeshComp : SkeletalMeshes)
    {
        ApplyLODToSkeletalMesh(MeshComp, TargetLODLevel);
    }
}

EPerf_LODLevel APerf_LODManager::CalculateLODLevelForDistance(float Distance)
{
    // Apply global LOD level bias
    float BiasMultiplier = 1.0f;
    switch (CurrentGlobalLODLevel)
    {
        case EPerf_LODLevel::Ultra:
            BiasMultiplier = 2.0f; // Extend high-quality distances
            break;
        case EPerf_LODLevel::High:
            BiasMultiplier = 1.5f;
            break;
        case EPerf_LODLevel::Medium:
            BiasMultiplier = 1.0f;
            break;
        case EPerf_LODLevel::Low:
            BiasMultiplier = 0.7f;
            break;
        case EPerf_LODLevel::Critical:
            BiasMultiplier = 0.5f; // Reduce high-quality distances
            break;
    }
    
    float AdjustedDistance = Distance / BiasMultiplier;
    
    if (AdjustedDistance <= LODSettings.UltraDistance)
    {
        return EPerf_LODLevel::Ultra;
    }
    else if (AdjustedDistance <= LODSettings.HighDistance)
    {
        return EPerf_LODLevel::High;
    }
    else if (AdjustedDistance <= LODSettings.MediumDistance)
    {
        return EPerf_LODLevel::Medium;
    }
    else if (AdjustedDistance <= LODSettings.LowDistance)
    {
        return EPerf_LODLevel::Low;
    }
    else
    {
        return EPerf_LODLevel::Critical;
    }
}

void APerf_LODManager::OptimizeBasedOnPerformance(float CurrentFrameTime)
{
    if (CurrentFrameTime <= TargetFrameTime)
    {
        // Performance is good, can potentially increase quality
        PerformanceAdjustmentCounter++;
        if (PerformanceAdjustmentCounter >= 30) // 3 seconds of good performance
        {
            if (CurrentGlobalLODLevel != EPerf_LODLevel::Ultra)
            {
                EPerf_LODLevel NewLevel = static_cast<EPerf_LODLevel>(static_cast<int32>(CurrentGlobalLODLevel) - 1);
                SetGlobalLODLevel(NewLevel);
                UE_LOG(LogTemp, Warning, TEXT("Perf_LODManager: Performance good, increasing quality to %d"), (int32)NewLevel);
            }
            PerformanceAdjustmentCounter = 0;
        }
    }
    else
    {
        // Performance is poor, need to reduce quality
        PerformanceAdjustmentCounter = 0;
        if (CurrentFrameTime > TargetFrameTime * 1.5f) // 50% over target
        {
            if (CurrentGlobalLODLevel != EPerf_LODLevel::Critical)
            {
                EPerf_LODLevel NewLevel = static_cast<EPerf_LODLevel>(static_cast<int32>(CurrentGlobalLODLevel) + 1);
                SetGlobalLODLevel(NewLevel);
                UE_LOG(LogTemp, Warning, TEXT("Perf_LODManager: Performance poor (%.2fms), reducing quality to %d"), 
                       CurrentFrameTime, (int32)NewLevel);
            }
        }
    }
}

void APerf_LODManager::RegisterActorForLOD(AActor* Actor)
{
    if (Actor && !ManagedActors.Contains(Actor))
    {
        ManagedActors.Add(Actor);
        UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Registered actor %s for LOD management"), *Actor->GetName());
    }
}

void APerf_LODManager::UnregisterActorFromLOD(AActor* Actor)
{
    if (Actor)
    {
        ManagedActors.Remove(Actor);
        ActorLODLevels.Remove(Actor);
        UE_LOG(LogTemp, Log, TEXT("Perf_LODManager: Unregistered actor %s from LOD management"), *Actor->GetName());
    }
}

FPerf_LODStats APerf_LODManager::GetCurrentLODStats()
{
    return CurrentStats;
}

float APerf_LODManager::GetEstimatedPerformanceGain()
{
    // Calculate estimated performance gain based on LOD distribution
    float BasePerformance = 100.0f;
    float CurrentPerformance = 0.0f;
    
    CurrentPerformance += CurrentStats.UltraLODActors * 1.0f;    // Full cost
    CurrentPerformance += CurrentStats.HighLODActors * 0.8f;     // 20% savings
    CurrentPerformance += CurrentStats.MediumLODActors * 0.6f;   // 40% savings
    CurrentPerformance += CurrentStats.LowLODActors * 0.4f;      // 60% savings
    CurrentPerformance += CurrentStats.CriticalLODActors * 0.2f; // 80% savings
    
    if (CurrentStats.TotalActors > 0)
    {
        float PerformanceRatio = CurrentPerformance / CurrentStats.TotalActors;
        return (1.0f - PerformanceRatio) * 100.0f; // Percentage gain
    }
    
    return 0.0f;
}

int32 APerf_LODManager::GetActorCountInLODLevel(EPerf_LODLevel LODLevel)
{
    switch (LODLevel)
    {
        case EPerf_LODLevel::Ultra: return CurrentStats.UltraLODActors;
        case EPerf_LODLevel::High: return CurrentStats.HighLODActors;
        case EPerf_LODLevel::Medium: return CurrentStats.MediumLODActors;
        case EPerf_LODLevel::Low: return CurrentStats.LowLODActors;
        case EPerf_LODLevel::Critical: return CurrentStats.CriticalLODActors;
        default: return 0;
    }
}

void APerf_LODManager::SetLODSettingsFromPerformanceLevel(EPerf_LODLevel PerformanceLevel)
{
    switch (PerformanceLevel)
    {
        case EPerf_LODLevel::Ultra:
            LODSettings.UltraDistance = 2000.0f;
            LODSettings.HighDistance = 5000.0f;
            LODSettings.MediumDistance = 10000.0f;
            LODSettings.LowDistance = 20000.0f;
            LODSettings.CriticalDistance = 30000.0f;
            break;
        case EPerf_LODLevel::High:
            LODSettings.UltraDistance = 1500.0f;
            LODSettings.HighDistance = 3500.0f;
            LODSettings.MediumDistance = 7500.0f;
            LODSettings.LowDistance = 15000.0f;
            LODSettings.CriticalDistance = 25000.0f;
            break;
        case EPerf_LODLevel::Medium:
            LODSettings.UltraDistance = 1000.0f;
            LODSettings.HighDistance = 2500.0f;
            LODSettings.MediumDistance = 5000.0f;
            LODSettings.LowDistance = 10000.0f;
            LODSettings.CriticalDistance = 15000.0f;
            break;
        case EPerf_LODLevel::Low:
            LODSettings.UltraDistance = 500.0f;
            LODSettings.HighDistance = 1500.0f;
            LODSettings.MediumDistance = 3000.0f;
            LODSettings.LowDistance = 6000.0f;
            LODSettings.CriticalDistance = 10000.0f;
            break;
        case EPerf_LODLevel::Critical:
            LODSettings.UltraDistance = 250.0f;
            LODSettings.HighDistance = 750.0f;
            LODSettings.MediumDistance = 1500.0f;
            LODSettings.LowDistance = 3000.0f;
            LODSettings.CriticalDistance = 5000.0f;
            break;
    }
}

void APerf_LODManager::ResetToDefaultLODSettings()
{
    LODSettings = FPerf_LODSettings();
    SetGlobalLODLevel(EPerf_LODLevel::High);
}

void APerf_LODManager::DebugPrintLODStats()
{
    UE_LOG(LogTemp, Warning, TEXT("=== LOD Manager Statistics ==="));
    UE_LOG(LogTemp, Warning, TEXT("Total Actors: %d"), CurrentStats.TotalActors);
    UE_LOG(LogTemp, Warning, TEXT("Ultra LOD: %d"), CurrentStats.UltraLODActors);
    UE_LOG(LogTemp, Warning, TEXT("High LOD: %d"), CurrentStats.HighLODActors);
    UE_LOG(LogTemp, Warning, TEXT("Medium LOD: %d"), CurrentStats.MediumLODActors);
    UE_LOG(LogTemp, Warning, TEXT("Low LOD: %d"), CurrentStats.LowLODActors);
    UE_LOG(LogTemp, Warning, TEXT("Critical LOD: %d"), CurrentStats.CriticalLODActors);
    UE_LOG(LogTemp, Warning, TEXT("Culled: %d"), CurrentStats.CulledActors);
    UE_LOG(LogTemp, Warning, TEXT("Average Distance: %.2f"), CurrentStats.AverageDistance);
    UE_LOG(LogTemp, Warning, TEXT("Performance Gain: %.1f%%"), GetEstimatedPerformanceGain());
    UE_LOG(LogTemp, Warning, TEXT("Current Global LOD: %d"), (int32)CurrentGlobalLODLevel);
    UE_LOG(LogTemp, Warning, TEXT("Last Frame Time: %.2fms"), LastFrameTime);
}

void APerf_LODManager::UpdateManagedActorsList()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Clear invalid actors
    for (int32 i = ManagedActors.Num() - 1; i >= 0; i--)
    {
        if (!ManagedActors[i].IsValid())
        {
            ManagedActors.RemoveAt(i);
        }
    }
    
    // Find all actors with mesh components
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != this && !ManagedActors.Contains(Actor))
        {
            // Check if actor has mesh components
            TArray<UStaticMeshComponent*> StaticMeshes;
            TArray<USkeletalMeshComponent*> SkeletalMeshes;
            Actor->GetComponents<UStaticMeshComponent>(StaticMeshes);
            Actor->GetComponents<USkeletalMeshComponent>(SkeletalMeshes);
            
            if (StaticMeshes.Num() > 0 || SkeletalMeshes.Num() > 0)
            {
                RegisterActorForLOD(Actor);
            }
        }
    }
}

void APerf_LODManager::ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp || !MeshComp->GetStaticMesh())
    {
        return;
    }
    
    int32 TargetLOD = 0;
    switch (LODLevel)
    {
        case EPerf_LODLevel::Ultra:
            TargetLOD = 0;
            break;
        case EPerf_LODLevel::High:
            TargetLOD = FMath::Min(1, MeshComp->GetStaticMesh()->GetNumLODs() - 1);
            break;
        case EPerf_LODLevel::Medium:
            TargetLOD = FMath::Min(2, MeshComp->GetStaticMesh()->GetNumLODs() - 1);
            break;
        case EPerf_LODLevel::Low:
            TargetLOD = FMath::Min(3, MeshComp->GetStaticMesh()->GetNumLODs() - 1);
            break;
        case EPerf_LODLevel::Critical:
            TargetLOD = MeshComp->GetStaticMesh()->GetNumLODs() - 1;
            break;
    }
    
    MeshComp->SetForcedLodModel(TargetLOD + 1); // +1 because 0 means no forced LOD
}

void APerf_LODManager::ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel)
{
    if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
    {
        return;
    }
    
    int32 TargetLOD = 0;
    switch (LODLevel)
    {
        case EPerf_LODLevel::Ultra:
            TargetLOD = 0;
            break;
        case EPerf_LODLevel::High:
            TargetLOD = FMath::Min(1, MeshComp->GetSkeletalMeshAsset()->GetLODNum() - 1);
            break;
        case EPerf_LODLevel::Medium:
            TargetLOD = FMath::Min(2, MeshComp->GetSkeletalMeshAsset()->GetLODNum() - 1);
            break;
        case EPerf_LODLevel::Low:
            TargetLOD = FMath::Min(3, MeshComp->GetSkeletalMeshAsset()->GetLODNum() - 1);
            break;
        case EPerf_LODLevel::Critical:
            TargetLOD = MeshComp->GetSkeletalMeshAsset()->GetLODNum() - 1;
            break;
    }
    
    MeshComp->SetForcedLOD(TargetLOD + 1); // +1 because 0 means no forced LOD
}

float APerf_LODManager::CalculateDistanceToPlayer(AActor* Actor)
{
    if (!Actor || !GetWorld())
    {
        return 99999.0f;
    }
    
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
        FVector ActorLocation = Actor->GetActorLocation();
        return FVector::Dist(PlayerLocation, ActorLocation);
    }
    
    return 99999.0f;
}

void APerf_LODManager::UpdateStatistics()
{
    // Reset stats
    CurrentStats = FPerf_LODStats();
    CurrentStats.TotalActors = ManagedActors.Num();
    
    float TotalDistance = 0.0f;
    int32 ValidActors = 0;
    
    // Count actors in each LOD level
    for (const auto& ActorLODPair : ActorLODLevels)
    {
        if (ActorLODPair.Key.IsValid())
        {
            ValidActors++;
            float Distance = CalculateDistanceToPlayer(ActorLODPair.Key.Get());
            TotalDistance += Distance;
            
            if (ShouldCullActor(ActorLODPair.Key.Get(), Distance))
            {
                CurrentStats.CulledActors++;
            }
            else
            {
                switch (ActorLODPair.Value)
                {
                    case EPerf_LODLevel::Ultra:
                        CurrentStats.UltraLODActors++;
                        break;
                    case EPerf_LODLevel::High:
                        CurrentStats.HighLODActors++;
                        break;
                    case EPerf_LODLevel::Medium:
                        CurrentStats.MediumLODActors++;
                        break;
                    case EPerf_LODLevel::Low:
                        CurrentStats.LowLODActors++;
                        break;
                    case EPerf_LODLevel::Critical:
                        CurrentStats.CriticalLODActors++;
                        break;
                }
            }
        }
    }
    
    // Calculate average distance
    if (ValidActors > 0)
    {
        CurrentStats.AverageDistance = TotalDistance / ValidActors;
    }
    
    // Calculate performance impact
    CurrentStats.PerformanceImpact = GetEstimatedPerformanceGain();
}

bool APerf_LODManager::ShouldCullActor(AActor* Actor, float Distance)
{
    if (!LODSettings.bEnableDistanceCulling || !Actor)
    {
        return false;
    }
    
    return Distance > LODSettings.CullingDistance;
}