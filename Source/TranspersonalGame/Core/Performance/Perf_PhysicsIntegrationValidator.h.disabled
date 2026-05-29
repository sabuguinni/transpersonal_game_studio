#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Perf_PhysicsIntegrationValidator.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_PhysicsValidationMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    int32 TotalPhysicsActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    int32 SimulatingActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    int32 StaticActors;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    float AveragePhysicsTickTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    float MaxPhysicsTickTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    int32 CollisionPairsCount;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Validation")
    bool bPhysicsSystemHealthy;

    FPerf_PhysicsValidationMetrics()
        : TotalPhysicsActors(0)
        , SimulatingActors(0)
        , StaticActors(0)
        , AveragePhysicsTickTime(0.0f)
        , MaxPhysicsTickTime(0.0f)
        , CollisionPairsCount(0)
        , bPhysicsSystemHealthy(true)
    {
    }
};

UENUM(BlueprintType)
enum class EPerf_PhysicsValidationResult : uint8
{
    Passed          UMETA(DisplayName = "Validation Passed"),
    Warning         UMETA(DisplayName = "Warning Detected"),
    Failed          UMETA(DisplayName = "Validation Failed"),
    Critical        UMETA(DisplayName = "Critical Issue")
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerf_PhysicsIntegrationValidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UPerf_PhysicsIntegrationValidator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Validation Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    EPerf_PhysicsValidationResult ValidatePhysicsSystem();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FPerf_PhysicsValidationMetrics GetPhysicsMetrics();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidatePhysicsActorIntegrity(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void RunPhysicsPerformanceTest(float TestDuration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    void ValidatePhysicsArchitectureIntegration();

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    float ValidationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bAutoValidateOnBeginPlay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Settings")
    bool bLogValidationResults;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MaxAcceptablePhysicsTickTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxPhysicsActorsWarningThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    int32 MaxPhysicsActorsCriticalThreshold;

    // Results
    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    FPerf_PhysicsValidationMetrics LastValidationMetrics;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    EPerf_PhysicsValidationResult LastValidationResult;

    UPROPERTY(BlueprintReadOnly, Category = "Validation Results")
    FString LastValidationMessage;

private:
    float LastValidationTime;
    TArray<float> PhysicsTickSamples;
    
    void CollectPhysicsMetrics(FPerf_PhysicsValidationMetrics& OutMetrics);
    void ValidatePhysicsPerformance(FPerf_PhysicsValidationMetrics& Metrics, EPerf_PhysicsValidationResult& Result);
    void LogValidationResult(const FPerf_PhysicsValidationMetrics& Metrics, EPerf_PhysicsValidationResult Result);
};