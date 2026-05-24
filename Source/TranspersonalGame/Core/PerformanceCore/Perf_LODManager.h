#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "../SharedTypes.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    High    UMETA(DisplayName = "High Detail"),
    Medium  UMETA(DisplayName = "Medium Detail"),
    Low     UMETA(DisplayName = "Low Detail"),
    Culled  UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
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

    FPerf_LODSettings()
    {
        HighDetailDistance = 1000.0f;
        MediumDetailDistance = 3000.0f;
        LowDetailDistance = 8000.0f;
        CullDistance = 15000.0f;
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    EPerf_LODLevel CurrentLODLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float DistanceToPlayer;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLODLevel();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_LODLevel CalculateLODLevel(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODLevel(EPerf_LODLevel NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void RegisterManagedActor(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UnregisterManagedActor(AActor* Actor);

private:
    UPROPERTY()
    TArray<AActor*> ManagedActors;

    float LastUpdateTime;
    APawn* PlayerPawn;

    void FindPlayerPawn();
    float CalculateDistanceToPlayer();
    void UpdateManagedActors();
};