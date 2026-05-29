#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_WaterSystemManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBody
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float WaterDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bIsRiver;

    FWorld_WaterBody()
    {
        Location = FVector::ZeroVector;
        Scale = FVector::OneVector;
        BiomeType = EBiomeType::Grassland;
        WaterDepth = 100.0f;
        bIsRiver = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FVector EndPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowRate;

    FWorld_RiverSegment()
    {
        StartPoint = FVector::ZeroVector;
        EndPoint = FVector::ZeroVector;
        Width = 500.0f;
        FlowRate = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_WaterSystemManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_WaterSystemManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Bodies")
    TArray<FWorld_WaterBody> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rivers")
    TArray<FWorld_RiverSegment> RiverSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
    float GlobalWaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
    float WaterFlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Settings")
    bool bEnableWaterPhysics;

public:
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void GenerateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void UpdateWaterLevels();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    FWorld_WaterBody CreateLakeAtLocation(FVector Location, EBiomeType BiomeType, float Size);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void ConnectBiomesWithRiver(FVector StartBiome, FVector EndBiome);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    bool IsLocationNearWater(FVector Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    float GetWaterDepthAtLocation(FVector Location) const;

private:
    void SpawnWaterMeshAtLocation(const FWorld_WaterBody& WaterBody);
    void CreateRiverMesh(const FWorld_RiverSegment& RiverSegment);
    void SetupWaterMaterial(UStaticMeshComponent* WaterMesh);
};