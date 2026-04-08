#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Components/LightComponent.h"
#include "ConsciousnessLighting.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessState : uint8
{
    Normal          UMETA(DisplayName = "Normal Consciousness"),
    Meditative      UMETA(DisplayName = "Meditative State"),
    Transcendent    UMETA(DisplayName = "Transcendent State"),
    Unity           UMETA(DisplayName = "Unity Consciousness"),
    Void            UMETA(DisplayName = "Void State"),
    Cosmic          UMETA(DisplayName = "Cosmic Awareness")
};

USTRUCT(BlueprintType)
struct FLightingProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DirectionalColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DirectionalIntensity = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator SunAngle = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumetricFogIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransitionDuration = 2.0f;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UConsciousnessLighting : public UActorComponent
{
    GENERATED_BODY()

public:
    UConsciousnessLighting();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Lighting Profiles for different consciousness states
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Lighting")
    TMap<EConsciousnessState, FLightingProfile> LightingProfiles;

    // Current consciousness state
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Lighting")
    EConsciousnessState CurrentState = EConsciousnessState::Normal;

    // Transition settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Lighting")
    bool bIsTransitioning = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness Lighting")
    float TransitionProgress = 0.0f;

    // Light references
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TArray<APointLight*> AmbientLights;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light References")
    TArray<ASpotLight*> AccentLights;

    // Atmospheric effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Atmosphere")
    class AVolumetricFog* VolumetricFog;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void TransitionToState(EConsciousnessState NewState);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void SetLightingProfile(EConsciousnessState State, const FLightingProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    FLightingProfile GetCurrentLightingProfile() const;

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void UpdateLighting();

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void InitializeLightReferences();

private:
    void UpdateTransition(float DeltaTime);
    void ApplyLightingProfile(const FLightingProfile& Profile);
    void InterpolateLighting(const FLightingProfile& From, const FLightingProfile& To, float Alpha);

    FLightingProfile PreviousProfile;
    FLightingProfile TargetProfile;
};