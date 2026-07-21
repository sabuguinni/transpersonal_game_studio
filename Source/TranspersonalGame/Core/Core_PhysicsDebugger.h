#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "PhysicsEngine/PhysicsSettings.h"
#include "Core_PhysicsDebugger.generated.h"

/**
 * Core Physics Debugger - Visual debugging and profiling for physics systems
 * Provides real-time visualization of physics interactions, collision detection,
 * and performance metrics for the Core Physics suite.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsDebugger : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsDebugger();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Debug Visualization Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Debug")
    bool bShowCollisionShapes = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Debug")
    bool bShowVelocityVectors = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Debug")
    bool bShowForceVectors = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Debug")
    bool bShowContactPoints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Debug")
    bool bShowRagdollConstraints = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Debug")
    float DebugLineThickness = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Debug")
    float VectorScale = 100.0f;

    // Performance Monitoring
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bMonitorPerformance = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float PerformanceUpdateInterval = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsFrameTime = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveRigidBodies = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveConstraints = 0;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 CollisionPairs = 0;

    // Debug Functions
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void ToggleCollisionShapes();

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void ToggleVelocityVectors();

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void ToggleForceVectors();

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void ToggleContactPoints();

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void ToggleRagdollConstraints();

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawPhysicsDebugInfo();

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void ClearDebugDrawing();

    // Performance Analysis
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void UpdatePerformanceMetrics();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    FString GetPerformanceReport() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void LogPerformanceMetrics();

    // Collision Testing
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    bool TestCollisionAtLocation(const FVector& Location, float Radius = 50.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawCollisionSphere(const FVector& Location, float Radius, const FColor& Color, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawVelocityVector(AActor* Actor, const FColor& Color, float Duration = 0.1f);

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void DrawForceVector(const FVector& Origin, const FVector& Force, const FColor& Color, float Duration = 0.1f);

    // Physics State Analysis
    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    TArray<AActor*> GetPhysicsActorsInRadius(const FVector& Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Physics Debug")
    void AnalyzePhysicsState();

    UFUNCTION(BlueprintCallable, Category = "Physics Debug", CallInEditor = true)
    void RunPhysicsValidation();

private:
    // Internal state
    float PerformanceTimer = 0.0f;
    TArray<FVector> ContactPointHistory;
    TArray<FVector> VelocityHistory;
    
    // Debug drawing helpers
    void DrawCollisionShapesForActor(AActor* Actor);
    void DrawVelocityVectorsForActor(AActor* Actor);
    void UpdateContactPointHistory();
    void CleanupOldDebugData();
    
    // Performance tracking
    void CollectPhysicsMetrics();
    void CalculateFrameTime();
    
    // Validation helpers
    bool ValidatePhysicsComponents();
    bool ValidateCollisionSettings();
    bool ValidateConstraintSetup();
};