#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Audio_ScreenShakeController.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light    UMETA(DisplayName = "Light"),
    Medium   UMETA(DisplayName = "Medium"),
    Heavy    UMETA(DisplayName = "Heavy"),
    Extreme  UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakePattern
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Pattern")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Pattern")
    float Magnitude;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Pattern")
    float Frequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Pattern")
    bool bFadeOut;

    FAudio_ShakePattern()
    {
        Duration = 1.0f;
        Magnitude = 5.0f;
        Frequency = 10.0f;
        bFadeOut = true;
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

    // Trigger screen shake based on distance and intensity
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerShakeFromDistance(FVector SourceLocation, float MaxDistance, EAudio_ShakeIntensity Intensity);

    // Trigger screen shake with custom pattern
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerCustomShake(const FAudio_ShakePattern& Pattern);

    // T-Rex specific shake (heavy footsteps)
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void TriggerTRexFootstep(FVector TRexLocation);

    // Stop all active shakes
    UFUNCTION(BlueprintCallable, Category = "Audio Screen Shake")
    void StopAllShakes();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    TMap<EAudio_ShakeIntensity, FAudio_ShakePattern> ShakePatterns;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    float MaxShakeDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Settings")
    bool bEnableShake;

private:
    void InitializeDefaultPatterns();
    void ApplyShakeToCamera(const FAudio_ShakePattern& Pattern, float DistanceFactor);
    float CalculateDistanceFactor(FVector SourceLocation, float MaxDistance);
    
    // Active shake tracking
    bool bIsShaking;
    float CurrentShakeTime;
    FAudio_ShakePattern CurrentPattern;
    float CurrentDistanceFactor;
    
    // Camera reference
    UPROPERTY()
    class UCameraComponent* PlayerCamera;
    
    UPROPERTY()
    class APlayerController* PlayerController;
};