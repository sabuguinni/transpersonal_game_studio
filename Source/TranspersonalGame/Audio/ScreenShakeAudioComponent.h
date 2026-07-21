#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "ScreenShakeAudioComponent.generated.h"

class UAudioComponent;
class UCameraShakeBase;

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FAudio_ScreenShakeConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Audio")
    float MinDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Audio")
    float MaxDistance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Audio")
    float MinIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Audio")
    float MaxIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Audio")
    float AudioVolumeMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screen Shake Audio")
    FString AudioCueName;

    FAudio_ScreenShakeConfig()
    {
        MinDistance = 500.0f;
        MaxDistance = 2000.0f;
        MinIntensity = 0.1f;
        MaxIntensity = 2.0f;
        AudioVolumeMultiplier = 1.0f;
        AudioCueName = TEXT("ScreenShake_Default");
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UScreenShakeAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UScreenShakeAudioComponent();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // Screen shake triggering
    UFUNCTION(BlueprintCallable, Category = "Screen Shake Audio")
    void TriggerScreenShake(const FVector& SourceLocation, float Intensity, EDinosaurSpecies DinosaurType = EDinosaurSpecies::TRex);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake Audio")
    void TriggerFootstepShake(const FVector& FootstepLocation, float DinosaurMass, float MovementSpeed);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake Audio")
    void TriggerImpactShake(const FVector& ImpactLocation, float ImpactForce);

    // Configuration
    UFUNCTION(BlueprintCallable, Category = "Screen Shake Audio")
    void SetShakeConfig(const FAudio_ScreenShakeConfig& NewConfig);

    UFUNCTION(BlueprintCallable, Category = "Screen Shake Audio")
    void EnableScreenShake(bool bEnabled);

    // Distance-based calculations
    UFUNCTION(BlueprintCallable, Category = "Screen Shake Audio")
    float CalculateShakeIntensity(const FVector& SourceLocation, const FVector& PlayerLocation) const;

    UFUNCTION(BlueprintCallable, Category = "Screen Shake Audio")
    bool ShouldTriggerShake(const FVector& SourceLocation, const FVector& PlayerLocation) const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Screen Shake Audio")
    FAudio_ScreenShakeConfig ShakeConfig;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Screen Shake Audio")
    bool bScreenShakeEnabled;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Screen Shake Audio")
    float ShakeCooldownTime;

    UPROPERTY(BlueprintReadOnly, Category = "Screen Shake Audio")
    float LastShakeTime;

    UPROPERTY(BlueprintReadOnly, Category = "Screen Shake Audio")
    TArray<UAudioComponent*> ActiveShakeAudioComponents;

private:
    void PlayShakeAudio(const FVector& Location, float Intensity);
    void CleanupFinishedAudio();
    FVector GetPlayerLocation() const;
    APlayerController* GetPlayerController() const;
    void ApplyScreenShake(float Intensity);
};