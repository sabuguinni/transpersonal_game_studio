#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/SkeletalMesh.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    High = 0,
    Medium = 1,
    Low = 2,
    Culled = 3
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighDetailDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDetailDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowDetailDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableFrustumCulling = true;

    FPerf_LODSettings()
    {
        HighDetailDistance = 2000.0f;
        MediumDetailDistance = 5000.0f;
        LowDetailDistance = 10000.0f;
        CullDistance = 15000.0f;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
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
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_LODLevel CurrentLODLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DistanceToPlayer;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ManagedActorsCount;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODForActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateAllManagedActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterActorForLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterActorFromLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_LODLevel CalculateLODLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODLevel(AActor* Actor, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeSkeletalMesh(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetDistanceToPlayer(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsActorInPlayerView(AActor* Actor) const;

private:
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ManagedActors;

    UPROPERTY()
    APawn* CachedPlayerPawn;

    float LastUpdateTime;

    void CachePlayerPawn();
    void CleanupInvalidActors();
    void ApplyLODToComponents(AActor* Actor, EPerf_LODLevel LODLevel);
};