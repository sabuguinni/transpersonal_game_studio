#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Pawn.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    LOD_High = 0     UMETA(DisplayName = "High Quality"),
    LOD_Medium = 1   UMETA(DisplayName = "Medium Quality"),
    LOD_Low = 2      UMETA(DisplayName = "Low Quality"),
    LOD_Culled = 3   UMETA(DisplayName = "Culled")
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighQualityDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumQualityDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowQualityDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance = 20000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableTriangleReduction = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float TriangleReductionPercent = 0.5f;
};

USTRUCT(BlueprintType)
struct FPerf_ActorLODData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    EPerf_LODLevel CurrentLOD = EPerf_LODLevel::LOD_High;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    int32 OriginalTriangleCount = 0;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
    bool bIsCulled = false;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data")
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
    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UpdateLODLevels();

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void RegisterActorForLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void UnregisterActorFromLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    EPerf_LODLevel GetActorLODLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void SetLODSettings(const FPerf_LODSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Performance|LOD")
    void ForceUpdateAllActors();

    // Distance Culling
    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void CullActorsByDistance();

    UFUNCTION(BlueprintCallable, Category = "Performance|Culling")
    void RestoreCulledActors();

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance|Monitoring")
    int32 GetManagedActorCount() const { return ManagedActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Performance|Monitoring")
    int32 GetCulledActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Monitoring")
    float GetAverageDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Performance|Monitoring")
    void GetLODDistribution(int32& HighLOD, int32& MediumLOD, int32& LowLOD, int32& CulledLOD) const;

protected:
    // Core LOD Functions
    void UpdateActorLOD(FPerf_ActorLODData& ActorData);
    void ApplyLODLevel(AActor* Actor, EPerf_LODLevel LODLevel);
    float CalculateDistanceToPlayer(AActor* Actor) const;
    APawn* GetPlayerPawn() const;

    // Mesh Optimization
    void OptimizeMeshLOD(UStaticMeshComponent* MeshComp, EPerf_LODLevel LODLevel);
    void SetMeshVisibility(UStaticMeshComponent* MeshComp, bool bVisible);

private:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "LOD Settings", meta = (AllowPrivateAccess = "true"))
    FPerf_LODSettings LODSettings;

    UPROPERTY(BlueprintReadOnly, Category = "LOD Data", meta = (AllowPrivateAccess = "true"))
    TArray<FPerf_ActorLODData> ManagedActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float LastUpdateTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    float UpdateInterval = 0.1f; // Update LOD every 100ms

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    int32 MaxActorsPerFrame = 50; // Limit updates per frame

    UPROPERTY(BlueprintReadOnly, Category = "Performance", meta = (AllowPrivateAccess = "true"))
    bool bEnableLODSystem = true;
};