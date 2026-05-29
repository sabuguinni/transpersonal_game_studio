#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    High = 0,
    Medium = 1,
    Low = 2,
    VeryLow = 3
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float HighQualityDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumQualityDistance = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LowQualityDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullingDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableAutomaticLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceCulling = true;

    FPerf_LODSettings()
    {
        HighQualityDistance = 1000.0f;
        MediumQualityDistance = 2500.0f;
        LowQualityDistance = 5000.0f;
        CullingDistance = 10000.0f;
        bEnableAutomaticLOD = true;
        bEnableDistanceCulling = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API APerf_LODManager : public AActor
{
    GENERATED_BODY()

public:
    APerf_LODManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UpdateLODForAllActors();

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODLevel(AActor* Actor, EPerf_LODLevel LODLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    EPerf_LODLevel CalculateLODLevel(AActor* Actor, const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void RegisterActorForLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void UnregisterActorFromLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    FPerf_LODSettings GetLODSettings() const;

    UFUNCTION(BlueprintCallable, Category = "LOD Management")
    int32 GetManagedActorCount() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float UpdateInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableLODManagement = true;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Management")
    TArray<AActor*> ManagedActors;

private:
    float LastUpdateTime = 0.0f;
    FVector LastViewerLocation = FVector::ZeroVector;

    void UpdateStaticMeshLOD(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    void UpdateSkeletalMeshLOD(USkeletalMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    void CullActor(AActor* Actor, bool bShouldCull);
    FVector GetViewerLocation() const;
};