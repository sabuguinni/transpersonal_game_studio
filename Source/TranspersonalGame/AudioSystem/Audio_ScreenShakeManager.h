#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraComponent.h"
#include "Audio_ScreenShakeManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light UMETA(DisplayName = "Light Shake"),
    Medium UMETA(DisplayName = "Medium Shake"),
    Heavy UMETA(DisplayName = "Heavy Shake"),
    Extreme UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bFadeIn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bFadeOut = true;

    FAudio_ShakeData()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        bFadeIn = true;
        bFadeOut = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ScreenShakeManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ScreenShakeManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    // Shake presets for different dinosaur types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Shakes")
    FAudio_ShakeData TRexFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Shakes")
    FAudio_ShakeData BrachiosaurusFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Shakes")
    FAudio_ShakeData RaptorLandingShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Shakes")
    FAudio_ShakeData PlayerDamageShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Shakes")
    FAudio_ShakeData SeismicRumbleShake;

public:
    // Main shake trigger functions
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDinosaurFootstep(const FString& DinosaurType, const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerPlayerDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerSeismicEvent(float Intensity, float Duration);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeData& ShakeData);

    // Distance-based shake calculation
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    float CalculateShakeIntensityByDistance(const FVector& SourceLocation, float MaxDistance = 2000.0f);

    // Get player camera for shake application
    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    class UCameraComponent* GetPlayerCamera();

    // Static instance for global access
    UFUNCTION(BlueprintCallable, Category = "Screen Shake", CallInEditor = true)
    static AAudio_ScreenShakeManager* GetInstance();

private:
    void ApplyShakeToCamera(const FAudio_ShakeData& ShakeData, float IntensityMultiplier = 1.0f);
    
    static AAudio_ScreenShakeManager* Instance;
};