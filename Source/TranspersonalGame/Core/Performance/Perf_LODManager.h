#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD_High     UMETA(DisplayName = "High Quality"),
    LOD_Medium   UMETA(DisplayName = "Medium Quality"),
    LOD_Low      UMETA(DisplayName = "Low Quality"),
    LOD_Culled   UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighQualityDistance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumQualityDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowQualityDistance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableFrustumCulling = true;

    FPerf_LODSettings()
    {
        HighQualityDistance = 1000.0f;
        MediumQualityDistance = 3000.0f;
        LowQualityDistance = 8000.0f;
        CullDistance = 15000.0f;
        bEnableDistanceCulling = true;
        bEnableFrustumCulling = true;
    }
};

USTRUCT(BlueprintType)
struct FPerf_MeshLODData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD Data")
    TWeakObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD Data")
    EPerf_LODLevel CurrentLODLevel = EPerf_LODLevel::LOD_High;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD Data")
    float LastUpdateTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD Data")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LOD Data")
    bool bIsVisible = true;

    FPerf_MeshLODData()
    {
        CurrentLODLevel = EPerf_LODLevel::LOD_High;
        LastUpdateTime = 0.0f;
        DistanceToPlayer = 0.0f;
        bIsVisible = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_LODManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_LODManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual void Tick(float DeltaTime) override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void RegisterMeshComponent(UStaticMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UnregisterMeshComponent(UStaticMeshComponent* MeshComponent);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UpdateLODForAllMeshes();

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    EPerf_LODLevel CalculateLODLevel(float Distance) const;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    FPerf_LODSettings GetLODSettings() const { return LODSettings; }

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    int32 GetRegisteredMeshCount() const { return RegisteredMeshes.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void ForceUpdateAllLODs();

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void EnableLODSystem(bool bEnabled);

    UFUNCTION(BlueprintPure, Category = "Performance LOD")
    bool IsLODSystemEnabled() const { return bLODSystemEnabled; }

private:
    UPROPERTY()
    FPerf_LODSettings LODSettings;

    UPROPERTY()
    TArray<FPerf_MeshLODData> RegisteredMeshes;

    UPROPERTY()
    bool bLODSystemEnabled = true;

    UPROPERTY()
    float UpdateFrequency = 0.1f; // Update every 0.1 seconds

    UPROPERTY()
    float LastUpdateTime = 0.0f;

    void UpdateMeshLOD(FPerf_MeshLODData& MeshData);
    void ApplyLODLevel(UStaticMeshComponent* MeshComponent, EPerf_LODLevel LODLevel);
    float GetDistanceToPlayer(const FVector& MeshLocation) const;
    bool IsMeshInViewFrustum(const FVector& MeshLocation) const;
    void CleanupInvalidMeshes();
};