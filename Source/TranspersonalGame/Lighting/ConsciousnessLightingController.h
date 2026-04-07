#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Engine/PostProcessVolume.h"
#include "ConsciousnessLightingController.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Ordinary        UMETA(DisplayName = "Ordinary"),
    Focused         UMETA(DisplayName = "Focused"),
    Meditative      UMETA(DisplayName = "Meditative"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Unity           UMETA(DisplayName = "Unity")
};

USTRUCT(BlueprintType)
struct FLightingProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyColor = FLinearColor(0.2f, 0.4f, 0.8f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkyIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.5f, 0.6f, 0.7f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float PostProcessIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionDuration = 2.0f;
};

UCLASS()
class TRANSPERSONALGAME_API AConsciousnessLightingController : public AActor
{
    GENERATED_BODY()

public:
    AConsciousnessLightingController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UDirectionalLightComponent* SunLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyLightComponent* SkyLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Atmosphere")
    class UExponentialHeightFogComponent* AtmosphericFog;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "PostProcess")
    class UPostProcessComponent* PostProcessComponent;

    // Lighting Profiles for each consciousness state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Lighting")
    TMap<EConsciousnessState, FLightingProfile> LightingProfiles;

    // Current state and transition
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessState CurrentState = EConsciousnessState::Ordinary;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessState TargetState = EConsciousnessState::Ordinary;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TransitionProgress = 0.0f;

    // Methods
    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void SetConsciousnessState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void SetLightingProfile(EConsciousnessState State, const FLightingProfile& Profile);

    UFUNCTION(BlueprintPure, Category = "Consciousness Lighting")
    FLightingProfile GetCurrentLightingProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void InitializeDefaultProfiles();

private:
    void UpdateLighting(float DeltaTime);
    void ApplyLightingProfile(const FLightingProfile& Profile);
    FLightingProfile InterpolateLightingProfiles(const FLightingProfile& From, const FLightingProfile& To, float Alpha);
    
    FLightingProfile StartProfile;
    FLightingProfile TargetProfile;
    float TransitionTimer = 0.0f;
    float TransitionDuration = 2.0f;
};