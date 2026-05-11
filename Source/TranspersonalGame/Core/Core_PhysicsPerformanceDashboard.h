#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Core_PhysicsPerformanceDashboard.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float FrameTimeMS;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 CollisionChecksPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 ActiveRagdolls;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    int32 DestructionEventsPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float FluidSimulationCost;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Performance")
    float PhysicsMemoryMB;

    FCore_PhysicsPerformanceMetrics()
    {
        FrameTimeMS = 0.0f;
        CollisionChecksPerFrame = 0;
        ActiveRagdolls = 0;
        DestructionEventsPerFrame = 0;
        FluidSimulationCost = 0.0f;
        PhysicsMemoryMB = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ACore_PhysicsPerformanceDashboard : public AActor
{
    GENERATED_BODY()

public:
    ACore_PhysicsPerformanceDashboard();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DashboardMesh;

    // Performance indicator meshes
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicators")
    UStaticMeshComponent* FrameTimeIndicator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicators")
    UStaticMeshComponent* CollisionIndicator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicators")
    UStaticMeshComponent* RagdollIndicator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicators")
    UStaticMeshComponent* DestructionIndicator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicators")
    UStaticMeshComponent* FluidIndicator;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Indicators")
    UStaticMeshComponent* MemoryIndicator;

    // Current performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    FCore_PhysicsPerformanceMetrics CurrentMetrics;

    // Performance thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float FrameTimeWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    int32 CollisionWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Thresholds")
    float MemoryWarningThreshold;

    // Update frequency
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateFrequency;

    float TimeSinceLastUpdate;

public:
    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FCore_PhysicsPerformanceMetrics GetCurrentMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void SetPerformanceThresholds(float FrameThreshold, int32 CollisionThreshold, float MemoryThreshold);

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    bool IsPerformanceWithinThresholds() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void ResetMetrics();

private:
    void UpdateIndicatorColors();
    void CreateIndicatorComponents();
    FLinearColor GetPerformanceColor(float Value, float Threshold, bool bInvertLogic = false) const;
    void CollectPhysicsMetrics();
};