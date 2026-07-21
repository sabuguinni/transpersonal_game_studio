#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "../SharedTypes.h"
#include "World_WaterBodyGenerator.generated.h"

UENUM(BlueprintType)
enum class EWorld_WaterBodyType : uint8
{
    River      UMETA(DisplayName = "River"),
    Lake       UMETA(DisplayName = "Lake"),
    Pond       UMETA(DisplayName = "Pond"),
    Stream     UMETA(DisplayName = "Stream"),
    Waterfall  UMETA(DisplayName = "Waterfall")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBodyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    EWorld_WaterBodyType WaterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    FVector StartLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    FVector EndLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    int32 SegmentCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    FString BiomeName;

    FWorld_WaterBodyData()
    {
        WaterType = EWorld_WaterBodyType::River;
        StartLocation = FVector::ZeroVector;
        EndLocation = FVector(1000.0f, 0.0f, 0.0f);
        Width = 500.0f;
        Depth = 100.0f;
        SegmentCount = 10;
        FlowSpeed = 1.0f;
        BiomeName = TEXT("Unknown");
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_WaterBodyGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_WaterBodyGenerator();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
    TArray<FWorld_WaterBodyData> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
    UMaterialInterface* WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
    UStaticMesh* WaterPlaneMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
    bool bAutoGenerateOnStart;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
    int32 MaxWaterBodiesPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Generation")
    float MinDistanceBetweenWaterBodies;

public:
    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void GenerateAllWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void GenerateWaterBodyForBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    AActor* CreateRiver(const FWorld_WaterBodyData& RiverData);

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    AActor* CreateLake(const FWorld_WaterBodyData& LakeData);

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    AActor* CreatePond(const FWorld_WaterBodyData& PondData);

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    void ClearAllWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    TArray<AActor*> GetWaterBodiesInBiome(const FString& BiomeName);

    UFUNCTION(BlueprintCallable, Category = "Water Generation", CallInEditor = true)
    void DebugGenerateTestWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Water Generation")
    bool IsLocationNearWater(const FVector& Location, float CheckRadius = 1000.0f);

private:
    void SetupDefaultWaterBodies();
    FVector GetBiomeCenterLocation(const FString& BiomeName);
    AActor* CreateWaterPlane(const FVector& Location, const FVector& Scale, const FString& Name);
    void ApplyWaterMaterial(AActor* WaterActor);
    TArray<FVector> GenerateRiverPath(const FVector& Start, const FVector& End, int32 Segments);
};