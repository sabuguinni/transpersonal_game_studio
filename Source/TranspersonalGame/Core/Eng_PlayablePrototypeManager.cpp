#include "Eng_PlayablePrototypeManager.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/SkyAtmosphere.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "GameFramework/PlayerStart.h"
#include "Landscape/Landscape.h"
#include "TranspersonalGame/TranspersonalCharacter.h"
#include "TranspersonalGame/TranspersonalGameMode.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayablePrototype, Log, All);

UEng_PlayablePrototypeManager::UEng_PlayablePrototypeManager()
{
    PrimaryComponentTick.bCanEverTick = false;
    
    // Initialize prototype configuration
    PrototypeConfig.bIsInitialized = false;
    PrototypeConfig.RequiredActorCount = 50;
    PrototypeConfig.MinimumTerrainSize = FVector(200000.0f, 200000.0f, 5000.0f);
    PrototypeConfig.RequiredLightingActors = 3; // Sun, Sky, Fog
    PrototypeConfig.RequiredDinosaurCount = 5;
    PrototypeConfig.RequiredPlayerStarts = 1;
}

void UEng_PlayablePrototypeManager::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogPlayablePrototype, Warning, TEXT("Playable Prototype Manager - Begin Play"));
    
    // Validate prototype requirements
    ValidatePrototypeRequirements();
}

bool UEng_PlayablePrototypeManager::ValidatePrototypeRequirements()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPlayablePrototype, Error, TEXT("No valid world found"));
        return false;
    }

    bool bAllRequirementsMet = true;
    FString ValidationReport = TEXT("PLAYABLE PROTOTYPE VALIDATION REPORT\n");
    ValidationReport += TEXT("=====================================\n\n");

    // Check for player character
    ATranspersonalCharacter* PlayerCharacter = Cast<ATranspersonalCharacter>(UGameplayStatics::GetPlayerCharacter(World, 0));
    if (PlayerCharacter)
    {
        ValidationReport += TEXT("✓ Player Character: FOUND\n");
        UE_LOG(LogPlayablePrototype, Log, TEXT("Player character found and valid"));
    }
    else
    {
        ValidationReport += TEXT("✗ Player Character: MISSING\n");
        UE_LOG(LogPlayablePrototype, Error, TEXT("Player character not found"));
        bAllRequirementsMet = false;
    }

    // Check for game mode
    ATranspersonalGameMode* GameMode = Cast<ATranspersonalGameMode>(World->GetAuthGameMode());
    if (GameMode)
    {
        ValidationReport += TEXT("✓ Game Mode: FOUND\n");
        UE_LOG(LogPlayablePrototype, Log, TEXT("TranspersonalGameMode found and active"));
    }
    else
    {
        ValidationReport += TEXT("✗ Game Mode: MISSING\n");
        UE_LOG(LogPlayablePrototype, Error, TEXT("TranspersonalGameMode not found"));
        bAllRequirementsMet = false;
    }

    // Check for terrain/landscape
    TArray<AActor*> LandscapeActors;
    UGameplayStatics::GetAllActorsOfClass(World, ALandscape::StaticClass(), LandscapeActors);
    if (LandscapeActors.Num() > 0)
    {
        ValidationReport += FString::Printf(TEXT("✓ Terrain: %d landscape(s) found\n"), LandscapeActors.Num());
        UE_LOG(LogPlayablePrototype, Log, TEXT("Landscape found: %d actors"), LandscapeActors.Num());
    }
    else
    {
        ValidationReport += TEXT("✗ Terrain: NO LANDSCAPE FOUND\n");
        UE_LOG(LogPlayablePrototype, Error, TEXT("No landscape actors found"));
        bAllRequirementsMet = false;
    }

    // Check for lighting
    TArray<AActor*> LightActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), LightActors);
    if (LightActors.Num() > 0)
    {
        ValidationReport += FString::Printf(TEXT("✓ Lighting: %d directional light(s) found\n"), LightActors.Num());
        UE_LOG(LogPlayablePrototype, Log, TEXT("Directional lights found: %d"), LightActors.Num());
    }
    else
    {
        ValidationReport += TEXT("✗ Lighting: NO DIRECTIONAL LIGHTS\n");
        UE_LOG(LogPlayablePrototype, Warning, TEXT("No directional lights found"));
    }

    // Check for sky atmosphere
    TArray<AActor*> SkyActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyActors);
    if (SkyActors.Num() > 0)
    {
        ValidationReport += FString::Printf(TEXT("✓ Sky: %d sky atmosphere(s) found\n"), SkyActors.Num());
    }
    else
    {
        ValidationReport += TEXT("⚠ Sky: NO SKY ATMOSPHERE\n");
        UE_LOG(LogPlayablePrototype, Warning, TEXT("No sky atmosphere found"));
    }

    // Check for player starts
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    if (PlayerStarts.Num() > 0)
    {
        ValidationReport += FString::Printf(TEXT("✓ Player Starts: %d found\n"), PlayerStarts.Num());
        UE_LOG(LogPlayablePrototype, Log, TEXT("Player starts found: %d"), PlayerStarts.Num());
    }
    else
    {
        ValidationReport += TEXT("✗ Player Starts: NONE FOUND\n");
        UE_LOG(LogPlayablePrototype, Error, TEXT("No player starts found"));
        bAllRequirementsMet = false;
    }

    // Count total actors
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    ValidationReport += FString::Printf(TEXT("\nTOTAL ACTORS IN LEVEL: %d\n"), AllActors.Num());
    
    if (AllActors.Num() >= PrototypeConfig.RequiredActorCount)
    {
        ValidationReport += TEXT("✓ Actor Count: SUFFICIENT\n");
    }
    else
    {
        ValidationReport += FString::Printf(TEXT("⚠ Actor Count: BELOW MINIMUM (%d required)\n"), PrototypeConfig.RequiredActorCount);
    }

    // Update prototype status
    PrototypeConfig.bIsInitialized = bAllRequirementsMet;
    PrototypeConfig.LastValidationTime = FDateTime::Now();
    PrototypeConfig.ValidationReport = ValidationReport;

    UE_LOG(LogPlayablePrototype, Warning, TEXT("Prototype validation complete: %s"), 
           bAllRequirementsMet ? TEXT("PASSED") : TEXT("FAILED"));

    return bAllRequirementsMet;
}

bool UEng_PlayablePrototypeManager::CreateMinimalPlayableSetup()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogPlayablePrototype, Error, TEXT("Cannot create setup - no valid world"));
        return false;
    }

    UE_LOG(LogPlayablePrototype, Warning, TEXT("Creating minimal playable setup..."));

    bool bSetupSuccess = true;

    // Ensure player start exists
    TArray<AActor*> PlayerStarts;
    UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
    if (PlayerStarts.Num() == 0)
    {
        // Create player start at origin
        FVector SpawnLocation = FVector(0.0f, 0.0f, 200.0f);
        FRotator SpawnRotation = FRotator::ZeroRotator;
        APlayerStart* NewPlayerStart = World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), SpawnLocation, SpawnRotation);
        if (NewPlayerStart)
        {
            UE_LOG(LogPlayablePrototype, Log, TEXT("Created PlayerStart at origin"));
        }
        else
        {
            UE_LOG(LogPlayablePrototype, Error, TEXT("Failed to create PlayerStart"));
            bSetupSuccess = false;
        }
    }

    // Ensure directional light exists
    TArray<AActor*> LightActors;
    UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), LightActors);
    if (LightActors.Num() == 0)
    {
        FVector LightLocation = FVector(0.0f, 0.0f, 1000.0f);
        FRotator LightRotation = FRotator(-45.0f, 0.0f, 0.0f);
        ADirectionalLight* NewLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), LightLocation, LightRotation);
        if (NewLight && NewLight->GetLightComponent())
        {
            NewLight->GetLightComponent()->SetIntensity(3.0f);
            UE_LOG(LogPlayablePrototype, Log, TEXT("Created Directional Light"));
        }
        else
        {
            UE_LOG(LogPlayablePrototype, Error, TEXT("Failed to create Directional Light"));
            bSetupSuccess = false;
        }
    }

    // Create sky atmosphere if missing
    TArray<AActor*> SkyActors;
    UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), SkyActors);
    if (SkyActors.Num() == 0)
    {
        ASkyAtmosphere* NewSky = World->SpawnActor<ASkyAtmosphere>(ASkyAtmosphere::StaticClass());
        if (NewSky)
        {
            UE_LOG(LogPlayablePrototype, Log, TEXT("Created Sky Atmosphere"));
        }
        else
        {
            UE_LOG(LogPlayablePrototype, Warning, TEXT("Failed to create Sky Atmosphere"));
        }
    }

    // Create exponential height fog if missing
    TArray<AActor*> FogActors;
    UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), FogActors);
    if (FogActors.Num() == 0)
    {
        AExponentialHeightFog* NewFog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass());
        if (NewFog && NewFog->GetComponent())
        {
            NewFog->GetComponent()->SetFogDensity(0.02f);
            NewFog->GetComponent()->SetFogHeightFalloff(0.2f);
            UE_LOG(LogPlayablePrototype, Log, TEXT("Created Exponential Height Fog"));
        }
        else
        {
            UE_LOG(LogPlayablePrototype, Warning, TEXT("Failed to create Exponential Height Fog"));
        }
    }

    UE_LOG(LogPlayablePrototype, Warning, TEXT("Minimal playable setup complete: %s"), 
           bSetupSuccess ? TEXT("SUCCESS") : TEXT("PARTIAL"));

    return bSetupSuccess;
}

FEng_PrototypeConfig UEng_PlayablePrototypeManager::GetPrototypeConfig() const
{
    return PrototypeConfig;
}

bool UEng_PlayablePrototypeManager::IsPrototypePlayable() const
{
    return PrototypeConfig.bIsInitialized;
}

FString UEng_PlayablePrototypeManager::GetValidationReport() const
{
    return PrototypeConfig.ValidationReport;
}