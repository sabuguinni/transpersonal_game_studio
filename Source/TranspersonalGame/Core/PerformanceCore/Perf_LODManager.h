#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Perf_LODManager.generated.h"

/**
 * Performance LOD Manager
 * Manages Level of Detail (LOD) for static meshes based on distance from player
 * Implements automatic LOD switching to maintain 60 FPS target
 */
UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_LODManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    /** Distance thresholds for LOD switching */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD0Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD1Distance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD2Distance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 10000.0f;

    /** Performance monitoring */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS = 60.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentFPS = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ManagedActorCount = 0;

    /** LOD management functions */
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RegisterStaticMeshActor(AStaticMeshActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UnregisterStaticMeshActor(AStaticMeshActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateLODForActor(AStaticMeshActor* Actor, float Distance);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODLevel(AStaticMeshActor* Actor, int32 LODLevel);

    /** Performance optimization */
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetPerformanceMode(bool bHighPerformanceMode);

private:
    /** Managed static mesh actors */
    UPROPERTY()
    TArray<AStaticMeshActor*> ManagedActors;

    /** Performance tracking */
    float FPSUpdateTimer = 0.0f;
    float LastFrameTime = 0.0f;
    bool bHighPerformanceMode = false;

    /** Internal LOD management */
    void UpdateAllLODs();
    void CalculateFPS(float DeltaTime);
    APawn* GetPlayerPawn();
    float GetDistanceToPlayer(AActor* Actor);
    int32 GetLODLevelForDistance(float Distance);
};