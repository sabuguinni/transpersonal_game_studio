#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/StaticMeshActor.h"
#include "RiverSystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRiverSystem, Log, All);

/**
 * River System for massive 200km2 world generation
 * Creates flowing rivers connecting all 5 biomes with realistic water flow
 * Supports dynamic river generation, width adjustment, and flow simulation
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ARiverSystem : public AActor
{
    GENERATED_BODY()

public:
    ARiverSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // River Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Settings")
    float RiverWidth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Settings")
    float RiverDepth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Settings")
    float FlowSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Settings")
    float WaterLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Settings")
    bool bIsFlowing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Settings")
    FString WaterMaterialPath;

    // River Points and Segments
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "River Geometry")
    TArray<FVector> RiverPoints;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "River Geometry")
    TArray<AStaticMeshActor*> RiverSegments;

    // River Generation Functions
    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void GenerateDefaultRiverPath();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void CreateRiverMesh();

    UFUNCTION(BlueprintCallable, Category = "River Generation")
    void CreateRiverSegment(const FVector& StartPoint, const FVector& EndPoint, int32 SegmentIndex);

    // River Control Functions
    UFUNCTION(BlueprintCallable, Category = "River Control")
    void SetRiverWidth(float NewWidth);

    UFUNCTION(BlueprintCallable, Category = "River Control")
    void SetFlowSpeed(float NewSpeed);

    UFUNCTION(BlueprintCallable, Category = "River Control")
    void AddRiverPoint(const FVector& NewPoint);

    UFUNCTION(BlueprintCallable, Category = "River Control")
    void ClearRiverPoints();

    // River Query Functions
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "River Query")
    TArray<FVector> GetRiverPoints() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "River Query")
    int32 GetSegmentCount() const;

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "River Query")
    bool IsPointNearRiver(const FVector& TestPoint, float MaxDistance = 1000.0f) const;

protected:
    // Internal river update
    void UpdateWaterFlow(float DeltaTime);
};