#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Perf_AdaptiveQualityController.generated.h"

UENUM(BlueprintType)
enum class EPerf_QualityLevel : uint8
{
    Ultra       UMETA(DisplayName = "Ultra"),
    High        UMETA(DisplayName = "High"), 
    Medium      UMETA(DisplayName = "Medium"),
    Low         UMETA(DisplayName = "Low"),
    Potato      UMETA(DisplayName = "Potato")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_QualitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 ShadowQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 TextureQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 EffectsQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 PostProcessQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    int32 AntiAliasingQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    float ViewDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    float FoliageDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bEnableLumen;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bEnableNanite;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quality")
    bool bEnableVSM;

    FPerf_QualitySettings()
    {
        ShadowQuality = 3;
        TextureQuality = 3;
        EffectsQuality = 3;
        PostProcessQuality = 3;
        AntiAliasingQuality = 3;
        ViewDistance = 1.0f;
        FoliageDistance = 1.0f;
        bEnableLumen = true;
        bEnableNanite = true;
        bEnableVSM = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerf_AdaptiveSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    float TargetFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    float MinFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    float MaxFPS;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    float AdjustmentThreshold;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    float StabilityTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    bool bEnableAdaptiveQuality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Adaptive")
    bool bEnableAggressiveOptimization;

    FPerf_AdaptiveSettings()
    {
        TargetFPS = 60.0f;
        MinFPS = 30.0f;
        MaxFPS = 120.0f;
        AdjustmentThreshold = 5.0f;
        StabilityTime = 3.0f;
        bEnableAdaptiveQuality = true;
        bEnableAggressiveOptimization = false;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UPerf_AdaptiveQualityController : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UPerf_AdaptiveQualityController();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Quality management
    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void SetQualityLevel(EPerf_QualityLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    EPerf_QualityLevel GetCurrentQualityLevel() const { return CurrentQualityLevel; }

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    FPerf_QualitySettings GetQualitySettings(EPerf_QualityLevel Level);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void ApplyQualitySettings(const FPerf_QualitySettings& Settings);

    // Adaptive quality control
    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void EnableAdaptiveQuality(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void SetAdaptiveSettings(const FPerf_AdaptiveSettings& Settings);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    FPerf_AdaptiveSettings GetAdaptiveSettings() const { return AdaptiveSettings; }

    // Performance monitoring
    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void UpdatePerformanceMetrics(float CurrentFPS, float FrameTime);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    bool ShouldAdjustQuality();

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void ForceQualityAdjustment();

    // Quality presets
    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void LoadQualityPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void SaveQualityPreset(const FString& PresetName);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    TArray<FString> GetAvailablePresets();

    // Console variable management
    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void SetConsoleVariable(const FString& VariableName, float Value);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void SetConsoleVariable(const FString& VariableName, int32 Value);

    UFUNCTION(BlueprintCallable, Category = "Adaptive Quality")
    void SetConsoleVariable(const FString& VariableName, const FString& Value);

protected:
    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "Quality", meta = (AllowPrivateAccess = "true"))
    EPerf_QualityLevel CurrentQualityLevel;

    UPROPERTY(BlueprintReadOnly, Category = "Quality", meta = (AllowPrivateAccess = "true"))
    FPerf_AdaptiveSettings AdaptiveSettings;

    UPROPERTY(BlueprintReadOnly, Category = "Quality", meta = (AllowPrivateAccess = "true"))
    TArray<float> FPSHistory;

    UPROPERTY(BlueprintReadOnly, Category = "Quality", meta = (AllowPrivateAccess = "true"))
    float LastAdjustmentTime;

    UPROPERTY(BlueprintReadOnly, Category = "Quality", meta = (AllowPrivateAccess = "true"))
    bool bIsStable;

    // Quality presets
    UPROPERTY(BlueprintReadOnly, Category = "Quality", meta = (AllowPrivateAccess = "true"))
    TMap<EPerf_QualityLevel, FPerf_QualitySettings> QualityPresets;

private:
    void InitializeQualityPresets();
    void ApplyEngineSettings(const FPerf_QualitySettings& Settings);
    float CalculateAverageFPS();
    bool IsPerformanceStable();
    void AdjustQualityBasedOnPerformance();
    EPerf_QualityLevel GetNextLowerQuality(EPerf_QualityLevel Current);
    EPerf_QualityLevel GetNextHigherQuality(EPerf_QualityLevel Current);

    FTimerHandle AdaptiveQualityTimer;
};