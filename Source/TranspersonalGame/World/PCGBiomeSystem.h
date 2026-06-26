#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCGBiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle      UMETA(DisplayName = "Jungle"),
    Savanna     UMETA(DisplayName = "Savanna"),
    Badlands    UMETA(DisplayName = "Badlands"),
    Wetlands    UMETA(DisplayName = "Wetlands"),
    Volcanic    UMETA(DisplayName = "Volcanic")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Jungle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.8f;
};

UCLASS(ClassGroup=(World), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API APCGBiomeSystem : public AActor
{
    GENERATED_BODY()

public:
    APCGBiomeSystem();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "PCG|Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "PCG|Biome")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "PCG|Biome")
    void RegisterBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(CallInEditor, Category = "PCG|Biome")
    void GenerateBiomeLayout();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Biome")
    TArray<FWorld_BiomeData> Biomes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Biome")
    bool bAutoGenerateOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCG|Biome")
    float WorldRadius = 20000.0f;

private:
    void InitializeDefaultBiomes();
    float CalculateBiomeBlend(const FVector& Location, const FWorld_BiomeData& Biome) const;
};
