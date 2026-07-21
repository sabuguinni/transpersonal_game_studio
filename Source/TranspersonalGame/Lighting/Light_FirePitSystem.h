#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Light_FirePitSystem.generated.h"

UENUM(BlueprintType)
enum class ELight_FireIntensity : uint8
{
    Embers      UMETA(DisplayName = "Embers"),
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Blazing     UMETA(DisplayName = "Blazing")
};

USTRUCT(BlueprintType)
struct FLight_FireSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float BaseIntensity = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float FlickerSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float FlickerAmount = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    FColor FireColor = FColor(255, 120, 60, 255);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float Temperature = 2700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float AttenuationRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    float VolumetricScatteringIntensity = 2.0f;

    FLight_FireSettings()
    {
        // Default fire pit settings
    }
};

/**
 * Fire pit lighting system with flickering flames, smoke effects, and ambient audio
 * Provides realistic fire illumination for Cretaceous period camps
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ALight_FirePitSystem : public AActor
{
    GENERATED_BODY()

public:
    ALight_FirePitSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    ELight_FireIntensity FireIntensity = ELight_FireIntensity::Medium;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    FLight_FireSettings FireSettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    bool bIsLit = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    bool bEnableFlicker = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    bool bEnableSmoke = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire")
    bool bEnableAudio = true;

    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FirePitMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* FireLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* FlameParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* SmokeParticles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* FireAudio;

    // Assets
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UStaticMesh* FirePitMeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UParticleSystem* FlameParticleAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class UParticleSystem* SmokeParticleAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    class USoundCue* FireSoundCue;

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Fire")
    void LightFire();

    UFUNCTION(BlueprintCallable, Category = "Fire")
    void ExtinguishFire();

    UFUNCTION(BlueprintCallable, Category = "Fire")
    void SetFireIntensity(ELight_FireIntensity NewIntensity);

    UFUNCTION(BlueprintCallable, Category = "Fire")
    void UpdateFireSettings(const FLight_FireSettings& NewSettings);

    UFUNCTION(BlueprintCallable, Category = "Fire")
    bool IsFireLit() const { return bIsLit; }

    UFUNCTION(BlueprintCallable, Category = "Fire")
    float GetCurrentLightIntensity() const;

private:
    float FlickerTimer = 0.0f;
    float BaseFlickerIntensity = 1.0f;
    float CurrentFlickerMultiplier = 1.0f;

    void UpdateFlicker(float DeltaTime);
    void UpdateFireEffects();
    void InitializeComponents();
    float GetIntensityMultiplierForLevel(ELight_FireIntensity Intensity) const;
};