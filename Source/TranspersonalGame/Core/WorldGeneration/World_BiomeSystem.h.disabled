#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "World_BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Swamp       UMETA(DisplayName = "Swamp"),
    Forest      UMETA(DisplayName = "Forest"), 
    Savanna     UMETA(DisplayName = "Savanna"),
    Desert      UMETA(DisplayName = "Desert"),
    SnowyMountain UMETA(DisplayName = "Snowy Mountain")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TSoftObjectPtr<UMaterialInterface> GroundMaterial;

    FWorld_BiomeConfig()
    {
        BiomeType = EWorld_BiomeType::Forest;
        AmbientColor = FLinearColor::White;
        Temperature = 20.0f;
        Humidity = 0.5f;
        VegetationDensity = 1.0f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zone")
    float Influence = 1.0f;

    FWorld_BiomeZone()
    {
        Center = FVector::ZeroVector;
        Radius = 5000.0f;
        BiomeType = EWorld_BiomeType::Forest;
        Influence = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Biome Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeZone> BiomeZones;

    // World Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    float WorldSize = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World")
    int32 BiomeTransitionSmoothness = 5;

    // Vegetation Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    float VegetationSpawnRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    int32 MaxVegetationPerBiome = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vegetation")
    bool bAutoSpawnVegetation = true;

    // Methods
    UFUNCTION(BlueprintCallable, Category = "Biome System")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FWorld_BiomeConfig GetBiomeConfig(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void SpawnVegetationInBiome(EWorld_BiomeType BiomeType, const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void CreateBiomeZone(const FVector& Center, float Radius, EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor)
    void RegenerateBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System", CallInEditor)
    void ClearAllVegetation();

    // Utility Methods
    UFUNCTION(BlueprintPure, Category = "Biome System")
    float GetTemperatureAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "Biome System")
    float GetHumidityAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintPure, Category = "Biome System")
    FLinearColor GetAmbientColorAtLocation(const FVector& Location) const;

private:
    // Internal state
    bool bBiomesInitialized = false;
    TArray<AActor*> SpawnedVegetation;

    // Helper methods
    void SetupDefaultBiomes();
    void SpawnVegetationMesh(UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation);
    FVector GetRandomLocationInRadius(const FVector& Center, float Radius) const;
    float CalculateBiomeInfluence(const FVector& Location, const FWorld_BiomeZone& Zone) const;
};