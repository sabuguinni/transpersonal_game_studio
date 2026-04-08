#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "ConsciousnessSystem.h"
#include "RealitySystem.h"
#include "PerceptionSystem.generated.h"

UENUM(BlueprintType)
enum class EPerceptionMode : uint8
{
	Normal			UMETA(DisplayName = "Normal Vision"),
	Aura			UMETA(DisplayName = "Aura Sight"),
	Symbolic		UMETA(DisplayName = "Symbolic Vision"),
	Energetic		UMETA(DisplayName = "Energy Patterns"),
	Archetypal		UMETA(DisplayName = "Archetypal Forms"),
	Unity			UMETA(DisplayName = "Unity Perception")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerceptionFilter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EPerceptionMode Mode;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Intensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bActive;

	FPerceptionFilter()
	{
		Mode = EPerceptionMode::Normal;
		Intensity = 1.0f;
		Range = 1000.0f;
		bActive = false;
	}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerceptualObject
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AActor* Actor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector AuraColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EnergyLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString SymbolicMeaning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsArchetypal;

	FPerceptualObject()
	{
		Actor = nullptr;
		AuraColor = FVector(1.0f, 1.0f, 1.0f);
		EnergyLevel = 0.0f;
		SymbolicMeaning = TEXT("");
		bIsArchetypal = false;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerceptionModeChanged, EPerceptionMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnObjectPerceived, AActor*, Object, FPerceptualObject, PerceptionData);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UPerceptionSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UPerceptionSystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Active perception filters
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	TArray<FPerceptionFilter> ActiveFilters;

	// Current perception mode
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	EPerceptionMode CurrentMode;

	// Perception range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception")
	float PerceptionRange;

	// Sensitivity to subtle phenomena
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perception", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Sensitivity;

	// References to other systems
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	UConsciousnessSystem* ConsciousnessRef;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	URealitySystem* RealityRef;

	// Perceived objects cache
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Perception")
	TArray<FPerceptualObject> PerceivedObjects;

	// Events
	UPROPERTY(BlueprintAssignable)
	FOnPerceptionModeChanged OnPerceptionModeChanged;

	UPROPERTY(BlueprintAssignable)
	FOnObjectPerceived OnObjectPerceived;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Perception")
	void SetPerceptionMode(EPerceptionMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "Perception")
	void AddPerceptionFilter(EPerceptionMode Mode, float Intensity);

	UFUNCTION(BlueprintCallable, Category = "Perception")
	void RemovePerceptionFilter(EPerceptionMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Perception")
	void ModifySensitivity(float Delta);

	UFUNCTION(BlueprintPure, Category = "Perception")
	bool CanPerceiveAuras() const;

	UFUNCTION(BlueprintPure, Category = "Perception")
	bool CanPerceiveSymbols() const;

	UFUNCTION(BlueprintPure, Category = "Perception")
	FPerceptualObject GetObjectPerception(AActor* Object) const;

	UFUNCTION(BlueprintCallable, Category = "Perception")
	void ScanEnvironment();

private:
	// Perception update timer
	float PerceptionUpdateTimer;
	float PerceptionUpdateInterval;

	void UpdatePerceptionMode();
	void ProcessPerceivedObjects();
	FPerceptualObject AnalyzeObject(AActor* Object);
	void ApplyPerceptionFilters(FPerceptualObject& PerceptionData);
};