#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Campfire        UMETA(DisplayName = "Campfire"),
    ForestNight     UMETA(DisplayName = "Forest Night"),
    TRexTerritory   UMETA(DisplayName = "T-Rex Territory"),
    River           UMETA(DisplayName = "River"),
    OpenPlain       UMETA(DisplayName = "Open Plain"),
    Cave            UMETA(DisplayName = "Cave")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::ForestNight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float BlendRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bLooping = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    bool bTriggersDangerMusic = false;
};

/**
 * AAudio_ZoneManager — Adaptive ambient audio zone actor.
 * Blends ambient sound based on player proximity.
 * Supports campfire, forest night, T-Rex territory, river, cave zones.
 */
UCLASS(BlueprintType, Blueprintable, meta = (DisplayName = "Audio Zone Manager"))
class TRANSPERSONALGAME_API AAudio_ZoneManager : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneManager();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* BlendSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone",
        meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AmbientAudioComponent;

    /** Current blend weight 0-1 based on player distance */
    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone")
    float CurrentBlendWeight = 0.0f;

    /** Is player currently inside this zone? */
    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone")
    bool bPlayerInZone = false;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetZoneActive(bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetDistanceToPlayer() const;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void TriggerDangerStinger();

private:
    void UpdateBlendWeight(float DeltaTime);

    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerLeaveZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    float TargetBlendWeight = 0.0f;
};
