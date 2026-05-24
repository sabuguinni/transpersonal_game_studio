#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Audio_ScreenShakeManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light      UMETA(DisplayName = "Light Shake"),
    Medium     UMETA(DisplayName = "Medium Shake"),
    Heavy      UMETA(DisplayName = "Heavy Shake"),
    Massive    UMETA(DisplayName = "Massive Shake")
};

UENUM(BlueprintType)
enum class EAudio_ShakeType : uint8
{
    Footstep   UMETA(DisplayName = "Footstep"),
    Impact     UMETA(DisplayName = "Impact"),
    Explosion  UMETA(DisplayName = "Explosion"),
    Earthquake UMETA(DisplayName = "Earthquake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float FadeInTime = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float FadeOutTime = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    bool bRandomizeAmplitude = true;

    FAudio_ShakeSettings()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        FadeInTime = 0.1f;
        FadeOutTime = 0.5f;
        bRandomizeAmplitude = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UAudio_ScreenShakeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Screen shake triggering functions
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerShake(EAudio_ShakeIntensity Intensity, EAudio_ShakeType ShakeType, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeSettings& Settings, FVector Location = FVector::ZeroVector);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerDinosaurFootstep(float DinosaurMass, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerDamageFlash(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StopAllShakes();

    // Configuration functions
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void SetShakeEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void SetShakeIntensityMultiplier(float Multiplier);

protected:
    // Shake settings for different intensities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeSettings> ShakePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    float MaxShakeDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    bool bShakeEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    float ShakeIntensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    float DamageFlashDuration = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    FLinearColor DamageFlashColor = FLinearColor::Red;

private:
    // Internal helper functions
    void ApplyShakeToPlayer(const FAudio_ShakeSettings& Settings, FVector Location);
    float CalculateDistanceAttenuation(FVector ShakeLocation, FVector PlayerLocation);
    FAudio_ShakeSettings GetShakeSettingsForIntensity(EAudio_ShakeIntensity Intensity);
    void InitializeShakePresets();
    
    // Damage flash effect
    void StartDamageFlash(float Duration, FLinearColor FlashColor);
    void UpdateDamageFlash();
    void EndDamageFlash();

    // Timer handles
    FTimerHandle DamageFlashTimerHandle;
    
    // Current flash state
    bool bDamageFlashActive = false;
    float DamageFlashStartTime = 0.0f;
    float CurrentDamageFlashDuration = 0.0f;
};