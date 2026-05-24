#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialParameterCollection.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "../SharedTypes.h"
#include "EnvArt_MaterialManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_MaterialType : uint8
{
    Rock_Sandstone      UMETA(DisplayName = "Rock - Sandstone"),
    Rock_Limestone      UMETA(DisplayName = "Rock - Limestone"),
    Rock_Granite        UMETA(DisplayName = "Rock - Granite"),
    Vegetation_Moss     UMETA(DisplayName = "Vegetation - Moss"),
    Vegetation_Bark     UMETA(DisplayName = "Vegetation - Tree Bark"),
    Vegetation_Leaves   UMETA(DisplayName = "Vegetation - Leaves"),
    Ground_Dirt         UMETA(DisplayName = "Ground - Dirt"),
    Ground_Mud          UMETA(DisplayName = "Ground - Mud"),
    Ground_Sand         UMETA(DisplayName = "Ground - Sand"),
    Water_Swamp         UMETA(DisplayName = "Water - Swamp"),
    Water_River         UMETA(DisplayName = "Water - River"),
    Water_Lake          UMETA(DisplayName = "Water - Lake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_MaterialConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Config")
    EEnvArt_MaterialType MaterialType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Config")
    TSoftObjectPtr<UMaterial> BaseMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Config")
    TSoftObjectPtr<UMaterialInstance> MaterialInstance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Config")
    float Roughness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Config")
    float Metallic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Config")
    FLinearColor BaseColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Config")
    float NormalStrength;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material Config")
    bool bUseWeatherEffects;

    FEnvArt_MaterialConfig()
    {
        MaterialType = EEnvArt_MaterialType::Rock_Sandstone;
        Roughness = 0.8f;
        Metallic = 0.0f;
        BaseColor = FLinearColor::White;
        NormalStrength = 1.0f;
        bUseWeatherEffects = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_WeatherMaterialParams
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    float WetnessLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    float MossGrowth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    float Weathering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather Effects")
    float DirtAccumulation;

    FEnvArt_WeatherMaterialParams()
    {
        WetnessLevel = 0.0f;
        MossGrowth = 0.0f;
        Weathering = 0.5f;
        DirtAccumulation = 0.3f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEnvArt_MaterialManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UEnvArt_MaterialManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Material management
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void InitializeMaterialSystem();

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    UMaterialInstance* GetMaterialForType(EEnvArt_MaterialType MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyMaterialToMesh(UStaticMeshComponent* MeshComponent, EEnvArt_MaterialType MaterialType);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void UpdateWeatherEffects(const FEnvArt_WeatherMaterialParams& WeatherParams);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void RegisterMaterialConfig(const FEnvArt_MaterialConfig& Config);

    // Biome-specific material application
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void ApplyBiomeMaterials(EBiomeType BiomeType, const TArray<UStaticMeshComponent*>& MeshComponents);

    // Material parameter control
    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetGlobalMaterialParameter(const FString& ParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Environment Art")
    void SetGlobalMaterialVectorParameter(const FString& ParameterName, const FLinearColor& Value);

    // Debug and validation
    UFUNCTION(BlueprintCallable, Category = "Environment Art", CallInEditor)
    void ValidateMaterialSetup();

    UFUNCTION(BlueprintCallable, Category = "Environment Art", CallInEditor)
    void LogMaterialStats();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material System")
    TArray<FEnvArt_MaterialConfig> MaterialConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Material System")
    TSoftObjectPtr<UMaterialParameterCollection> GlobalMaterialParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather System")
    FEnvArt_WeatherMaterialParams CurrentWeatherParams;

    UPROPERTY(BlueprintReadOnly, Category = "Runtime")
    TMap<EEnvArt_MaterialType, UMaterialInstance*> LoadedMaterials;

private:
    void LoadDefaultMaterialConfigs();
    void CreateMaterialInstances();
    UMaterialInstance* CreateDynamicMaterialInstance(const FEnvArt_MaterialConfig& Config);
    void UpdateMaterialParameterCollection();
};