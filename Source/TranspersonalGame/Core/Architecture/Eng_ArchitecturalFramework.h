#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Eng_ArchitecturalFramework.generated.h"

UENUM(BlueprintType)
enum class EEng_ArchitecturalLayer : uint8
{
	Foundation		UMETA(DisplayName = "Foundation Layer"),
	Core			UMETA(DisplayName = "Core Systems"),
	Gameplay		UMETA(DisplayName = "Gameplay Layer"),
	Presentation	UMETA(DisplayName = "Presentation Layer")
};

UENUM(BlueprintType)
enum class EEng_SystemPriority : uint8
{
	Critical		UMETA(DisplayName = "Critical"),
	High			UMETA(DisplayName = "High"),
	Medium			UMETA(DisplayName = "Medium"),
	Low				UMETA(DisplayName = "Low")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SystemDependency
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Architecture")
	FString SystemName;

	UPROPERTY(BlueprintReadOnly, Category = "Architecture")
	EEng_ArchitecturalLayer Layer;

	UPROPERTY(BlueprintReadOnly, Category = "Architecture")
	EEng_SystemPriority Priority;

	UPROPERTY(BlueprintReadOnly, Category = "Architecture")
	TArray<FString> Dependencies;

	FEng_SystemDependency()
	{
		SystemName = TEXT("");
		Layer = EEng_ArchitecturalLayer::Foundation;
		Priority = EEng_SystemPriority::Medium;
	}
};

UCLASS(BlueprintType)
class TRANSPERSONALGAME_API UEng_ArchitecturalFramework : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void RegisterSystem(const FString& SystemName, EEng_ArchitecturalLayer Layer, EEng_SystemPriority Priority);

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void AddSystemDependency(const FString& SystemName, const FString& DependencyName);

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	TArray<FEng_SystemDependency> GetSystemsByLayer(EEng_ArchitecturalLayer Layer) const;

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	bool ValidateSystemDependencies() const;

	UFUNCTION(BlueprintCallable, Category = "Architecture")
	void EnforceArchitecturalRules();

	UFUNCTION(BlueprintCallable, CallInEditor = true, Category = "Architecture")
	void GenerateArchitectureReport();

private:
	UPROPERTY()
	TArray<FEng_SystemDependency> RegisteredSystems;

	void InitializeCoreSystems();
	bool CheckCircularDependencies() const;
	void ValidateLayerHierarchy() const;
};