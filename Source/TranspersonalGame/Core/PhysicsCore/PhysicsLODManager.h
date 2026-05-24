#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "PhysicsLODManager.generated.h"

/**
 * Physics Level of Detail system for performance optimization
 * Manages physics simulation quality based on distance and importance
 */

UENUM(BlueprintType)
enum class ECore_PhysicsLODLevel : uint8
{
    High        UMETA(DisplayName = "High Quality"),
    Medium      UMETA(DisplayName = "Medium Quality"), 
    Low         UMETA(DisplayName = "Low Quality"),
    Disabled    UMETA(DisplayName = "Disabled")
};

USTRUCT(BlueprintType)
struct FCore_PhysicsLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float HighQualityDistance = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float MediumQualityDistance = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LowQualityDistance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxHighQualityObjects = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    int32 MaxMediumQualityObjects = 150;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float UpdateFrequency = 0.5f;

    FCore_PhysicsLODSettings()
    {
        HighQualityDistance = 500.0f;
        MediumQualityDistance = 1500.0f;
        LowQualityDistance = 3000.0f;
        MaxHighQualityObjects = 50;
        MaxMediumQualityObjects = 150;
        UpdateFrequency = 0.5f;
    }
};

USTRUCT(BlueprintType)
struct FCore_PhysicsObjectInfo
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Object")
    TWeakObjectPtr<AActor> Actor;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Object")
    ECore_PhysicsLODLevel CurrentLOD = ECore_PhysicsLODLevel::High;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Object")
    float DistanceToPlayer = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Object")
    float ImportanceScore = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Object")
    bool bIsVisible = true;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Object")
    bool bIsMoving = false;

    FCore_PhysicsObjectInfo()
    {
        CurrentLOD = ECore_PhysicsLODLevel::High;
        DistanceToPlayer = 0.0f;
        ImportanceScore = 1.0f;
        bIsVisible = true;
        bIsMoving = false;
    }
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPhysicsLODComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhysicsLODComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    float ImportanceMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    bool bAlwaysHighQuality = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    bool bCanDisablePhysics = true;

    UPROPERTY(BlueprintReadOnly, Category = "Physics LOD")
    ECore_PhysicsLODLevel CurrentLODLevel = ECore_PhysicsLODLevel::High;

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void SetLODLevel(ECore_PhysicsLODLevel NewLODLevel);

    UFUNCTION(BlueprintPure, Category = "Physics LOD")
    ECore_PhysicsLODLevel GetCurrentLODLevel() const { return CurrentLODLevel; }

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void ForceHighQuality(bool bForce);

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
    void ApplyLODSettings(ECore_PhysicsLODLevel LODLevel);
    void UpdatePhysicsComplexity(ECore_PhysicsLODLevel LODLevel);

    bool bForcedHighQuality = false;
    float LastUpdateTime = 0.0f;
};

UCLASS(Blueprintable, BlueprintType)
class TRANSPERSONALGAME_API APhysicsLODManager : public AActor
{
    GENERATED_BODY()

public:
    APhysicsLODManager();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics LOD")
    FCore_PhysicsLODSettings LODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bEnableAutomaticLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceThreshold = 30.0f; // Target FPS

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 TotalManagedObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 HighQualityObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 MediumQualityObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 LowQualityObjects = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Statistics")
    int32 DisabledObjects = 0;

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void RegisterPhysicsObject(AActor* Actor, float ImportanceScore = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void UnregisterPhysicsObject(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void UpdateAllLODLevels();

    UFUNCTION(BlueprintCallable, Category = "Physics LOD")
    void SetGlobalLODLevel(ECore_PhysicsLODLevel LODLevel);

    UFUNCTION(BlueprintPure, Category = "Physics LOD")
    ECore_PhysicsLODLevel GetRecommendedLODLevel(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeForPerformance();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
    void DebugPrintLODStatistics();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    TArray<FCore_PhysicsObjectInfo> ManagedObjects;

    UPROPERTY()
    APawn* PlayerPawn = nullptr;

    float LastLODUpdateTime = 0.0f;
    float LastPerformanceCheck = 0.0f;

    void UpdateLODForObject(FCore_PhysicsObjectInfo& ObjectInfo);
    void CalculateDistanceToPlayer(FCore_PhysicsObjectInfo& ObjectInfo);
    void CalculateImportanceScore(FCore_PhysicsObjectInfo& ObjectInfo);
    void ApplyLODToActor(AActor* Actor, ECore_PhysicsLODLevel LODLevel);
    void UpdateStatistics();
    float GetCurrentFrameRate() const;
    void PerformanceBasedOptimization();
    void CleanupInvalidObjects();
};

#include "PhysicsLODManager.generated.h"