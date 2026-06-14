#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/Engine.h"
#include "Perf_CollisionOptimizer.generated.h"

UENUM(BlueprintType)
enum class EPerf_CollisionOptLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"), 
    Low         UMETA(DisplayName = "Low Quality"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FPerf_CollisionProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    int32 MaxCollisionChecks = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 0.016f; // 60fps

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableComplexCollision = true;

    FPerf_CollisionProfile()
    {
        MaxDistance = 5000.0f;
        MaxCollisionChecks = 100;
        UpdateFrequency = 0.016f;
        bEnableComplexCollision = true;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_CollisionOptimizer : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_CollisionOptimizer();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Performance optimization methods
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeCollisionForDistance(float Distance);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetCollisionOptimizationLevel(EPerf_CollisionOptLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableDynamicLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdateCollisionComplexity();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CullDistantCollisions();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetActiveCollisionCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCollisionPerformanceScore() const;

protected:
    // Performance settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_CollisionOptLevel OptimizationLevel = EPerf_CollisionOptLevel::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_CollisionProfile HighQualityProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_CollisionProfile MediumQualityProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_CollisionProfile LowQualityProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableDynamicLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceCheckInterval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFrameTime = 0.016f; // 60fps target

private:
    // Internal tracking
    TArray<TWeakObjectPtr<UPrimitiveComponent>> TrackedComponents;
    float LastPerformanceCheck = 0.0f;
    int32 CurrentActiveCollisions = 0;
    float CurrentPerformanceScore = 1.0f;

    // Optimization methods
    void UpdatePerformanceMetrics();
    void ApplyCollisionProfile(const FPerf_CollisionProfile& Profile);
    void OptimizeComponentCollision(UPrimitiveComponent* Component, float Distance);
    FPerf_CollisionProfile GetCurrentProfile() const;
};