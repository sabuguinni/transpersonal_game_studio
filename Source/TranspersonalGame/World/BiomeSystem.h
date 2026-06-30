#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
#include "BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Forest       UMETA(DisplayName = "Dense Forest"),
    OpenPlains   UMETA(DisplayName = "Open Plains"),
    RockyHighlands UMETA(DisplayName = "Rocky Highlands"),
    RiverDelta   UMETA(DisplayName = "River Delta"),
    VolcanicZone UMETA(DisplayName = "Volcanic Zone"),
    Swampland    UMETA(DisplayName = "Swampland")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector2D WorldCenter = FVector2D::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.6f, 0.8f, 0.6f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ABiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    ABiomeSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    void InitializeBiomes();

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    void SpawnBiomeVegetation(const FWorld_BiomeData& BiomeData, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetBlendWeight(const FVector& Location, const FWorld_BiomeData& Biome) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<FWorld_BiomeData> Biomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float BiomeBlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    bool bAutoInitializeOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    int32 VegetationSeedPerBiome = 50;
};
