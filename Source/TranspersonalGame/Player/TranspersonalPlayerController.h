#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "../Core/ConsciousnessSystem.h"
#include "../Core/RealitySystem.h"
#include "../Core/PerceptionSystem.h"
#include "TranspersonalPlayerController.generated.h"

UCLASS()
class TRANSPERSONALGAME_API ATranspersonalPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATranspersonalPlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:
	virtual void Tick(float DeltaTime) override;

	// System references
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transpersonal Systems")
	UConsciousnessSystem* ConsciousnessSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transpersonal Systems")
	URealitySystem* RealitySystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Transpersonal Systems")
	UPerceptionSystem* PerceptionSystem;

	// Input actions
	UFUNCTION(BlueprintCallable, Category = "Consciousness")
	void EnterMeditationState();

	UFUNCTION(BlueprintCallable, Category = "Consciousness")
	void ExitMeditationState();

	UFUNCTION(BlueprintCallable, Category = "Perception")
	void CyclePerceptionMode();

	UFUNCTION(BlueprintCallable, Category = "Reality")
	void ShiftRealityLayer();

	UFUNCTION(BlueprintCallable, Category = "Consciousness")
	void DeepBreathing();

	// UI and feedback
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateConsciousnessUI(EConsciousnessState State, float Depth, float Clarity);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdateRealityUI(ERealityLayer Layer, float Stability);

	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdatePerceptionUI(EPerceptionMode Mode, float Sensitivity);

private:
	// Input handlers
	void OnMeditationPressed();
	void OnMeditationReleased();
	void OnPerceptionCyclePressed();
	void OnRealityShiftPressed();
	void OnBreathingPressed();

	// Meditation state
	bool bIsMeditating;
	float MeditationTimer;
	float MeditationDepthRate;

	// UI update timer
	float UIUpdateTimer;
	float UIUpdateInterval;

	void UpdateUI();
	void InitializeSystems();
};