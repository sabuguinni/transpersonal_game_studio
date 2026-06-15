#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "World_RiverSystemGenerator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    FVector StartPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    FVector EndPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    float Width;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Segment")
    bool bIsMainChannel;

    FWorld_RiverSegment()
    {
        StartPoint = FVector::ZeroVector;
        EndPoint = FVector::ZeroVector;
        Width = 500.0f;
        Depth = 100.0f;
        FlowSpeed = 200.0f;
        bIsMainChannel = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_RiverNetwork
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    TArray<FWorld_RiverSegment> MainRiver;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    TArray<FWorld_RiverSegment> Tributaries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    TArray<FVector> LakePositions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    FVector SourceLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    FVector DeltaLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Network")
    float TotalLength;

    FWorld_RiverNetwork()
    {
        SourceLocation = FVector::ZeroVector;
        DeltaLocation = FVector::ZeroVector;
        TotalLength = 0.0f;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_RiverSystemGenerator : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_RiverSystemGenerator();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // River Generation Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    int32 MaxRiverSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float MinRiverWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float MaxRiverWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float RiverMeandering;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    int32 TributaryCount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Generation")
    float LakeProbability;

    // Water Materials
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* RiverWaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* LakeWaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* RiverbankMaterial;

    // Generated Network
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generated Data")
    FWorld_RiverNetwork GeneratedNetwork;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generated Data")
    TArray<AActor*> SpawnedRiverActors;

    // Blueprint-Callable Functions
    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void GenerateRiverNetwork(FVector StartLocation, FVector EndLocation);

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void GenerateTributaries(const FWorld_RiverSegment& MainSegment);

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void SpawnRiverMeshes();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void SpawnLakes();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void SpawnRiverbanks();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void ClearGeneratedRivers();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    FVector CalculateRiverFlow(const FVector& CurrentPoint, const FVector& TargetPoint);

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    bool IsValidRiverLocation(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "River Generation", CallInEditor = true)
    void GenerateTestRiver();

private:
    // Internal generation functions
    FWorld_RiverSegment CreateRiverSegment(const FVector& Start, const FVector& End, float Width, bool bMainChannel);
    void ApplyRiverMeandering(FWorld_RiverSegment& Segment);
    AActor* SpawnRiverSegmentMesh(const FWorld_RiverSegment& Segment);
    AActor* SpawnLakeAtLocation(const FVector& Location, float Radius);
    void GenerateRiverbankGeometry(const FWorld_RiverSegment& Segment);
    float CalculateTerrainElevation(const FVector& Location);
    FVector GetDownhillDirection(const FVector& Location);
};

#include "World_RiverSystemGenerator.generated.h"