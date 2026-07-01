#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "DawnAtmosphereController.generated.h"

UENUM(BlueprintType)
enum class ELight_DawnPhase : uint8
{
    PreDawn     UMETA(DisplayName = "Pre-Dawn (Night)"),
    CivilTwilight UMETA(DisplayName = "Civil Twilight"),
    Sunrise     UMETA(DisplayName = "Sunrise"),
    EarlyMorning UMETA(DisplayName = "Early Morning"),
    Midmorning  UMETA(DisplayName = "Mid-Morning")
};

USTRUCT(BlueprintType)
struct FLight_DawnPaletteSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    float SunPitch = -12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    float SunYaw = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    float SunIntensity = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sun")
    FLinearColor SunColor = FLinearColor(1.0f, 0.72f, 0.45f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    float FogDensity = 0.035f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    FLinearColor FogInscatterColor = FLinearColor(0.9f, 0.55f, 0.35f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Fog")
    bool bVolumetricFog = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sky")
    float SkyLightIntensity = 1.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Sky")
    FLinearColor SkyLightColor = FLinearColor(0.85f, 0.75f, 0.95f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|PostProcess")
    float BloomIntensity = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|PostProcess")
    float VignetteIntensity = 0.45f;
};

UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Dawn Atmosphere Controller"))
class TRANSPERSONALGAME_API ADawnAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    ADawnAtmosphereController();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Dawn Phase ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Phase")
    ELight_DawnPhase CurrentDawnPhase = ELight_DawnPhase::Sunrise;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Settings")
    FLight_DawnPaletteSettings DawnPalette;

    // --- Transition Speed ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Transition")
    float TransitionDurationSeconds = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Transition")
    bool bAutoAdvanceDawnPhase = false;

    // --- Light References ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Refs")
    TObjectPtr<ADirectionalLight> SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Refs")
    TObjectPtr<AExponentialHeightFog> HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Refs")
    TObjectPtr<ASkyLight> SkyLightActor;

    // --- Mist Glow Lights ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Mist")
    float MistGlowIntensity = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Mist")
    float MistGlowRadius = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|Mist")
    FLinearColor MistGlowColor = FLinearColor(1.0f, 0.75f, 0.45f, 1.0f);

    // --- Sunrise Rim Lights ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|RimLight")
    float RimLightIntensity = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dawn|RimLight")
    FLinearColor RimLightColor = FLinearColor(1.0f, 0.55f, 0.15f, 1.0f);

    // --- Functions ---
    UFUNCTION(BlueprintCallable, Category = "Dawn|Control")
    void ApplyDawnPalette();

    UFUNCTION(BlueprintCallable, Category = "Dawn|Control")
    void SetDawnPhase(ELight_DawnPhase NewPhase);

    UFUNCTION(BlueprintCallable, Category = "Dawn|Control")
    void TransitionToPhase(ELight_DawnPhase TargetPhase, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Dawn|Control")
    FLight_DawnPaletteSettings GetPaletteForPhase(ELight_DawnPhase Phase) const;

    UFUNCTION(BlueprintCallable, Category = "Dawn|Mist")
    void SpawnMistGlowLights(int32 Count = 6);

    UFUNCTION(BlueprintCallable, Category = "Dawn|RimLight")
    void SpawnSunriseRimLights();

    UFUNCTION(BlueprintPure, Category = "Dawn|State")
    ELight_DawnPhase GetCurrentDawnPhase() const { return CurrentDawnPhase; }

    UFUNCTION(BlueprintPure, Category = "Dawn|State")
    float GetTransitionProgress() const { return TransitionProgress; }

private:
    float TransitionProgress = 0.0f;
    float TransitionElapsed = 0.0f;
    bool bTransitioning = false;
    ELight_DawnPhase TargetPhaseInternal = ELight_DawnPhase::Sunrise;
    FLight_DawnPaletteSettings SourcePalette;
    FLight_DawnPaletteSettings TargetPalette;

    void UpdateLightingFromPalette(const FLight_DawnPaletteSettings& Palette);
    FLight_DawnPaletteSettings LerpPalettes(const FLight_DawnPaletteSettings& A, const FLight_DawnPaletteSettings& B, float Alpha) const;
    void FindLightReferences();
};
