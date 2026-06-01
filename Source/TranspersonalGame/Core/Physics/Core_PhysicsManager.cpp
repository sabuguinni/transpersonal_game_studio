#include "Core_PhysicsManager.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/WorldSettings.h"

UCore_PhysicsManager::UCore_PhysicsManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickGroup = TG_PrePhysics;
    
    PhysicsSettings = FCore_PhysicsSettings();
    bEnablePhysicsOptimization = true;
    PhysicsUpdateRate = 60.0f;
    CurrentPhysicsTime = 0.0f;
    ActivePhysicsBodies = 0;
    AveragePhysicsTime = 0.0f;
    
    PhysicsTimeSamples.Reserve(60); // 1 second of samples at 60fps
}

void UCore_PhysicsManager::BeginPlay()
{
    Super::BeginPlay();
    
    ApplyQualitySettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Initialized with quality level %d"), 
           (int32)PhysicsSettings.QualityLevel);
}

void UCore_PhysicsManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (bEnablePhysicsOptimization)
    {
        UpdatePerformanceMetrics(DeltaTime);
        MonitorPhysicsPerformance();
    }
}

void UCore_PhysicsManager::SetPhysicsQuality(ECore_PhysicsQuality NewQuality)
{
    PhysicsSettings.QualityLevel = NewQuality;
    ApplyQualitySettings();
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Physics quality changed to %d"), (int32)NewQuality);
}

void UCore_PhysicsManager::ApplyPhysicsSettings(const FCore_PhysicsSettings& NewSettings)
{
    PhysicsSettings = NewSettings;
    ApplyQualitySettings();
    
    UWorld* World = GetWorld();
    if (World && World->GetWorldSettings())
    {
        AWorldSettings* WorldSettings = World->GetWorldSettings();
        WorldSettings->GlobalGravityZ = -980.0f * PhysicsSettings.GravityScale;
    }
}

void UCore_PhysicsManager::SetGlobalGravityScale(float NewGravityScale)
{
    PhysicsSettings.GravityScale = FMath::Clamp(NewGravityScale, 0.1f, 5.0f);
    
    UWorld* World = GetWorld();
    if (World && World->GetWorldSettings())
    {
        AWorldSettings* WorldSettings = World->GetWorldSettings();
        WorldSettings->GlobalGravityZ = -980.0f * PhysicsSettings.GravityScale;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Core_PhysicsManager: Gravity scale set to %f"), PhysicsSettings.GravityScale);
}

float UCore_PhysicsManager::GetGlobalGravityScale() const
{
    return PhysicsSettings.GravityScale;
}

void UCore_PhysicsManager::OptimizePhysicsPerformance()
{
    if (ShouldReducePhysicsQuality())
    {
        // Automatically reduce quality if performance is poor
        switch (PhysicsSettings.QualityLevel)
        {
            case ECore_PhysicsQuality::Ultra:
                SetPhysicsQuality(ECore_PhysicsQuality::High);
                break;
            case ECore_PhysicsQuality::High:
                SetPhysicsQuality(ECore_PhysicsQuality::Medium);
                break;
            case ECore_PhysicsQuality::Medium:
                SetPhysicsQuality(ECore_PhysicsQuality::Low);
                break;
            default:
                break;
        }
    }
}

void UCore_PhysicsManager::UpdatePhysicsStatistics()
{
    UWorld* World = GetWorld();
    if (!World) return;
    
    ActivePhysicsBodies = 0;
    
    // Count active physics bodies
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor)
        {
            TArray<UPrimitiveComponent*> PrimitiveComponents;
            Actor->GetComponents<UPrimitiveComponent>(PrimitiveComponents);
            
            for (UPrimitiveComponent* Primitive : PrimitiveComponents)
            {
                if (Primitive && Primitive->IsSimulatingPhysics())
                {
                    ActivePhysicsBodies++;
                }
            }
        }
    }
}

bool UCore_PhysicsManager::ShouldReducePhysicsQuality() const
{
    // Reduce quality if average physics time exceeds threshold
    const float PhysicsTimeThreshold = 1.0f / PhysicsUpdateRate * 0.3f; // 30% of frame budget
    return AveragePhysicsTime > PhysicsTimeThreshold;
}

void UCore_PhysicsManager::DebugPhysicsSettings()
{
    UE_LOG(LogTemp, Warning, TEXT("=== Core Physics Manager Debug ==="));
    UE_LOG(LogTemp, Warning, TEXT("Quality Level: %d"), (int32)PhysicsSettings.QualityLevel);
    UE_LOG(LogTemp, Warning, TEXT("Gravity Scale: %f"), PhysicsSettings.GravityScale);
    UE_LOG(LogTemp, Warning, TEXT("Linear Damping: %f"), PhysicsSettings.LinearDamping);
    UE_LOG(LogTemp, Warning, TEXT("Angular Damping: %f"), PhysicsSettings.AngularDamping);
    UE_LOG(LogTemp, Warning, TEXT("Max Substeps: %d"), PhysicsSettings.MaxSubsteps);
    UE_LOG(LogTemp, Warning, TEXT("Max Substep Delta: %f"), PhysicsSettings.MaxSubstepDeltaTime);
    UE_LOG(LogTemp, Warning, TEXT("Active Physics Bodies: %d"), ActivePhysicsBodies);
    UE_LOG(LogTemp, Warning, TEXT("Average Physics Time: %f ms"), AveragePhysicsTime * 1000.0f);
}

void UCore_PhysicsManager::LogPhysicsStatistics()
{
    UpdatePhysicsStatistics();
    
    UE_LOG(LogTemp, Log, TEXT("Physics Statistics:"));
    UE_LOG(LogTemp, Log, TEXT("- Active Bodies: %d"), ActivePhysicsBodies);
    UE_LOG(LogTemp, Log, TEXT("- Current Physics Time: %f ms"), CurrentPhysicsTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("- Average Physics Time: %f ms"), AveragePhysicsTime * 1000.0f);
    UE_LOG(LogTemp, Log, TEXT("- Physics Update Rate: %f Hz"), PhysicsUpdateRate);
}

void UCore_PhysicsManager::UpdatePerformanceMetrics(float DeltaTime)
{
    // Track physics performance
    PhysicsTimeAccumulator += DeltaTime;
    PhysicsFrameCount++;
    
    // Update samples for rolling average
    PhysicsTimeSamples.Add(DeltaTime);
    if (PhysicsTimeSamples.Num() > 60)
    {
        PhysicsTimeSamples.RemoveAt(0);
    }
    
    // Calculate average every second
    if (PhysicsTimeAccumulator >= 1.0f)
    {
        CurrentPhysicsTime = PhysicsTimeAccumulator / PhysicsFrameCount;
        
        // Calculate rolling average
        float TotalTime = 0.0f;
        for (float Sample : PhysicsTimeSamples)
        {
            TotalTime += Sample;
        }
        AveragePhysicsTime = TotalTime / PhysicsTimeSamples.Num();
        
        PhysicsTimeAccumulator = 0.0f;
        PhysicsFrameCount = 0;
        
        UpdatePhysicsStatistics();
    }
}

void UCore_PhysicsManager::ApplyQualitySettings()
{
    UPhysicsSettings* PhysSettings = UPhysicsSettings::Get();
    if (!PhysSettings) return;
    
    switch (PhysicsSettings.QualityLevel)
    {
        case ECore_PhysicsQuality::Low:
            PhysicsSettings.MaxSubsteps = 2;
            PhysicsSettings.MaxSubstepDeltaTime = 0.033333f; // 30fps
            break;
            
        case ECore_PhysicsQuality::Medium:
            PhysicsSettings.MaxSubsteps = 4;
            PhysicsSettings.MaxSubstepDeltaTime = 0.025f; // 40fps
            break;
            
        case ECore_PhysicsQuality::High:
            PhysicsSettings.MaxSubsteps = 6;
            PhysicsSettings.MaxSubstepDeltaTime = 0.016667f; // 60fps
            break;
            
        case ECore_PhysicsQuality::Ultra:
            PhysicsSettings.MaxSubsteps = 8;
            PhysicsSettings.MaxSubstepDeltaTime = 0.008333f; // 120fps
            break;
    }
}

void UCore_PhysicsManager::MonitorPhysicsPerformance()
{
    static float LastOptimizationTime = 0.0f;
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Only optimize every 5 seconds to avoid oscillation
    if (CurrentTime - LastOptimizationTime > 5.0f)
    {
        if (bEnablePhysicsOptimization)
        {
            OptimizePhysicsPerformance();
        }
        LastOptimizationTime = CurrentTime;
    }
}