#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    TropicalJungle   UMETA(DisplayName = "Tropical Jungle"),
    OpenSavanna      UMETA(DisplayName = "Open Savanna"),
    RockyHighlands   UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta       UMETA(DisplayName = "River Delta"),
    DenseForest      UMETA(DisplayName = "Dense Forest"),
    VolcanicPlains   UMETA(DisplayName = "Volcanic Plains"),
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::OpenSavanna;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DangerLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.7f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Biome query
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeData GetBiomeDataAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetDangerLevelAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetFoliageDensityAtLocation(const FVector& WorldLocation) const;

    // Biome registration
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    void RegisterBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    void ClearAllBiomes();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biome")
    void InitializeDefaultBiomes();

    // Debug
    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    int32 GetBiomeCount() const { return RegisteredBiomes.Num(); }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<FWorld_BiomeData> RegisteredBiomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float BiomeBlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    bool bAutoInitOnBeginPlay = true;

private:
    const FWorld_BiomeData* FindClosestBiome(const FVector& WorldLocation) const;
    float CalculateBlendWeight(const FVector& Location, const FWorld_BiomeData& Biome) const;
};
