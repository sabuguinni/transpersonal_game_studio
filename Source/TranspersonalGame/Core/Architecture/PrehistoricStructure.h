#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "../SharedTypes.h"
#include "PrehistoricStructure.generated.h"

UENUM(BlueprintType)
enum class EArch_StructureType : uint8
{
    StonePillar     UMETA(DisplayName = "Stone Pillar"),
    RockShelter     UMETA(DisplayName = "Rock Shelter"), 
    StoneCircle     UMETA(DisplayName = "Stone Circle"),
    CaveEntrance    UMETA(DisplayName = "Cave Entrance"),
    TribalHut       UMETA(DisplayName = "Tribal Hut")
};

USTRUCT(BlueprintType)
struct FArch_StructureConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EArch_StructureType StructureType = EArch_StructureType::StonePillar;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    FVector Scale = FVector(1.0f, 1.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    float WeatheringLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    bool bHasMossGrowth = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure")
    EBiomeType BiomeType = EBiomeType::Savana;

    FArch_StructureConfig()
    {
        StructureType = EArch_StructureType::StonePillar;
        Scale = FVector(1.0f, 1.0f, 1.0f);
        WeatheringLevel = 0.5f;
        bHasMossGrowth = true;
        BiomeType = EBiomeType::Savana;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APrehistoricStructure : public AActor
{
    GENERATED_BODY()

public:
    APrehistoricStructure();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* StructureMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structure Config")
    FArch_StructureConfig StructureConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* BaseStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* WeatheredStoneMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* MossyStoneMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetStructureType(EArch_StructureType NewType);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetWeatheringLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void SetMossGrowth(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Structure")
    void ApplyBiomeAdaptation(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Structure")
    void RegenerateStructure();

    UFUNCTION(BlueprintPure, Category = "Structure")
    EArch_StructureType GetStructureType() const { return StructureConfig.StructureType; }

    UFUNCTION(BlueprintPure, Category = "Structure")
    float GetWeatheringLevel() const { return StructureConfig.WeatheringLevel; }

protected:
    UFUNCTION()
    void UpdateStructureMesh();

    UFUNCTION()
    void UpdateMaterials();

    UFUNCTION()
    void ApplyWeatheringEffects();
};