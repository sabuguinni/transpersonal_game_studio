#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Components/LightComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "ConsciousnessLighting.generated.h"

UENUM(BlueprintType)
enum class EConsciousnessLightingState : uint8
{
    Mundane         UMETA(DisplayName = "Mundane"),
    Awakening       UMETA(DisplayName = "Awakening"),
    Meditative      UMETA(DisplayName = "Meditative"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Unity           UMETA(DisplayName = "Unity"),
    Void            UMETA(DisplayName = "Void")
};

USTRUCT(BlueprintType)
struct FLightingStateData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor DirectionalColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DirectionalIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkyLightIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EtherealParticleIntensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ChromaticAberration = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Bloom = 0.675f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Saturation = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Contrast = 1.0f;
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

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void SetConsciousnessState(EConsciousnessLightingState NewState, float TransitionDuration = 2.0f);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void UpdateLightingForConsciousnessLevel(float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void TriggerEnlightenmentFlash();

    UFUNCTION(BlueprintCallable, Category = "Consciousness Lighting")
    void SetMeditationAmbience(bool bEnabled);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting States")
    TMap<EConsciousnessLightingState, FLightingStateData> LightingStates;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class ASkyLight* SkyLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    class AExponentialHeightFog* HeightFog;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    UMaterialParameterCollection* GlobalMaterialParameters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process")
    class APostProcessVolume* PostProcessVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ethereal Effects")
    TArray<class UNiagaraComponent*> EtherealParticleSystems;

private:
    EConsciousnessLightingState CurrentState;
    EConsciousnessLightingState TargetState;
    float TransitionProgress;
    float TransitionDuration;
    bool bIsTransitioning;

    FLightingStateData CurrentLightingData;
    FLightingStateData TargetLightingData;

    void InitializeLightingStates();
    void UpdateTransition(float DeltaTime);
    void ApplyLightingData(const FLightingStateData& Data);
    void UpdatePostProcessEffects(const FLightingStateData& Data);
    void UpdateEtherealParticles(float Intensity);
    FLightingStateData InterpolateLightingData(const FLightingStateData& From, const FLightingStateData& To, float Alpha);
};