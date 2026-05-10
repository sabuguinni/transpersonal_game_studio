#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "SharedTypes.h"
#include "Eng_ArchitecturalFramework.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEngArchFramework, Log, All);

/**
 * Architectural Framework - Core system that defines and enforces
 * the technical architecture standards across all game systems.
 * This is the foundation that all other systems must conform to.
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEng_ArchitecturalFramework();

	// Subsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Core architectural validation
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	bool ValidateSystemArchitecture();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	bool ValidateModuleDependencies();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	bool ValidatePerformanceStandards();

	// System registration and management
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void RegisterCoreSystem(const FString& SystemName, UObject* SystemInstance);

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	UObject* GetCoreSystem(const FString& SystemName);

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	TArray<FString> GetRegisteredSystems() const;

	// Performance monitoring
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StartPerformanceMonitoring();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void StopPerformanceMonitoring();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	FString GetPerformanceReport() const;

	// Architecture enforcement
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void EnforceArchitecturalStandards();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	bool CheckSystemCompliance(const FString& SystemName);

	// Debug and diagnostics
	UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
	void RunArchitecturalDiagnostics();

	UFUNCTION(BlueprintCallable, Category = "Debug", CallInEditor = true)
	void GenerateArchitectureReport();

protected:
	// Core system registry
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
	TMap<FString, TWeakObjectPtr<UObject>> RegisteredSystems;

	// Performance metrics
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	bool bPerformanceMonitoringEnabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float FrameTimeThreshold;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float MemoryUsageThreshold;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	int32 MaxSimultaneousPhysicsObjects;

	// Architecture validation settings
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
	bool bEnforceStrictCompliance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
	bool bLogArchitecturalViolations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
	bool bAutoFixMinorViolations;

private:
	// Internal validation methods
	bool ValidatePhysicsArchitecture();
	bool ValidateWorldGenerationArchitecture();
	bool ValidateBiomeArchitecture();
	bool ValidateCharacterArchitecture();
	bool ValidateAIArchitecture();

	// Performance tracking
	void UpdatePerformanceMetrics();
	void CheckFrameTimeCompliance();
	void CheckMemoryUsageCompliance();

	// System health monitoring
	void MonitorSystemHealth();
	bool IsSystemHealthy(UObject* System);

	// Architecture enforcement helpers
	void FixArchitecturalViolations();
	void LogArchitecturalIssue(const FString& Issue, const FString& System);
};

/**
 * Architectural Standards Enforcer Component
 * Attach to critical actors to ensure they follow architectural guidelines
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Architecture), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_ArchitecturalEnforcer : public UActorComponent
{
	GENERATED_BODY()

public:
	UEng_ArchitecturalEnforcer();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Compliance checking
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	bool CheckActorCompliance();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void EnforceActorStandards();

	// Performance monitoring for this actor
	UFUNCTION(BlueprintCallable, Category = "Performance")
	float GetActorPerformanceCost() const;

	UFUNCTION(BlueprintCallable, Category = "Performance")
	bool IsActorPerformanceCompliant() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
	bool bEnforcePerformanceStandards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
	float MaxAllowedPerformanceCost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
	bool bAutoOptimizeForPerformance;

private:
	float CurrentPerformanceCost;
	bool bComplianceCheckPassed;

	void CalculatePerformanceCost();
	void OptimizeForPerformance();
};

/**
 * Architectural Metrics Structure
 * Contains comprehensive metrics about system architecture health
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ArchitecturalMetrics
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 TotalRegisteredSystems;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 CompliantSystems;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 NonCompliantSystems;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float AverageFrameTime;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float PeakMemoryUsage;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	int32 ActivePhysicsObjects;

	UPROPERTY(BlueprintReadOnly, Category = "Metrics")
	float OverallArchitecturalHealth;

	FEng_ArchitecturalMetrics()
	{
		TotalRegisteredSystems = 0;
		CompliantSystems = 0;
		NonCompliantSystems = 0;
		AverageFrameTime = 0.0f;
		PeakMemoryUsage = 0.0f;
		ActivePhysicsObjects = 0;
		OverallArchitecturalHealth = 100.0f;
	}
};

#include "Eng_ArchitecturalFramework.generated.h"