#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Animation/SkeletalMeshActor.h"
#include "Perf_LODController.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODQuality : uint8
{
    Ultra          UMETA(DisplayName = "Ultra Quality"),
    High           UMETA(DisplayName = "High Quality"),
    Medium         UMETA(DisplayName = "Medium Quality"),
    Low            UMETA(DisplayName = "Low Quality"),
    Potato         UMETA(DisplayName = "Potato Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 6000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD3Distance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableDistanceLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bEnableScreenSizeLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MinScreenSize = 0.01f;

    FPerf_LODSettings()
    {
        LOD0Distance = 1000.0f;
        LOD1Distance = 3000.0f;
        LOD2Distance = 6000.0f;
        LOD3Distance = 10000.0f;
        bEnableDistanceLOD = true;
        bEnableScreenSizeLOD = true;
        MinScreenSize = 0.01f;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_MeshLODData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    TWeakObjectPtr<UStaticMeshComponent> StaticMeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    TWeakObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    int32 CurrentLODLevel = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    float LastDistance = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    float LastScreenSize = 1.0f;

    FPerf_MeshLODData()
    {
        CurrentLODLevel = 0;
        LastDistance = 0.0f;
        LastScreenSize = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_LODController : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_LODController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    EPerf_LODQuality QualityLevel = EPerf_LODQuality::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxMeshesPerFrame = 50;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 TotalMeshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 LOD0Meshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 LOD1Meshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 LOD2Meshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 LOD3Meshes = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    float AverageTriangleReduction = 0.0f;

    UFUNCTION(BlueprintCallable, Category = "LOD Control")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "LOD Control")
    void SetQualityLevel(EPerf_LODQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "LOD Control")
    void SetLODDistance(int32 LODLevel, float Distance);

    UFUNCTION(BlueprintCallable, Category = "LOD Control")
    void EnableDistanceLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "LOD Control")
    void EnableScreenSizeLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    float GetTriangleReduction() const { return AverageTriangleReduction; }

    UFUNCTION(BlueprintCallable, Category = "Statistics")
    int32 GetMeshCountAtLOD(int32 LODLevel) const;

    UFUNCTION(BlueprintCallable, Category = "LOD Control")
    void ForceLODLevel(int32 LODLevel);

    UFUNCTION(BlueprintCallable, Category = "LOD Control")
    void ResetLODLevels();

private:
    float TimeSinceLastUpdate = 0.0f;
    int32 CurrentMeshIndex = 0;
    
    UPROPERTY()
    TArray<FPerf_MeshLODData> TrackedMeshes;
    
    void GatherMeshComponents();
    void UpdateMeshLOD(FPerf_MeshLODData& MeshData);
    int32 CalculateLODLevel(float Distance, float ScreenSize) const;
    float CalculateScreenSize(const FVector& MeshLocation, const FVector& MeshBounds) const;
    float GetDistanceToPlayer(const FVector& Location) const;
    void ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, int32 LODLevel);
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, int32 LODLevel);
    void UpdateStatistics();
    void ApplyQualitySettings();
};