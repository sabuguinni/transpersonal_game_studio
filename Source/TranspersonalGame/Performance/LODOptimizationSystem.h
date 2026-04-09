#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "PerformanceTargets.h"
#include "LODOptimizationSystem.generated.h"

class UStaticMeshComponent;
class USkeletalMeshComponent;
class ALandscapeProxy;

/**
 * LOD Optimization System for the Transpersonal Game Studio
 * Manages Level of Detail for massive dinosaur ecosystems
 * Ensures 60fps PC / 30fps console performance targets
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FLODSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD0Distance = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD1Distance = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD2Distance = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float LOD3Distance = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float CullDistance = 25000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    bool bUseDynamicLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD")
    float PerformanceScaleFactor = 1.0f;

    FLODSettings()
    {
        // Default values for medium-sized objects
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FDinosaurLODProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur LOD")
    FLODSettings SmallDinosaur;  // Compsognathus, small herbivores

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur LOD")
    FLODSettings MediumDinosaur; // Velociraptors, medium herbivores

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur LOD")
    FLODSettings LargeDinosaur;  // T-Rex, Triceratops, Brontosaurus

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur LOD")
    FLODSettings HerdMember;     // Optimized for mass simulation

    FDinosaurLODProfile()
    {
        // Small dinosaurs - closer LOD transitions
        SmallDinosaur.LOD0Distance = 500.0f;
        SmallDinosaur.LOD1Distance = 1500.0f;
        SmallDinosaur.LOD2Distance = 4000.0f;
        SmallDinosaur.LOD3Distance = 8000.0f;
        SmallDinosaur.CullDistance = 12000.0f;

        // Medium dinosaurs - standard distances
        MediumDinosaur.LOD0Distance = 1000.0f;
        MediumDinosaur.LOD1Distance = 3000.0f;
        MediumDinosaur.LOD2Distance = 8000.0f;
        MediumDinosaur.LOD3Distance = 15000.0f;
        MediumDinosaur.CullDistance = 20000.0f;

        // Large dinosaurs - extended distances (iconic creatures)
        LargeDinosaur.LOD0Distance = 2000.0f;
        LargeDinosaur.LOD1Distance = 5000.0f;
        LargeDinosaur.LOD2Distance = 12000.0f;
        LargeDinosaur.LOD3Distance = 25000.0f;
        LargeDinosaur.CullDistance = 35000.0f;

        // Herd members - aggressive LOD for performance
        HerdMember.LOD0Distance = 300.0f;
        HerdMember.LOD1Distance = 1000.0f;
        HerdMember.LOD2Distance = 3000.0f;
        HerdMember.LOD3Distance = 6000.0f;
        HerdMember.CullDistance = 8000.0f;
    }
};

UCLASS(ClassGroup=(Performance), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API ULODOptimizationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    ULODOptimizationSystem();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Core LOD management
    UFUNCTION(BlueprintCallable, Category = "LOD Optimization")
    void OptimizeLODsForPerformance();

    UFUNCTION(BlueprintCallable, Category = "LOD Optimization")
    void SetDynamicLODEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "LOD Optimization")
    void UpdateLODDistances(float PerformanceScale);

    // Dinosaur-specific LOD management
    UFUNCTION(BlueprintCallable, Category = "Dinosaur LOD")
    void ApplyDinosaurLODProfile(AActor* DinosaurActor, const FLODSettings& LODSettings);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur LOD")
    void OptimizeHerdLODs(const TArray<AActor*>& HerdMembers);

    UFUNCTION(BlueprintCallable, Category = "Dinosaur LOD")
    void SetLODForMassSimulation(bool bEnableMassOptimization);

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetCurrentFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    int32 GetVisibleActorCount() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void AdjustLODBasedOnPerformance();

    // Nanite integration
    UFUNCTION(BlueprintCallable, Category = "Nanite")
    void OptimizeNaniteLODs();

    UFUNCTION(BlueprintCallable, Category = "Nanite")
    bool ShouldUseNaniteForActor(AActor* Actor) const;

protected:
    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    FDinosaurLODProfile DinosaurLODProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    float PerformanceTargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    float LODUpdateFrequency = 0.1f;  // Update every 100ms

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    bool bEnableDynamicLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    bool bUsePerformanceBasedLOD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "LOD Configuration")
    float PerformanceThreshold = 0.9f;  // 90% of target frame time

    // Mass simulation optimization
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    bool bOptimizeForMassSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    int32 MaxHighDetailDinosaurs = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass Simulation")
    float MassSimulationLODBias = 1.5f;

private:
    // Internal state
    float LastLODUpdateTime = 0.0f;
    TArray<TWeakObjectPtr<UStaticMeshComponent>> TrackedMeshComponents;
    TArray<TWeakObjectPtr<USkeletalMeshComponent>> TrackedSkeletalMeshComponents;
    
    // Performance tracking
    float AverageFrameTime = 16.67f;
    int32 FrameTimeHistoryIndex = 0;
    TArray<float> FrameTimeHistory;
    
    // Helper functions
    void UpdateTrackedComponents();
    void ApplyLODSettings(UMeshComponent* MeshComponent, const FLODSettings& Settings);
    float CalculateDistanceToCamera(const AActor* Actor) const;
    int32 CalculateAppropriiateLOD(float Distance, const FLODSettings& Settings) const;
    void UpdatePerformanceMetrics();
    bool ShouldCullActor(const AActor* Actor, float Distance, const FLODSettings& Settings) const;
    
    // Nanite helpers
    void EnableNaniteForActor(AActor* Actor);
    void ConfigureNaniteSettings(UStaticMeshComponent* MeshComponent);
};

/**
 * Global LOD optimization manager
 * Singleton that manages LOD optimization across the entire world
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALODOptimizationManager : public AActor
{
    GENERATED_BODY()

public:
    ALODOptimizationManager();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    static ALODOptimizationManager* GetInstance(UWorld* World);

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    void RegisterDinosaur(AActor* DinosaurActor, const FString& DinosaurType);

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    void UnregisterDinosaur(AActor* DinosaurActor);

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    void OptimizeAllDinosaurs();

    UFUNCTION(BlueprintCallable, Category = "LOD Manager")
    void SetGlobalLODScale(float Scale);

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class ULODOptimizationSystem* LODSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float GlobalLODScale = 1.0f;

private:
    static TWeakObjectPtr<ALODOptimizationManager> Instance;
    
    TMap<TWeakObjectPtr<AActor>, FString> RegisteredDinosaurs;
    float LastOptimizationTime = 0.0f;
    float OptimizationInterval = 0.5f;  // Optimize every 500ms
};