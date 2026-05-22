#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD0_HighDetail     UMETA(DisplayName = "LOD0 - High Detail"),
    LOD1_MediumDetail   UMETA(DisplayName = "LOD1 - Medium Detail"), 
    LOD2_LowDetail      UMETA(DisplayName = "LOD2 - Low Detail"),
    LOD3_VeryLowDetail  UMETA(DisplayName = "LOD3 - Very Low Detail"),
    LOD_Culled          UMETA(DisplayName = "Culled - Not Rendered")
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD0Distance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD1Distance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD2Distance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD3Distance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableAutomaticLOD = true;

    FPerf_LODSettings()
    {
        LOD0Distance = 500.0f;
        LOD1Distance = 1500.0f;
        LOD2Distance = 3000.0f;
        LOD3Distance = 5000.0f;
        CullDistance = 8000.0f;
        bEnableAutomaticLOD = true;
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
    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UpdateLODForAllActors();

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetLODForActor(AActor* Actor, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    EPerf_LODLevel CalculateLODLevel(AActor* Actor, const FVector& ViewerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    float GetDistanceToViewer(AActor* Actor) const;

    // LOD Settings
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    FPerf_LODSettings GetLODSettings() const { return LODSettings; }

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void EnableLODDebugging(bool bEnable);

    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    int32 GetManagedActorCount() const { return ManagedActors.Num(); }

    UFUNCTION(BlueprintPure, Category = "Performance|LOD")
    int32 GetCulledActorCount() const;

    // Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void RegisterActorForLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UnregisterActorFromLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void OptimizeForFrameRate(float TargetFrameRate);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bDebugLOD = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxActorsPerFrame = 50;

private:
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> ManagedActors;

    UPROPERTY()
    TMap<TWeakObjectPtr<AActor>, EPerf_LODLevel> CurrentLODLevels;

    float LastUpdateTime;
    int32 CurrentUpdateIndex;

    void UpdateLODForActorBatch();
    void ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    FVector GetViewerLocation() const;
};