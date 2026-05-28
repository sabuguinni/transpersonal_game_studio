#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "EngineArchitect.generated.h"

/**
 * Engine Architect - Core architecture validation and monitoring system
 * Responsible for ensuring architectural integrity across all game systems
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AEng_EngineArchitect : public AActor
{
	GENERATED_BODY()

public:
	AEng_EngineArchitect();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// Core architecture components
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
	UStaticMeshComponent* ArchitectureMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Architecture")
	UTextRenderComponent* StatusDisplay;

	// Architecture validation properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	bool bValidateModules;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	bool bValidateCompilation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	bool bValidatePerformance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Validation")
	float ValidationInterval;

	// Architecture status
	UPROPERTY(BlueprintReadOnly, Category = "Status")
	int32 ModulesValidated;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	int32 CompilationErrors;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	float PerformanceScore;

	UPROPERTY(BlueprintReadOnly, Category = "Status")
	FString ArchitectureStatus;

public:
	// Architecture validation functions
	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void ValidateModuleArchitecture();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void ValidateCompilationReadiness();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void ValidatePerformanceArchitecture();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void UpdateArchitectureStatus();

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	bool IsArchitectureHealthy() const;

	// Movement architecture validation
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void ValidateMovementArchitecture();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	bool IsMovementSystemReady() const;

	// Input architecture validation
	UFUNCTION(BlueprintCallable, Category = "Input")
	void ValidateInputArchitecture();

	UFUNCTION(BlueprintCallable, Category = "Input")
	bool IsInputSystemConfigured() const;

private:
	// Internal validation state
	float LastValidationTime;
	bool bArchitectureHealthy;
	bool bMovementSystemReady;
	bool bInputSystemConfigured;

	// Architecture monitoring
	void UpdateStatusDisplay();
	void LogArchitectureState();
	FLinearColor GetStatusColor() const;
};