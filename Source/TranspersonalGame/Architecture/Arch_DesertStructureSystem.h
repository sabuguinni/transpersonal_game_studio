#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "Arch_DesertStructureSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DesertStructureData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Structure")
    FString StructureName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Structure")
    EArch_StructureType StructureType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Structure")
    float WeatheringLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Structure")
    float SandAccumulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Structure")
    bool bIsPartiallyBuried;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Structure")
    FVector OriginalDimensions;

    FArch_DesertStructureData()
    {
        StructureName = TEXT("Unknown Desert Structure");
        StructureType = EArch_StructureType::Ruins;
        WeatheringLevel = 0.5f;
        SandAccumulation = 0.3f;
        bIsPartiallyBuried = false;
        OriginalDimensions = FVector(100.0f, 100.0f, 100.0f);
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_DesertEnvironmentalFactors
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Environment")
    float SandstormIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Environment")
    float TemperatureVariation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Environment")
    float WindErosionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Environment")
    bool bIsOasisNearby;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Environment")
    float DistanceToWater;

    FArch_DesertEnvironmentalFactors()
    {
        SandstormIntensity = 0.4f;
        TemperatureVariation = 40.0f;
        WindErosionRate = 0.1f;
        bIsOasisNearby = false;
        DistanceToWater = 10000.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AArch_DesertStructureSystem : public AActor
{
    GENERATED_BODY()

public:
    AArch_DesertStructureSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PrimaryStructureMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* SandAccumulationMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Structure")
    FArch_DesertStructureData StructureData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Desert Environment")
    FArch_DesertEnvironmentalFactors EnvironmentalFactors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* SandMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* DesertPillarMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* DesertArchMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    UStaticMesh* SandDuneMesh;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    void InitializeDesertStructure(const FArch_DesertStructureData& InStructureData);

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    void ApplyWeatheringEffects(float WeatheringIntensity);

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    void UpdateSandAccumulation(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    void ProcessEnvironmentalFactors();

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    bool IsStructureAccessible() const;

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    FVector GetOptimalExplorationPoint() const;

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    float CalculateStructuralIntegrity() const;

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    void SimulateSandstormDamage(float StormIntensity);

    UFUNCTION(BlueprintCallable, Category = "Desert Structure")
    TArray<FVector> GetShelterPoints() const;

private:
    void UpdateMeshBasedOnWeathering();
    void ApplySandAccumulationToMesh();
    void CalculateOptimalSandPlacement();
    
    float CurrentWeatheringLevel;
    float CurrentSandLevel;
    bool bIsInitialized;
};