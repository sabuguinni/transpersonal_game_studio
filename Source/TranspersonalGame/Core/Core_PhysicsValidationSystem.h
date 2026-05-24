#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Subsystems/WorldSubsystem.h"
#include "SharedTypes.h"
#include "Core_PhysicsValidationSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPhysicsValidationComplete, bool, bSuccess, FString, ValidationReport);

/**
 * Physics Validation System for TranspersonalGame
 * Validates physics simulation integrity, collision detection accuracy, and performance metrics
 * Ensures physics systems meet quality standards for realistic dinosaur survival gameplay
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UCore_PhysicsValidationSystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UCore_PhysicsValidationSystem();

    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

    // Physics validation functions
    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidatePhysicsSimulation();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidateCollisionDetection();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidateRigidBodyIntegrity();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    bool ValidatePhysicsPerformance();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation")
    FString GenerateValidationReport();

    UFUNCTION(BlueprintCallable, Category = "Physics Validation", CallInEditor)
    void RunFullPhysicsValidation();

    // Physics test creation
    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    AActor* CreatePhysicsTestActor(FVector Location, FString TestName);

    UFUNCTION(BlueprintCallable, Category = "Physics Testing")
    void CleanupPhysicsTestActors();

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetPhysicsFrameTime();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    int32 GetActivePhysicsBodies();

    UFUNCTION(BlueprintCallable, Category = "Physics Performance")
    float GetCollisionQueryTime();

    // Event delegates
    UPROPERTY(BlueprintAssignable, Category = "Physics Events")
    FOnPhysicsValidationComplete OnValidationComplete;

protected:
    // Validation state
    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    bool bValidationInProgress;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    float LastValidationTime;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    int32 ValidationPassCount;

    UPROPERTY(BlueprintReadOnly, Category = "Validation State")
    int32 ValidationFailCount;

    // Performance metrics
    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float AveragePhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    int32 MaxPhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Performance Metrics")
    float MaxCollisionQueryTime;

    // Test actors
    UPROPERTY()
    TArray<TWeakObjectPtr<AActor>> PhysicsTestActors;

    // Validation thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxAcceptableFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Thresholds", meta = (AllowPrivateAccess = "true"))
    int32 MaxAcceptablePhysicsBodies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation Thresholds", meta = (AllowPrivateAccess = "true"))
    float MaxAcceptableCollisionQueryTime;

private:
    // Internal validation functions
    bool ValidatePhysicsWorld();
    bool ValidatePhysicsConstraints();
    bool ValidatePhysicsMaterials();
    bool ValidateCollisionChannels();
    
    // Performance measurement
    void UpdatePerformanceMetrics();
    void ResetPerformanceCounters();
    
    // Reporting
    void LogValidationResult(const FString& TestName, bool bPassed, const FString& Details = TEXT(""));
    FString FormatValidationReport(const TArray<FString>& TestResults);
    
    // Test data
    TArray<FString> ValidationResults;
    float ValidationStartTime;
    
    // Physics world reference
    UWorld* CachedWorld;
};