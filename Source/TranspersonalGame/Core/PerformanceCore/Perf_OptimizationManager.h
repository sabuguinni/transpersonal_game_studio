#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/TriggerBox.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Perf_OptimizationManager.generated.h"

UENUM(BlueprintType)
enum class EPerf_PerformanceLevel : uint8
{
    High        UMETA(DisplayName = "High Performance"),
    Medium      UMETA(DisplayName = "Medium Performance"), 
    Low         UMETA(DisplayName = "Low Performance"),
    Critical    UMETA(DisplayName = "Critical Performance")
};

USTRUCT(BlueprintType)
struct FPerf_PerformanceZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FVector Bounds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    EPerf_PerformanceLevel TargetLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FString ZoneName;

    FPerf_PerformanceZone()
    {
        Location = FVector::ZeroVector;
        Bounds = FVector(1000.0f, 1000.0f, 500.0f);
        TargetLevel = EPerf_PerformanceLevel::Medium;
        ZoneName = TEXT("DefaultZone");
    }
};

USTRUCT(BlueprintType)
struct FPerf_LODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CloseDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float MediumDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float FarDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    int32 MaxLODLevel;

    FPerf_LODSettings()
    {
        CloseDistance = 1000.0f;
        MediumDistance = 3000.0f;
        FarDistance = 8000.0f;
        MaxLODLevel = 3;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_OptimizationManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_OptimizationManager();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopPerformanceMonitoring();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFPS() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    // LOD management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void ApplyLODSettings(AActor* Actor, const FPerf_LODSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeActorsInRadius(const FVector& Center, float Radius);

    // Performance zones
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CreatePerformanceZone(const FPerf_PerformanceZone& Zone);

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceZone(const FString& ZoneName, EPerf_PerformanceLevel NewLevel);

    // Memory management
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void CleanupDuplicateActors();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void OptimizeTextureStreaming();

    // Culling optimization
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void EnableOcclusionCulling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetViewDistanceScale(float Scale);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    TArray<FPerf_PerformanceZone> PerformanceZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    FPerf_LODSettings DefaultLODSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bPerformanceMonitoringEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float FPSSampleInterval;

private:
    TArray<float> FPSSamples;
    FTimerHandle PerformanceTimer;
    
    void UpdatePerformanceMetrics();
    void OptimizeActorLOD(AActor* Actor, float Distance);
    void CleanupLightingActors();
};