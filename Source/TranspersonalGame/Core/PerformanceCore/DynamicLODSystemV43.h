#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "DynamicLODSystemV43.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDynamicLOD, Log, All);

DECLARE_STATS_GROUP(TEXT("DynamicLOD"), STATGROUP_DynamicLOD, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("LOD Calculation"), STAT_LODCalculation, STATGROUP_DynamicLOD);
DECLARE_CYCLE_STAT(TEXT("Distance Culling"), STAT_DistanceCulling, STATGROUP_DynamicLOD);
DECLARE_CYCLE_STAT(TEXT("Screen Size Calculation"), STAT_ScreenSizeCalculation, STATGROUP_DynamicLOD);

UENUM(BlueprintType)
enum class ELODCalculationMethod : uint8
{
    Distance        UMETA(DisplayName = "Distance Based"),
    ScreenSize      UMETA(DisplayName = "Screen Size Based"),
    Performance     UMETA(DisplayName = "Performance Based"),
    Hybrid          UMETA(DisplayName = "Hybrid (Distance + Performance)")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD0Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD1Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD2Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float LOD3Distance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float CullDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    float ScreenSizeThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Settings")
    bool bUsePerformanceBasedLOD;

    FLODSettings()
    {
        LOD0Distance = 500.0f;
        LOD1Distance = 1000.0f;
        LOD2Distance = 2000.0f;
        LOD3Distance = 4000.0f;
        CullDistance = 8000.0f;
        ScreenSizeThreshold = 0.01f;
        bUsePerformanceBasedLOD = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurLODProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur LOD")
    FLODSettings SmallDinosaur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur LOD")
    FLODSettings MediumDinosaur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur LOD")
    FLODSettings LargeDinosaur;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur LOD")
    FLODSettings GiantDinosaur;

    FDinosaurLODProfile()
    {
        // Small dinosaurs (Compsognathus, etc.)
        SmallDinosaur.LOD0Distance = 300.0f;
        SmallDinosaur.LOD1Distance = 600.0f;
        SmallDinosaur.LOD2Distance = 1200.0f;
        SmallDinosaur.LOD3Distance = 2400.0f;
        SmallDinosaur.CullDistance = 4000.0f;

        // Medium dinosaurs (Velociraptor, etc.)
        MediumDinosaur.LOD0Distance = 500.0f;
        MediumDinosaur.LOD1Distance = 1000.0f;
        MediumDinosaur.LOD2Distance = 2000.0f;
        MediumDinosaur.LOD3Distance = 4000.0f;
        MediumDinosaur.CullDistance = 6000.0f;

        // Large dinosaurs (T-Rex, Triceratops, etc.)
        LargeDinosaur.LOD0Distance = 800.0f;
        LargeDinosaur.LOD1Distance = 1600.0f;
        LargeDinosaur.LOD2Distance = 3200.0f;
        LargeDinosaur.LOD3Distance = 6400.0f;
        LargeDinosaur.CullDistance = 10000.0f;

        // Giant dinosaurs (Brontosaurus, etc.)
        GiantDinosaur.LOD0Distance = 1200.0f;
        GiantDinosaur.LOD1Distance = 2400.0f;
        GiantDinosaur.LOD2Distance = 4800.0f;
        GiantDinosaur.LOD3Distance = 9600.0f;
        GiantDinosaur.CullDistance = 15000.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDynamicLODSystemV43 : public UActorComponent
{
    GENERATED_BODY()

public:
    UDynamicLODSystemV43();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // LOD Management
    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void UpdateLODForActor(AActor* Actor, const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void UpdateAllActorsLOD(const FVector& ViewerLocation);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void RegisterActorForLOD(AActor* Actor, const FLODSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void UnregisterActorFromLOD(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void SetLODCalculationMethod(ELODCalculationMethod Method);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void SetPerformanceBasedLOD(bool bEnabled);

    // Dinosaur-specific LOD
    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void RegisterDinosaurForLOD(AActor* Dinosaur, const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void SetDinosaurLODProfile(const FDinosaurLODProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    FDinosaurLODProfile GetDinosaurLODProfile() const { return DinosaurProfile; }

    // Performance optimization
    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void OptimizeLODForFrameRate(float TargetFrameTime);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void SetLODBias(float Bias);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    float GetCurrentLODBias() const { return CurrentLODBias; }

    // Distance culling
    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void SetGlobalCullDistanceScale(float Scale);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void EnableDistanceCulling(bool bEnabled);

    // Screen size based LOD
    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void SetScreenSizeThreshold(float Threshold);

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    float CalculateScreenSize(AActor* Actor, const FVector& ViewerLocation) const;

    // Statistics and monitoring
    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    int32 GetRegisteredActorCount() const { return RegisteredActors.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void GetLODStatistics(int32& LOD0Count, int32& LOD1Count, int32& LOD2Count, int32& LOD3Count, int32& CulledCount) const;

    UFUNCTION(BlueprintCallable, Category = "Dynamic LOD")
    void ResetLODStatistics();

protected:
    // Internal LOD calculation methods
    int32 CalculateLODByDistance(float Distance, const FLODSettings& Settings) const;
    int32 CalculateLODByScreenSize(float ScreenSize, const FLODSettings& Settings) const;
    int32 CalculateLODByPerformance(const FLODSettings& Settings) const;
    int32 CalculateHybridLOD(float Distance, float ScreenSize, const FLODSettings& Settings) const;

    // LOD application methods
    void ApplyLODToStaticMesh(UStaticMeshComponent* MeshComp, int32 LODLevel);
    void ApplyLODToSkeletalMesh(USkeletalMeshComponent* MeshComp, int32 LODLevel);
    void ApplyDistanceCulling(UPrimitiveComponent* Component, float Distance, float CullDistance);

    // Performance monitoring
    void MonitorPerformance();
    void AdjustLODBasedOnPerformance(float CurrentFrameTime);

    // Utility functions
    FVector GetViewerLocation() const;
    float GetCurrentFrameTime() const;
    bool ShouldUpdateLOD(float DeltaTime) const;

private:
    UPROPERTY(EditAnywhere, Category = "Dynamic LOD Settings")
    ELODCalculationMethod CalculationMethod;

    UPROPERTY(EditAnywhere, Category = "Dynamic LOD Settings")
    FDinosaurLODProfile DinosaurProfile;

    UPROPERTY(EditAnywhere, Category = "Dynamic LOD Settings")
    float UpdateFrequency;

    UPROPERTY(EditAnywhere, Category = "Dynamic LOD Settings")
    float PerformanceThreshold;

    UPROPERTY(EditAnywhere, Category = "Dynamic LOD Settings")
    bool bEnablePerformanceBasedLOD;

    UPROPERTY(EditAnywhere, Category = "Dynamic LOD Settings")
    bool bEnableDistanceCulling;

    UPROPERTY(EditAnywhere, Category = "Dynamic LOD Settings")
    float GlobalCullDistanceScale;

    UPROPERTY(EditAnywhere, Category = "Dynamic LOD Settings")
    float ScreenSizeThreshold;

    // Runtime data
    TMap<AActor*, FLODSettings> RegisteredActors;
    TMap<AActor*, int32> CurrentLODLevels;
    TMap<AActor*, float> LastUpdateTimes;

    float CurrentLODBias;
    float LastUpdateTime;
    float AccumulatedDeltaTime;

    // Performance monitoring
    TArray<float> FrameTimeHistory;
    int32 MaxFrameTimeHistory;
    float AverageFrameTime;

    // Statistics
    mutable int32 LOD0Count;
    mutable int32 LOD1Count;
    mutable int32 LOD2Count;
    mutable int32 LOD3Count;
    mutable int32 CulledCount;
};