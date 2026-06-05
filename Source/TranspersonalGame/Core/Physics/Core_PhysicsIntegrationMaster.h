#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/TranspersonalGame.h"
#include "Core_PhysicsIntegrationMaster.generated.h"

/**
 * Master physics integration component that coordinates all physics systems
 * for the WALK AROUND milestone. Manages character physics, collision systems,
 * terrain interaction, and camera physics in a unified framework.
 * 
 * This component serves as the central hub for all physics-related functionality,
 * ensuring proper integration between movement, collision, and environmental physics.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsIntegrationMaster : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsIntegrationMaster();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === MILESTONE VALIDATION SYSTEM ===
    
    /** Validates all physics systems for WALK AROUND milestone completion */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration", CallInEditor = true)
    bool ValidateWalkAroundMilestone();
    
    /** Gets current milestone completion percentage (0.0 to 1.0) */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration")
    float GetMilestoneCompletionPercentage() const;
    
    /** Forces a complete physics system refresh and validation */
    UFUNCTION(BlueprintCallable, Category = "Physics Integration", CallInEditor = true)
    void RefreshPhysicsIntegration();

    // === CHARACTER PHYSICS INTEGRATION ===
    
    /** Integrates character movement physics with terrain and collision systems */
    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    bool IntegrateCharacterPhysics(ACharacter* Character);
    
    /** Validates character movement component physics settings */
    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    bool ValidateMovementPhysics(UCharacterMovementComponent* MovementComp);
    
    /** Configures optimal physics parameters for character movement */
    UFUNCTION(BlueprintCallable, Category = "Character Physics")
    void ConfigureCharacterPhysics(ACharacter* Character);

    // === COLLISION SYSTEM INTEGRATION ===
    
    /** Validates collision detection systems across all physics components */
    UFUNCTION(BlueprintCallable, Category = "Collision Physics")
    bool ValidateCollisionSystems();
    
    /** Tests collision response between character and environment */
    UFUNCTION(BlueprintCallable, Category = "Collision Physics")
    bool TestCollisionResponse(ACharacter* Character);
    
    /** Optimizes collision settings for performance and accuracy */
    UFUNCTION(BlueprintCallable, Category = "Collision Physics")
    void OptimizeCollisionSettings();

    // === TERRAIN PHYSICS INTEGRATION ===
    
    /** Integrates character physics with landscape/terrain systems */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool IntegrateTerrainPhysics();
    
    /** Validates terrain collision and physics material setup */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    bool ValidateTerrainPhysics();
    
    /** Configures physics materials for realistic terrain interaction */
    UFUNCTION(BlueprintCallable, Category = "Terrain Physics")
    void ConfigureTerrainPhysicsMaterials();

    // === CAMERA PHYSICS INTEGRATION ===
    
    /** Integrates camera physics with character movement and collision */
    UFUNCTION(BlueprintCallable, Category = "Camera Physics")
    bool IntegrateCameraPhysics(ACharacter* Character);
    
    /** Validates spring arm physics and collision avoidance */
    UFUNCTION(BlueprintCallable, Category = "Camera Physics")
    bool ValidateCameraPhysics(USpringArmComponent* SpringArm);
    
    /** Configures camera physics for smooth third-person experience */
    UFUNCTION(BlueprintCallable, Category = "Camera Physics")
    void ConfigureCameraPhysics(USpringArmComponent* SpringArm, UCameraComponent* Camera);

    // === PHYSICS PERFORMANCE MONITORING ===
    
    /** Gets current physics performance metrics */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    FString GetPhysicsPerformanceReport() const;
    
    /** Monitors physics system performance in real-time */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StartPerformanceMonitoring();
    
    /** Stops physics performance monitoring */
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    void StopPerformanceMonitoring();

protected:
    // === MILESTONE TRACKING PROPERTIES ===
    
    /** Current milestone completion status for each physics system */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Status")
    TMap<FString, bool> MilestoneRequirements;
    
    /** Overall milestone completion percentage */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Status")
    float MilestoneCompletion;
    
    /** Last validation timestamp */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Milestone Status")
    FDateTime LastValidationTime;

    // === PHYSICS INTEGRATION SETTINGS ===
    
    /** Enable automatic physics optimization */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bAutoOptimizePhysics;
    
    /** Enable real-time performance monitoring */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings")
    bool bEnablePerformanceMonitoring;
    
    /** Physics validation frequency (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float ValidationFrequency;
    
    /** Maximum allowed physics frame time (milliseconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Settings", meta = (ClampMin = "1.0", ClampMax = "33.3"))
    float MaxPhysicsFrameTime;

    // === PERFORMANCE TRACKING ===
    
    /** Current physics frame time in milliseconds */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    float CurrentPhysicsFrameTime;
    
    /** Average physics frame time over last 60 frames */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    float AveragePhysicsFrameTime;
    
    /** Number of active physics bodies being simulated */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    int32 ActivePhysicsBodies;
    
    /** Number of collision checks performed per frame */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance Metrics")
    int32 CollisionChecksPerFrame;

private:
    // === INTERNAL VALIDATION METHODS ===
    
    /** Validates character movement physics integration */
    bool ValidateCharacterMovementIntegration();
    
    /** Validates collision detection and response systems */
    bool ValidateCollisionIntegration();
    
    /** Validates terrain physics interaction */
    bool ValidateTerrainIntegration();
    
    /** Validates camera physics and spring arm setup */
    bool ValidateCameraIntegration();
    
    /** Validates overall physics simulation performance */
    bool ValidatePhysicsPerformance();
    
    /** Updates milestone completion status */
    void UpdateMilestoneStatus();
    
    /** Logs detailed validation results */
    void LogValidationResults();

    // === PERFORMANCE MONITORING INTERNALS ===
    
    /** Timer for periodic validation */
    FTimerHandle ValidationTimer;
    
    /** Performance monitoring state */
    bool bIsMonitoringPerformance;
    
    /** Frame time history for averaging */
    TArray<float> FrameTimeHistory;
    
    /** Maximum frame time history size */
    static constexpr int32 MaxFrameTimeHistory = 60;
};