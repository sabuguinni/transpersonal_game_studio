#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "ConsciousnessSystem.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
	Ordinary		UMETA(DisplayName = "Ordinary Waking"),
	Dreaming		UMETA(DisplayName = "Dream State"),
	Meditation		UMETA(DisplayName = "Meditative"),
	Transcendent	UMETA(DisplayName = "Transcendent"),
	Unity			UMETA(DisplayName = "Unity Consciousness"),
	Shadow			UMETA(DisplayName = "Shadow Integration")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FConsciousnessLevel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EConsciousnessState State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Depth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Clarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Integration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTagContainer ActiveTags;

	FConsciousnessLevel()
	{
		State = EConsciousnessState::Ordinary;
		Depth = 0.0f;
		Clarity = 1.0f;
		Integration = 0.0f;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnConsciousnessChanged, EConsciousnessState, NewState, float, Intensity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAwarenessShift, float, AwarenessLevel);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessSystem : public UActorComponent
{
	GENERATED_BODY()

public:
	UConsciousnessSystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Current consciousness state
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Consciousness")
	FConsciousnessLevel CurrentLevel;

	// Base awareness level (0-1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float BaseAwareness;

	// Transition speed between states
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
	float TransitionSpeed;

	// Events
	UPROPERTY(BlueprintAssignable)
	FOnConsciousnessChanged OnConsciousnessChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAwarenessShift OnAwarenessShift;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Consciousness")
	void TransitionToState(EConsciousnessState NewState, float TargetDepth = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "Consciousness")
	void ModifyAwareness(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Consciousness")
	void IntegrateExperience(float IntegrationAmount);

	UFUNCTION(BlueprintPure, Category = "Consciousness")
	float GetConsciousnessDepth() const { return CurrentLevel.Depth; }

	UFUNCTION(BlueprintPure, Category = "Consciousness")
	float GetClarity() const { return CurrentLevel.Clarity; }

	UFUNCTION(BlueprintPure, Category = "Consciousness")
	bool IsInTranscendentState() const;

private:
	// Target state for transitions
	FConsciousnessLevel TargetLevel;
	bool bIsTransitioning;

	void UpdateTransition(float DeltaTime);
	void CalculateClarity();
};