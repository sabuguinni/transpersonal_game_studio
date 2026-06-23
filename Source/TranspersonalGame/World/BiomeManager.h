// BiomeManager.h
// Engine Architect #02 — P1 World Generation
// Manages biome detection, transitions, and environmental data for the prehistoric survival world.
// Pairs with BiomeManager.cpp

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TranspersonalGame/SharedTypes.h"
#include "BiomeManager.generated.h"

// -------------------------------------------------------
// USTRUCT: Biome Definition (static data per biome type)
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EBiomeType BiomeType = EBiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float BaseTemperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Climate")
    float BaseHumidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    float FogDensity = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Visual")
    FLinearColor AmbientColor = FLinearColor(0.1f, 0.1f, 0.1f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    bool bAllowsDinosaurSpawn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    float DinosaurDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Gameplay")
    float VegetationDensityMultiplier = 1.0f;
};

// -------------------------------------------------------
// USTRUCT: Biome Runtime Data (live state per biome)
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeRuntimeData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Runtime")
    EBiomeType BiomeType = EBiomeType::Plains;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Runtime")
    float CurrentTemperature = 20.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Runtime")
    float CurrentHumidity = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Runtime")
    int32 ActiveDinosaurCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Runtime")
    bool bIsActive = false;
};

// -------------------------------------------------------
// USTRUCT: Biome Transition (blend state)
// -------------------------------------------------------
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeTransition
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    EBiomeType FromBiome = EBiomeType::Plains;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    EBiomeType ToBiome = EBiomeType::Plains;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    float Progress = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Transition")
    bool bIsActive = false;
};

// -------------------------------------------------------
// DELEGATE: Biome change notification
// -------------------------------------------------------
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEng_OnBiomeChanged, EBiomeType, OldBiome, EBiomeType, NewBiome);

// -------------------------------------------------------
// UCLASS: ABiomeManager
// -------------------------------------------------------
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Biome Manager"))
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    // ---- Biome Definitions (editable in editor) ----
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Definitions")
    TArray<FEng_BiomeDefinition> BiomeDefinitions;

    // ---- Runtime State ----
    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    EBiomeType CurrentBiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    EBiomeType PreviousBiomeType;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    float TransitionProgress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|State")
    float TransitionSpeed;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|State")
    bool bIsTransitioning;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome|Settings")
    float BiomeTransitionBlendRadius;

    UPROPERTY(BlueprintReadOnly, Category = "Biome|Runtime")
    TArray<FEng_BiomeRuntimeData> ActiveBiomeData;

    // ---- Delegate ----
    UPROPERTY(BlueprintAssignable, Category = "Biome|Events")
    FEng_OnBiomeChanged OnBiomeChanged;

    // ---- Public API ----
    UFUNCTION(BlueprintCallable, Category = "Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FEng_BiomeDefinition GetBiomeDefinition(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RequestBiomeTransition(EBiomeType NewBiome);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void UpdatePlayerBiome(const FVector& PlayerLocation);

    UFUNCTION(BlueprintPure, Category = "Biome")
    float GetCurrentBiomeTemperature() const;

    UFUNCTION(BlueprintPure, Category = "Biome")
    float GetCurrentBiomeHumidity() const;

    UFUNCTION(BlueprintPure, Category = "Biome")
    int32 GetActiveDinosaurCountForBiome(EBiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterDinosaurInBiome(EBiomeType BiomeType, bool bAdding);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
};
