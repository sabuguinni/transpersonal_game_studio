#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    None            UMETA(DisplayName = "None"),
    RaptorTension   UMETA(DisplayName = "Raptor Tension"),
    HerdAmbient     UMETA(DisplayName = "Herd Ambient"),
    ScoutStinger    UMETA(DisplayName = "Scout Stinger"),
    CampfireLoop    UMETA(DisplayName = "Campfire Loop"),
    DangerAlert     UMETA(DisplayName = "Danger Alert"),
    SafeZone        UMETA(DisplayName = "Safe Zone")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float TriggerRadius = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutDuration = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bOneShot = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float CooldownSeconds = 30.0f;
};

/**
 * AAudio_ZoneActor — proximity-based audio zone that triggers ambient sounds,
 * tension stingers, or looping ambience when the player enters its radius.
 * Wires directly to ANarr_DialogueTrigger zones placed by Agent #15.
 */
UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AAudio_ZoneActor : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneActor();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone",
              meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone",
              meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioComponent;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetZoneType(EAudio_ZoneType NewType);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    bool IsPlayerInside() const { return bPlayerInside; }

    UFUNCTION(BlueprintNativeEvent, Category = "Audio Zone")
    void OnPlayerEnterZone(EAudio_ZoneType ZoneType);

    UFUNCTION(BlueprintNativeEvent, Category = "Audio Zone")
    void OnPlayerExitZone(EAudio_ZoneType ZoneType);

protected:
    UFUNCTION()
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                            bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                          UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void FadeInAudio();
    void FadeOutAudio();
    void ResetCooldown();

private:
    bool bPlayerInside = false;
    bool bOnCooldown = false;
    float CurrentVolume = 0.0f;

    FTimerHandle FadeInHandle;
    FTimerHandle FadeOutHandle;
    FTimerHandle CooldownHandle;
};
