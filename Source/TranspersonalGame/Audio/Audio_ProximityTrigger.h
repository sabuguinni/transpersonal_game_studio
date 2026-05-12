#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Audio_ProximityTrigger.generated.h"

UENUM(BlueprintType)
enum class EAudio_TriggerType : uint8
{
    Predator_Approach UMETA(DisplayName = "Predator Approach"),
    Environmental_Hazard UMETA(DisplayName = "Environmental Hazard"),
    Ambient_Zone UMETA(DisplayName = "Ambient Zone"),
    Survival_Alert UMETA(DisplayName = "Survival Alert")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ProximitySettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Proximity")
    float TriggerRadius = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Proximity")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Proximity")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Proximity")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Proximity")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Proximity")
    EAudio_TriggerType TriggerType = EAudio_TriggerType::Ambient_Zone;

    FAudio_ProximitySettings()
    {
        TriggerRadius = 1000.0f;
        VolumeMultiplier = 1.0f;
        FadeInTime = 2.0f;
        FadeOutTime = 3.0f;
        bLooping = true;
        TriggerType = EAudio_TriggerType::Ambient_Zone;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ProximityTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ProximityTrigger();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FAudio_ProximitySettings ProximitySettings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    class USoundBase* AudioClip;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Settings")
    FString TriggerName = "AudioTrigger";

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bPlayerInRange = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float CurrentVolume = 0.0f;

    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                              UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                            UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void StartAudioFadeIn();

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void StartAudioFadeOut();

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetTriggerRadius(float NewRadius);

    UFUNCTION(BlueprintCallable, Category = "Audio Control")
    void SetAudioClip(USoundBase* NewClip);

    UFUNCTION(BlueprintImplementableEvent, Category = "Audio Events")
    void OnPlayerEnterTrigger();

    UFUNCTION(BlueprintImplementableEvent, Category = "Audio Events")
    void OnPlayerExitTrigger();

public:
    virtual void Tick(float DeltaTime) override;

private:
    float FadeTimer = 0.0f;
    bool bFadingIn = false;
    bool bFadingOut = false;
    float TargetVolume = 0.0f;
    float StartVolume = 0.0f;
};

#include "Audio_ProximityTrigger.generated.h"