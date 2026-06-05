#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Arch_StructuralOptimizationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_StructuralMetrics
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Analysis")
    float LoadCapacity = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Analysis")
    float CurrentStress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Analysis")
    float WeatheringFactor = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Analysis")
    float StructuralIntegrity = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Analysis")
    bool bRequiresReinforcement = false;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FArch_OptimizationParameters
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float PerformanceBudget = 16.67f; // 60fps target

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    int32 MaxVisibleStructures = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float CullingDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    bool bEnableLODOptimization = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Optimization")
    float LODDistanceMultiplier = 1.5f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UArch_StructuralOptimizationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UArch_StructuralOptimizationComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Optimization")
    FArch_StructuralMetrics StructuralMetrics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Optimization")
    FArch_OptimizationParameters OptimizationParams;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Structural Optimization")
    TArray<AActor*> ManagedStructures;

    UFUNCTION(BlueprintCallable, Category = "Structural Optimization")
    void AnalyzeStructuralIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Structural Optimization")
    void OptimizePerformance();

    UFUNCTION(BlueprintCallable, Category = "Structural Optimization")
    void UpdateLODSettings();

    UFUNCTION(BlueprintCallable, Category = "Structural Optimization")
    float CalculateStressDistribution(AActor* Structure);

    UFUNCTION(BlueprintCallable, Category = "Structural Optimization")
    bool ValidateStructuralStability(AActor* Structure);

    UFUNCTION(BlueprintCallable, Category = "Structural Optimization")
    void ApplyWeatheringEffects(float DeltaTime);

private:
    float AccumulatedTime = 0.0f;
    float OptimizationInterval = 1.0f;

    void UpdateStructuralMetrics();
    void CullDistantStructures();
    void AdjustLODLevels();
};

UCLASS()
class TRANSPERSONALGAME_API AArch_StructuralOptimizationManager : public AActor
{
    GENERATED_BODY()

public:
    AArch_StructuralOptimizationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UArch_StructuralOptimizationComponent* OptimizationComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Optimization")
    float GlobalPerformanceBudget = 16.67f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Optimization")
    int32 MaxActiveStructures = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Global Optimization")
    bool bEnableGlobalOptimization = true;

    UFUNCTION(BlueprintCallable, Category = "Global Optimization")
    void RegisterStructure(AActor* Structure);

    UFUNCTION(BlueprintCallable, Category = "Global Optimization")
    void UnregisterStructure(AActor* Structure);

    UFUNCTION(BlueprintCallable, Category = "Global Optimization")
    void OptimizeAllStructures();

    UFUNCTION(BlueprintCallable, Category = "Global Optimization")
    FArch_StructuralMetrics GetGlobalStructuralHealth();

private:
    TArray<AActor*> RegisteredStructures;
    float LastOptimizationTime = 0.0f;
    float OptimizationFrequency = 2.0f;

    void PerformGlobalOptimization();
    void UpdateGlobalMetrics();
    void ManageStructureVisibility();
};