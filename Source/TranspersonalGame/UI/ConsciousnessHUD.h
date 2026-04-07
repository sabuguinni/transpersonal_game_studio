#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Core/ConsciousnessSystem.h"
#include "ConsciousnessHUD.generated.h"

class UUserWidget;
class UConsciousnessWidget;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FHUDConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    bool bShowConsciousnessMetrics = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    bool bShowCurrentState = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    bool bShowRealityLayer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    bool bShowMeditationTimer = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    float HUDOpacity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Display")
    bool bMinimalMode = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    float TransitionDuration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    bool bUseBreathingAnimation = true;
};

/**
 * HUD system for displaying consciousness-related information
 * Provides visual feedback for meditation, awareness levels, and reality states
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AConsciousnessHUD : public AHUD
{
    GENERATED_BODY()

public:
    AConsciousnessHUD();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
    virtual void DrawHUD() override;
    virtual void Tick(float DeltaTime) override;

    // Widget Management
    UFUNCTION(BlueprintCallable, Category = "UI")
    void CreateConsciousnessWidget();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowConsciousnessWidget();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void HideConsciousnessWidget();

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ToggleConsciousnessWidget();

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetHUDConfiguration(const FHUDConfiguration& NewConfig);

    UFUNCTION(BlueprintPure, Category = "Configuration")
    FHUDConfiguration GetHUDConfiguration() const { return HUDConfig; }

    // Display Methods
    UFUNCTION(BlueprintCallable, Category = "Display")
    void ShowMeditationGuidance(bool bShow);

    UFUNCTION(BlueprintCallable, Category = "Display")
    void ShowStateTransitionFeedback(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Display")
    void ShowRealityShiftFeedback(ERealityLayer NewLayer);

    // Breathing Visualization
    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StartBreathingVisualization();

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StopBreathingVisualization();

    UFUNCTION(BlueprintPure, Category = "Meditation")
    bool IsShowingBreathingVisualization() const { return bShowingBreathingVisualization; }

protected:
    // Widget Classes
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> ConsciousnessWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> MeditationGuidanceWidgetClass;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI")
    TSubclassOf<UUserWidget> BreathingVisualizationWidgetClass;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FHUDConfiguration HUDConfig;

    // State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bConsciousnessWidgetVisible = true;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bShowingBreathingVisualization = false;

private:
    // Widget Instances
    UPROPERTY()
    TObjectPtr<UUserWidget> ConsciousnessWidget;

    UPROPERTY()
    TObjectPtr<UUserWidget> MeditationGuidanceWidget;

    UPROPERTY()
    TObjectPtr<UUserWidget> BreathingVisualizationWidget;

    // Consciousness System Reference
    UPROPERTY()
    TObjectPtr<UConsciousnessSystem> ConsciousnessSystem;

    // Event Handlers
    UFUNCTION()
    void OnConsciousnessStateChanged(EConsciousnessState OldState, EConsciousnessState NewState);

    UFUNCTION()
    void OnRealityLayerChanged(ERealityLayer OldLayer, ERealityLayer NewLayer);

    UFUNCTION()
    void OnConsciousnessMetricsUpdated(const FConsciousnessMetrics& NewMetrics);

    // Drawing Methods
    void DrawConsciousnessMetrics();
    void DrawCurrentState();
    void DrawRealityLayer();
    void DrawMeditationTimer();
    void DrawBreathingGuide();

    // Utility
    FString GetConsciousnessStateDisplayName(EConsciousnessState State) const;
    FString GetRealityLayerDisplayName(ERealityLayer Layer) const;
    FLinearColor GetStateColor(EConsciousnessState State) const;
    FLinearColor GetLayerColor(ERealityLayer Layer) const;

    // Animation
    void UpdateBreathingAnimation(float DeltaTime);
    float BreathingAnimationTime = 0.0f;
    float BreathingCycleDuration = 8.0f;

    // Feedback Timers
    FTimerHandle StateTransitionFeedbackTimer;
    FTimerHandle LayerShiftFeedbackTimer;
    
    bool bShowingStateTransition = false;
    bool bShowingLayerShift = false;
    EConsciousnessState FeedbackState;
    ERealityLayer FeedbackLayer;
};