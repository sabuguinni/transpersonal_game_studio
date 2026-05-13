#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraShakeBase.h"
#include "Audio_ScreenShakeSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ShakeProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Duration = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Frequency = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    float Range = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake")
    bool bFalloffByDistance = true;

    FAudio_ShakeProfile()
    {
        Intensity = 1.0f;
        Duration = 0.5f;
        Frequency = 10.0f;
        Range = 1000.0f;
        bFalloffByDistance = true;
    }
};

UCLASS(ClassGroup=(Audio), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UAudio_ScreenShakeSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UAudio_ScreenShakeSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Profiles")
    FAudio_ShakeProfile TRexFootstepShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Profiles")
    FAudio_ShakeProfile DinosaurImpactShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Profiles")
    FAudio_ShakeProfile PlayerDamageShake;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Profiles")
    FAudio_ShakeProfile EnvironmentalShake;

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerTRexFootstep(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerDinosaurImpact(FVector Location, float ImpactForce);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerPlayerDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerEnvironmentalShake(FVector Location, const FAudio_ShakeProfile& Profile);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake")
    void TriggerCustomShake(FVector Location, float Intensity, float Duration, float Range = 1000.0f);

private:
    void ApplyScreenShake(const FAudio_ShakeProfile& Profile, FVector Location = FVector::ZeroVector);
    float CalculateDistanceFalloff(FVector ShakeLocation, float MaxRange);
    
    UPROPERTY()
    class APlayerController* CachedPlayerController;

    UPROPERTY()
    class APawn* CachedPlayerPawn;

    void CachePlayerReferences();
    bool IsPlayerValid();
};