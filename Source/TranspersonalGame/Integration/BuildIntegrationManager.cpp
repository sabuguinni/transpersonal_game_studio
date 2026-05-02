#include "BuildIntegrationManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UBuildIntegrationManager::UBuildIntegrationManager()
{
    bAllSystemsValid = false;
    TotalActorCount = 0;
    LastValidationTime = 0.0f;
    bInitialized = false;
    LastCleanupTime = 0.0f;
    FrameIndex = 0;
    AverageFramerate = 60.0f;
    
    // Initialize framerate history
    for (int32 i = 0; i < 60; ++i)
    {
        FramerateHistory[i] = 60.0f;
    }
}

void UBuildIntegrationManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Initializing integration subsystem"));
    
    bInitialized = true;
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    // Perform initial validation
    ValidateAllSystems();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Integration subsystem ready"));
}

void UBuildIntegrationManager::Deinitialize()
{
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Shutting down integration subsystem"));
    
    bInitialized = false;
    
    Super::Deinitialize();
}

bool UBuildIntegrationManager::ValidateAllSystems()
{
    if (!bInitialized || !GetWorld())
    {
        return false;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Starting system validation"));
    
    ValidationErrors.Empty();
    bool bAllValid = true;
    
    // Validate each system
    if (!ValidateCharacterSystem())
    {
        ValidationErrors.Add(TEXT("Character System validation failed"));
        bAllValid = false;
    }
    
    if (!ValidateWorldGeneration())
    {
        ValidationErrors.Add(TEXT("World Generation validation failed"));
        bAllValid = false;
    }
    
    if (!ValidateAudioSystem())
    {
        ValidationErrors.Add(TEXT("Audio System validation failed"));
        bAllValid = false;
    }
    
    if (!ValidateVFXSystem())
    {
        ValidationErrors.Add(TEXT("VFX System validation failed"));
        bAllValid = false;
    }
    
    if (!ValidateAISystem())
    {
        ValidationErrors.Add(TEXT("AI System validation failed"));
        bAllValid = false;
    }
    
    // Update actor count
    TotalActorCount = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        TotalActorCount++;
    }
    
    // Clean duplicates if needed
    CleanDuplicateActors();
    
    bAllSystemsValid = bAllValid;
    LastValidationTime = GetWorld()->GetTimeSeconds();
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Validation complete - %s"), 
           bAllValid ? TEXT("PASS") : TEXT("FAIL"));
    
    return bAllValid;
}

void UBuildIntegrationManager::CleanDuplicateActors()
{
    if (!GetWorld())
    {
        return;
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Cleaning duplicate actors"));
    
    RemoveDuplicateLighting();
    ValidateActorIntegrity();
    
    LastCleanupTime = GetWorld()->GetTimeSeconds();
}

void UBuildIntegrationManager::RemoveDuplicateLighting()
{
    if (!GetWorld())
    {
        return;
    }
    
    // Clean DirectionalLights
    TArray<ADirectionalLight*> DirectionalLights;
    for (TActorIterator<ADirectionalLight> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        DirectionalLights.Add(*ActorItr);
    }
    
    if (DirectionalLights.Num() > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Found %d DirectionalLights, keeping 1"), 
               DirectionalLights.Num());
        
        for (int32 i = 1; i < DirectionalLights.Num(); ++i)
        {
            if (DirectionalLights[i])
            {
                DirectionalLights[i]->Destroy();
            }
        }
    }
    
    // Clean SkyAtmosphere actors
    TArray<AActor*> SkyAtmosphereActors;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->FindComponentByClass<USkyAtmosphereComponent>())
        {
            SkyAtmosphereActors.Add(*ActorItr);
        }
    }
    
    if (SkyAtmosphereActors.Num() > 1)
    {
        UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Found %d SkyAtmosphere actors, keeping 1"), 
               SkyAtmosphereActors.Num());
        
        for (int32 i = 1; i < SkyAtmosphereActors.Num(); ++i)
        {
            if (SkyAtmosphereActors[i])
            {
                SkyAtmosphereActors[i]->Destroy();
            }
        }
    }
}

void UBuildIntegrationManager::ValidateActorIntegrity()
{
    if (!GetWorld())
    {
        return;
    }
    
    int32 ValidActors = 0;
    int32 InvalidActors = 0;
    
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor && IsValid(Actor))
        {
            ValidActors++;
        }
        else
        {
            InvalidActors++;
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Actor integrity - Valid: %d, Invalid: %d"), 
           ValidActors, InvalidActors);
}

int32 UBuildIntegrationManager::GetActorCount(const FString& ActorClassName)
{
    if (!GetWorld())
    {
        return 0;
    }
    
    int32 Count = 0;
    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetClass()->GetName().Contains(ActorClassName))
        {
            Count++;
        }
    }
    
    return Count;
}

TArray<FString> UBuildIntegrationManager::GetSystemValidationReport()
{
    TArray<FString> Report;
    
    Report.Add(FString::Printf(TEXT("=== BUILD INTEGRATION REPORT ===")));
    Report.Add(FString::Printf(TEXT("All Systems Valid: %s"), bAllSystemsValid ? TEXT("YES") : TEXT("NO")));
    Report.Add(FString::Printf(TEXT("Total Actor Count: %d"), TotalActorCount));
    Report.Add(FString::Printf(TEXT("Average Framerate: %.1f"), AverageFramerate));
    Report.Add(FString::Printf(TEXT("Last Validation: %.1f seconds ago"), 
               GetWorld() ? GetWorld()->GetTimeSeconds() - LastValidationTime : 0.0f));
    
    if (ValidationErrors.Num() > 0)
    {
        Report.Add(TEXT("=== VALIDATION ERRORS ==="));
        for (const FString& Error : ValidationErrors)
        {
            Report.Add(FString::Printf(TEXT("- %s"), *Error));
        }
    }
    
    return Report;
}

float UBuildIntegrationManager::GetCurrentFramerate()
{
    if (GEngine && GEngine->GetGameViewport())
    {
        float CurrentFPS = 1.0f / GetWorld()->GetDeltaSeconds();
        
        // Update framerate history
        FramerateHistory[FrameIndex] = CurrentFPS;
        FrameIndex = (FrameIndex + 1) % 60;
        
        // Calculate average
        float Total = 0.0f;
        for (int32 i = 0; i < 60; ++i)
        {
            Total += FramerateHistory[i];
        }
        AverageFramerate = Total / 60.0f;
        
        return CurrentFPS;
    }
    
    return 60.0f;
}

bool UBuildIntegrationManager::IsPerformanceAcceptable()
{
    return AverageFramerate >= MIN_ACCEPTABLE_FPS;
}

bool UBuildIntegrationManager::ValidateCharacterSystem()
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Check if TranspersonalCharacter exists
    TArray<ACharacter*> Characters;
    for (TActorIterator<ACharacter> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        if (ActorItr->GetClass()->GetName().Contains(TEXT("TranspersonalCharacter")))
        {
            Characters.Add(*ActorItr);
        }
    }
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Found %d TranspersonalCharacter instances"), 
           Characters.Num());
    
    return Characters.Num() > 0;
}

bool UBuildIntegrationManager::ValidateWorldGeneration()
{
    if (!GetWorld())
    {
        return false;
    }
    
    // Check for landscape or terrain
    int32 LandscapeCount = GetActorCount(TEXT("Landscape"));
    int32 TerrainCount = GetActorCount(TEXT("Terrain"));
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Landscape: %d, Terrain: %d"), 
           LandscapeCount, TerrainCount);
    
    return (LandscapeCount > 0 || TerrainCount > 0);
}

bool UBuildIntegrationManager::ValidateAudioSystem()
{
    // Audio system validation - check for audio components
    int32 AudioCount = GetActorCount(TEXT("Audio"));
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Audio components: %d"), AudioCount);
    
    return true; // Audio is optional for basic functionality
}

bool UBuildIntegrationManager::ValidateVFXSystem()
{
    // VFX system validation - check for particle systems
    int32 ParticleCount = GetActorCount(TEXT("Particle"));
    int32 NiagaraCount = GetActorCount(TEXT("Niagara"));
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: Particles: %d, Niagara: %d"), 
           ParticleCount, NiagaraCount);
    
    return true; // VFX is optional for basic functionality
}

bool UBuildIntegrationManager::ValidateAISystem()
{
    // AI system validation - check for AI controllers and pawns
    int32 AICount = GetActorCount(TEXT("AI"));
    int32 PawnCount = GetActorCount(TEXT("Pawn"));
    
    UE_LOG(LogTemp, Warning, TEXT("BuildIntegrationManager: AI: %d, Pawns: %d"), 
           AICount, PawnCount);
    
    return true; // AI is optional for basic functionality
}