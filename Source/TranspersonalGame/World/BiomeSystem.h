#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "BiomeSystem.generated.h"

UENUM(BlueprintType)
enum class EWorld_BiomeType : uint8
{
    Jungle    UMETA(DisplayName = "Jungle"),
    Plains    UMETA(DisplayName = "Plains"),
    Rocky     UMETA(DisplayName = "Rocky Highlands"),
    Wetlands  UMETA(DisplayName = "Wetlands"),
    Volcanic  UMETA(DisplayName = "Volcanic"),
    Count     UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FWorld_BiomeZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EWorld_BiomeType BiomeType = EWorld_BiomeType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FVector CentreLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Radius = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseElevation = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float FoliageDensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Temperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float Humidity = 0.5f;
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
    EWorld_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    FWorld_BiomeZone GetBiomeZoneData(EWorld_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetTemperatureAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    float GetHumidityAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "World|Biome")
    void InitialiseBiomeZones();

    UFUNCTION(BlueprintCallable, Category = "World|Biome")
    bool IsInWaterZone(const FVector& WorldLocation) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    TArray<FWorld_BiomeZone> BiomeZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float TransitionBlendRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    float WaterZoneRadius = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World|Biome")
    FVector WaterSourceLocation = FVector(8000.0f, 0.0f, 50.0f);

private:
    void SetupDefaultBiomes();
};
