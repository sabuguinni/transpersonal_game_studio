#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_WaterSystem.generated.h"

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
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    float FlowSpeed;

    FWorld_RiverSegment()
    {
        StartPoint = FVector::ZeroVector;
        EndPoint = FVector::ZeroVector;
        Width = 1000.0f;
        Depth = 100.0f;
        FlowSpeed = 50.0f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_Lake
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    FString LakeName;

    FWorld_Lake()
    {
        Location = FVector::ZeroVector;
        Radius = 2000.0f;
        Depth = 200.0f;
        BiomeType = EBiomeType::Savanna;
        LakeName = TEXT("Unnamed Lake");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_WaterSystem : public AActor
{
    GENERATED_BODY()

public:
    AWorld_WaterSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    TArray<FWorld_RiverSegment> RiverSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    TArray<FWorld_Lake> Lakes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    UStaticMesh* WaterPlaneMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    UMaterialInterface* WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    float GlobalWaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    bool bEnableWaterFlow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    float WaterQuality;

public:
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void GenerateRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateLake(const FWorld_Lake& LakeData);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void ConnectBiomesWithRivers();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Water System")
    void RegenerateWaterSystem();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    bool IsLocationNearWater(const FVector& Location, float MaxDistance = 5000.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    FVector GetNearestWaterSource(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    float GetWaterDepthAtLocation(const FVector& Location) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void SetSeasonalWaterLevels(float SeasonMultiplier);

protected:
    UFUNCTION()
    void CreateRiverSegmentMesh(const FWorld_RiverSegment& Segment);

    UFUNCTION()
    void CreateLakeMesh(const FWorld_Lake& Lake);

    UFUNCTION()
    void UpdateWaterFlow();

private:
    TArray<UStaticMeshComponent*> WaterMeshComponents;
    
    void InitializeDefaultWaterSystem();
    void CleanupExistingWater();
};