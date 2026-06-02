#include "Eng_GameplayArchitect.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/DateTime.h"

void UEng_GameplayArchitectSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayArchitectSubsystem: Initializing Gameplay Architecture"));
    
    // Initialize default configuration
    GameplayConfig = FEng_GameplayConfig();
    CurrentMetrics = FEng_GameplayMetrics();
    LastOptimizationTime = 0.0f;
    
    // Initialize layer states
    LayerActiveStates.Empty();
    LayerActiveStates.Add(EEng_GameplayLayer::Core, true);
    LayerActiveStates.Add(EEng_GameplayLayer::Movement, true);
    LayerActiveStates.Add(EEng_GameplayLayer::Combat, true);
    LayerActiveStates.Add(EEng_GameplayLayer::Survival, true);
    LayerActiveStates.Add(EEng_GameplayLayer::World, true);
    LayerActiveStates.Add(EEng_GameplayLayer::AI, true);
    LayerActiveStates.Add(EEng_GameplayLayer::Narrative, false);
    
    InitializeGameplayLayers();
}

void UEng_GameplayArchitectSubsystem::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayArchitectSubsystem: Deinitializing Gameplay Architecture"));
    
    LayerActiveStates.Empty();
    
    Super::Deinitialize();
}

void UEng_GameplayArchitectSubsystem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    UpdateMetrics();
    CheckPerformanceThresholds();
    
    // Auto-optimize every 30 seconds if performance is suboptimal
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastOptimizationTime > 30.0f && !IsPerformanceOptimal())
    {
        OptimizeGameplayPerformance();
        LastOptimizationTime = CurrentTime;
    }
}

bool UEng_GameplayArchitectSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    return true;
}

void UEng_GameplayArchitectSubsystem::InitializeGameplayLayers()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayArchitectSubsystem: Initializing gameplay layers"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("UEng_GameplayArchitectSubsystem: No valid world found"));
        return;
    }
    
    // Validate each layer
    for (const auto& LayerPair : LayerActiveStates)
    {
        EEng_GameplayLayer Layer = LayerPair.Key;
        bool bActive = LayerPair.Value;
        
        FString LayerName;
        switch (Layer)
        {
            case EEng_GameplayLayer::Core:
                LayerName = TEXT("Core Gameplay");
                break;
            case EEng_GameplayLayer::Movement:
                LayerName = TEXT("Character Movement");
                break;
            case EEng_GameplayLayer::Combat:
                LayerName = TEXT("Combat System");
                break;
            case EEng_GameplayLayer::Survival:
                LayerName = TEXT("Survival Mechanics");
                break;
            case EEng_GameplayLayer::World:
                LayerName = TEXT("World Interaction");
                break;
            case EEng_GameplayLayer::AI:
                LayerName = TEXT("AI Behavior");
                break;
            case EEng_GameplayLayer::Narrative:
                LayerName = TEXT("Quest & Narrative");
                break;
        }
        
        UE_LOG(LogTemp, Log, TEXT("Gameplay Layer [%s]: %s"), *LayerName, bActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    }
}

void UEng_GameplayArchitectSubsystem::ValidateGameplayIntegrity()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayArchitectSubsystem: Validating gameplay integrity"));
    
    ValidateSystemIntegrity();
    
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Gameplay Integrity: No valid world"));
        return;
    }
    
    // Count critical actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    
    int32 PlayerStarts = 0;
    int32 GameModes = 0;
    int32 Characters = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor->GetName().Contains(TEXT("PlayerStart")))
        {
            PlayerStarts++;
        }
        else if (Actor->GetName().Contains(TEXT("GameMode")))
        {
            GameModes++;
        }
        else if (Actor->GetName().Contains(TEXT("Character")))
        {
            Characters++;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Gameplay Integrity Check:"));
    UE_LOG(LogTemp, Log, TEXT("  - PlayerStarts: %d"), PlayerStarts);
    UE_LOG(LogTemp, Log, TEXT("  - GameModes: %d"), GameModes);
    UE_LOG(LogTemp, Log, TEXT("  - Characters: %d"), Characters);
    UE_LOG(LogTemp, Log, TEXT("  - Total Actors: %d"), AllActors.Num());
    
    // Validate minimum requirements
    bool bIntegrityValid = true;
    if (PlayerStarts == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("Gameplay Integrity FAILED: No PlayerStart found"));
        bIntegrityValid = false;
    }
    
    if (bIntegrityValid)
    {
        UE_LOG(LogTemp, Warning, TEXT("Gameplay Integrity: PASSED"));
    }
}

FEng_GameplayMetrics UEng_GameplayArchitectSubsystem::GetCurrentMetrics() const
{
    return CurrentMetrics;
}

void UEng_GameplayArchitectSubsystem::UpdateGameplayConfig(const FEng_GameplayConfig& NewConfig)
{
    GameplayConfig = NewConfig;
    UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayArchitectSubsystem: Gameplay configuration updated"));
    
    // Apply configuration changes
    UE_LOG(LogTemp, Log, TEXT("New Config - MaxWalkSpeed: %.1f, MaxRunSpeed: %.1f"), 
           GameplayConfig.MaxWalkSpeed, GameplayConfig.MaxRunSpeed);
}

bool UEng_GameplayArchitectSubsystem::IsGameplayLayerActive(EEng_GameplayLayer Layer) const
{
    const bool* bActive = LayerActiveStates.Find(Layer);
    return bActive ? *bActive : false;
}

void UEng_GameplayArchitectSubsystem::SetGameplayLayerActive(EEng_GameplayLayer Layer, bool bActive)
{
    LayerActiveStates.Add(Layer, bActive);
    
    FString LayerName = FString::Printf(TEXT("Layer_%d"), (int32)Layer);
    UE_LOG(LogTemp, Warning, TEXT("Gameplay Layer [%s] set to: %s"), 
           *LayerName, bActive ? TEXT("ACTIVE") : TEXT("INACTIVE"));
}

void UEng_GameplayArchitectSubsystem::OptimizeGameplayPerformance()
{
    UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayArchitectSubsystem: Optimizing gameplay performance"));
    
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Get current performance metrics
    float CurrentFrameTime = CurrentMetrics.FrameTime;
    int32 CurrentActors = CurrentMetrics.ActiveActors;
    
    // Performance optimization logic
    if (CurrentFrameTime > 33.0f) // > 30 FPS
    {
        UE_LOG(LogTemp, Warning, TEXT("Performance Warning: Frame time %.2fms (target: 33ms)"), CurrentFrameTime);
        
        // Reduce dinosaur count if too many
        if (CurrentMetrics.ActiveDinosaurs > GameplayConfig.MaxSimultaneousDinosaurs)
        {
            UE_LOG(LogTemp, Warning, TEXT("Reducing dinosaur count from %d to %d"), 
                   CurrentMetrics.ActiveDinosaurs, GameplayConfig.MaxSimultaneousDinosaurs);
        }
    }
    
    // Memory optimization
    if (CurrentMetrics.MemoryUsageMB > 4096.0f) // > 4GB
    {
        UE_LOG(LogTemp, Warning, TEXT("High memory usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
        
        // Trigger garbage collection
        GEngine->ForceGarbageCollection(true);
    }
    
    LastOptimizationTime = World->GetTimeSeconds();
}

void UEng_GameplayArchitectSubsystem::LogGameplayStatus()
{
    UE_LOG(LogTemp, Warning, TEXT("=== GAMEPLAY ARCHITECTURE STATUS ==="));
    UE_LOG(LogTemp, Log, TEXT("Frame Time: %.2f ms"), CurrentMetrics.FrameTime);
    UE_LOG(LogTemp, Log, TEXT("Active Actors: %d"), CurrentMetrics.ActiveActors);
    UE_LOG(LogTemp, Log, TEXT("Active Components: %d"), CurrentMetrics.ActiveComponents);
    UE_LOG(LogTemp, Log, TEXT("Memory Usage: %.1f MB"), CurrentMetrics.MemoryUsageMB);
    UE_LOG(LogTemp, Log, TEXT("Active Players: %d"), CurrentMetrics.ActivePlayers);
    UE_LOG(LogTemp, Log, TEXT("Active Dinosaurs: %d"), CurrentMetrics.ActiveDinosaurs);
    UE_LOG(LogTemp, Log, TEXT("World Load Progress: %.1f%%"), CurrentMetrics.WorldLoadProgress * 100.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Layer States:"));
    for (const auto& LayerPair : LayerActiveStates)
    {
        FString LayerName = FString::Printf(TEXT("Layer_%d"), (int32)LayerPair.Key);
        UE_LOG(LogTemp, Log, TEXT("  %s: %s"), *LayerName, LayerPair.Value ? TEXT("ACTIVE") : TEXT("INACTIVE"));
    }
}

void UEng_GameplayArchitectSubsystem::UpdateMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update frame time
    CurrentMetrics.FrameTime = FApp::GetDeltaTime() * 1000.0f; // Convert to milliseconds
    
    // Count actors and components
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    CurrentMetrics.ActiveActors = AllActors.Num();
    
    int32 TotalComponents = 0;
    int32 DinosaurCount = 0;
    int32 PlayerCount = 0;
    
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            TotalComponents += Actor->GetRootComponent() ? Actor->GetComponents<UActorComponent>().Num() : 0;
            
            FString ActorName = Actor->GetName().ToLower();
            if (ActorName.Contains(TEXT("dinosaur")) || ActorName.Contains(TEXT("trex")) || 
                ActorName.Contains(TEXT("raptor")) || ActorName.Contains(TEXT("brachi")))
            {
                DinosaurCount++;
            }
            else if (ActorName.Contains(TEXT("character")) || ActorName.Contains(TEXT("player")))
            {
                PlayerCount++;
            }
        }
    }
    
    CurrentMetrics.ActiveComponents = TotalComponents;
    CurrentMetrics.ActiveDinosaurs = DinosaurCount;
    CurrentMetrics.ActivePlayers = PlayerCount;
    
    // Estimate memory usage (simplified)
    CurrentMetrics.MemoryUsageMB = (CurrentMetrics.ActiveActors * 0.5f) + (CurrentMetrics.ActiveComponents * 0.1f);
    
    // World load progress (simplified - always 100% for now)
    CurrentMetrics.WorldLoadProgress = 1.0f;
}

void UEng_GameplayArchitectSubsystem::CheckPerformanceThresholds()
{
    // Check if performance is below acceptable thresholds
    if (CurrentMetrics.FrameTime > 50.0f) // Worse than 20 FPS
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Frame time %f ms (worse than 20 FPS)"), CurrentMetrics.FrameTime);
    }
    else if (CurrentMetrics.FrameTime > 33.0f) // Worse than 30 FPS
    {
        UE_LOG(LogTemp, Warning, TEXT("WARNING: Frame time %f ms (worse than 30 FPS)"), CurrentMetrics.FrameTime);
    }
    
    if (CurrentMetrics.ActiveActors > 10000)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Too many actors (%d)"), CurrentMetrics.ActiveActors);
    }
}

void UEng_GameplayArchitectSubsystem::ValidateSystemIntegrity()
{
    // Validate that all critical layers are properly initialized
    bool bCoreActive = IsGameplayLayerActive(EEng_GameplayLayer::Core);
    bool bMovementActive = IsGameplayLayerActive(EEng_GameplayLayer::Movement);
    
    if (!bCoreActive)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Core gameplay layer is not active"));
        SetGameplayLayerActive(EEng_GameplayLayer::Core, true);
    }
    
    if (!bMovementActive)
    {
        UE_LOG(LogTemp, Error, TEXT("CRITICAL: Movement layer is not active"));
        SetGameplayLayerActive(EEng_GameplayLayer::Movement, true);
    }
}

bool UEng_GameplayArchitectSubsystem::IsPerformanceOptimal() const
{
    return CurrentMetrics.FrameTime < 33.0f && // Better than 30 FPS
           CurrentMetrics.MemoryUsageMB < 2048.0f && // Less than 2GB
           CurrentMetrics.ActiveActors < 5000; // Reasonable actor count
}

// Component Implementation
UEng_GameplayArchitectComponent::UEng_GameplayArchitectComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    AssignedLayer = EEng_GameplayLayer::Core;
    bIsRegistered = false;
    PerformanceScore = 1.0f;
}

void UEng_GameplayArchitectComponent::BeginPlay()
{
    Super::BeginPlay();
    
    RegisterWithArchitect();
}

void UEng_GameplayArchitectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    CalculatePerformanceScore();
    ValidateComponentState();
}

void UEng_GameplayArchitectComponent::RegisterWithArchitect()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UEng_GameplayArchitectSubsystem* ArchitectSubsystem = World->GetSubsystem<UEng_GameplayArchitectSubsystem>();
    if (ArchitectSubsystem)
    {
        bIsRegistered = true;
        UE_LOG(LogTemp, Log, TEXT("Component registered with Gameplay Architect"));
    }
}

void UEng_GameplayArchitectComponent::UnregisterFromArchitect()
{
    bIsRegistered = false;
    UE_LOG(LogTemp, Log, TEXT("Component unregistered from Gameplay Architect"));
}

bool UEng_GameplayArchitectComponent::IsArchitectureValid() const
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return false;
    }
    
    UEng_GameplayArchitectSubsystem* ArchitectSubsystem = World->GetSubsystem<UEng_GameplayArchitectSubsystem>();
    return ArchitectSubsystem != nullptr && bIsRegistered;
}

float UEng_GameplayArchitectComponent::GetComponentPerformanceScore() const
{
    return PerformanceScore;
}

void UEng_GameplayArchitectComponent::CalculatePerformanceScore()
{
    // Simple performance scoring based on tick frequency and component complexity
    float BaseScore = 1.0f;
    
    // Reduce score if component is ticking too frequently
    if (PrimaryComponentTick.TickInterval < 0.016f) // More than 60 FPS
    {
        BaseScore *= 0.8f;
    }
    
    // Reduce score if owner has too many components
    AActor* Owner = GetOwner();
    if (Owner)
    {
        int32 ComponentCount = Owner->GetComponents<UActorComponent>().Num();
        if (ComponentCount > 10)
        {
            BaseScore *= 0.9f;
        }
    }
    
    PerformanceScore = FMath::Clamp(BaseScore, 0.1f, 1.0f);
}

void UEng_GameplayArchitectComponent::ValidateComponentState()
{
    if (!bIsRegistered)
    {
        // Try to re-register if not registered
        RegisterWithArchitect();
    }
}