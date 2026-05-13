#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Audio_ScreenShakeSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light Shake"),
    Medium      UMETA(DisplayName = "Medium Shake"),
    Heavy       UMETA(DisplayName = "Heavy Shake"),
    Extreme     UMETA(DisplayName = "Extreme Shake")
};

USTRUCT(BlueprintType)
struct FAudio_ShakeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Amplitude = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float FalloffDistance = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    bool bUseDistanceFalloff = true;

    FAudio_ShakeProfile()
    {
        Duration = 1.0f;
        Amplitude = 5.0f;
        Frequency = 10.0f;
        FalloffDistance = 2000.0f;
        bUseDistanceFalloff = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeSystem();

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerShake(EAudio_ShakeIntensity Intensity, FVector ShakeOrigin);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeProfile& ShakeProfile, FVector ShakeOrigin);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDinosaurFootstep(float DinosaurMass, FVector FootstepLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void StopAllShakes();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profiles")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeProfile> ShakeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Settings")
    float MinDinosaurMassForShake = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dinosaur Settings")
    float MaxShakeDistance = 3000.0f;

private:
    void InitializeShakeProfiles();
    float CalculateShakeIntensity(float Distance, float MaxDistance, float BaseMagnitude);
    
    UPROPERTY()
    APlayerController* CachedPlayerController;
};