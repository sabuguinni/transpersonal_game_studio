#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Eng_SystemIntegration.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogSystemIntegration, Log, All);

/**
 * System Integration Event Types
 */
UENUM(BlueprintType)
enum class EEng_SystemEvent : uint8
{
	SystemInitialized		UMETA(DisplayName = "System Initialized"),
	SystemShutdown			UMETA(DisplayName = "System Shutdown"),
	ModuleLoaded			UMETA(DisplayName = "Module Loaded"),
	ModuleUnloaded			UMETA(DisplayName = "Module Unloaded"),
	PerformanceWarning		UMETA(DisplayName = "Performance Warning"),
	ArchitectureViolation	UMETA(DisplayName = "Architecture Violation"),
	IntegrationComplete		UMETA(DisplayName = "Integration Complete")
};

/**
 * System Integration Priority Levels
 */
UENUM(BlueprintType)
enum class EEng_IntegrationPriority : uint8
{
	Critical		UMETA(DisplayName = "Critical"),
	High			UMETA(DisplayName = "High"),
	Medium			UMETA(DisplayName = "Medium"),
	Low				UMETA(DisplayName = "Low"),
	Background		UMETA(DisplayName = "Background")
};

/**
 * System Integration Data Structure
 */
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemIntegrationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
	FString SystemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
	EEng_IntegrationPriority Priority;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
	bool bIsInitialized;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
	float InitializationTime;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
	TArray<FString> Dependencies;

	FEng_SystemIntegrationData()
	{
		SystemName = TEXT("");
		Priority = EEng_IntegrationPriority::Medium;
		bIsInitialized = false;
		InitializationTime = 0.0f;
	}
};

/**
 * System Integration Manager - Coordinates all game systems
 * Ensures proper initialization order and dependency management
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_SystemIntegration : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// System registration
	UFUNCTION(BlueprintCallable, Category = "System Integration")
	void RegisterSystem(const FString& SystemName, EEng_IntegrationPriority Priority, const TArray<FString>& Dependencies);

	UFUNCTION(BlueprintCallable, Category = "System Integration")
	bool UnregisterSystem(const FString& SystemName);

	// System lifecycle
	UFUNCTION(BlueprintCallable, Category = "System Integration")
	bool InitializeSystem(const FString& SystemName);

	UFUNCTION(BlueprintCallable, Category = "System Integration")
	bool ShutdownSystem(const FString& SystemName);

	// System queries
	UFUNCTION(BlueprintCallable, Category = "System Integration")
	bool IsSystemRegistered(const FString& SystemName) const;

	UFUNCTION(BlueprintCallable, Category = "System Integration")
	bool IsSystemInitialized(const FString& SystemName) const;

	UFUNCTION(BlueprintCallable, Category = "System Integration")
	TArray<FString> GetSystemDependencies(const FString& SystemName) const;

	// Integration validation
	UFUNCTION(BlueprintCallable, Category = "System Integration")
	bool ValidateSystemIntegration();

	UFUNCTION(BlueprintCallable, Category = "System Integration")
	void LogIntegrationStatus() const;

	// Event handling
	UFUNCTION(BlueprintCallable, Category = "System Integration")
	void BroadcastSystemEvent(EEng_SystemEvent EventType, const FString& SystemName, const FString& Message);

protected:
	// System registry
	UPROPERTY(BlueprintReadOnly, Category = "Integration")
	TMap<FString, FEng_SystemIntegrationData> RegisteredSystems;

	// Integration state
	UPROPERTY(BlueprintReadOnly, Category = "Integration")
	bool bIntegrationComplete;

	UPROPERTY(BlueprintReadOnly, Category = "Integration")
	float TotalInitializationTime;

private:
	void InitializeCriticalSystems();
	bool ValidateDependencies(const FString& SystemName) const;
	void SortSystemsByPriority(TArray<FString>& SystemNames) const;
};

/**
 * System Integration Component - Per-actor integration tracking
 * Monitors individual actor system dependencies and initialization
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_SystemIntegrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEng_SystemIntegrationComponent();

protected:
	virtual void BeginPlay() override;

public:
	// Integration management
	UFUNCTION(BlueprintCallable, Category = "System Integration")
	void RegisterActorWithSystems();

	UFUNCTION(BlueprintCallable, Category = "System Integration")
	bool ValidateActorIntegration();

	UFUNCTION(BlueprintCallable, Category = "System Integration")
	void NotifySystemDependency(const FString& SystemName);

	// Component lifecycle
	UFUNCTION(BlueprintCallable, Category = "System Integration")
	void InitializeIntegrationComponent();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
	TArray<FString> RequiredSystems;

	UPROPERTY(BlueprintReadOnly, Category = "Integration")
	bool bIntegrationValid;

	UPROPERTY(BlueprintReadOnly, Category = "Integration")
	float IntegrationStartTime;

private:
	void CheckSystemAvailability();
	void ValidateRequiredSystems();
};