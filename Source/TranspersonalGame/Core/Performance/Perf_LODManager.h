#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "SharedTypes.h"
#include "Perf_LODManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighDetailDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDetailDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowDetailDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableFrustumCulling = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_ActorLODData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LastDistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    EPerf_LODLevel CurrentLODLevel = EPerf_LODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bIsVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LastUpdateTime = 0.0f;
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
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void RegisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UnregisterActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UpdateLODForActor(AActor* Actor, float DistanceToPlayer);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    FPerf_LODSettings GetLODSettings() const { return LODSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void ForceUpdateAllActors();

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    int32 GetRegisteredActorCount() const { return RegisteredActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetUpdateFrequency(float NewFrequency);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    TArray<FPerf_ActorLODData> RegisteredActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    float TimeSinceLastUpdate = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance|LOD")
    bool bIsEnabled = true;

private:
    APawn* GetPlayerPawn() const;
    void UpdateActorLOD(FPerf_ActorLODData& ActorData, float DistanceToPlayer);
    void SetActorLODLevel(AActor* Actor, EPerf_LODLevel LODLevel);
    void CleanupInvalidActors();
};