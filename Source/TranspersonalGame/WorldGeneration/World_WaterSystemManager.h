#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "World_WaterSystemManager.generated.h"

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
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River")
    FString BiomeType;

    FWorld_RiverSegment()
    {
        StartPoint = FVector::ZeroVector;
        EndPoint = FVector::ZeroVector;
        Width = 200.0f;
        FlowSpeed = 1.0f;
        BiomeType = TEXT("Forest");
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    FVector Position;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    float Radius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    FString LakeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lake")
    bool bHasFish;

    FWorld_LakeData()
    {
        Position = FVector::ZeroVector;
        Radius = 500.0f;
        Depth = 100.0f;
        LakeType = TEXT("Freshwater");
        bHasFish = true;
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
    TArray<FWorld_RiverSegment> RiverSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water System")
    TArray<FWorld_LakeData> Lakes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    class UMaterialInterface* RiverMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meshes")
    class UStaticMesh* WaterPlaneMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxRiverSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxLakes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinDistanceBetweenLakes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseInstancedMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float WaterLODDistance;

public:
    UFUNCTION(BlueprintCallable, Category = "Water System")
    void GenerateRiverSystem();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void GenerateLakes();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateRiverSegment(const FWorld_RiverSegment& SegmentData);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void CreateLake(const FWorld_LakeData& LakeData);

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void ClearWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Water System")
    bool IsPositionNearWater(const FVector& Position, float Threshold = 500.0f) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    FVector GetNearestWaterPosition(const FVector& FromPosition) const;

    UFUNCTION(BlueprintCallable, Category = "Water System")
    void UpdateWaterFlow(float DeltaTime);

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void GenerateWaterSystemInEditor();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void ValidateWaterSystem();

protected:
    UFUNCTION()
    void OnRiverSegmentCreated(class AActor* NewActor);

    UFUNCTION()
    void OnLakeCreated(class AActor* NewActor);

    void SetupWaterMaterial(class UStaticMeshComponent* MeshComponent, bool bIsRiver = false);
    void OptimizeWaterRendering();
    FVector CalculateRiverFlowDirection(int32 SegmentIndex) const;

private:
    TArray<class AActor*> SpawnedWaterActors;
    float WaterSystemBounds;
    bool bWaterSystemInitialized;
};