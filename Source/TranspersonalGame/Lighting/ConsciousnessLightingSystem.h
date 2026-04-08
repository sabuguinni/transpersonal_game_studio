#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "ConsciousnessLightingSystem.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Ordinary        UMETA(DisplayName = "Ordinary Consciousness"),
    Meditative      UMETA(DisplayName = "Meditative State"),
    Transcendent    UMETA(DisplayName = "Transcendent Experience"),
    Shadow          UMETA(DisplayName = "Shadow Integration"),
    Unity           UMETA(DisplayName = "Unity Consciousness"),
    Void            UMETA(DisplayName = "Void State")
};

USTRUCT(BlueprintType)
struct FLightingProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DirectionalLightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DirectionalLightIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyLightColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogHeightFalloff = 0.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SunDirection = FVector(0.0f, 0.0f, -1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionDuration = 2.0f;
};

UCLASS()
class TRANSPERSONALGAME_API AConsciousnessLightingSystem : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessLightingSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UDirectionalLightComponent* DirectionalLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class UExponentialHeightFogComponent* AtmosphericFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Post Process")
    class UPostProcessComponent* PostProcessVolume;

    // Lighting Profiles for Different States
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Profiles")
    TMap<EConsciousnessState, FLightingProfile> LightingProfiles;

    // Current State Management
    UPROPERTY(BlueprintReadWrite, Category = "State")
    EConsciousnessState CurrentState = EConsciousnessState::Ordinary;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    EConsciousnessState TargetState = EConsciousnessState::Ordinary;

    // Transition Control
    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "Transition")
    float TransitionProgress = 0.0f;

    // Dynamic Lighting Parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    float PulseFrequency = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    float PulseAmplitude = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    bool bEnablePulsing = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    bool bEnableColorShifting = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dynamic Effects")
    float ColorShiftSpeed = 0.5f;

    // Public Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void TransitionToState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void SetInstantState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void EnableDynamicEffects(bool bEnable);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void SetPulseParameters(float Frequency, float Amplitude);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnStateTransitionComplete(EConsciousnessState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Events")
    void OnStateTransitionStart(EConsciousnessState FromState, EConsciousnessState ToState);

private:
    void InitializeLightingProfiles();
    void UpdateTransition(float DeltaTime);
    void ApplyLightingProfile(const FLightingProfile& Profile);
    FLightingProfile InterpolateLightingProfiles(const FLightingProfile& From, const FLightingProfile& To, float Alpha);
    void UpdateDynamicEffects(float DeltaTime);
    
    FLightingProfile StartProfile;
    FLightingProfile EndProfile;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 2.0f;
    float TimeAccumulator = 0.0f;
};