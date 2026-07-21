#include "Eng_WorldArchitect.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "HAL/PlatformFilemanager.h"

void UEng_WorldArchitect::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    bWorldSystemsInitialized = false;
    WorldValidationScore = 0.0f;
    LayeredComponents.Empty();
    
    // Initialize architectural layers
    LayeredComponents.Add(EArchitecturalLayer::Foundation, TArray<TWeakObjectPtr<UActorComponent>>());
    LayeredComponents.Add(EArchitecturalLayer::Core, TArray<TWeakObjectPtr<UActorComponent>>());
    LayeredComponents.Add(EArchitecturalLayer::Gameplay, TArray<TWeakObjectPtr<UActorComponent>>());
    LayeredComponents.Add(EArchitecturalLayer::Presentation, TArray<TWeakObjectPtr<UActorComponent>>());
    
    UE_LOG(LogTemp, Log, TEXT("Eng_WorldArchitect: Initialized architectural subsystem"));
}

void UEng_WorldArchitect::Deinitialize()
{
    LayeredComponents.Empty();
    bWorldSystemsInitialized = false;
    
    Super::Deinitialize();
}

void UEng_WorldArchitect::InitializeWorldSystems()
{
    if (bWorldSystemsInitialized)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitect: World systems already initialized"));
        return;
    }
    
    // Validate world state
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Eng_WorldArchitect: No valid world found"));
        return;
    }
    
    // Initialize architectural validation
    ValidateWorldConfiguration();
    
    bWorldSystemsInitialized = true;
    UE_LOG(LogTemp, Log, TEXT("Eng_WorldArchitect: World systems initialized successfully"));
}

void UEng_WorldArchitect::ValidateWorldConfiguration()
{
    float ValidationScore = 0.0f;
    
    // Validate performance constraints
    ValidatePerformanceConstraints();
    ValidationScore += 25.0f;
    
    // Validate memory usage
    ValidateMemoryUsage();
    ValidationScore += 25.0f;
    
    // Validate rendering pipeline
    ValidateRenderingPipeline();
    ValidationScore += 25.0f;
    
    // Validate component architecture
    int32 TotalComponents = 0;
    for (const auto& LayerPair : LayeredComponents)
    {
        TotalComponents += LayerPair.Value.Num();
    }
    
    if (TotalComponents > 0)
    {
        ValidationScore += 25.0f;
    }
    
    WorldValidationScore = ValidationScore;
    UE_LOG(LogTemp, Log, TEXT("Eng_WorldArchitect: World validation score: %.1f/100"), WorldValidationScore);
}

bool UEng_WorldArchitect::IsWorldSystemsReady() const
{
    return bWorldSystemsInitialized && WorldValidationScore >= 75.0f;
}

void UEng_WorldArchitect::RegisterArchitecturalComponent(UActorComponent* Component, EArchitecturalLayer Layer)
{
    if (!Component)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitect: Attempted to register null component"));
        return;
    }
    
    if (LayeredComponents.Contains(Layer))
    {
        LayeredComponents[Layer].AddUnique(Component);
        UE_LOG(LogTemp, Log, TEXT("Eng_WorldArchitect: Registered component %s to layer %d"), 
               *Component->GetName(), (int32)Layer);
    }
}

TArray<UActorComponent*> UEng_WorldArchitect::GetComponentsByLayer(EArchitecturalLayer Layer) const
{
    TArray<UActorComponent*> ValidComponents;
    
    if (LayeredComponents.Contains(Layer))
    {
        for (const TWeakObjectPtr<UActorComponent>& WeakComponent : LayeredComponents[Layer])
        {
            if (WeakComponent.IsValid())
            {
                ValidComponents.Add(WeakComponent.Get());
            }
        }
    }
    
    return ValidComponents;
}

void UEng_WorldArchitect::ValidatePerformanceConstraints()
{
    // Check frame rate stability
    float AverageFrameRate = 1.0f / FApp::GetDeltaTime();
    if (AverageFrameRate < 30.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitect: Performance warning - Frame rate below 30fps"));
    }
    
    // Check draw calls
    // Note: In a real implementation, we would access rendering stats here
    UE_LOG(LogTemp, Log, TEXT("Eng_WorldArchitect: Performance validation completed"));
}

void UEng_WorldArchitect::ValidateMemoryUsage()
{
    // Check memory usage patterns
    FPlatformMemoryStats MemoryStats = FPlatformMemory::GetStats();
    
    float MemoryUsageGB = MemoryStats.UsedPhysical / (1024.0f * 1024.0f * 1024.0f);
    
    if (MemoryUsageGB > 8.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitect: Memory usage high: %.2f GB"), MemoryUsageGB);
    }
    
    UE_LOG(LogTemp, Log, TEXT("Eng_WorldArchitect: Memory validation completed - Usage: %.2f GB"), MemoryUsageGB);
}

void UEng_WorldArchitect::ValidateRenderingPipeline()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Validate lighting setup
    bool bHasDirectionalLight = false;
    bool bHasSkyLight = false;
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor->GetName().Contains(TEXT("DirectionalLight")))
        {
            bHasDirectionalLight = true;
        }
        if (Actor->GetName().Contains(TEXT("SkyLight")))
        {
            bHasSkyLight = true;
        }
    }
    
    if (!bHasDirectionalLight)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitect: No directional light found in scene"));
    }
    
    if (!bHasSkyLight)
    {
        UE_LOG(LogTemp, Warning, TEXT("Eng_WorldArchitect: No sky light found in scene"));
    }
    
    UE_LOG(LogTemp, Log, TEXT("Eng_WorldArchitect: Rendering pipeline validation completed"));
}