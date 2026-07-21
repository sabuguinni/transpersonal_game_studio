#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "EnvArt_CretaceousFallenLogSystem.generated.h"

/**
 * Cretaceous Fallen Log System - Environmental storytelling through weathered prehistoric logs
 * Creates realistic fallen tree debris with moss, decay, and environmental integration
 * Supports procedural placement and performance optimization
 */

UENUM(BlueprintType)
enum class EEnvArt_LogDecayState : uint8
{
    Fresh           UMETA(DisplayName = "Fresh Fall"),
    Weathered       UMETA(DisplayName = "Weathered"),
    MossyCovered    UMETA(DisplayName = "Moss Covered"),
    Decomposing     UMETA(DisplayName = "Decomposing"),
    Fossilized      UMETA(DisplayName = "Fossilized")
};

UENUM(BlueprintType)
enum class EEnvArt_CretaceousTreeSpecies : uint8
{
    Araucaria       UMETA(DisplayName = "Araucaria (Conifer)"),
    Ginkgo          UMETA(DisplayName = "Ginkgo"),
    Cycad           UMETA(DisplayName = "Cycad Palm"),
    Fern            UMETA(DisplayName = "Tree Fern"),
    Sequoia         UMETA(DisplayName = "Dawn Redwood")
};

USTRUCT(BlueprintType)
struct FEnvArt_FallenLogProperties
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    EEnvArt_CretaceousTreeSpecies TreeSpecies = EEnvArt_CretaceousTreeSpecies::Araucaria;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties")
    EEnvArt_LogDecayState DecayState = EEnvArt_LogDecayState::Weathered;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties", meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float LogLength = 2.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Properties", meta = (ClampMin = "0.1", ClampMax = "1.0"))
    float LogDiameter = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MossIntensity = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float WeatheringIntensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement")
    bool bSinkIntoTerrain = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Placement", meta = (ClampMin = "0.0", ClampMax = "0.5"))
    float TerrainSinkDepth = 0.15f;

    FEnvArt_FallenLogProperties()
    {
        TreeSpecies = EEnvArt_CretaceousTreeSpecies::Araucaria;
        DecayState = EEnvArt_LogDecayState::Weathered;
        LogLength = 2.5f;
        LogDiameter = 0.4f;
        MossIntensity = 0.6f;
        WeatheringIntensity = 0.7f;
        bSinkIntoTerrain = true;
        TerrainSinkDepth = 0.15f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvArt_CretaceousFallenLogSystem : public AActor
{
    GENERATED_BODY()

public:
    AEnvArt_CretaceousFallenLogSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* LogMeshComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Log Configuration")
    FEnvArt_FallenLogProperties LogProperties;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> LogMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    TArray<UMaterialInterface*> MossMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
    float LODDistance = 2000.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Log System")
    void InitializeFallenLog(const FEnvArt_FallenLogProperties& InLogProperties);

    UFUNCTION(BlueprintCallable, Category = "Log System")
    void UpdateLogAppearance();

    UFUNCTION(BlueprintCallable, Category = "Log System")
    void SetDecayState(EEnvArt_LogDecayState NewDecayState);

    UFUNCTION(BlueprintCallable, Category = "Log System")
    void SetTreeSpecies(EEnvArt_CretaceousTreeSpecies NewSpecies);

    UFUNCTION(BlueprintCallable, Category = "Log System")
    void ApplyEnvironmentalWeathering(float WeatheringAmount);

    UFUNCTION(BlueprintCallable, Category = "Log System")
    void SinkIntoTerrain(float SinkDepth);

    UFUNCTION(BlueprintCallable, Category = "Placement", CallInEditor = true)
    void RandomizeLogProperties();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLOD(float DistanceToPlayer);

    UFUNCTION(BlueprintPure, Category = "Log System")
    FString GetLogDescription() const;

protected:
    UFUNCTION()
    void ApplyMaterialBasedOnDecay();

    UFUNCTION()
    void AdjustLogScale();

    UFUNCTION()
    void SetupCollision();

private:
    float CurrentLODLevel;
    bool bIsInitialized;
    
    // Performance tracking
    float LastLODUpdateTime;
    static constexpr float LODUpdateInterval = 1.0f;
};