#include "IntegrationSystemManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/PlayerStart.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Character.h"
#include "AIController.h"

UIntegrationSystemManager::UIntegrationSystemManager()
{
    LastValidationTime = 0.0f;
    ValidationInterval = 5.0f; // Validate every 5 seconds
    bAutoCleanupEnabled = true;
    AverageFramerate = 60.0f;
    LastActorCount = 0;
    LastOptimizationTime = 0.0f;
}

void UIntegrationSystemManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Initialized"));
    
    // Initialize system statuses
    SystemStatuses.Empty();
    
    // Register core systems
    FInteg_SystemStatus WorldGenStatus;
    WorldGenStatus.SystemName = TEXT("WorldGeneration");
    SystemStatuses.Add(TEXT("WorldGeneration"), WorldGenStatus);
    
    FInteg_SystemStatus CharacterStatus;
    CharacterStatus.SystemName = TEXT("Characters");
    SystemStatuses.Add(TEXT("Characters"), CharacterStatus);
    
    FInteg_SystemStatus AIStatus;
    AIStatus.SystemName = TEXT("AI");
    SystemStatuses.Add(TEXT("AI"), AIStatus);
    
    FInteg_SystemStatus LightingStatus;
    LightingStatus.SystemName = TEXT("Lighting");
    SystemStatuses.Add(TEXT("Lighting"), LightingStatus);
    
    FInteg_SystemStatus VFXStatus;
    VFXStatus.SystemName = TEXT("VFX");
    SystemStatuses.Add(TEXT("VFX"), VFXStatus);
    
    // Perform initial validation
    ValidateAllSystems();
}

void UIntegrationSystemManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Deinitializing"));
    Super::Deinitialize();
}

void UIntegrationSystemManager::ValidateAllSystems()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("IntegrationSystemManager: No valid world"));
        return;
    }
    
    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastValidationTime < ValidationInterval)
    {
        return; // Too soon for another validation
    }
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Starting system validation"));
    
    // Validate all systems
    ValidateWorldGeneration();
    ValidateCharacterSystems();
    ValidateAISystems();
    ValidateLightingSystems();
    ValidateVFXSystems();
    
    // Update performance metrics
    UpdatePerformanceMetrics();
    
    // Auto-cleanup if enabled
    if (bAutoCleanupEnabled)
    {
        CleanupDuplicateActors();
    }
    
    LastValidationTime = CurrentTime;
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: System validation complete"));
}

void UIntegrationSystemManager::CleanupDuplicateActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Starting duplicate actor cleanup"));
    
    CleanupLightingActors();
    CleanupAtmosphereActors();
    ValidateEssentialActors();
    
    UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Duplicate cleanup complete"));
}

FInteg_ActorInventory UIntegrationSystemManager::GetActorInventory()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return FInteg_ActorInventory();
    }
    
    FInteg_ActorInventory Inventory;
    
    // Count all actors
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (!Actor)
        {
            continue;
        }
        
        Inventory.TotalActors++;
        
        // Count specific types
        if (Actor->IsA<ADirectionalLight>())
        {
            Inventory.DirectionalLights++;
        }
        else if (Actor->IsA<ASkyLight>())
        {
            Inventory.SkyLights++;
        }
        else if (Actor->IsA<APlayerStart>())
        {
            Inventory.PlayerStarts++;
        }
        else if (Actor->IsA<ACharacter>())
        {
            // Check if it's a dinosaur (has "Dino" or "Rex" in name)
            FString ActorName = Actor->GetName();
            if (ActorName.Contains(TEXT("Dino")) || ActorName.Contains(TEXT("Rex")) || ActorName.Contains(TEXT("Raptor")))
            {
                Inventory.DinosaurActors++;
            }
        }
        
        // Count components
        if (Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            Inventory.AtmosphereComponents++;
        }
        
        if (Actor->FindComponentByClass<UExponentialHeightFogComponent>())
        {
            Inventory.FogComponents++;
        }
    }
    
    CachedInventory = Inventory;
    return Inventory;
}

TArray<FInteg_SystemStatus> UIntegrationSystemManager::GetSystemStatuses()
{
    TArray<FInteg_SystemStatus> Statuses;
    
    for (auto& StatusPair : SystemStatuses)
    {
        Statuses.Add(StatusPair.Value);
    }
    
    return Statuses;
}

bool UIntegrationSystemManager::IsSystemHealthy(const FString& SystemName)
{
    if (SystemStatuses.Contains(SystemName))
    {
        return SystemStatuses[SystemName].bIsHealthy;
    }
    
    return false;
}

void UIntegrationSystemManager::ForceSystemValidation(const FString& SystemName)
{
    if (SystemName == TEXT("WorldGeneration"))
    {
        ValidateWorldGeneration();
    }
    else if (SystemName == TEXT("Characters"))
    {
        ValidateCharacterSystems();
    }
    else if (SystemName == TEXT("AI"))
    {
        ValidateAISystems();
    }
    else if (SystemName == TEXT("Lighting"))
    {
        ValidateLightingSystems();
    }
    else if (SystemName == TEXT("VFX"))
    {
        ValidateVFXSystems();
    }
}

float UIntegrationSystemManager::GetCurrentFramerate()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        return 1.0f / GEngine->GetGameViewport()->GetWorld()->GetDeltaSeconds();
    }
    
    return AverageFramerate;
}

int32 UIntegrationSystemManager::GetActorCount()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        Count++;
    }
    
    LastActorCount = Count;
    return Count;
}

void UIntegrationSystemManager::OptimizeLevel()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    float CurrentTime = World->GetTimeSeconds();
    if (CurrentTime - LastOptimizationTime < 30.0f)
    {
        return; // Don't optimize too frequently
    }
    
    UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Starting level optimization"));
    
    // Force garbage collection
    GEngine->ForceGarbageCollection(true);
    
    // Cleanup duplicate actors
    CleanupDuplicateActors();
    
    LastOptimizationTime = CurrentTime;
    
    UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Level optimization complete"));
}

void UIntegrationSystemManager::ValidateWorldGeneration()
{
    FInteg_SystemStatus& Status = SystemStatuses[TEXT("WorldGeneration")];
    Status.bIsHealthy = true;
    Status.LastError = TEXT("");
    Status.LastCheckTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // Basic world validation - check if we have terrain
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsHealthy = false;
        Status.LastError = TEXT("No valid world");
        return;
    }
    
    // Check for landscape or terrain actors
    bool bHasTerrain = false;
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && (Actor->GetName().Contains(TEXT("Landscape")) || Actor->GetName().Contains(TEXT("Terrain"))))
        {
            bHasTerrain = true;
            break;
        }
    }
    
    if (!bHasTerrain)
    {
        Status.bIsHealthy = false;
        Status.LastError = TEXT("No terrain found");
    }
}

void UIntegrationSystemManager::ValidateCharacterSystems()
{
    FInteg_SystemStatus& Status = SystemStatuses[TEXT("Characters")];
    Status.bIsHealthy = true;
    Status.LastError = TEXT("");
    Status.LastCheckTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsHealthy = false;
        Status.LastError = TEXT("No valid world");
        return;
    }
    
    // Check for player start
    bool bHasPlayerStart = false;
    for (TActorIterator<APlayerStart> PlayerStartItr(World); PlayerStartItr; ++PlayerStartItr)
    {
        bHasPlayerStart = true;
        break;
    }
    
    if (!bHasPlayerStart)
    {
        Status.bIsHealthy = false;
        Status.LastError = TEXT("No PlayerStart found");
    }
}

void UIntegrationSystemManager::ValidateAISystems()
{
    FInteg_SystemStatus& Status = SystemStatuses[TEXT("AI")];
    Status.bIsHealthy = true;
    Status.LastError = TEXT("");
    Status.LastCheckTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // AI validation - check for AI controllers
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsHealthy = false;
        Status.LastError = TEXT("No valid world");
        return;
    }
    
    // Count AI controllers
    int32 AIControllerCount = 0;
    for (TActorIterator<AAIController> AIItr(World); AIItr; ++AIItr)
    {
        AIControllerCount++;
    }
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Found %d AI controllers"), AIControllerCount);
}

void UIntegrationSystemManager::ValidateLightingSystems()
{
    FInteg_SystemStatus& Status = SystemStatuses[TEXT("Lighting")];
    Status.bIsHealthy = true;
    Status.LastError = TEXT("");
    Status.LastCheckTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsHealthy = false;
        Status.LastError = TEXT("No valid world");
        return;
    }
    
    // Check for essential lighting
    bool bHasDirectionalLight = false;
    int32 DirectionalLightCount = 0;
    
    for (TActorIterator<ADirectionalLight> LightItr(World); LightItr; ++LightItr)
    {
        bHasDirectionalLight = true;
        DirectionalLightCount++;
    }
    
    if (!bHasDirectionalLight)
    {
        Status.bIsHealthy = false;
        Status.LastError = TEXT("No directional light found");
    }
    else if (DirectionalLightCount > 1)
    {
        Status.bIsHealthy = false;
        Status.LastError = FString::Printf(TEXT("Too many directional lights: %d"), DirectionalLightCount);
    }
}

void UIntegrationSystemManager::ValidateVFXSystems()
{
    FInteg_SystemStatus& Status = SystemStatuses[TEXT("VFX")];
    Status.bIsHealthy = true;
    Status.LastError = TEXT("");
    Status.LastCheckTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    
    // VFX validation - basic check for now
    UWorld* World = GetWorld();
    if (!World)
    {
        Status.bIsHealthy = false;
        Status.LastError = TEXT("No valid world");
        return;
    }
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: VFX system validation complete"));
}

void UIntegrationSystemManager::CleanupLightingActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<ADirectionalLight*> DirectionalLights;
    
    // Collect all directional lights
    for (TActorIterator<ADirectionalLight> LightItr(World); LightItr; ++LightItr)
    {
        DirectionalLights.Add(*LightItr);
    }
    
    // Keep only the first one, destroy the rest
    if (DirectionalLights.Num() > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Found %d directional lights, keeping 1"), DirectionalLights.Num());
        
        for (int32 i = 1; i < DirectionalLights.Num(); i++)
        {
            if (DirectionalLights[i])
            {
                DirectionalLights[i]->Destroy();
                UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Destroyed duplicate directional light"));
            }
        }
    }
}

void UIntegrationSystemManager::CleanupAtmosphereActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    TArray<AActor*> SkyLights;
    TArray<AActor*> AtmosphereActors;
    
    // Collect sky lights and atmosphere actors
    for (TActorIterator<ASkyLight> SkyLightItr(World); SkyLightItr; ++SkyLightItr)
    {
        SkyLights.Add(*SkyLightItr);
    }
    
    for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && Actor->FindComponentByClass<USkyAtmosphereComponent>())
        {
            AtmosphereActors.Add(Actor);
        }
    }
    
    // Cleanup excess sky lights
    if (SkyLights.Num() > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Found %d sky lights, keeping 1"), SkyLights.Num());
        
        for (int32 i = 1; i < SkyLights.Num(); i++)
        {
            if (SkyLights[i])
            {
                SkyLights[i]->Destroy();
            }
        }
    }
    
    // Cleanup excess atmosphere actors
    if (AtmosphereActors.Num() > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("IntegrationSystemManager: Found %d atmosphere actors, keeping 1"), AtmosphereActors.Num());
        
        for (int32 i = 1; i < AtmosphereActors.Num(); i++)
        {
            if (AtmosphereActors[i])
            {
                AtmosphereActors[i]->Destroy();
            }
        }
    }
}

void UIntegrationSystemManager::ValidateEssentialActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Ensure we have essential actors
    bool bHasPlayerStart = false;
    
    for (TActorIterator<APlayerStart> PlayerStartItr(World); PlayerStartItr; ++PlayerStartItr)
    {
        bHasPlayerStart = true;
        break;
    }
    
    if (!bHasPlayerStart)
    {
        UE_LOG(LogTemp, Error, TEXT("IntegrationSystemManager: No PlayerStart found - level may not be playable"));
    }
}

void UIntegrationSystemManager::UpdatePerformanceMetrics()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    // Update framerate
    float CurrentFramerate = GetCurrentFramerate();
    AverageFramerate = (AverageFramerate * 0.9f) + (CurrentFramerate * 0.1f);
    
    // Update actor count
    GetActorCount();
    
    UE_LOG(LogTemp, Log, TEXT("IntegrationSystemManager: Performance - FPS: %.1f, Actors: %d"), AverageFramerate, LastActorCount);
}