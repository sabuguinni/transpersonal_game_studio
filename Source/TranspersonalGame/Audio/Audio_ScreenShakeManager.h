#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ShakeIntensity : uint8
{
    Light       UMETA(DisplayName = "Light"),
    Medium      UMETA(DisplayName = "Medium"),
    Heavy       UMETA(DisplayName = "Heavy"),
    Extreme     UMETA(DisplayName = "Extreme")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Duration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Amplitude = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    float FalloffRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profile")
    bool bUseDistanceAttenuation = true;

    FAudio_ShakeProfile()
    {
        Duration = 1.0f;
        Amplitude = 1.0f;
        Frequency = 10.0f;
        FalloffRadius = 5000.0f;
        bUseDistanceAttenuation = true;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shake Profiles")
    TMap<EAudio_ShakeIntensity, FAudio_ShakeProfile> ShakeProfiles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float GlobalShakeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    bool bEnableScreenShake = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxShakeDistance = 10000.0f;

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerShake(EAudio_ShakeIntensity Intensity, FVector SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(const FAudio_ShakeProfile& Profile, FVector SourceLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDinosaurFootstep(const FString& DinosaurType, FVector FootstepLocation);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void SetGlobalShakeMultiplier(float NewMultiplier);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void EnableScreenShake(bool bEnable);

private:
    void InitializeShakeProfiles();
    float CalculateDistanceAttenuation(FVector SourceLocation, FVector PlayerLocation, float FalloffRadius);
    APlayerController* GetPlayerController();
};