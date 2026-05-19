#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "SharedTypes.h"
#include "World_ProceduralBiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Savana      UMETA(DisplayName = "Savana"),
    Pantano     UMETA(DisplayName = "Pantano"),
    Floresta    UMETA(DisplayName = "Floresta"),
    Deserto     UMETA(DisplayName = "Deserto"),
    Montanha    UMETA(DisplayName = "Montanha")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    int32 TargetActorCount = 500;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.7f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float RockDensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TerrainRoughness = 0.5f;

    FWorld_BiomeConfig()
    {
        BiomeType = EWorld_BiomeType::Savana;
        CenterLocation = FVector::ZeroVector;
        Radius = 10000.0f;
        TargetActorCount = 500;
        VegetationDensity = 0.7f;
        RockDensity = 0.3f;
        TerrainRoughness = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FWorld_ProceduralAsset
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    TSoftObjectPtr<UStaticMesh> AssetMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    FVector ScaleRange = FVector(0.8f, 1.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    float SpawnWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Asset")
    bool bRandomRotation = true;

    FWorld_ProceduralAsset()
    {
        ScaleRange = FVector(0.8f, 1.2f, 1.0f);
        SpawnWeight = 1.0f;
        bRandomRotation = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_ProceduralBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_ProceduralBiomeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome System")
    TArray<FWorld_BiomeConfig> BiomeConfigs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TMap<EWorld_BiomeType, TArray<FWorld_ProceduralAsset>> BiomeAssets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float GenerationRadius = 50000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUsePerformanceBudget = true;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    int32 TotalGeneratedActors = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Status")
    bool bGenerationComplete = false;

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void GenerateBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void PopulateBiomeWithAssets(const FWorld_BiomeConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    FVector GetRandomLocationInBiome(const FWorld_BiomeConfig& BiomeConfig);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    AActor* SpawnProceduralAsset(const FWorld_ProceduralAsset& Asset, const FVector& Location, EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Biome System")
    void ClearBiome(EWorld_BiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceBudget(int32 MaxActors, float UpdateRadius);

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void DebugBiomeStatus();

private:
    void SetupDefaultBiomeConfigs();
    void LoadBiomeAssets();
    bool IsLocationValid(const FVector& Location, const FWorld_BiomeConfig& BiomeConfig);
    float GetTerrainHeight(const FVector& Location);

    UPROPERTY()
    TArray<AActor*> GeneratedActors;

    int32 CurrentGenerationIndex = 0;
    bool bIsGenerating = false;
    float LastGenerationTime = 0.0f;
};