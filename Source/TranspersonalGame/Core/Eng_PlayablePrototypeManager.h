#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/GameModeBase.h"
#include "UObject/NoExportTypes.h"
#include "Eng_PlayablePrototypeManager.generated.h"

/**
 * Playable Prototype Manager - Engine Architect Cycle 002
 * Ensures the minimum viable playable prototype is functional
 * Creates a working character that can walk around with WASD movement
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_PlayablePrototypeManager : public UObject
{
    GENERATED_BODY()

public:
    UEng_PlayablePrototypeManager();

    // Playable prototype creation methods
    UFUNCTION(BlueprintCallable, Category = "Playable Prototype")
    bool CreateMinimumViablePrototype();

    UFUNCTION(BlueprintCallable, Category = "Playable Prototype")
    bool SetupPlayerCharacter();

    UFUNCTION(BlueprintCallable, Category = "Playable Prototype")
    bool SetupBasicTerrain();

    UFUNCTION(BlueprintCallable, Category = "Playable Prototype")
    bool SetupBasicLighting();

    UFUNCTION(BlueprintCallable, Category = "Playable Prototype")
    bool PlaceDinosaurPlaceholders();

    // Prototype validation methods
    UFUNCTION(BlueprintCallable, Category = "Playable Prototype")
    bool ValidatePrototypeReadiness();

    UFUNCTION(BlueprintCallable, Category = "Playable Prototype")
    bool TestPlayerMovement();

    UFUNCTION(BlueprintCallable, Category = "Playable Prototype")
    bool TestCameraControls();

    // Biome coordinate system (from brain memories)
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetBiomeCenter(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetRandomLocationInBiome(const FString& BiomeName);

protected:
    // Prototype status tracking
    UPROPERTY(BlueprintReadOnly, Category = "Prototype Status")
    bool bCharacterSetup;

    UPROPERTY(BlueprintReadOnly, Category = "Prototype Status")
    bool bTerrainSetup;

    UPROPERTY(BlueprintReadOnly, Category = "Prototype Status")
    bool bLightingSetup;

    UPROPERTY(BlueprintReadOnly, Category = "Prototype Status")
    bool bDinosaursPlaced;

    UPROPERTY(BlueprintReadOnly, Category = "Prototype Status")
    bool bPrototypeReady;

    // Biome coordinates (from brain memories)
    UPROPERTY(BlueprintReadOnly, Category = "Biome System")
    TMap<FString, FVector> BiomeCenters;

private:
    void InitializeBiomeCoordinates();
    bool SpawnActorInBiome(UClass* ActorClass, const FString& BiomeName, const FString& ActorLabel);
};