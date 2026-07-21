#include "Core_PhysicsSimulationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Chaos/ChaosEngineInterface.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

UCore_PhysicsSimulationManager::UCore_PhysicsSimulationManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update 10 times per second
    
    // Initialize default settings
    SimulationSettings = FCore_SimulationSettings();
    bEnablePhysicsSimulation = true;
    bEnableAsyncPhysics = true;
    bEnableSubstepping = true;
    
    // Initialize performance tracking
    CurrentSimulatedBodies = 0;
    CurrentPhysicsTime = 0.0f;
    AveragePhysicsTime = 0.0f;
    PhysicsTimeAccumulator = 0.0f;
    PhysicsFrameCounter = 0;
    LastPerformanceCheck = 0.0f;
    
    // Reserve space for performance samples
    PhysicsTimeSamples.Reserve(60); // 6 seconds of samples at 10Hz
}

void UCore_PhysicsSimulationManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulationManager: BeginPlay - Initializing physics simulation manager"));
    
    // Apply initial simulation settings
    ApplySimulationSettings();
    
    // Start performance monitoring
    LastPerformanceCheck = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulationManager: Initialized with quality %d, mode %d"), 
           (int32)SimulationSettings.Quality, (int32)SimulationSettings.PhysicsMode);
}

void UCore_PhysicsSimulationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bEnablePhysicsSimulation || bSimulationPaused)
    {
        return;
    }
    
    // Monitor performance
    MonitorPerformance(DeltaTime);
    
    // Update performance metrics periodically
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastPerformanceCheck >= PerformanceCheckInterval)
    {
        UpdatePerformanceMetrics();
        AdjustQualityBasedOnPerformance();
        LastPerformanceCheck = CurrentTime;
    }
}

void UCore_PhysicsSimulationManager::SetSimulationQuality(ECore_SimulationQuality NewQuality)
{
    if (SimulationSettings.Quality != NewQuality)
    {
        SimulationSettings.Quality = NewQuality;
        ApplySimulationSettings();
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulationManager: Simulation quality changed to %d"), (int32)NewQuality);
    }
}

void UCore_PhysicsSimulationManager::SetPhysicsMode(ECore_PhysicsMode NewMode)
{
    if (SimulationSettings.PhysicsMode != NewMode)
    {
        SimulationSettings.PhysicsMode = NewMode;
        ApplySimulationSettings();
        
        UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulationManager: Physics mode changed to %d"), (int32)NewMode);
    }
}

void UCore_PhysicsSimulationManager::ApplySimulationSettings()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSimulationManager: Cannot apply settings - no world"));
        return;
    }
    
    // Update physics settings based on quality level
    UpdatePhysicsSettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulationManager: Applied simulation settings - Quality: %d, Mode: %d"), 
           (int32)SimulationSettings.Quality, (int32)SimulationSettings.PhysicsMode);
}

void UCore_PhysicsSimulationManager::UpdatePhysicsSettings()
{
    // Adjust settings based on quality level
    switch (SimulationSettings.Quality)
    {
        case ECore_SimulationQuality::Low:
            SimulationSettings.TimeStep = 0.02f; // 50 FPS
            SimulationSettings.MaxSubSteps = 3;
            SimulationSettings.MaxSubStepDeltaTime = 0.01f;
            SimulationSettings.MaxSimulatedBodies = 500;
            SimulationSettings.CullingDistance = 3000.0f;
            break;
            
        case ECore_SimulationQuality::Medium:
            SimulationSettings.TimeStep = 0.016667f; // 60 FPS
            SimulationSettings.MaxSubSteps = 6;
            SimulationSettings.MaxSubStepDeltaTime = 0.008333f;
            SimulationSettings.MaxSimulatedBodies = 1000;
            SimulationSettings.CullingDistance = 5000.0f;
            break;
            
        case ECore_SimulationQuality::High:
            SimulationSettings.TimeStep = 0.0125f; // 80 FPS
            SimulationSettings.MaxSubSteps = 8;
            SimulationSettings.MaxSubStepDeltaTime = 0.00625f;
            SimulationSettings.MaxSimulatedBodies = 1500;
            SimulationSettings.CullingDistance = 7000.0f;
            break;
            
        case ECore_SimulationQuality::Ultra:
            SimulationSettings.TimeStep = 0.008333f; // 120 FPS
            SimulationSettings.MaxSubSteps = 12;
            SimulationSettings.MaxSubStepDeltaTime = 0.004167f;
            SimulationSettings.MaxSimulatedBodies = 2000;
            SimulationSettings.CullingDistance = 10000.0f;
            break;
    }
    
    // Adjust settings based on physics mode
    switch (SimulationSettings.PhysicsMode)
    {
        case ECore_PhysicsMode::Realistic:
            // Use default realistic settings
            break;
            
        case ECore_PhysicsMode::Arcade:
            // Reduce gravity, increase bounce
            SimulationSettings.MaxSubSteps = FMath::Max(1, SimulationSettings.MaxSubSteps / 2);
            break;
            
        case ECore_PhysicsMode::Cinematic:
            // Optimize for visual appeal over accuracy
            SimulationSettings.MaxSubSteps = FMath::Max(2, SimulationSettings.MaxSubSteps / 3);
            break;
    }
}

void UCore_PhysicsSimulationManager::PausePhysicsSimulation()
{
    bSimulationPaused = true;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulationManager: Physics simulation paused"));
}

void UCore_PhysicsSimulationManager::ResumePhysicsSimulation()
{
    bSimulationPaused = false;
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulationManager: Physics simulation resumed"));
}

void UCore_PhysicsSimulationManager::ResetPhysicsSimulation()
{
    // Reset performance tracking
    PhysicsTimeAccumulator = 0.0f;
    PhysicsFrameCounter = 0;
    PhysicsTimeSamples.Empty();
    CurrentPhysicsTime = 0.0f;
    AveragePhysicsTime = 0.0f;
    
    // Resume simulation if paused
    bSimulationPaused = false;
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsSimulationManager: Physics simulation reset"));
}

void UCore_PhysicsSimulationManager::MonitorPerformance(float DeltaTime)
{
    // Track physics timing
    PhysicsTimeAccumulator += DeltaTime;
    PhysicsFrameCounter++;
    
    // Add sample to rolling window
    PhysicsTimeSamples.Add(DeltaTime);
    if (PhysicsTimeSamples.Num() > 60)
    {
        PhysicsTimeSamples.RemoveAt(0);
    }
    
    // Update current physics time
    CurrentPhysicsTime = DeltaTime;
}

void UCore_PhysicsSimulationManager::UpdatePerformanceMetrics()
{
    if (PhysicsFrameCounter > 0)
    {
        AveragePhysicsTime = PhysicsTimeAccumulator / PhysicsFrameCounter;
    }
    
    // Count simulated bodies in the world
    UWorld* World = GetWorld();
    if (World)
    {
        CurrentSimulatedBodies = 0;
        for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
        {
            AActor* Actor = *ActorItr;
            if (Actor && Actor->GetRootComponent())
            {
                UPrimitiveComponent* PrimComp = Cast<UPrimitiveComponent>(Actor->GetRootComponent());
                if (PrimComp && PrimComp->IsSimulatingPhysics())
                {
                    CurrentSimulatedBodies++;
                }
            }
        }
    }
}

void UCore_PhysicsSimulationManager::AdjustQualityBasedOnPerformance()
{
    if (!IsPhysicsPerformanceGood())
    {
        ECore_SimulationQuality OptimalQuality = GetOptimalQualityForPerformance();
        if (OptimalQuality != SimulationSettings.Quality)
        {
            UE_LOG(LogTemp, Warning, TEXT("Core_PhysicsSimulationManager: Auto-adjusting quality from %d to %d due to performance"), 
                   (int32)SimulationSettings.Quality, (int32)OptimalQuality);
            SetSimulationQuality(OptimalQuality);
        }
    }
}

ECore_SimulationQuality UCore_PhysicsSimulationManager::GetOptimalQualityForPerformance() const
{
    float FrameRate = GetPhysicsFrameRate();
    
    if (FrameRate >= 60.0f)
    {
        return ECore_SimulationQuality::Ultra;
    }
    else if (FrameRate >= 45.0f)
    {
        return ECore_SimulationQuality::High;
    }
    else if (FrameRate >= 30.0f)
    {
        return ECore_SimulationQuality::Medium;
    }
    else
    {
        return ECore_SimulationQuality::Low;
    }
}

void UCore_PhysicsSimulationManager::OptimizeForPerformance()
{
    // Reduce quality if performance is poor
    if (!IsPhysicsPerformanceGood())
    {
        ECore_SimulationQuality NewQuality = SimulationSettings.Quality;
        
        switch (SimulationSettings.Quality)
        {
            case ECore_SimulationQuality::Ultra:
                NewQuality = ECore_SimulationQuality::High;
                break;
            case ECore_SimulationQuality::High:
                NewQuality = ECore_SimulationQuality::Medium;
                break;
            case ECore_SimulationQuality::Medium:
                NewQuality = ECore_SimulationQuality::Low;
                break;
            case ECore_SimulationQuality::Low:
                // Already at lowest quality
                break;
        }
        
        if (NewQuality != SimulationSettings.Quality)
        {
            SetSimulationQuality(NewQuality);
        }
    }
}

float UCore_PhysicsSimulationManager::GetPhysicsFrameRate() const
{
    if (AveragePhysicsTime > 0.0f)
    {
        return 1.0f / AveragePhysicsTime;
    }
    return 0.0f;
}

bool UCore_PhysicsSimulationManager::IsPhysicsPerformanceGood() const
{
    float FrameRate = GetPhysicsFrameRate();
    return FrameRate >= 30.0f && CurrentSimulatedBodies <= SimulationSettings.MaxSimulatedBodies;
}

void UCore_PhysicsSimulationManager::LogSimulationStatus()
{
    UE_LOG(LogTemp, Log, TEXT("=== Physics Simulation Status ==="));
    UE_LOG(LogTemp, Log, TEXT("Quality: %d"), (int32)SimulationSettings.Quality);
    UE_LOG(LogTemp, Log, TEXT("Mode: %d"), (int32)SimulationSettings.PhysicsMode);
    UE_LOG(LogTemp, Log, TEXT("Enabled: %s"), bEnablePhysicsSimulation ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Paused: %s"), bSimulationPaused ? TEXT("Yes") : TEXT("No"));
    UE_LOG(LogTemp, Log, TEXT("Simulated Bodies: %d"), CurrentSimulatedBodies);
    UE_LOG(LogTemp, Log, TEXT("Frame Rate: %.2f FPS"), GetPhysicsFrameRate());
    UE_LOG(LogTemp, Log, TEXT("Performance Good: %s"), IsPhysicsPerformanceGood() ? TEXT("Yes") : TEXT("No"));
}

void UCore_PhysicsSimulationManager::DumpPhysicsStats()
{
    FString StatsString = FString::Printf(TEXT("Physics Stats - Quality: %d, Bodies: %d, FPS: %.2f, Time: %.4fms"), 
                                        (int32)SimulationSettings.Quality, 
                                        CurrentSimulatedBodies, 
                                        GetPhysicsFrameRate(), 
                                        CurrentPhysicsTime * 1000.0f);
    
    UE_LOG(LogTemp, Log, TEXT("%s"), *StatsString);
    
    // Also log to screen if available
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, StatsString);
    }
}