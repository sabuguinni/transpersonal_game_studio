#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "EnvironmentManager.generated.h"

UENUM(BlueprintType)
enum class EEnvArt_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Floresta    UMETA(DisplayName = "Floresta"), 
    Pantano     UMETA(DisplayName = "Pantano"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_BiomeCoordinates
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEnvArt_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    FEnvArt_BiomeCoordinates()
    {
        BiomeType = EEnvArt_BiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEnvArt_EnvironmentAsset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    FString AssetName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> MeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    EEnvArt_BiomeType PreferredBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    float SpawnDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    FVector ScaleRange;

    FEnvArt_EnvironmentAsset()
    {
        AssetName = TEXT("DefaultAsset");
        PreferredBiome = EEnvArt_BiomeType::Savana;
        SpawnDensity = 1.0f;
        ScaleRange = FVector(0.8f, 1.2f, 1.0f);
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEnvironmentManager : public AActor
{
    GENERATED_BODY()

public:
    AEnvironmentManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FEnvArt_BiomeCoordinates> BiomeDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    TArray<FEnvArt_EnvironmentAsset> EnvironmentAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    int32 AssetsPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    float SpawnRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PopulateBiome(EEnvArt_BiomeType BiomeType, int32 AssetCount = 100);

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void PopulateAllBiomes();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    FVector GetBiomeCenter(EEnvArt_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetGoldenHourLighting();

    UFUNCTION(BlueprintCallable, Category = "Environment")
    void AddVolumetricFog();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void InitializeBiomeDefinitions();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Environment")
    void DiscoverEnvironmentAssets();

private:
    void SpawnAssetAtLocation(const FEnvArt_EnvironmentAsset& Asset, const FVector& Location, const FRotator& Rotation);
    FVector GetRandomLocationInBiome(const FEnvArt_BiomeCoordinates& Biome) const;
    FRotator GetRandomRotation() const;
};