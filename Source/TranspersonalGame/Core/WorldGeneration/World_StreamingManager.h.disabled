#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "World_StreamingManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_StreamingCell
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    FVector CellLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    float CellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    EBiomeType BiomeType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsLoaded;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    bool bIsVisible;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming")
    TArray<AActor*> CellActors;

    FWorld_StreamingCell()
    {
        CellLocation = FVector::ZeroVector;
        CellSize = 100000.0f; // 1km
        BiomeType = EBiomeType::Forest;
        bIsLoaded = false;
        bIsVisible = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FWorld_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance;

    FWorld_LODSettings()
    {
        HighDetailDistance = 100000.0f;   // 1km
        MediumDetailDistance = 300000.0f; // 3km
        LowDetailDistance = 800000.0f;    // 8km
        CullDistance = 1600000.0f;        // 16km
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UWorld_StreamingManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UWorld_StreamingManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core streaming functionality
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void InitializeStreamingGrid(int32 GridSize, float CellSize);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void UpdatePlayerLocation(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void LoadCell(int32 CellX, int32 CellY);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void UnloadCell(int32 CellX, int32 CellY);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void UpdateCellLOD(int32 CellX, int32 CellY, float DistanceToPlayer);

    // Biome management
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void SetCellBiome(int32 CellX, int32 CellY, EBiomeType BiomeType);

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    EBiomeType GetCellBiome(int32 CellX, int32 CellY) const;

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void OptimizeMemoryUsage();

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void UpdateRenderingBudget();

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    int32 GetActiveDrawCalls() const;

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    float GetMemoryUsageMB() const;

    // Debug and monitoring
    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    void DebugDrawStreamingGrid();

    UFUNCTION(BlueprintCallable, Category = "World Streaming")
    TArray<FString> GetStreamingStats() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    int32 StreamingGridSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    float StreamingCellSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    float StreamingRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Streaming Settings")
    FWorld_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxDrawCallsPerFrame;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxMemoryUsageMB;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency;

private:
    TMap<FIntPoint, FWorld_StreamingCell> StreamingCells;
    FVector LastPlayerLocation;
    float TimeSinceLastUpdate;
    
    // Helper functions
    FIntPoint WorldLocationToCellCoords(const FVector& WorldLocation) const;
    FVector CellCoordsToWorldLocation(const FIntPoint& CellCoords) const;
    bool IsCellInRange(const FIntPoint& CellCoords, const FVector& PlayerLocation) const;
    void CreateCellActors(FWorld_StreamingCell& Cell);
    void DestroyCellActors(FWorld_StreamingCell& Cell);
    void UpdateActorLOD(AActor* Actor, float DistanceToPlayer);
};