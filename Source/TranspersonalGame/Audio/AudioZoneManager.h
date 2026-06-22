#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
#include "AudioZoneManager.generated.h"

UENUM(BlueprintType)
enum class EAudio_ZoneType : uint8
{
    Ambient_Plains      UMETA(DisplayName = "Ambient Plains"),
    Ambient_Forest      UMETA(DisplayName = "Ambient Forest"),
    Danger_Raptor       UMETA(DisplayName = "Danger Raptor"),
    Danger_TRex         UMETA(DisplayName = "Danger TRex"),
    Tribal_Camp         UMETA(DisplayName = "Tribal Camp"),
    Water_River         UMETA(DisplayName = "Water River")
};

USTRUCT(BlueprintType)
struct FAudio_ZoneConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    EAudio_ZoneType ZoneType = EAudio_ZoneType::Ambient_Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float TriggerRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float MaxVolume = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeInTime = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    float FadeOutTime = 3.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
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
    USphereComponent* TriggerSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio Zone")
    UAudioComponent* AudioComponent;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone")
    bool bPlayerInZone;

    UPROPERTY(BlueprintReadOnly, Category = "Audio Zone")
    float CurrentVolume;

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerEnterZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void OnPlayerExitZone(AActor* PlayerActor);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    void SetZoneType(EAudio_ZoneType NewType);

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    EAudio_ZoneType GetZoneType() const { return ZoneConfig.ZoneType; }

    UFUNCTION(BlueprintCallable, Category = "Audio Zone")
    float GetDistanceToPlayer() const;

private:
    void UpdateVolumeFade(float DeltaTime);
    float TargetVolume;
};
