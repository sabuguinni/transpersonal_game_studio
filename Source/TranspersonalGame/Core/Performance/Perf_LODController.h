#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Perf_LODController.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableFrustumCulling = true;

    FPerf_LODSettings()
    {
        LOD0Distance = 1000.0f;
        LOD1Distance = 2500.0f;
        LOD2Distance = 5000.0f;
        CullDistance = 10000.0f;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
    }
};

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD0    UMETA(DisplayName = "High Detail"),
    LOD1    UMETA(DisplayName = "Medium Detail"),
    LOD2    UMETA(DisplayName = "Low Detail"),
    Culled  UMETA(DisplayName = "Culled")
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_LODController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_LODController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateLOD();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerf_LODSettings GetLODSettings() const { return LODSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerf_LODLevel GetCurrentLODLevel() const { return CurrentLODLevel; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetDistanceToPlayer() const { return DistanceToPlayer; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsEnabled() const { return bIsEnabled; }

private:
    void CalculateDistanceToPlayer();
    void ApplyLODLevel(EPerf_LODLevel NewLODLevel);
    bool IsInPlayerView() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (AllowPrivateAccess = "true"))
    FPerf_LODSettings LODSettings;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD", meta = (AllowPrivateAccess = "true"))
    EPerf_LODLevel CurrentLODLevel = EPerf_LODLevel::LOD0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD", meta = (AllowPrivateAccess = "true"))
    float DistanceToPlayer = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (AllowPrivateAccess = "true"))
    bool bIsEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD", meta = (AllowPrivateAccess = "true"))
    float UpdateFrequency = 0.1f;

    float LastUpdateTime = 0.0f;
    bool bWasCulled = false;
};