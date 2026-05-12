#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeController.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Shake"),
    Medium      UMETA(DisplayName = "Medium Shake"),
    Heavy       UMETA(DisplayName = "Heavy Shake"),
    Massive     UMETA(DisplayName = "Massive Shake")
};

USTRUCT(BlueprintType)
struct FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float FalloffDistance = 2000.0f;

    FAudio_ShakeSettings()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        FalloffDistance = 2000.0f;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeController : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeController();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main shake trigger functions
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerShakeAtLocation(FVector Location, EAudio_ShakeIntensity Intensity);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDinosaurFootstepShake(FVector FootstepLocation, float DinosaurMass);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDamageShake(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeSettings& Settings, FVector Location);

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Presets")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeSettings> ShakePresets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float MaxShakeDistance = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bEnableScreenShake = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float ShakeMultiplier = 1.0f;

protected:
    // Internal shake execution
    void ExecuteShake(const FAudio_ShakeSettings& Settings, FVector Location);
    
    // Distance-based intensity calculation
    float CalculateShakeIntensity(FVector ShakeLocation, float BaseIntensity) const;
    
    // Get player camera for shake application
    class UCameraComponent* GetPlayerCamera() const;
    
    // Preset initialization
    void InitializeShakePresets();

private:
    // Cached references
    UPROPERTY()
    class APlayerController* CachedPlayerController;

    UPROPERTY()
    class UCameraComponent* CachedPlayerCamera;

    // Shake tracking
    float LastShakeTime;
    float ShakeCooldown = 0.1f;
};