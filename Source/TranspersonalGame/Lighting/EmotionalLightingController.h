#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "EmotionalLightingController.generated.h"

UENUM(BlueprintType)
enum class EPredatorProximity : uint8
{
    Safe        UMETA(DisplayName = "Safe - No predators detected"),
    Distant     UMETA(DisplayName = "Distant - Predator far away"),
    Near        UMETA(DisplayName = "Near - Predator approaching"),
    Stalking    UMETA(DisplayName = "Stalking - Being hunted"),
    Imminent    UMETA(DisplayName = "Imminent - Attack incoming")
};

UENUM(BlueprintType)
enum class EEmotionalState : uint8
{
    Wonder      UMETA(DisplayName = "Wonder - Discovery and awe"),
    Isolation   UMETA(DisplayName = "Isolation - Alone in vastness"),
    Tension     UMETA(DisplayName = "Tension - Something is wrong"),
    Fear        UMETA(DisplayName = "Fear - Danger is near"),
    Terror      UMETA(DisplayName = "Terror - Life or death"),
    Relief      UMETA(DisplayName = "Relief - Momentary safety"),
    Despair     UMETA(DisplayName = "Despair - Hope is fading")
};

USTRUCT(BlueprintType)
struct FEmotionalLightingProfile
{
    GENERATED_BODY()

    // === CORE EMOTIONAL PARAMETERS ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    FString ProfileName = "Default";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float TensionLevel = 0.0f; // 0.0 = calm, 1.0 = maximum tension
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float IsolationFactor = 0.0f; // How alone the player feels
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotion")
    float ThreatImminence = 0.0f; // How immediate the danger feels

    // === LIGHTING MODIFICATIONS ===
    
    // Shadow intensity and contrast
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowIntensityMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowContrastBoost = 0.0f; // Additional contrast for harsh shadows
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shadows")
    float ShadowTemperatureShift = 0.0f; // -1.0 = cooler, +1.0 = warmer
    
    // Visibility and fog
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
    float VisibilityReduction = 0.0f; // 0.0 = no change, 1.0 = heavy fog
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
    float FogDensityMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visibility")
    FLinearColor FogTintShift = FLinearColor::White;
    
    // Color grading and mood
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    float SaturationMultiplier = 1.0f; // Desaturate for bleakness
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    float ContrastMultiplier = 1.0f; // High contrast for tension
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    FLinearColor ColorTint = FLinearColor::White; // Overall color cast
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Color")
    float TemperatureShift = 0.0f; // -1.0 = cold blue, +1.0 = warm orange
    
    // Light intensity modifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intensity")
    float SunIntensityMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intensity")
    float SkyLightMultiplier = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Intensity")
    float AmbientOcclusionStrength = 1.0f; // Darken crevices for claustrophobia

    // === DYNAMIC EFFECTS ===
    
    // Flickering and instability
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    float LightFlickerIntensity = 0.0f; // Subtle light variations
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    float LightFlickerSpeed = 1.0f;
    
    // Pulsing effects for heartbeat/tension
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    float TensionPulseIntensity = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic")
    float TensionPulseSpeed = 1.0f; // Heartbeat rhythm
    
    // Transition timing
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Timing")
    float TransitionSpeed = 1.0f; // How fast to blend to this profile
};

/**
 * Specialized controller for emotional lighting that responds to gameplay states
 * Designed specifically for the "constant fear" theme of the Jurassic survival game
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(Lighting), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEmotionalLightingController : public UActorComponent
{
    GENERATED_BODY()

public:    
    UEmotionalLightingController();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // ========== CORE STATE MANAGEMENT ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    EEmotionalState CurrentEmotionalState = EEmotionalState::Wonder;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    EPredatorProximity PredatorProximity = EPredatorProximity::Safe;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float BaseTensionLevel = 0.3f; // Always some tension in this world
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float CurrentTensionLevel = 0.3f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotional State")
    float MaxTensionLevel = 1.0f;

    // ========== LIGHTING PROFILES ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Profiles")
    TMap<EEmotionalState, FEmotionalLightingProfile> EmotionalProfiles;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Profiles")
    TMap<EPredatorProximity, FEmotionalLightingProfile> PredatorProfiles;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Profiles")
    FEmotionalLightingProfile CurrentProfile;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting Profiles")
    FEmotionalLightingProfile TargetProfile;

    // ========== COMPONENT REFERENCES ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    TObjectPtr<ADirectionalLight> SunLight;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    TObjectPtr<ASkyLight> SkyLight;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    TObjectPtr<AExponentialHeightFog> HeightFog;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    TObjectPtr<APostProcessVolume> PostProcessVolume;

    // ========== DYNAMIC EFFECTS SETTINGS ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    bool bEnableLightFlicker = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    bool bEnableTensionPulse = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    float FlickerNoiseScale = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    float PulseNoiseScale = 1.0f;

    // ========== PERFORMANCE SETTINGS ==========
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float UpdateFrequency = 10.0f; // Updates per second
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    bool bUseDistanceBasedUpdates = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
    float MaxUpdateDistance = 10000.0f; // cm

    // ========== PUBLIC INTERFACE ==========
    
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void SetEmotionalState(EEmotionalState NewState, float TransitionTime = 2.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void SetPredatorProximity(EPredatorProximity Proximity, float TransitionTime = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void ModifyTensionLevel(float TensionDelta, float TransitionTime = 1.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void TriggerTensionSpike(float Intensity = 1.0f, float Duration = 3.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void ApplyEmotionalProfile(const FEmotionalLightingProfile& Profile, float BlendWeight = 1.0f);
    
    UFUNCTION(BlueprintPure, Category = "Emotional Lighting")
    float GetCurrentTensionLevel() const { return CurrentTensionLevel; }
    
    UFUNCTION(BlueprintPure, Category = "Emotional Lighting")
    EEmotionalState GetCurrentEmotionalState() const { return CurrentEmotionalState; }
    
    UFUNCTION(BlueprintPure, Category = "Emotional Lighting")
    float GetVisibilityFactor() const; // For AI systems to query
    
    UFUNCTION(BlueprintCallable, Category = "Emotional Lighting")
    void ResetToBaseline(float TransitionTime = 5.0f);

    // ========== EVENT DISPATCHERS ==========
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEmotionalStateChanged, EEmotionalState, NewState, float, TensionLevel);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEmotionalStateChanged OnEmotionalStateChanged;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTensionLevelChanged, float, NewTensionLevel);
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTensionLevelChanged OnTensionLevelChanged;

private:
    // ========== INTERNAL STATE ==========
    
    float ProfileBlendAlpha = 0.0f;
    float TransitionSpeed = 1.0f;
    float LastUpdateTime = 0.0f;
    bool bIsTransitioning = false;
    
    // Dynamic effects state
    float FlickerTime = 0.0f;
    float PulseTime = 0.0f;
    float TensionSpikeTimer = 0.0f;
    float TensionSpikeIntensity = 0.0f;
    float TensionSpikeBaseline = 0.0f;
    
    // Cached base values for restoration
    float BaseSunIntensity = 10.0f;
    float BaseSkyIntensity = 1.0f;
    float BaseFogDensity = 0.02f;
    FLinearColor BaseFogColor = FLinearColor::White;

    // ========== INTERNAL FUNCTIONS ==========
    
    void InitializeProfiles();
    void FindLightingComponents();
    void UpdateEmotionalLighting(float DeltaTime);
    void BlendToTargetProfile(float DeltaTime);
    void ApplyCurrentProfile();
    void ApplyDynamicEffects(float DeltaTime);
    void UpdateTensionSpike(float DeltaTime);
    void CacheBaseLightingValues();
    
    // Profile blending
    FEmotionalLightingProfile BlendProfiles(const FEmotionalLightingProfile& A, const FEmotionalLightingProfile& B, float Alpha) const;
    FEmotionalLightingProfile CalculateTargetProfile() const;
    
    // Component application
    void ApplyToSunLight(const FEmotionalLightingProfile& Profile);
    void ApplyToSkyLight(const FEmotionalLightingProfile& Profile);
    void ApplyToHeightFog(const FEmotionalLightingProfile& Profile);
    void ApplyToPostProcess(const FEmotionalLightingProfile& Profile);
    
    // Utility functions
    float CalculateFlickerValue(float Time, float Intensity, float Speed) const;
    float CalculatePulseValue(float Time, float Intensity, float Speed) const;
    FLinearColor ShiftTemperature(const FLinearColor& BaseColor, float TemperatureShift) const;
    bool ShouldUpdate() const;
};