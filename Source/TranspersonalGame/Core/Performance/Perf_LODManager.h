#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Perf_LODManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_LODLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra Quality"),
    High        UMETA(DisplayName = "High Quality"), 
    Medium      UMETA(DisplayName = "Medium Quality"),
    Low         UMETA(DisplayName = "Low Quality"),
    Minimal     UMETA(DisplayName = "Minimal Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MaxDrawDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullingDistance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxPhysicsActors = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bEnableDistanceCulling = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bOptimizeCollision = true;

    FPerf_LODSettings()
    {
        MaxDrawDistance = 10000.0f;
        CullingDistance = 15000.0f;
        MaxPhysicsActors = 100;
        bEnableDistanceCulling = true;
        bOptimizeCollision = true;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance LOD")
    EPerf_LODLevel CurrentLODLevel = EPerf_LODLevel::High;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance LOD")
    FPerf_LODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance LOD")
    float PerformanceCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance LOD")
    float TargetFrameRate = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance LOD")
    float MinAcceptableFrameRate = 45.0f;

private:
    UPROPERTY()
    TArray<AActor*> ManagedActors;

    UPROPERTY()
    APawn* PlayerPawn;

    float LastPerformanceCheck = 0.0f;
    float FrameTimeAccumulator = 0.0f;
    int32 FrameCount = 0;
    float AverageFrameTime = 0.0f;

public:
    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void RegisterActorForLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UnregisterActorFromLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void SetLODLevel(EPerf_LODLevel NewLODLevel);

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void UpdateDistanceCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    float GetCurrentFrameRate() const;

    UFUNCTION(BlueprintCallable, Category = "Performance LOD")
    void AutoAdjustLODLevel();

private:
    void UpdatePerformanceMetrics(float DeltaTime);
    void ApplyLODToActor(AActor* Actor, EPerf_LODLevel LODLevel);
    float GetDistanceToPlayer(AActor* Actor) const;
    void OptimizeActorCollision(AActor* Actor, bool bOptimize);
};