// BiomeSystem.h
// Agent #05 — Procedural World Generator
// Biome system for prehistoric survival world — 4 biomes: Jungle, Plains, Badlands, Swamp

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Plains      UMETA(DisplayName = "Plains"),
    Badlands    UMETA(DisplayName = "Volcanic Badlands"),
    Swamp       UMETA(DisplayName = "Swamp"),
    None        UMETA(DisplayName = "None")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector WorldCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel;

    FWorld_BiomeData()
        : BiomeType(EWorld_BiomeType::None)
        , WorldCenter(FVector::ZeroVector)
        , Radius(3000.0f)
        , Temperature(25.0f)
        , Humidity(0.5f)
        , VegetationDensity(0.5f)
        , DangerLevel(0.5f)
    {}
};

UCLASS(ClassGroup = (TranspersonalGame), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_BiomeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_BiomeComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FWorld_BiomeData BiomeData;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetBiomeBlendWeight(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    bool IsLocationInBiome(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

protected:
    virtual void BeginPlay() override;
};

UCLASS(ClassGroup = (TranspersonalGame), Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Manager")
    TArray<FWorld_BiomeData> RegisteredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome Manager")
    float BiomeTransitionRadius;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    EWorld_BiomeType GetDominantBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    TArray<FWorld_BiomeData> GetBlendedBiomesAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    void RegisterBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Manager")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Biome Manager")
    void InitializeDefaultBiomes();

protected:
    virtual void BeginPlay() override;

private:
    void SetupJungleBiome();
    void SetupPlainsBiome();
    void SetupBadlandsBiome();
    void SetupSwampBiome();
};
