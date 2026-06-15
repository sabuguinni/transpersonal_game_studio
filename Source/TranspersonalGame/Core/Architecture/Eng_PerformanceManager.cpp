#include "Eng_PerformanceManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "TimerManager.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/GameplayStatics.h"

void UEng_PerformanceManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Initialized with target %d"), (int32)TargetPerformance);

    // Set up performance monitoring timer
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(PerformanceTimerHandle, this, 
            &UEng_PerformanceManager::UpdatePerformanceMetrics, 1.0f, true);
    }

    ApplyPerformanceSettings();
}

void UEng_PerformanceManager::Deinitialize()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(PerformanceTimerHandle);
    }

    Super::Deinitialize();
}

void UEng_PerformanceManager::SetPerformanceTarget(EEng_PerformanceTarget NewTarget)
{
    TargetPerformance = NewTarget;
    
    // Adjust limits based on target
    switch (NewTarget)
    {
        case EEng_PerformanceTarget::PC_60FPS:
            MaxActors = 8000;
            MaxDinosaurs = 150;
            MaxMemoryMB = 4096.0f;
            break;
        case EEng_PerformanceTarget::Console_30FPS:
            MaxActors = 5000;
            MaxDinosaurs = 100;
            MaxMemoryMB = 2048.0f;
            break;
        case EEng_PerformanceTarget::Mobile_30FPS:
            MaxActors = 2000;
            MaxDinosaurs = 50;
            MaxMemoryMB = 1024.0f;
            break;
    }

    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Target set to %d - MaxActors: %d, MaxDinosaurs: %d"), 
           (int32)NewTarget, MaxActors, MaxDinosaurs);

    ApplyPerformanceSettings();
}

FEng_PerformanceMetrics UEng_PerformanceManager::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

bool UEng_PerformanceManager::IsPerformanceWithinTarget() const
{
    float TargetFPS = (TargetPerformance == EEng_PerformanceTarget::PC_60FPS) ? 60.0f : 30.0f;
    
    return CurrentMetrics.CurrentFPS >= (TargetFPS * 0.9f) &&
           CurrentMetrics.ActorCount <= MaxActors &&
           CurrentMetrics.DinosaurCount <= MaxDinosaurs &&
           CurrentMetrics.MemoryUsageMB <= MaxMemoryMB;
}

void UEng_PerformanceManager::EnforceActorLimits()
{
    UWorld* World = GetWorld();
    if (!World) return;

    // Get all actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);

    CurrentMetrics.ActorCount = AllActors.Num();

    // Count dinosaurs (actors with "dino" in their name)
    int32 DinoCount = 0;
    TArray<AActor*> DinosaurActors;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(TEXT("dino"), ESearchCase::IgnoreCase))
        {
            DinoCount++;
            DinosaurActors.Add(Actor);
        }
    }
    CurrentMetrics.DinosaurCount = DinoCount;

    // Enforce limits
    if (CurrentMetrics.ActorCount > MaxActors)
    {
        int32 ActorsToRemove = CurrentMetrics.ActorCount - MaxActors;
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Removing %d actors to enforce limit"), ActorsToRemove);
        
        // Remove non-essential actors first
        int32 Removed = 0;
        for (AActor* Actor : AllActors)
        {
            if (Removed >= ActorsToRemove) break;
            
            if (Actor && !Actor->GetName().Contains(TEXT("PlayerStart")) &&
                !Actor->GetName().Contains(TEXT("Light")) &&
                !Actor->GetName().Contains(TEXT("Sky")))
            {
                Actor->Destroy();
                Removed++;
            }
        }
    }

    if (CurrentMetrics.DinosaurCount > MaxDinosaurs)
    {
        int32 DinosToRemove = CurrentMetrics.DinosaurCount - MaxDinosaurs;
        UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Removing %d dinosaurs to enforce limit"), DinosToRemove);
        
        for (int32 i = 0; i < DinosToRemove && i < DinosaurActors.Num(); i++)
        {
            if (DinosaurActors[i])
            {
                DinosaurActors[i]->Destroy();
            }
        }
    }
}

void UEng_PerformanceManager::OptimizeForTarget()
{
    UE_LOG(LogTemp, Warning, TEXT("PerformanceManager: Optimizing for target %d"), (int32)TargetPerformance);

    // Apply console variables based on target
    switch (TargetPerformance)
    {
        case EEng_PerformanceTarget::PC_60FPS:
            // High quality settings
            GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 1.0"));
            GEngine->Exec(GetWorld(), TEXT("foliage.LODDistanceScale 1.0"));
            GEngine->Exec(GetWorld(), TEXT("r.DefaultFeature.AntiAliasing 2"));
            break;
            
        case EEng_PerformanceTarget::Console_30FPS:
            // Medium quality settings
            GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.8"));
            GEngine->Exec(GetWorld(), TEXT("foliage.LODDistanceScale 0.7"));
            GEngine->Exec(GetWorld(), TEXT("r.DefaultFeature.AntiAliasing 1"));
            break;
            
        case EEng_PerformanceTarget::Mobile_30FPS:
            // Low quality settings
            GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.5"));
            GEngine->Exec(GetWorld(), TEXT("foliage.LODDistanceScale 0.4"));
            GEngine->Exec(GetWorld(), TEXT("r.DefaultFeature.AntiAliasing 0"));
            break;
    }

    EnforceActorLimits();
}

void UEng_PerformanceManager::UpdatePerformanceMetrics()
{
    if (UWorld* World = GetWorld())
    {
        // Get FPS
        CurrentMetrics.CurrentFPS = 1.0f / World->GetDeltaSeconds();
        CurrentMetrics.FrameTime = World->GetDeltaSeconds() * 1000.0f; // Convert to ms

        // Get actor counts
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentMetrics.ActorCount = AllActors.Num();

        // Count dinosaurs
        int32 DinoCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName().Contains(TEXT("dino"), ESearchCase::IgnoreCase))
            {
                DinoCount++;
            }
        }
        CurrentMetrics.DinosaurCount = DinoCount;

        // Estimate memory usage (simplified)
        CurrentMetrics.MemoryUsageMB = (CurrentMetrics.ActorCount * 0.5f) + (CurrentMetrics.DinosaurCount * 2.0f);
    }
}

void UEng_PerformanceManager::ApplyPerformanceSettings()
{
    OptimizeForTarget();
}