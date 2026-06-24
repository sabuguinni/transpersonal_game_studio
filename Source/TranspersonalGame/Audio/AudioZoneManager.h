#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Ambient     UMETA(DisplayName = "Ambient"),
    Danger      UMETA(DisplayName = "Danger"),
    Dialogue    UMETA(DisplayName = "Dialogue"),
    Combat      UMETA(DisplayName = "Combat"),
    Safe        UMETA(DisplayName = "Safe")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Ambient;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float VolumeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString VoiceLineURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FString LinkedQuestID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bOneShot = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    bool bLooping = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudioZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudioZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    FAudio_ZoneConfig ZoneConfig;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void DeactivateZone();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void SetVolumeMultiplier(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsPlayerInZone() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    EAudio_ZoneType GetZoneType() const;

    UFUNCTION(BlueprintCallable, Category = "Audio")
    FString GetLinkedVoiceURL() const;

private:
    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    bool bPlayerInZone = false;
    bool bHasPlayedOneShot = false;
};
