#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "../SharedTypes.h"
#include "Narr_EnvironmentalNarrator.generated.h"

UENUM(BlueprintType)
enum class ENarr_EnvironmentalEvent : uint8
{
    None UMETA(DisplayName = "None"),
    EnteringForest UMETA(DisplayName = "Entering Forest"),
    LeavingForest UMETA(DisplayName = "Leaving Forest"),
    NearWater UMETA(DisplayName = "Near Water"),
    HighGround UMETA(DisplayName = "High Ground"),
    DangerousArea UMETA(DisplayName = "Dangerous Area"),
    SafeZone UMETA(DisplayName = "Safe Zone"),
    WeatherChange UMETA(DisplayName = "Weather Change"),
    TimeOfDay UMETA(DisplayName = "Time of Day")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_EnvironmentalTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    ENarr_EnvironmentalEvent EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    FString NarrativeText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    float CooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental")
    bool bIsActive;

    FNarr_EnvironmentalTrigger()
    {
        EventType = ENarr_EnvironmentalEvent::None;
        NarrativeText = TEXT("The environment whispers its secrets...");
        TriggerRadius = 500.0f;
        CooldownTime = 30.0f;
        bIsActive = true;
    }
};

UCLASS(ClassGroup=(Narrative), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_EnvironmentalNarrator : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_EnvironmentalNarrator();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    TArray<FNarr_EnvironmentalTrigger> EnvironmentalTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    float CheckInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    bool bEnableEnvironmentalNarration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    float NarrationVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Narrative")
    float NarrationDuration;

private:
    UPROPERTY()
    class APawn* PlayerPawn;

    UPROPERTY()
    TMap<ENarr_EnvironmentalEvent, float> LastTriggerTimes;

    float TimeSinceLastCheck;

public:
    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    void TriggerEnvironmentalNarration(ENarr_EnvironmentalEvent EventType, const FString& CustomText = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    void AddEnvironmentalTrigger(ENarr_EnvironmentalEvent EventType, const FString& NarrativeText, float Radius, float Cooldown);

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    void SetEnvironmentalNarrationEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Environmental Narrative")
    bool IsEventOnCooldown(ENarr_EnvironmentalEvent EventType) const;

protected:
    UFUNCTION()
    void CheckEnvironmentalConditions();

    UFUNCTION()
    void ProcessEnvironmentalEvent(const FNarr_EnvironmentalTrigger& Trigger);

    UFUNCTION()
    bool IsPlayerInForest() const;

    UFUNCTION()
    bool IsPlayerNearWater() const;

    UFUNCTION()
    bool IsPlayerOnHighGround() const;

    UFUNCTION()
    bool IsPlayerInDangerousArea() const;
};