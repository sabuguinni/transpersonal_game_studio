#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsManager.generated.h"

UENUM(BlueprintType)
enum class ECore_PhysicsQuality : uint8
{
    Low         UMETA(DisplayName = "Low Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"),
    High        UMETA(DisplayName = "High Quality"),
    Ultra       UMETA(DisplayName = "Ultra Quality")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float GravityScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float LinearDamping = 0.01f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float AngularDamping = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    ECore_PhysicsQuality QualityLevel = ECore_PhysicsQuality::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    int32 MaxSubsteps = 6;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
    float MaxSubstepDeltaTime = 0.016667f;

    FCore_PhysicsSettings()
    {
        GravityScale = 1.0f;
        LinearDamping = 0.01f;
        AngularDamping = 0.0f;
        QualityLevel = ECore_PhysicsQuality::Medium;
        MaxSubsteps = 6;
        MaxSubstepDeltaTime = 0.016667f;
    }
};

/**
 * Core Physics Manager - Manages global physics settings and performance
 * Handles physics quality scaling, gravity modifications, and performance optimization
 */
UCLASS(BlueprintType, Blueprintable, meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsManager();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Physics Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    FCore_PhysicsSettings PhysicsSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePhysicsOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    float PhysicsUpdateRate = 60.0f;

    // Performance Monitoring
    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float CurrentPhysicsTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActivePhysicsBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float AveragePhysicsTime = 0.0f;

    // Physics Quality Management
    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetPhysicsQuality(ECore_PhysicsQuality NewQuality);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void ApplyPhysicsSettings(const FCore_PhysicsSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    void SetGlobalGravityScale(float NewGravityScale);

    UFUNCTION(BlueprintCallable, Category = "Physics")
    float GetGlobalGravityScale() const;

    // Performance Optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePhysicsStatistics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool ShouldReducePhysicsQuality() const;

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor)
    void DebugPhysicsSettings();

    UFUNCTION(BlueprintCallable, Category = "Debug")
    void LogPhysicsStatistics();

private:
    // Internal tracking
    float PhysicsTimeAccumulator = 0.0f;
    int32 PhysicsFrameCount = 0;
    TArray<float> PhysicsTimeSamples;

    void UpdatePerformanceMetrics(float DeltaTime);
    void ApplyQualitySettings();
    void MonitorPhysicsPerformance();
};