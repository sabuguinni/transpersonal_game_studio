#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Perf_LODManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODLevel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 LODIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bShouldRender;

    FPerf_LODLevel()
    {
        Distance = 1000.0f;
        LODIndex = 0;
        bShouldRender = true;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_LODManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    TArray<FPerf_LODLevel> LODLevels;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bAutoManageLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullingDistance;

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateLODForActor(AActor* Actor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODLevel(AActor* Actor, int32 LODIndex);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void CullActor(AActor* Actor, bool bShouldCull);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    int32 GetCurrentLODLevel(float Distance);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RegisterActorForLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UnregisterActorFromLOD(AActor* Actor);

private:
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ManagedActors;

    UPROPERTY()
    APawn* PlayerPawn;

    float TimeSinceLastUpdate;

    void FindPlayerPawn();
    void UpdateAllManagedActors();
    float CalculateDistanceToPlayer(AActor* Actor);
};