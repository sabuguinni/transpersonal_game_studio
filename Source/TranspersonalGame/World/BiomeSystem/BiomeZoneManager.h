#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../../SharedTypes.h"
#include "BiomeZoneManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    None         UMETA(DisplayName = "None"),
    DenseJungle  UMETA(DisplayName = "Dense Jungle"),
    RockyHighlands UMETA(DisplayName = "Rocky Highlands"),
    RiverValley  UMETA(DisplayName = "River Valley"),
    OpenPlains   UMETA(DisplayName = "Open Plains"),
    VolcanicZone UMETA(DisplayName = "Volcanic Zone"),
    Marshland    UMETA(DisplayName = "Marshland")
};

USTRUCT(BlueprintType)
struct FWorld_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CenterLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor AmbientColor = FLinearColor(0.1f, 0.9f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float TemperatureCelsius = 28.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityPercent = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bHasPredators = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurDensity = 1.0f;
};

UCLASS(ClassGroup = (World), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ABiomeZoneManager : public AActor
{
    GENERATED_BODY()

public:
    ABiomeZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    FWorld_BiomeData GetBiomeData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    void RegisterBiome(const FWorld_BiomeData& BiomeData);

    UFUNCTION(BlueprintCallable, Category = "Biome")
    TArray<FWorld_BiomeData> GetAllBiomes() const;

    UFUNCTION(BlueprintCallable, Category = "Biome")
    float GetBiomeBlendFactor(const FVector& WorldLocation, EWorld_BiomeType BiomeType) const;

    UFUNCTION(CallInEditor, Category = "Biome")
    void InitializeDefaultBiomes();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<FWorld_BiomeData> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    bool bShowDebugBiomeBounds = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BiomeBlendRadius = 500.0f;

private:
    void SetupDefaultBiomes();
};
