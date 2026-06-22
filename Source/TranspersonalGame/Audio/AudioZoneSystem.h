#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "AudioZoneSystem.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Safe_Camp       UMETA(DisplayName = "Safe Camp"),
    Danger_Predator UMETA(DisplayName = "Danger Predator"),
    River_Ambient   UMETA(DisplayName = "River Ambient"),
    Forest_Dense    UMETA(DisplayName = "Dense Forest"),
    Open_Plains     UMETA(DisplayName = "Open Plains")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Safe_Camp;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float BlendOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float AmbientVolume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    float MusicIntensity = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    bool bLooping = true;
};

UCLASS(ClassGroup = (Audio), meta = (BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API AAudio_ZoneTrigger : public AActor
{
    GENERATED_BODY()

public:
    AAudio_ZoneTrigger();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio|Zone",
        meta = (AllowPrivateAccess = "true"))
    USphereComponent* TriggerSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FAudio_ZoneConfig ZoneConfig;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio|Zone")
    FString LinkedDialogueTriggerType;

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerEnterZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    void OnPlayerExitZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

    UFUNCTION(BlueprintCallable, Category = "Audio|Zone")
    float GetMusicIntensity() const { return ZoneConfig.MusicIntensity; }

protected:
    virtual void BeginPlay() override;

private:
    bool bPlayerInside = false;

    UFUNCTION()
    void HandleOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void HandleOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};
