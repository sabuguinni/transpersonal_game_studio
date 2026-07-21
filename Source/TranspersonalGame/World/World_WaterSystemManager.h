#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "World_WaterSystemManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_WaterType : uint8
{
    River           UMETA(DisplayName = "River"),
    Lake            UMETA(DisplayName = "Lake"),
    Swamp           UMETA(DisplayName = "Swamp"),
    Stream          UMETA(DisplayName = "Stream"),
    Pond            UMETA(DisplayName = "Pond")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBodyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    EWorld_WaterType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector Scale;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    bool bHasCurrentFlow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water")
    FVector FlowDirection;

    FWorld_WaterBodyData()
    {
        WaterType = EWorld_WaterType::Lake;
        Location = FVector::ZeroVector;
        Scale = FVector(10.0f, 10.0f, 1.0f);
        Depth = 100.0f;
        bHasCurrentFlow = false;
        FlowDirection = FVector::ForwardVector;
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
    class USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    TArray<FWorld_WaterBodyData> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    UStaticMesh* WaterPlaneMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    UMaterialInterface* WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    bool bEnableWaterPhysics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    float WaterDensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    float WaterViscosity;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateWaterBody(const FWorld_WaterBodyData& WaterData);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void RemoveWaterBody(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void UpdateWaterLevel(float NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    bool IsLocationInWater(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    float GetWaterDepthAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    FVector GetWaterFlowAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void GenerateBiomeWaterSystems();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateRiverSystem(const FVector& StartLocation, const FVector& EndLocation, float Width = 500.0f);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateLakeSystem(const FVector& CenterLocation, float Radius = 2000.0f);

private:
    TArray<UStaticMeshComponent*> WaterMeshComponents;

    void SpawnWaterMesh(const FWorld_WaterBodyData& WaterData);
    void UpdateWaterFlow(float DeltaTime);
    void ApplyWaterPhysics();
};