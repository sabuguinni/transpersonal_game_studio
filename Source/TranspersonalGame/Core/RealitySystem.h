#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "ConsciousnessSystem.h"
#include "RealitySystem.generated.h"

UENUM(BlueprintType)
enum class ERealityLayer : uint8
{
	Physical		UMETA(DisplayName = "Physical Reality"),
	Emotional		UMETA(DisplayName = "Emotional Layer"),
	Mental			UMETA(DisplayName = "Mental Constructs"),
	Archetypal		UMETA(DisplayName = "Archetypal Realm"),
	Causal			UMETA(DisplayName = "Causal Dimension"),
	Void			UMETA(DisplayName = "The Void")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FRealityParameters
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Stability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Coherence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Permeability;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SymbolicDensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ERealityLayer ActiveLayer;

	FRealityParameters()
	{
		Stability = 1.0f;
		Coherence = 1.0f;
		Permeability = 0.0f;
		SymbolicDensity = 0.0f;
		ActiveLayer = ERealityLayer::Physical;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRealityShift, ERealityLayer, NewLayer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRealityDistortion, float, Intensity, float, Duration);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API URealitySystem : public UActorComponent
{
	GENERATED_BODY()

public:
	URealitySystem();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Current reality parameters
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reality")
	FRealityParameters CurrentReality;

	// Reference to consciousness system
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reality")
	UConsciousnessSystem* ConsciousnessRef;

	// Reality distortion effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
	float DistortionIntensity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reality")
	float LayerTransitionSpeed;

	// Events
	UPROPERTY(BlueprintAssignable)
	FOnRealityShift OnRealityShift;

	UPROPERTY(BlueprintAssignable)
	FOnRealityDistortion OnRealityDistortion;

	// Functions
	UFUNCTION(BlueprintCallable, Category = "Reality")
	void ShiftToLayer(ERealityLayer NewLayer);

	UFUNCTION(BlueprintCallable, Category = "Reality")
	void ApplyRealityDistortion(float Intensity, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Reality")
	void ModifyStability(float Delta);

	UFUNCTION(BlueprintCallable, Category = "Reality")
	void SetPermeability(float NewPermeability);

	UFUNCTION(BlueprintPure, Category = "Reality")
	float GetRealityCoherence() const { return CurrentReality.Coherence; }

	UFUNCTION(BlueprintPure, Category = "Reality")
	bool IsRealityStable() const { return CurrentReality.Stability > 0.7f; }

	UFUNCTION(BlueprintPure, Category = "Reality")
	ERealityLayer GetCurrentLayer() const { return CurrentReality.ActiveLayer; }

private:
	// Target reality for transitions
	FRealityParameters TargetReality;
	bool bIsTransitioning;

	// Distortion effects
	float DistortionTimer;
	float DistortionDuration;
	float BaseDistortionIntensity;

	void UpdateRealityTransition(float DeltaTime);
	void UpdateDistortionEffects(float DeltaTime);
	void SyncWithConsciousness();
	void CalculateRealityParameters();
};