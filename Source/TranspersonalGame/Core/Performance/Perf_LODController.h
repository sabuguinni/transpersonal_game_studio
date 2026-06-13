#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Perf_LODController.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODQuality : uint8
{
    Ultra = 0,
    High = 1,
    Medium = 2,
    Low = 3,
    Potato = 4
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float NearDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float FarDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    EPerf_LODQuality QualityLevel = EPerf_LODQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableFrustumCulling = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MeshLODData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    TWeakObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    int32 CurrentLODLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    bool bIsVisible = true;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    bool bIsCulled = false;
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_LODController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_LODController();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    FPerf_LODSettings GetLODSettings() const { return LODSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void RegisterMeshComponent(UStaticMeshComponent* MeshComp);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UnregisterMeshComponent(UStaticMeshComponent* MeshComp);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UpdateLODForAllMeshes();

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void SetGlobalLODQuality(EPerf_LODQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    TArray<FPerf_MeshLODData> GetMeshLODData() const;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    int32 GetVisibleMeshCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    int32 GetCulledMeshCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void EnableLODSystem(bool bEnable);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Controller")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Controller")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Controller")
    bool bEnableLODSystem = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Controller")
    bool bDebugDrawLOD = false;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Controller")
    TArray<FPerf_MeshLODData> ManagedMeshes;

private:
    float TimeSinceLastUpdate = 0.0f;
    TWeakObjectPtr<APawn> PlayerPawn;

    void UpdateLODForMesh(FPerf_MeshLODData& MeshData);
    int32 CalculateLODLevel(float Distance) const;
    bool ShouldCullMesh(const FPerf_MeshLODData& MeshData) const;
    void FindPlayerPawn();
    void DebugDrawLODInfo();
};