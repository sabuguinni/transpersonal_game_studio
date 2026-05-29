#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "World_BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Grasslands      UMETA(DisplayName = "Grasslands"),
    Forest          UMETA(DisplayName = "Forest"),
    Mountains       UMETA(DisplayName = "Mountains"),
    Rivers          UMETA(DisplayName = "Rivers"),
    Wetlands        UMETA(DisplayName = "Wetlands"),
    Volcanic        UMETA(DisplayName = "Volcanic"),
    Desert          UMETA(DisplayName = "Desert")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Grasslands;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Size = FVector(10000.0f, 10000.0f, 1000.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Elevation = 0.0f;

    FWorld_BiomeData()
    {
        BiomeType = EWorld_BiomeType::Grasslands;
        Location = FVector::ZeroVector;
        Size = FVector(10000.0f, 10000.0f, 1000.0f);
        Temperature = 25.0f;
        Humidity = 0.5f;
        Elevation = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_BiomeManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_BiomeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biomes")
    TArray<FWorld_BiomeData> BiomeRegions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSizeX = 100000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    int32 WorldSizeY = 100000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Generation")
    float BiomeBlendDistance = 5000.0f;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    void AddBiomeRegion(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    TArray<FWorld_BiomeData> GetNearbyBiomes(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World Generation")
    void GenerateDefaultBiomes();

    UFUNCTION(BlueprintCallable, Category = "Biome Management")
    float GetBiomeInfluenceAtLocation(const FVector& Location, EWorld_BiomeType BiomeType) const;

private:
    void CreateDefaultBiomeLayout();
    float CalculateDistanceToBiome(const FVector& Location, const FWorld_BiomeData& BiomeData) const;
};