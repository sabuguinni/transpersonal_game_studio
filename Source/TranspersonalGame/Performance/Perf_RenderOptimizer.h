#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Perf_RenderOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_RenderQuality : uint8
{
    Ultra = 0,
    High = 1,
    Medium = 2,
    Low = 3,
    Minimal = 4
};

USTRUCT(BlueprintType)
struct FPerf_RenderProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Performance")
    float MaxRenderDistance = 10000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Performance")
    int32 MaxDrawCalls = 2000;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Performance")
    float LODBias = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Performance")
    bool bEnableDynamicLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Performance")
    float ShadowDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Performance")
    int32 MaxShadowCascades = 4;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_RenderOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_RenderOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Performance")
    FPerf_RenderProfile RenderProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Render Performance")
    EPerf_RenderQuality CurrentRenderQuality = EPerf_RenderQuality::High;

    UPROPERTY(BlueprintReadOnly, Category = "Render Performance")
    int32 CurrentDrawCalls = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Render Performance")
    float CurrentGPUTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Render Performance")
    int32 VisibleActorsCount = 0;

public:
    UFUNCTION(BlueprintCallable, Category = "Render Performance")
    void OptimizeRenderingForPerformance();

    UFUNCTION(BlueprintCallable, Category = "Render Performance")
    void SetRenderQuality(EPerf_RenderQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Render Performance")
    void UpdateRenderLOD();

    UFUNCTION(BlueprintCallable, Category = "Render Performance")
    void CullDistantObjects();

    UFUNCTION(BlueprintCallable, Category = "Render Performance")
    void OptimizeShadows();

    UFUNCTION(BlueprintCallable, Category = "Render Performance")
    void OptimizeMaterials();

    UFUNCTION(BlueprintCallable, Category = "Render Performance")
    int32 GetCurrentDrawCalls() const { return CurrentDrawCalls; }

    UFUNCTION(BlueprintCallable, Category = "Render Performance")
    float GetCurrentGPUTime() const { return CurrentGPUTime; }

private:
    void CountDrawCalls();
    void ApplyRenderQualitySettings();
    void OptimizeActorRendering(AActor* Actor, float Distance);
    float CalculateDistanceToCamera(AActor* Actor);
    void SetActorLOD(AActor* Actor, int32 LODLevel);
};