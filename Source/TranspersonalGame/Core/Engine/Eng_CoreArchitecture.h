#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Eng_CoreArchitecture.generated.h"

TRANSPERSONALGAME_API DECLARE_LOG_CATEGORY_EXTERN(LogEngineArchitect, Log, All);

/**
 * Engine Architecture Subsystem - Core technical foundation
 * Manages module loading order, dependency validation, and system initialization
 */
UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_CoreArchitecture : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Core architecture management
	UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
	bool ValidateSystemDependencies();

	UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
	void RegisterCoreModule(const FString& ModuleName, int32 Priority);

	UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
	bool IsModuleLoaded(const FString& ModuleName) const;

	// Performance monitoring
	UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
	float GetCurrentFrameTime() const;

	UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
	int32 GetActiveActorCount() const;

	UFUNCTION(BlueprintCallable, Category = "Engine Architecture")
	void LogSystemStatus() const;

protected:
	// Module registry
	UPROPERTY(BlueprintReadOnly, Category = "Architecture")
	TMap<FString, int32> LoadedModules;

	// Performance tracking
	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	float LastFrameTime;

	UPROPERTY(BlueprintReadOnly, Category = "Performance")
	int32 MaxActorLimit;

private:
	void InitializeCoreModules();
	void ValidatePerformanceLimits();
};

/**
 * Engine Architecture Component - Per-actor architecture validation
 * Ensures actors follow proper initialization patterns
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_ArchitectureComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEng_ArchitectureComponent();

protected:
	virtual void BeginPlay() override;

public:
	// Architecture validation
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	bool ValidateActorArchitecture();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void RegisterWithArchitectureSystem();

	// Component lifecycle
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void InitializeArchitectureComponent();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
	bool bValidateOnBeginPlay;

	UPROPERTY(BlueprintReadOnly, Category = "Architecture")
	bool bIsArchitectureValid;

	UPROPERTY(BlueprintReadOnly, Category = "Architecture")
	FString ComponentVersion;

private:
	void PerformValidationChecks();
};