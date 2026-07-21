#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMeshActor.h"
#include "World_CaveSystemGenerator.generated.h"

UENUM(BlueprintType)
enum class EWorld_CaveType : uint8
{
    SmallCave,
    MediumCave,
    LargeCave,
    UndergroundTunnel,
    CrystalCave,
    WaterCave
};

USTRUCT(BlueprintType)
struct FWorld_CaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EWorld_CaveType CaveType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EntranceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CaveDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float CaveWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TunnelCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasWater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bHasCrystals;

    FWorld_CaveData()
    {
        CaveType = EWorld_CaveType::SmallCave;
        EntranceLocation = FVector::ZeroVector;
        CaveDepth = 500.0f;
        CaveWidth = 300.0f;
        TunnelCount = 3;
        bHasWater = false;
        bHasCrystals = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_CaveSystemGenerator : public AActor
{
    GENERATED_BODY()

public:
    AWorld_CaveSystemGenerator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generation")
    TArray<FWorld_CaveData> CaveConfigurations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generation")
    int32 MaxCavesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generation")
    float MinDistanceBetweenCaves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generation")
    bool bGenerateUndergroundNetwork;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cave Generation")
    float UndergroundNetworkDepth;

public:
    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    void GenerateCaveSystem();

    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    void GenerateCaveEntrance(const FWorld_CaveData& CaveData);

    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    void GenerateUndergroundTunnels(const FVector& StartLocation, int32 TunnelCount);

    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    void CreateCaveLighting(const FVector& Location, EWorld_CaveType CaveType);

    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    void AddCaveWaterFeatures(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    void AddCaveCrystals(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    bool IsValidCaveLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Cave Generation")
    TArray<FVector> GenerateTunnelPath(const FVector& Start, const FVector& End);

private:
    void InitializeCaveConfigurations();
    FVector GetRandomCaveLocation() const;
    float CalculateCaveComplexity(EWorld_CaveType CaveType) const;
};