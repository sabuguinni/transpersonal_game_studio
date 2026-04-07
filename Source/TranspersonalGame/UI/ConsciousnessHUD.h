#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "TranspersonalGame/Core/ConsciousnessComponent.h"
#include "ConsciousnessHUD.generated.h"

USTRUCT(BlueprintType)
struct FConsciousnessVisualTheme
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Theme")
    EConsciousnessState ConsciousnessState = EConsciousnessState::Ordinary;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Theme")
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Theme")
    FLinearColor SecondaryColor = FLinearColor::Gray;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Theme")
    FLinearColor BackgroundColor = FLinearColor::Black;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Theme")
    float PulseSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Theme")
    float GeometryComplexity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Theme")
    bool bEnableParticleEffects = false;
};

UCLASS()
class TRANSPERSONALGAME_API UConsciousnessHUD : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    // UI Update Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness HUD")
    void UpdateConsciousnessDisplay(EConsciousnessState State, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Consciousness HUD")
    void SetVisualizationMode(bool bEnableAdvancedVisuals);

    UFUNCTION(BlueprintImplementableEvent, Category = "Consciousness HUD")
    void OnConsciousnessStateChanged(EConsciousnessState NewState, float Intensity);

    UFUNCTION(BlueprintImplementableEvent, Category = "Consciousness HUD")
    void TriggerTranspersonalVisualEffect(EConsciousnessState State);

protected:
    // UI Components
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UProgressBar> ConsciousnessIntensityBar;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UTextBlock> ConsciousnessStateText;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ConsciousnessVisualization;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCanvasPanel> GeometricOverlay;

    // Visual Themes
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Themes")
    TMap<EConsciousnessState, FConsciousnessVisualTheme> VisualThemes;

    // Current state
    EConsciousnessState CurrentDisplayState = EConsciousnessState::Ordinary;
    float CurrentDisplayIntensity = 0.0f;
    float PulseTimer = 0.0f;

    void InitializeVisualThemes();
    void ApplyVisualTheme(const FConsciousnessVisualTheme& Theme, float IntensityModifier);
    void UpdateGeometricVisualization(EConsciousnessState State, float Intensity, float DeltaTime);
    FLinearColor GetInterpolatedColor(const FConsciousnessVisualTheme& Theme, float Intensity);
};