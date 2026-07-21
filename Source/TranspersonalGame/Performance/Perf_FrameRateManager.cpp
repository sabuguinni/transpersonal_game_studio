#include "Perf_FrameRateManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "HAL/IConsoleManager.h"
#include "Stats/Stats.h"

UPerf_FrameRateManager::UPerf_FrameRateManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickInterval = 0.0f; // Tick every frame
    
    FrameHistory.Reserve(FrameHistorySize);
    
    // Initialize performance tracking variables
    FrameRateAccumulator = 0.0f;
    FrameCount = 0;
    LastOptimizationTime = 0.0f;
    bIsPerformanceCritical = false;
    AverageFrameRate = 0.0f;
    MinFrameRateRecorded = 999.0f;
    MaxFrameRateRecorded = 0.0f;
}

void UPerf_FrameRateManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Set initial frame rate target
    if (GEngine)
    {
        GEngine->SetMaxFPS(PerformanceSettings.TargetFrameRate);
    }
    
    // Initialize console variables for performance monitoring
    static IConsoleVariable* CVarShowFPS = IConsoleManager::Get().FindConsoleVariable(TEXT("stat.fps"));
    if (CVarShowFPS)
    {
        CVarShowFPS->Set(1);
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Initialized with target FPS: %.1f"), PerformanceSettings.TargetFrameRate);
}

void UPerf_FrameRateManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Update frame data every tick
    UpdateFrameData(DeltaTime);
    UpdateFrameHistory();
    CheckPerformanceThresholds();
    
    // Apply automatic optimizations at regular intervals
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (PerformanceSettings.bEnableAutomaticOptimization && 
        (CurrentTime - LastOptimizationTime) >= OptimizationInterval)
    {
        ApplyAutomaticOptimizations();
        LastOptimizationTime = CurrentTime;
    }
}

void UPerf_FrameRateManager::UpdateFrameData(float DeltaTime)
{
    CurrentFrameData.DeltaTime = DeltaTime;
    CurrentFrameData.FrameRate = (DeltaTime > 0.0f) ? (1.0f / DeltaTime) : 0.0f;
    
    // Count actors in the world
    if (UWorld* World = GetWorld())
    {
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
        CurrentFrameData.ActorCount = AllActors.Num();
        
        // Count physics-enabled actors
        CurrentFrameData.PhysicsActorCount = 0;
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetRootComponent())
            {
                if (UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent()))
                {
                    if (PrimComp->IsSimulatingPhysics())
                    {
                        CurrentFrameData.PhysicsActorCount++;
                    }
                }
            }
        }
    }
    
    // Get thread timing information
    CurrentFrameData.GameThreadTime = FPlatformTime::ToMilliseconds(GGameThreadTime);
    CurrentFrameData.RenderThreadTime = FPlatformTime::ToMilliseconds(GRenderThreadTime);
    
    // Update running averages
    FrameRateAccumulator += CurrentFrameData.FrameRate;
    FrameCount++;
    AverageFrameRate = FrameRateAccumulator / FrameCount;
    
    // Update min/max records
    if (CurrentFrameData.FrameRate < MinFrameRateRecorded)
    {
        MinFrameRateRecorded = CurrentFrameData.FrameRate;
    }
    if (CurrentFrameData.FrameRate > MaxFrameRateRecorded)
    {
        MaxFrameRateRecorded = CurrentFrameData.FrameRate;
    }
}

void UPerf_FrameRateManager::UpdateFrameHistory()
{
    FrameHistory.Add(CurrentFrameData);
    
    // Maintain history size limit
    if (FrameHistory.Num() > FrameHistorySize)
    {
        FrameHistory.RemoveAt(0);
    }
}

void UPerf_FrameRateManager::CheckPerformanceThresholds()
{
    float PerformanceRatio = CurrentFrameData.FrameRate / PerformanceSettings.TargetFrameRate;
    bIsPerformanceCritical = PerformanceRatio < PerformanceSettings.OptimizationThreshold;
    
    // Log critical performance issues
    if (bIsPerformanceCritical)
    {
        if (CurrentFrameData.FrameRate < PerformanceSettings.MinFrameRate)
        {
            LogPerformanceWarning(FString::Printf(TEXT("Critical FPS drop: %.1f (target: %.1f, min: %.1f)"), 
                CurrentFrameData.FrameRate, PerformanceSettings.TargetFrameRate, PerformanceSettings.MinFrameRate));
        }
    }
    
    // Check physics actor count
    if (CurrentFrameData.PhysicsActorCount > PerformanceSettings.MaxPhysicsActors)
    {
        LogPerformanceWarning(FString::Printf(TEXT("Physics actor count exceeded: %d (max: %d)"), 
            CurrentFrameData.PhysicsActorCount, PerformanceSettings.MaxPhysicsActors));
    }
}

void UPerf_FrameRateManager::ApplyAutomaticOptimizations()
{
    if (!bIsPerformanceCritical)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("FrameRateManager: Applying automatic optimizations - FPS: %.1f"), CurrentFrameData.FrameRate);
    
    OptimizePhysicsActors();
    OptimizeRenderingSettings();
}

void UPerf_FrameRateManager::OptimizePhysicsActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);
    
    APawn* PlayerPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
    FVector PlayerLocation = PlayerPawn ? PlayerPawn->GetActorLocation() : FVector::ZeroVector;
    
    int32 OptimizedCount = 0;
    for (AActor* Actor : AllActors)
    {
        if (!Actor || !Actor->GetRootComponent())
        {
            continue;
        }
        
        UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
        if (!PrimComp || !PrimComp->IsSimulatingPhysics())
        {
            continue;
        }
        
        float Distance = FVector::Dist(Actor->GetActorLocation(), PlayerLocation);
        if (Distance > PerformanceSettings.PhysicsCullingDistance)
        {
            // Disable physics simulation for distant actors
            PrimComp->SetSimulatePhysics(false);
            OptimizedCount++;
        }
    }
    
    if (OptimizedCount > 0)
    {
        UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Optimized %d physics actors"), OptimizedCount);
    }
}

void UPerf_FrameRateManager::OptimizeRenderingSettings()
{
    // Adjust rendering quality based on performance
    if (CurrentFrameData.FrameRate < PerformanceSettings.MinFrameRate * 0.8f)
    {
        // Severe performance issues - aggressive optimization
        static IConsoleVariable* CVarShadowQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ShadowQuality"));
        if (CVarShadowQuality)
        {
            CVarShadowQuality->Set(1); // Low shadow quality
        }
        
        static IConsoleVariable* CVarPostProcessQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.PostProcessAAQuality"));
        if (CVarPostProcessQuality)
        {
            CVarPostProcessQuality->Set(0); // Disable AA
        }
    }
    else if (CurrentFrameData.FrameRate < PerformanceSettings.MinFrameRate)
    {
        // Moderate performance issues - moderate optimization
        static IConsoleVariable* CVarShadowQuality = IConsoleManager::Get().FindConsoleVariable(TEXT("r.ShadowQuality"));
        if (CVarShadowQuality)
        {
            CVarShadowQuality->Set(2); // Medium shadow quality
        }
    }
}

void UPerf_FrameRateManager::SetTargetFrameRate(float NewTargetFrameRate)
{
    PerformanceSettings.TargetFrameRate = FMath::Clamp(NewTargetFrameRate, 30.0f, 120.0f);
    
    if (GEngine)
    {
        GEngine->SetMaxFPS(PerformanceSettings.TargetFrameRate);
    }
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Target FPS set to %.1f"), PerformanceSettings.TargetFrameRate);
}

void UPerf_FrameRateManager::EnableAutomaticOptimization(bool bEnable)
{
    PerformanceSettings.bEnableAutomaticOptimization = bEnable;
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Automatic optimization %s"), bEnable ? TEXT("enabled") : TEXT("disabled"));
}

void UPerf_FrameRateManager::ResetPerformanceStats()
{
    FrameRateAccumulator = 0.0f;
    FrameCount = 0;
    AverageFrameRate = 0.0f;
    MinFrameRateRecorded = 999.0f;
    MaxFrameRateRecorded = 0.0f;
    FrameHistory.Empty();
    bIsPerformanceCritical = false;
    
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Performance stats reset"));
}

void UPerf_FrameRateManager::ForceOptimizationPass()
{
    UE_LOG(LogTemp, Log, TEXT("FrameRateManager: Forcing optimization pass"));
    ApplyAutomaticOptimizations();
}

void UPerf_FrameRateManager::TestPerformanceBaseline()
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("=== PERFORMANCE BASELINE TEST ==="));
    UE_LOG(LogTemp, Log, TEXT("Current FPS: %.1f"), CurrentFrameData.FrameRate);
    UE_LOG(LogTemp, Log, TEXT("Target FPS: %.1f"), PerformanceSettings.TargetFrameRate);
    UE_LOG(LogTemp, Log, TEXT("Actor Count: %d"), CurrentFrameData.ActorCount);
    UE_LOG(LogTemp, Log, TEXT("Physics Actors: %d"), CurrentFrameData.PhysicsActorCount);
    UE_LOG(LogTemp, Log, TEXT("Game Thread: %.2fms"), CurrentFrameData.GameThreadTime);
    UE_LOG(LogTemp, Log, TEXT("Render Thread: %.2fms"), CurrentFrameData.RenderThreadTime);
    UE_LOG(LogTemp, Log, TEXT("Performance Critical: %s"), bIsPerformanceCritical ? TEXT("YES") : TEXT("NO"));
    UE_LOG(LogTemp, Log, TEXT("=== END BASELINE TEST ==="));
}

void UPerf_FrameRateManager::LogPerformanceWarning(const FString& Warning)
{
    UE_LOG(LogTemp, Warning, TEXT("FrameRateManager: %s"), *Warning);
    
    // Also display on screen for immediate visibility
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Yellow, 
            FString::Printf(TEXT("PERFORMANCE: %s"), *Warning));
    }
}