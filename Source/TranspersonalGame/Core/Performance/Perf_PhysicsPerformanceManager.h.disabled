#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Subsystems/WorldSubsystem.h"
#include "Perf_PhysicsPerformanceManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PhysicsQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActivePhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 SleepingPhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsStepTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float CollisionQueryTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 TotalCollisionQueries;

    FPerf_PhysicsMetrics()
    {
        ActivePhysicsActors = 0;
        SleepingPhysicsActors = 0;
        PhysicsStepTime = 0.0f;
        CollisionQueryTime = 0.0f;
        TotalCollisionQueries = 0;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    int32 MaxActivePhysicsActors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float PhysicsTimeStep;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bEnablePhysicsLOD;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float PhysicsLODDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    bool bEnablePhysicsCulling;

    FPerf_PhysicsSettings()
    {
        MaxActivePhysicsActors = 500;
        PhysicsTimeStep = 0.016667f; // 60fps
        bEnablePhysicsLOD = true;
        PhysicsLODDistance = 5000.0f;
        bEnablePhysicsCulling = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_PhysicsPerformanceManager : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UPerf_PhysicsPerformanceManager();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics Performance Management
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FPerf_PhysicsMetrics GetPhysicsMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPhysicsQuality(EPerf_PhysicsQuality Quality);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void OptimizePhysicsActors();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void EnablePhysicsLOD(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetMaxActivePhysicsActors(int32 MaxActors);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance", CallInEditor)
    void AnalyzePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void CullDistantPhysicsActors(float CullDistance);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void PutPhysicsActorsToSleep(float SleepThreshold);

    UFUNCTION(BlueprintPure, Category = "Physics Performance")
    bool IsPhysicsPerformanceGood() const;

    UFUNCTION(BlueprintPure, Category = "Physics Performance")
    float GetCurrentPhysicsLoad() const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    FPerf_PhysicsMetrics CurrentMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    FPerf_PhysicsSettings PhysicsSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    EPerf_PhysicsQuality CurrentQuality;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    TArray<TWeakObjectPtr<AActor>> TrackedPhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float LastUpdateTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    bool bPhysicsOptimizationEnabled;

private:
    void CollectPhysicsMetrics();
    void ApplyPhysicsQualitySettings();
    void UpdatePhysicsActorLOD();
    void ManagePhysicsActorSleep();
    bool ShouldCullPhysicsActor(AActor* Actor, float CullDistance) const;
    void OptimizePhysicsActor(AActor* Actor);
};