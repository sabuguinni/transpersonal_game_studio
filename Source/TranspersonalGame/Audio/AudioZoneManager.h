#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Jungle_Day      UMETA(DisplayName = "Jungle Day"),
    Jungle_Night    UMETA(DisplayName = "Jungle Night"),
    Campfire        UMETA(DisplayName = "Campfire"),
    River           UMETA(DisplayName = "River"),
    Cave            UMETA(DisplayName = "Cave"),
    OpenPlain       UMETA(DisplayName = "Open Plain"),
    DinoTerritory   UMETA(DisplayName = "Dino Territory")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Jungle_Day;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping = true;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone")
    USphereComponent* ZoneSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone")
    UAudioComponent* AmbientAudioComponent;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetZoneVolume(float NewVolume);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void FadeInAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void FadeOutAudio();

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    bool IsPlayerInZone() const;

private:
    UPROPERTY()
    float CurrentVolume = 0.0f;

    UPROPERTY()
    bool bPlayerInZone = false;

    void UpdateVolumeByDistance(float DeltaTime);
};
