#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Eng_TechnicalArchitecture.h"
#include "SharedTypes.h"
#include "Core_PhysicsArchitecturalCompliance.generated.h"

/**
 * Physics Architectural Compliance System
 * Ensures all physics systems comply with Technical Architecture requirements
 * Integrates with Engine Architect's performance budgets and validation framework
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsComplianceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Compliance")
    float PhysicsFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Compliance")
    int32 ActivePhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Compliance")
    int32 CollisionChecksPerFrame;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Compliance")
    float MemoryUsageMB;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Compliance")
    bool bWithinPerformanceBudget;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Compliance")
    TArray<FString> ComplianceViolations;

    FCore_PhysicsComplianceMetrics()
    {
        PhysicsFrameTime = 0.0f;
        ActivePhysicsBodies = 0;
        CollisionChecksPerFrame = 0;
        MemoryUsageMB = 0.0f;
        bWithinPerformanceBudget = true;
        ComplianceViolations.Empty();
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FCore_PhysicsSystemRegistration
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registration")
    FString SystemName;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registration")
    int32 SystemPriority;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registration")
    float MaxFrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registration")
    int32 MaxPhysicsBodies;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registration")
    bool bThreadSafe;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Registration")
    bool bRegisteredWithArchitecture;

    FCore_PhysicsSystemRegistration()
    {
        SystemName = TEXT("Unknown");
        SystemPriority = 50;
        MaxFrameTime = 16.67f; // 60 FPS default
        MaxPhysicsBodies = 1000;
        bThreadSafe = false;
        bRegisteredWithArchitecture = false;
    }
};

/**
 * Physics Architectural Compliance Subsystem
 * Manages compliance of all physics systems with Technical Architecture
 */
UCLASS()
class TRANSPERSONALGAME_API UCore_PhysicsArchitecturalComplianceSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Physics System Registration
    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    bool RegisterPhysicsSystem(const FString& SystemName, int32 Priority, float MaxFrameTime, int32 MaxBodies, bool bIsThreadSafe);

    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    bool UnregisterPhysicsSystem(const FString& SystemName);

    // Compliance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    FCore_PhysicsComplianceMetrics GetCurrentComplianceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    bool ValidatePhysicsCompliance();

    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    TArray<FString> GetComplianceViolations() const;

    // Performance Budget Integration
    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    bool IsWithinPerformanceBudget() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    void SetPerformanceProfile(EEng_PerformanceProfile Profile);

    // Technical Architecture Integration
    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    bool RegisterWithTechnicalArchitecture();

    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    void ReportComplianceViolation(const FString& SystemName, const FString& ViolationType, const FString& Details);

protected:
    // Registered physics systems
    UPROPERTY()
    TMap<FString, FCore_PhysicsSystemRegistration> RegisteredSystems;

    // Current compliance metrics
    UPROPERTY()
    FCore_PhysicsComplianceMetrics CurrentMetrics;

    // Performance profile
    UPROPERTY()
    EEng_PerformanceProfile CurrentPerformanceProfile;

    // Technical Architecture reference
    UPROPERTY()
    class UEng_TechnicalArchitectureSubsystem* TechnicalArchitecture;

    // Compliance validation timer
    FTimerHandle ComplianceValidationTimer;

    // Internal methods
    void UpdateComplianceMetrics();
    void ValidateSystemPerformance();
    void CheckMemoryUsage();
    void ValidateThreadSafety();
    bool IsSystemCompliant(const FCore_PhysicsSystemRegistration& System) const;
};

/**
 * Physics Architectural Compliance Component
 * Per-actor compliance monitoring for physics systems
 */
UCLASS(ClassGroup=(Physics), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCore_PhysicsArchitecturalComplianceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCore_PhysicsArchitecturalComplianceComponent();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Compliance Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Compliance")
    bool bEnableComplianceMonitoring;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Compliance")
    float ComplianceCheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Compliance")
    bool bAutoReportViolations;

    // Performance Limits
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float MaxPhysicsFrameTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    int32 MaxCollisionChecks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics Performance")
    float MaxMemoryUsageMB;

    // Compliance Status
    UPROPERTY(BlueprintReadOnly, Category = "Physics Compliance")
    bool bIsCompliant;

    UPROPERTY(BlueprintReadOnly, Category = "Physics Compliance")
    TArray<FString> CurrentViolations;

    // Compliance Methods
    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    bool ValidateActorCompliance();

    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    FCore_PhysicsComplianceMetrics GetActorComplianceMetrics() const;

    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    void ReportActorViolation(const FString& ViolationType, const FString& Details);

    UFUNCTION(BlueprintCallable, Category = "Physics Compliance")
    void SetComplianceLimits(float MaxFrameTime, int32 MaxCollisions, float MaxMemory);

protected:
    // Compliance subsystem reference
    UPROPERTY()
    UCore_PhysicsArchitecturalComplianceSubsystem* ComplianceSubsystem;

    // Performance tracking
    float LastFrameTime;
    int32 LastCollisionCount;
    float LastMemoryUsage;
    float ComplianceCheckTimer;

    // Internal validation methods
    void CheckPhysicsPerformance();
    void CheckCollisionCompliance();
    void CheckMemoryCompliance();
    void UpdateComplianceStatus();
};