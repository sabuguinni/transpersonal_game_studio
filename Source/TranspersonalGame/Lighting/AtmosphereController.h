// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/PostProcessComponent.h"
#include "Engine/PostProcessVolume.h"
#include "../Core/ConsciousnessSystem.h"
#include "AtmosphereController.generated.h"

USTRUCT(BlueprintType)
struct FAtmosphereSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkyColor = FLinearColor(0.2f, 0.6f, 1.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SunColor = FLinearColor(1.0f, 0.9f, 0.7f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SunIntensity = 3.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SkyIntensity = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor FogColor = FLinearColor(0.5f, 0.7f, 1.0f);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAtmosphereController : public AActor
{
    GENERATED_BODY()

public:
    AAtmosphereController();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // Lighting Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class UDirectionalLightComponent* SunLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
    class USkyLightComponent* SkyLight;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Post Process")
    class UPostProcessComponent* PostProcessComponent;

    // Atmosphere Settings for Different Consciousness States
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FAtmosphereSettings HighConsciousnessSettings;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FAtmosphereSettings NormalConsciousnessSettings;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    FAtmosphereSettings LowConsciousnessSettings;

    // Transition Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float TransitionSpeed = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    bool bSmoothTransitions = true;

    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentConsciousnessLevel = 0.5f;
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FAtmosphereSettings CurrentSettings;
    
    UPROPERTY(BlueprintReadOnly, Category = "State")
    FAtmosphereSettings TargetSettings;

    // Consciousness System Reference
    UPROPERTY()
    class UConsciousnessSystem* ConsciousnessSystem;

public:
    // Public Interface
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void UpdateAtmosphere(float ConsciousnessLevel);
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    void SetAtmosphereSettings(const FAtmosphereSettings& NewSettings, bool bInstant = false);
    
    UFUNCTION(BlueprintCallable, Category = "Atmosphere")
    FAtmosphereSettings GetCurrentAtmosphereSettings() const { return CurrentSettings; }
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Atmosphere")
    void OnAtmosphereChanged(float ConsciousnessLevel);

protected:
    // Internal Methods
    void InitializeComponents();
    void UpdateLightingComponents();
    void InterpolateSettings(float DeltaTime);
    FAtmosphereSettings GetSettingsForConsciousnessLevel(float Level);
    
    UFUNCTION()
    void OnConsciousnessChanged(float NewLevel, float PreviousLevel);
};