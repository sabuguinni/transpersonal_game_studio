#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "World_WaterBodyManager.generated.h"

UENUM(BlueprintType)
enum class EWorld_WaterBodyType : uint8
{
    Lake        UMETA(DisplayName = "Lake"),
    River       UMETA(DisplayName = "River"),
    Pond        UMETA(DisplayName = "Pond"),
    Stream      UMETA(DisplayName = "Stream"),
    Waterfall   UMETA(DisplayName = "Waterfall")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_WaterBodyData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    EWorld_WaterBodyType WaterType = EWorld_WaterBodyType::Lake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    FVector Scale = FVector(10.0f, 10.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float Depth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    bool bIsSwimmable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Body")
    float CurrentStrength = 0.0f;

    FWorld_WaterBodyData()
    {
        WaterType = EWorld_WaterBodyType::Lake;
        Location = FVector::ZeroVector;
        Scale = FVector(10.0f, 10.0f, 1.0f);
        Depth = 100.0f;
        bIsSwimmable = true;
        CurrentStrength = 0.0f;
    }
};

/**
 * Manages procedural water bodies in the game world
 * Creates and maintains lakes, rivers, ponds, and streams
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AWorld_WaterBodyManager : public AActor
{
    GENERATED_BODY()

public:
    AWorld_WaterBodyManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Bodies")
    TArray<FWorld_WaterBodyData> WaterBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Bodies")
    UStaticMesh* WaterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Water Bodies")
    UMaterialInterface* WaterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    int32 MaxWaterBodies = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    float MinWaterBodyDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Generation")
    FVector WorldBounds = FVector(10000.0f, 10000.0f, 2000.0f);

public:
    UFUNCTION(BlueprintCallable, Category = "Water Bodies")
    void GenerateWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Water Bodies")
    void ClearWaterBodies();

    UFUNCTION(BlueprintCallable, Category = "Water Bodies")
    AActor* CreateWaterBody(const FWorld_WaterBodyData& WaterData);

    UFUNCTION(BlueprintCallable, Category = "Water Bodies")
    void CreateRiverSystem(const FVector& StartLocation, const FVector& EndLocation, int32 Segments = 5);

    UFUNCTION(BlueprintCallable, Category = "Water Bodies")
    void CreateLake(const FVector& Location, float Radius = 1000.0f);

    UFUNCTION(BlueprintCallable, Category = "Water Bodies")
    TArray<AActor*> GetWaterActorsInRadius(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "Water Bodies")
    bool IsLocationNearWater(const FVector& Location, float CheckRadius = 200.0f) const;

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Editor")
    void RegenerateAllWaterBodies();

private:
    UPROPERTY()
    TArray<AActor*> SpawnedWaterActors;

    void InitializeWaterMesh();
    FVector GetRandomLocationInBounds() const;
    bool IsValidWaterLocation(const FVector& Location) const;
};

#include "World_WaterBodyManager.generated.h"