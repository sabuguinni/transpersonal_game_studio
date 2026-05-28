#include "Perf_OptimizationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "HAL/PlatformFilemanager.h"
#include "Kismet/GameplayStatics.h"

UPerf_OptimizationManager::UPerf_OptimizationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    LastUpdateTime = 0.0f;
    FrameCounter = 0;
    FPSHistory.Reserve(100); // Keep last 100 FPS samples
}

void UPerf_OptimizationManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize performance monitoring
    LastUpdateTime = GetWorld()->GetTimeSeconds();
    ApplyOptimizationSettings();
    
    UE_LOG(LogTemp, Warning, TEXT("Performance Optimization Manager initialized"));
}

void UPerf_OptimizationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    FrameCounter++;
    
    // Update metrics every 0.5 seconds
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastUpdateTime >= 0.5f)
    {
        UpdatePerformanceMetrics();
        CheckPerformanceThresholds();
        LastUpdateTime = CurrentTime;
    }
}

void UPerf_OptimizationManager::UpdatePerformanceMetrics()
{
    if (!GetWorld())
    {
        return;
    }

    // Calculate current FPS
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentMetrics.CurrentFPS = DeltaTime > 0.0f ? 1.0f / DeltaTime : 0.0f;
    
    // Add to history and calculate average
    FPSHistory.Add(CurrentMetrics.CurrentFPS);
    if (FPSHistory.Num() > 100)
    {
        FPSHistory.RemoveAt(0);
    }
    CalculateAverageFPS();

    // Count actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    CurrentMetrics.ActiveActorCount = AllActors.Num();

    // Count physics actors
    int32 PhysicsCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (StaticMesh && StaticMesh->IsSimulatingPhysics())
            {
                PhysicsCount++;
            }
        }
    }
    
    CurrentMetrics.PhysicsActorCount = PhysicsCount;
    CurrentMetrics.PhysicsActorRatio = CurrentMetrics.ActiveActorCount > 0 ? 
        (float)PhysicsCount / (float)CurrentMetrics.ActiveActorCount * 100.0f : 0.0f;

    // Memory usage (approximate)
    CurrentMetrics.MemoryUsageMB = FPlatformMemory::GetStats().UsedPhysical / (1024.0f * 1024.0f);
}

void UPerf_OptimizationManager::OptimizeScene()
{
    if (!GetWorld())
    {
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Running scene optimization..."));

    // Cull distant actors
    CullDistantActors();
    
    // Optimize physics actors
    OptimizePhysicsActors();
    
    // Apply LOD optimizations
    if (OptimizationSettings.bEnableAutoLOD)
    {
        APlayerController* PC = GetWorld()->GetFirstPlayerController();
        if (PC && PC->GetPawn())
        {
            FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
            TArray<AActor*> AllActors;
            UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
            
            for (AActor* Actor : AllActors)
            {
                if (Actor && Actor != PC->GetPawn())
                {
                    float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
                    OptimizeActorLOD(Actor, Distance);
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Scene optimization complete"));
}

void UPerf_OptimizationManager::SetOptimizationLevel(EPerf_OptimizationLevel NewLevel)
{
    OptimizationSettings.OptimizationLevel = NewLevel;
    
    // Adjust settings based on optimization level
    switch (NewLevel)
    {
        case EPerf_OptimizationLevel::Low:
            OptimizationSettings.MaxActiveActors = 500;
            OptimizationSettings.CullingDistance = 5000.0f;
            OptimizationSettings.MaxPhysicsActorRatio = 5.0f;
            break;
        case EPerf_OptimizationLevel::Medium:
            OptimizationSettings.MaxActiveActors = 1500;
            OptimizationSettings.CullingDistance = 8000.0f;
            OptimizationSettings.MaxPhysicsActorRatio = 10.0f;
            break;
        case EPerf_OptimizationLevel::High:
            OptimizationSettings.MaxActiveActors = 3000;
            OptimizationSettings.CullingDistance = 12000.0f;
            OptimizationSettings.MaxPhysicsActorRatio = 15.0f;
            break;
        case EPerf_OptimizationLevel::Ultra:
            OptimizationSettings.MaxActiveActors = 5000;
            OptimizationSettings.CullingDistance = 20000.0f;
            OptimizationSettings.MaxPhysicsActorRatio = 20.0f;
            break;
    }
    
    ApplyOptimizationSettings();
    UE_LOG(LogTemp, Warning, TEXT("Optimization level set to: %d"), (int32)NewLevel);
}

bool UPerf_OptimizationManager::IsPerformanceAcceptable() const
{
    // Check if performance meets minimum requirements
    bool bFPSAcceptable = CurrentMetrics.AverageFPS >= 30.0f; // Minimum 30 FPS
    bool bActorCountAcceptable = CurrentMetrics.ActiveActorCount <= OptimizationSettings.MaxActiveActors;
    bool bPhysicsRatioAcceptable = CurrentMetrics.PhysicsActorRatio <= OptimizationSettings.MaxPhysicsActorRatio;
    
    return bFPSAcceptable && bActorCountAcceptable && bPhysicsRatioAcceptable;
}

void UPerf_OptimizationManager::CullDistantActors()
{
    if (!GetWorld())
    {
        return;
    }

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC || !PC->GetPawn())
    {
        return;
    }

    FVector PlayerLocation = PC->GetPawn()->GetActorLocation();
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 CulledCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor != PC->GetPawn())
        {
            float Distance = FVector::Dist(PlayerLocation, Actor->GetActorLocation());
            if (Distance > OptimizationSettings.CullingDistance)
            {
                Actor->SetActorHiddenInGame(true);
                Actor->SetActorEnableCollision(false);
                CulledCount++;
            }
            else
            {
                Actor->SetActorHiddenInGame(false);
                Actor->SetActorEnableCollision(true);
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Culled %d distant actors"), CulledCount);
}

void UPerf_OptimizationManager::OptimizePhysicsActors()
{
    if (!GetWorld())
    {
        return;
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    int32 OptimizedCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>();
            if (StaticMesh && StaticMesh->IsSimulatingPhysics())
            {
                // Optimize physics settings for performance
                StaticMesh->SetNotifyRigidBodyCollision(false);
                StaticMesh->SetGenerateOverlapEvents(false);
                OptimizedCount++;
            }
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("Optimized %d physics actors"), OptimizedCount);
}

void UPerf_OptimizationManager::RunPerformanceAnalysis()
{
    UE_LOG(LogTemp, Warning, TEXT("=== PERFORMANCE ANALYSIS ==="));
    UE_LOG(LogTemp, Warning, TEXT("Current FPS: %.1f"), CurrentMetrics.CurrentFPS);
    UE_LOG(LogTemp, Warning, TEXT("Average FPS: %.1f"), CurrentMetrics.AverageFPS);
    UE_LOG(LogTemp, Warning, TEXT("Active Actors: %d"), CurrentMetrics.ActiveActorCount);
    UE_LOG(LogTemp, Warning, TEXT("Physics Actors: %d (%.1f%%)"), CurrentMetrics.PhysicsActorCount, CurrentMetrics.PhysicsActorRatio);
    UE_LOG(LogTemp, Warning, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Warning, TEXT("Performance Acceptable: %s"), IsPerformanceAcceptable() ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Warning, TEXT("============================"));
}

void UPerf_OptimizationManager::CalculateAverageFPS()
{
    if (FPSHistory.Num() == 0)
    {
        CurrentMetrics.AverageFPS = 0.0f;
        return;
    }

    float Sum = 0.0f;
    for (float FPS : FPSHistory)
    {
        Sum += FPS;
    }
    CurrentMetrics.AverageFPS = Sum / FPSHistory.Num();
}

void UPerf_OptimizationManager::CheckPerformanceThresholds()
{
    if (!IsPerformanceAcceptable())
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance below threshold - triggering optimization"));
        OptimizeScene();
    }
}

void UPerf_OptimizationManager::ApplyOptimizationSettings()
{
    // Apply console commands for performance
    if (GEngine)
    {
        switch (OptimizationSettings.OptimizationLevel)
        {
            case EPerf_OptimizationLevel::Low:
                GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.5"));
                GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 1"));
                break;
            case EPerf_OptimizationLevel::Medium:
                GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 0.75"));
                GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 2"));
                break;
            case EPerf_OptimizationLevel::High:
                GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 1.0"));
                GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 3"));
                break;
            case EPerf_OptimizationLevel::Ultra:
                GEngine->Exec(GetWorld(), TEXT("r.ViewDistanceScale 1.25"));
                GEngine->Exec(GetWorld(), TEXT("r.ShadowQuality 4"));
                break;
        }
    }
}

TArray<AActor*> UPerf_OptimizationManager::GetActorsInRadius(FVector Center, float Radius)
{
    TArray<AActor*> ActorsInRadius;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            float Distance = FVector::Dist(Center, Actor->GetActorLocation());
            if (Distance <= Radius)
            {
                ActorsInRadius.Add(Actor);
            }
        }
    }
    
    return ActorsInRadius;
}

void UPerf_OptimizationManager::OptimizeActorLOD(AActor* Actor, float Distance)
{
    if (!Actor)
    {
        return;
    }

    // Simple distance-based LOD optimization
    UStaticMeshComponent* StaticMesh = Actor->FindComponentByClass<UStaticMeshComponent>();
    if (StaticMesh)
    {
        if (Distance > 5000.0f)
        {
            StaticMesh->SetForcedLodModel(3); // Lowest LOD
        }
        else if (Distance > 2000.0f)
        {
            StaticMesh->SetForcedLodModel(2); // Medium LOD
        }
        else if (Distance > 1000.0f)
        {
            StaticMesh->SetForcedLodModel(1); // High LOD
        }
        else
        {
            StaticMesh->SetForcedLodModel(0); // Highest LOD
        }
    }
}