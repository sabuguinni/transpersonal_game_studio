#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    bool bFadeIn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    bool bFadeOut = true;

    FAudio_ScreenShakeSettings()
    {
        Intensity = 1.0f;
        Duration = 0.5f;
        Frequency = 10.0f;
        bFadeIn = true;
        bFadeOut = true;
    }
};

UENUM(BlueprintType)
enum class EAudio_ShakeType : uint8
{
    TRexFootstep UMETA(DisplayName = "T-Rex Footstep"),
    DinosaurImpact UMETA(DisplayName = "Dinosaur Impact"),
    GroundPound UMETA(DisplayName = "Ground Pound"),
    Earthquake UMETA(DisplayName = "Earthquake"),
    Explosion UMETA(DisplayName = "Explosion")
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Trigger screen shake based on predefined types
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerScreenShake(EAudio_ShakeType ShakeType, float IntensityMultiplier = 1.0f);

    // Trigger custom screen shake
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomScreenShake(const FAudio_ScreenShakeSettings& ShakeSettings);

    // Trigger screen shake based on distance from source
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerProximityScreenShake(FVector SourceLocation, float MaxDistance, EAudio_ShakeType ShakeType);

    // Check if player is within shake range
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    bool IsPlayerInShakeRange(FVector SourceLocation, float MaxDistance) const;

protected:
    // Predefined shake settings for different types
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    TMap<EAudio_ShakeType, FAudio_ScreenShakeSettings> ShakeTypeSettings;

    // Reference to player controller
    UPROPERTY()
    APlayerController* PlayerController;

    // Camera shake class to use
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Settings")
    TSubclassOf<UCameraShakeBase> CameraShakeClass;

private:
    void InitializeShakeSettings();
    void ExecuteScreenShake(const FAudio_ScreenShakeSettings& Settings, float IntensityMultiplier = 1.0f);
    float CalculateDistanceBasedIntensity(FVector SourceLocation, float MaxDistance) const;
};