// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "Director_PlayablePrototypeManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPlayablePrototype, Log, All);

/**
 * Playable Prototype Status Tracking
 * Tracks completion of Milestone 1 requirements
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_PrototypeStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bCharacterMovementWorking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bCameraSystemWorking = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bTerrainWithHeightVariation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bDinosaursPlacedInWorld = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bLightingSystemActive = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    bool bSurvivalHUDVisible = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    int32 DinosaurCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype")
    float TerrainComplexity = 0.0f;

    FDir_PrototypeStatus()
    {
        bCharacterMovementWorking = false;
        bCameraSystemWorking = false;
        bTerrainWithHeightVariation = false;
        bDinosaursPlacedInWorld = false;
        bLightingSystemActive = false;
        bSurvivalHUDVisible = false;
        DinosaurCount = 0;
        TerrainComplexity = 0.0f;
    }

    bool IsPrototypeComplete() const
    {
        return bCharacterMovementWorking && 
               bCameraSystemWorking && 
               bTerrainWithHeightVariation && 
               bDinosaursPlacedInWorld && 
               bLightingSystemActive &&
               DinosaurCount >= 3;
    }
};

/**
 * Dinosaur Spawn Configuration
 * Defines where and how dinosaurs should be placed
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDir_DinosaurSpawnConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString DinosaurType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString AssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator SpawnRotation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FString BiomeName;

    FDir_DinosaurSpawnConfig()
    {
        DinosaurType = TEXT("Unknown");
        AssetPath = TEXT("");
        SpawnLocation = FVector::ZeroVector;
        SpawnRotation = FRotator::ZeroRotator;
        BiomeName = TEXT("Savana");
    }
};

/**
 * Playable Prototype Manager
 * 
 * Studio Director's coordination system for ensuring the MinPlayableMap
 * meets Milestone 1 requirements. Tracks all essential systems and
 * provides validation for the "Walk Around" prototype.
 * 
 * Responsibilities:
 * - Monitor character movement implementation
 * - Verify camera system functionality
 * - Track dinosaur placement and count
 * - Validate terrain complexity
 * - Ensure lighting and atmosphere
 * - Coordinate with survival HUD system
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDir_PlayablePrototypeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UDir_PlayablePrototypeManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Prototype Status
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Status")
    FDir_PrototypeStatus CurrentStatus;

    // Dinosaur Management
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Management")
    TArray<FDir_DinosaurSpawnConfig> DinosaurSpawnConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Management")
    TArray<AActor*> SpawnedDinosaurs;

    // Validation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    float ValidationInterval = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bAutoSpawnDinosaurs = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
    bool bAutoValidatePrototype = true;

    // Performance Tracking
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float LastValidationTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 ValidationCount = 0;

public:
    // Prototype Validation
    UFUNCTION(BlueprintCallable, Category = "Prototype")
    bool ValidatePrototypeStatus();

    UFUNCTION(BlueprintCallable, Category = "Prototype")
    FDir_PrototypeStatus GetPrototypeStatus() const { return CurrentStatus; }

    UFUNCTION(BlueprintCallable, Category = "Prototype")
    bool IsPrototypeComplete() const { return CurrentStatus.IsPrototypeComplete(); }

    // Character System Validation
    UFUNCTION(BlueprintCallable, Category = "Character")
    bool ValidateCharacterMovement();

    UFUNCTION(BlueprintCallable, Category = "Character")
    bool ValidateCameraSystem();

    // World Validation
    UFUNCTION(BlueprintCallable, Category = "World")
    bool ValidateTerrainComplexity();

    UFUNCTION(BlueprintCallable, Category = "World")
    bool ValidateLightingSystem();

    // Dinosaur Management
    UFUNCTION(BlueprintCallable, Category = "Dinosaurs")
    bool ValidateDinosaurPlacement();

    UFUNCTION(BlueprintCallable, Category = "Dinosaurs")
    int32 CountDinosaursInWorld();

    UFUNCTION(BlueprintCallable, Category = "Dinosaurs")
    bool SpawnDinosaursFromConfig();

    UFUNCTION(BlueprintCallable, Category = "Dinosaurs")
    void SetupDefaultDinosaurSpawns();

    // HUD Integration
    UFUNCTION(BlueprintCallable, Category = "HUD")
    bool ValidateSurvivalHUD();

    // Reporting
    UFUNCTION(BlueprintCallable, Category = "Reporting")
    FString GeneratePrototypeReport();

    UFUNCTION(BlueprintCallable, Category = "Reporting")
    void LogPrototypeStatus();

    // Editor Functions
    UFUNCTION(CallInEditor, Category = "Editor")
    void ValidatePrototypeInEditor();

    UFUNCTION(CallInEditor, Category = "Editor")
    void SpawnDinosaursInEditor();

    UFUNCTION(CallInEditor, Category = "Editor")
    void GenerateReportInEditor();

protected:
    // Internal Validation
    void PerformPeriodicValidation();
    bool FindCharacterInWorld();
    bool CheckCameraComponents();
    bool AnalyzeTerrainHeight();
    bool CheckLightingSources();
    void UpdateDinosaurTracking();

    // Utility Functions
    FVector GetBiomeLocation(const FString& BiomeName);
    bool IsValidDinosaurAsset(const FString& AssetPath);
    void InitializeDefaultSpawnConfigs();
};