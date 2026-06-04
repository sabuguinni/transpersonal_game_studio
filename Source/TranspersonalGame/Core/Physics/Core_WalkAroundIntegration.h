#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Landscape/Landscape.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "SharedTypes.h"
#include "Core_WalkAroundIntegration.generated.h"

// Forward declarations
class ATranspersonalCharacter;
class ALandscape;
class ADirectionalLight;
class ASkyAtmosphere;
class AExponentialHeightFog;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_WalkAroundRequirements
{
    GENERATED_BODY()

    // Character Movement Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Character")
    bool bHasThirdPersonCharacter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Character")
    bool bHasWASDMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Character")
    bool bHasCameraBoom = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Character")
    bool bHasFollowCamera = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Character")
    bool bCanWalk = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Character")
    bool bCanRun = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Character")
    bool bCanJump = false;

    // Terrain Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Terrain")
    bool bHasLandscape = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Terrain")
    bool bHasTerrainVariation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Terrain")
    float TerrainHeightVariation = 0.0f;

    // Lighting Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Lighting")
    bool bHasDirectionalLight = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Lighting")
    bool bHasSkyAtmosphere = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Lighting")
    bool bHasFog = false;

    // Dinosaur Requirements
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Dinosaurs")
    int32 DinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Dinosaurs")
    bool bHasStaticDinosaurs = false;

    // Overall Progress
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Progress")
    float OverallProgress = 0.0f;

    FCore_WalkAroundRequirements()
    {
        bHasThirdPersonCharacter = false;
        bHasWASDMovement = false;
        bHasCameraBoom = false;
        bHasFollowCamera = false;
        bCanWalk = false;
        bCanRun = false;
        bCanJump = false;
        bHasLandscape = false;
        bHasTerrainVariation = false;
        TerrainHeightVariation = 0.0f;
        bHasDirectionalLight = false;
        bHasSkyAtmosphere = false;
        bHasFog = false;
        DinosaurCount = 0;
        bHasStaticDinosaurs = false;
        OverallProgress = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_WalkAroundValidationResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bIsValid = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FString ValidationMessage = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float CompletionPercentage = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    TArray<FString> MissingRequirements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    FCore_WalkAroundRequirements CurrentState;

    FCore_WalkAroundValidationResult()
    {
        bIsValid = false;
        ValidationMessage = TEXT("Not validated");
        CompletionPercentage = 0.0f;
        MissingRequirements.Empty();
    }
};

/**
 * Core_WalkAroundIntegration - Integrates all physics systems for WALK AROUND milestone
 * 
 * This component validates and ensures all requirements for the minimum viable playable prototype
 * are met. It checks character movement, terrain, lighting, and dinosaur placement to confirm
 * the player can walk around in a basic prehistoric world.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_WalkAroundIntegration : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_WalkAroundIntegration();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Validation Methods
    UFUNCTION(BlueprintCallable, Category = "Walk Around|Validation")
    FCore_WalkAroundValidationResult ValidateWalkAroundMilestone();

    UFUNCTION(BlueprintCallable, Category = "Walk Around|Character")
    bool ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "Walk Around|Terrain")
    bool ValidateTerrainSystem();

    UFUNCTION(BlueprintCallable, Category = "Walk Around|Lighting")
    bool ValidateLightingSystem();

    UFUNCTION(BlueprintCallable, Category = "Walk Around|Dinosaurs")
    bool ValidateDinosaurPlacement();

    // Integration Methods
    UFUNCTION(BlueprintCallable, Category = "Walk Around|Integration")
    void IntegratePhysicsSystems();

    UFUNCTION(BlueprintCallable, Category = "Walk Around|Integration")
    void SetupCharacterPhysics();

    UFUNCTION(BlueprintCallable, Category = "Walk Around|Integration")
    void ConfigureTerrainInteraction();

    UFUNCTION(BlueprintCallable, Category = "Walk Around|Integration")
    void OptimizePerformance();

    // Milestone Progress
    UFUNCTION(BlueprintCallable, Category = "Walk Around|Progress")
    float GetMilestoneProgress() const;

    UFUNCTION(BlueprintCallable, Category = "Walk Around|Progress")
    FString GetProgressReport() const;

    UFUNCTION(BlueprintCallable, Category = "Walk Around|Progress")
    TArray<FString> GetMissingRequirements() const;

    // Debug and Testing
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Walk Around|Debug")
    void DebugValidateAll();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Walk Around|Debug")
    void ForceIntegrationUpdate();

protected:
    // Current milestone state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Walk Around|State")
    FCore_WalkAroundRequirements CurrentRequirements;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Walk Around|State")
    FCore_WalkAroundValidationResult LastValidationResult;

    // Validation timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Walk Around|Config")
    float ValidationInterval = 5.0f;

    UPROPERTY()
    float LastValidationTime = 0.0f;

    // Component references
    UPROPERTY()
    TWeakObjectPtr<ATranspersonalCharacter> PlayerCharacter;

    UPROPERTY()
    TWeakObjectPtr<ALandscape> WorldLandscape;

    UPROPERTY()
    TWeakObjectPtr<ADirectionalLight> SunLight;

    // Helper methods
    void UpdateComponentReferences();
    void CalculateOverallProgress();
    bool IsCharacterMovementValid() const;
    bool IsTerrainSystemValid() const;
    bool IsLightingSystemValid() const;
    bool AreDinosaursValid() const;
    
    // Performance monitoring
    void MonitorFrameRate();
    float AverageFrameRate = 60.0f;
    TArray<float> RecentFrameTimes;
};