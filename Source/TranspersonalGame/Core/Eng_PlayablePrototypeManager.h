#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Subsystems/WorldSubsystem.h"
#include "Components/ActorComponent.h"
#include "Eng_PlayablePrototypeManager.generated.h"

/**
 * Engine Architect's Playable Prototype Manager
 * Ensures the game has a minimum viable playable prototype with character movement,
 * terrain, lighting, and basic dinosaur placement
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PlayablePrototypeManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEng_PlayablePrototypeManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void OnWorldBeginPlay(UWorld& InWorld) override;

    // Prototype validation and setup
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Playable Prototype")
    void ValidateMinimumPlayableState();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Playable Prototype")
    void SetupBasicTerrain();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Playable Prototype")
    void SetupPlayerCharacter();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Playable Prototype")
    void SetupBasicLighting();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Playable Prototype")
    void PlaceDinosaurPlaceholders();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Playable Prototype")
    void ValidatePlayerMovement();

    // Quick prototype generation
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Playable Prototype")
    void CreateMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Playable Prototype")
    bool IsPrototypePlayable();

protected:
    // Internal setup functions
    void EnsurePlayerStart();
    void EnsureGameMode();
    void EnsureBasicGeometry();
    void EnsureEnvironmentalLighting();
    void EnsureDinosaurPresence();
    
    // Validation functions
    bool ValidateCharacterMovement();
    bool ValidateTerrainNavigation();
    bool ValidateCameraSystem();
    bool ValidateBasicInteraction();

private:
    // Prototype state tracking
    UPROPERTY()
    bool bTerrainSetup;

    UPROPERTY()
    bool bPlayerCharacterSetup;

    UPROPERTY()
    bool bLightingSetup;

    UPROPERTY()
    bool bDinosaursPlaced;

    UPROPERTY()
    bool bMovementValidated;

    UPROPERTY()
    bool bPrototypeComplete;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Settings", meta = (AllowPrivateAccess = "true"))
    bool bAutoSetupOnWorldStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Settings", meta = (AllowPrivateAccess = "true"))
    bool bValidateOnEveryPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Settings", meta = (AllowPrivateAccess = "true"))
    int32 MinimumDinosaurCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Settings", meta = (AllowPrivateAccess = "true"))
    float TerrainSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Prototype Settings", meta = (AllowPrivateAccess = "true"))
    float PlayerSpawnHeight;

    // References to key prototype actors
    UPROPERTY()
    class APlayerStart* PlayerStartRef;

    UPROPERTY()
    class ADirectionalLight* SunLightRef;

    UPROPERTY()
    class ALandscape* TerrainRef;

    UPROPERTY()
    TArray<class APawn*> DinosaurRefs;
};