#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Core_PhysicsConsolidator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPhysicsConsolidator, Log, All);

/**
 * Physics system consolidation states
 */
UENUM(BlueprintType)
enum class ECore_PhysicsConsolidationState : uint8
{
    Initializing    UMETA(DisplayName = "Initializing"),
    Consolidating   UMETA(DisplayName = "Consolidating"),
    Validating      UMETA(DisplayName = "Validating"),
    Complete        UMETA(DisplayName = "Complete"),
    Failed          UMETA(DisplayName = "Failed"),
    Disabled        UMETA(DisplayName = "Disabled")
};

/**
 * Physics subsystem integration data
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSubsystemData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Subsystem")
    FString SubsystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Subsystem")
    bool bIsActive;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Subsystem")
    bool bIsValidated;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Subsystem")
    float ValidationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Subsystem")
    int32 ComponentCount;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Subsystem")
    float LastUpdateTime;

    FCore_PhysicsSubsystemData()
    {
        SubsystemName = TEXT("");
        bIsActive = false;
        bIsValidated = false;
        ValidationScore = 0.0f;
        ComponentCount = 0;
        LastUpdateTime = 0.0f;
    }
};

/**
 * Physics consolidation report
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsConsolidationReport
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Consolidation Report")
    ECore_PhysicsConsolidationState ConsolidationState;

    UPROPERTY(BlueprintReadOnly, Category = "Consolidation Report")
    float OverallIntegrationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Consolidation Report")
    int32 TotalSubsystems;

    UPROPERTY(BlueprintReadOnly, Category = "Consolidation Report")
    int32 ActiveSubsystems;

    UPROPERTY(BlueprintReadOnly, Category = "Consolidation Report")
    int32 ValidatedSubsystems;

    UPROPERTY(BlueprintReadOnly, Category = "Consolidation Report")
    TArray<FCore_PhysicsSubsystemData> SubsystemData;

    UPROPERTY(BlueprintReadOnly, Category = "Consolidation Report")
    float ConsolidationDuration;

    UPROPERTY(BlueprintReadOnly, Category = "Consolidation Report")
    FString LastError;

    FCore_PhysicsConsolidationReport()
    {
        ConsolidationState = ECore_PhysicsConsolidationState::Initializing;
        OverallIntegrationScore = 0.0f;
        TotalSubsystems = 0;
        ActiveSubsystems = 0;
        ValidatedSubsystems = 0;
        ConsolidationDuration = 0.0f;
        LastError = TEXT("");
    }
};

/**
 * Core Physics Consolidator Component
 * 
 * Consolidates all physics subsystems into a unified framework for the WALK AROUND milestone.
 * Manages integration between character physics, collision systems, terrain interaction,
 * and performance optimization. Ensures all physics components work together seamlessly.
 * 
 * Key Features:
 * - Unified physics subsystem management
 * - Cross-system validation and integration
 * - Performance monitoring and optimization
 * - Real-time consolidation reporting
 * - Automatic error detection and recovery
 * 
 * Integration Points:
 * - Character movement and physics
 * - Collision detection and response
 * - Terrain interaction systems
 * - Rigid body dynamics
 * - Physics performance optimization
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsConsolidator : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsConsolidator();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // === CONSOLIDATION MANAGEMENT ===

    /**
     * Initialize physics consolidation system
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    void InitializeConsolidation();

    /**
     * Start physics subsystem consolidation process
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    void StartConsolidation();

    /**
     * Stop physics consolidation process
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    void StopConsolidation();

    /**
     * Reset consolidation system to initial state
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    void ResetConsolidation();

    // === SUBSYSTEM INTEGRATION ===

    /**
     * Register a physics subsystem for consolidation
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    bool RegisterPhysicsSubsystem(const FString& SubsystemName, UActorComponent* SubsystemComponent);

    /**
     * Unregister a physics subsystem
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    bool UnregisterPhysicsSubsystem(const FString& SubsystemName);

    /**
     * Validate all registered subsystems
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    void ValidateAllSubsystems();

    /**
     * Update subsystem integration status
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    void UpdateSubsystemIntegration();

    // === VALIDATION AND REPORTING ===

    /**
     * Get current consolidation report
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    FCore_PhysicsConsolidationReport GetConsolidationReport() const;

    /**
     * Get subsystem data by name
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    FCore_PhysicsSubsystemData GetSubsystemData(const FString& SubsystemName) const;

    /**
     * Check if consolidation is complete
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    bool IsConsolidationComplete() const;

    /**
     * Get overall integration score
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    float GetIntegrationScore() const;

    // === PERFORMANCE OPTIMIZATION ===

    /**
     * Optimize physics performance across all subsystems
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    void OptimizePhysicsPerformance();

    /**
     * Enable/disable performance monitoring
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    void SetPerformanceMonitoring(bool bEnabled);

    /**
     * Get performance metrics
     */
    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    float GetAverageFrameTime() const { return AverageFrameTime; }

    UFUNCTION(BlueprintCallable, Category = "Physics Consolidation")
    float GetPhysicsStepTime() const { return PhysicsStepTime; }

protected:
    // === CONSOLIDATION STATE ===

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    ECore_PhysicsConsolidationState ConsolidationState;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    float OverallIntegrationScore;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    float ConsolidationStartTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    float LastConsolidationTime;

    // === SUBSYSTEM MANAGEMENT ===

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    TArray<FCore_PhysicsSubsystemData> RegisteredSubsystems;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    TMap<FString, TWeakObjectPtr<UActorComponent>> SubsystemComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    int32 TotalSubsystems;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    int32 ActiveSubsystems;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    int32 ValidatedSubsystems;

    // === PERFORMANCE METRICS ===

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    float AverageFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    float PhysicsStepTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    float CollisionQueryTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Consolidation", meta = (AllowPrivateAccess = "true"))
    bool bPerformanceMonitoring;

    // === CONFIGURATION ===

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consolidation")
    bool bAutoStartConsolidation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consolidation")
    bool bDetailedLogging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consolidation")
    float ConsolidationTickInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consolidation")
    float ValidationThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Consolidation")
    int32 MaxSubsystems;

private:
    // === INTERNAL METHODS ===

    /**
     * Perform consolidation cycle
     */
    void PerformConsolidationCycle();

    /**
     * Validate individual subsystem
     */
    float ValidateSubsystem(const FString& SubsystemName, UActorComponent* Component);

    /**
     * Update performance metrics
     */
    void UpdatePerformanceMetrics(float DeltaTime);

    /**
     * Calculate overall integration score
     */
    void CalculateIntegrationScore();

    /**
     * Handle consolidation error
     */
    void HandleConsolidationError(const FString& ErrorMessage);

    // === INTERNAL STATE ===

    FTimerHandle ConsolidationTimerHandle;
    FString LastErrorMessage;
    float ConsolidationDuration;
};