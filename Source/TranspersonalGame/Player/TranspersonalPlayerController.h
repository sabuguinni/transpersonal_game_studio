#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/ConsciousnessSystem.h"
#include "Core/RealitySystem.h"
#include "Core/PerceptionSystem.h"
#include "TranspersonalPlayerController.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATranspersonalPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    ATranspersonalPlayerController();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;
    virtual void Tick(float DeltaTime) override;

    // System References
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    class UConsciousnessSystem* ConsciousnessSystem;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    class URealitySystem* RealitySystem;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    class UPerceptionSystem* PerceptionSystem;

    // Input Actions
    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnMeditatePressed();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnMeditateReleased();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnCyclePerceptionMode();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnToggleEnergyVision();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnToggleSymbolicVision();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnIncreaseAwareness();

    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnDecreaseAwareness();

    // Consciousness Control
    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void InitiateConsciousnessShift(EConsciousnessState TargetState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ModifyConsciousnessMetric(const FString& MetricName, float Delta);

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void EnterMeditativeState();

    UFUNCTION(BlueprintCallable, Category = "Consciousness")
    void ExitMeditativeState();

    // Reality Manipulation
    UFUNCTION(BlueprintCallable, Category = "Reality")
    void CreateRealityRipple(FVector Location, float Intensity = 0.5f);

    UFUNCTION(BlueprintCallable, Category = "Reality")
    void ShiftRealityLayer(ERealityLayer NewLayer);

    UFUNCTION(BlueprintCallable, Category = "Reality")
    void StabilizeReality();

    // Perception Control
    UFUNCTION(BlueprintCallable, Category = "Perception")
    void CyclePerceptionMode();

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void TogglePerceptionFilter(EPerceptionFilter Filter);

    UFUNCTION(BlueprintCallable, Category = "Perception")
    void FocusPerception(AActor* Target);

    // Event Handlers
    UFUNCTION()
    void OnConsciousnessStateChanged(EConsciousnessState OldState, EConsciousnessState NewState);

    UFUNCTION()
    void OnRealityLayerChanged(ERealityLayer NewLayer);

    UFUNCTION()
    void OnPerceptionModeChanged(EPerceptionMode NewMode);

    // UI Integration
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateConsciousnessUI(const FConsciousnessMetrics& Metrics);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateRealityUI(ERealityLayer Layer, ERealityStability Stability);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdatePerceptionUI(EPerceptionMode Mode, const TArray<EPerceptionFilter>& ActiveFilters);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowTransitionEffect(EConsciousnessState FromState, EConsciousnessState ToState);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float MeditationRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float AwarenessModificationRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float RealityRippleRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    float RealityRippleDuration = 5.0f;

private:
    bool bIsMeditating;
    float MeditationTimer;
    int32 CurrentPerceptionModeIndex;

    void UpdateMeditation(float DeltaTime);
    void BindSystemEvents();
    void UnbindSystemEvents();
};