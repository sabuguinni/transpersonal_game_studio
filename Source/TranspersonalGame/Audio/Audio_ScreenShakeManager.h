#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light      UMETA(DisplayName = "Light"),
    Medium     UMETA(DisplayName = "Medium"),
    Heavy      UMETA(DisplayName = "Heavy"),
    Massive    UMETA(DisplayName = "Massive")
};

USTRUCT(BlueprintType)
struct FAudio_ShakeSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake")
    float MaxDistance = 2000.0f;

    FAudio_ShakeSettings()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        MaxDistance = 2000.0f;
    }
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

    // Trigger screen shake based on distance and intensity
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerScreenShake(FVector SourceLocation, EAudio_ShakeIntensity Intensity);

    // Trigger custom screen shake
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(FVector SourceLocation, FAudio_ShakeSettings ShakeSettings);

    // Check if player is within shake range
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    bool IsPlayerInRange(FVector SourceLocation, float MaxDistance);

protected:
    // Shake settings for different intensities
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeSettings> ShakePresets;

    // Cache player controller
    UPROPERTY()
    APlayerController* CachedPlayerController;

    // Initialize default shake presets
    void InitializeShakePresets();

    // Calculate shake intensity based on distance
    float CalculateDistanceAttenuation(FVector SourceLocation, float MaxDistance);
};