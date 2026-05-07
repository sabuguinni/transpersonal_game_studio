#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Engine/TriggerBox.h"
#include "SharedTypes.h"
#include "Narr_ContextualVoiceSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_VoiceContextType : uint8
{
    Discovery,
    Warning,
    Instruction,
    Emergency,
    Observation,
    Achievement
};

USTRUCT(BlueprintType)
struct FNarr_VoiceLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    FString Text;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    ENarr_VoiceContextType ContextType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    float Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice Line")
    bool bIsRepeatable;

    FNarr_VoiceLine()
    {
        Text = TEXT("");
        AudioURL = TEXT("");
        ContextType = ENarr_VoiceContextType::Observation;
        Priority = 1.0f;
        bIsRepeatable = false;
    }
};

USTRUCT(BlueprintType)
struct FNarr_ContextualTrigger
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    FVector Location;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    float TriggerRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    TArray<FNarr_VoiceLine> VoiceLines;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    int32 MaxTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Trigger")
    int32 CurrentTriggerCount;

    FNarr_ContextualTrigger()
    {
        Location = FVector::ZeroVector;
        TriggerRadius = 500.0f;
        bIsActive = true;
        MaxTriggers = 1;
        CurrentTriggerCount = 0;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ANarr_ContextualVoiceSystem : public AActor
{
    GENERATED_BODY()

public:
    ANarr_ContextualVoiceSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    TArray<FNarr_ContextualTrigger> ContextualTriggers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    float VoiceVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    float MinTimeBetweenVoiceLines;

    UPROPERTY(BlueprintReadOnly, Category = "Voice System")
    float LastVoiceLineTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Voice System")
    bool bSystemEnabled;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void PlayVoiceLine(const FNarr_VoiceLine& VoiceLine);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void TriggerContextualVoice(ENarr_VoiceContextType ContextType, const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void AddContextualTrigger(const FNarr_ContextualTrigger& NewTrigger);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void SetSystemEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    bool IsVoiceLineAvailable() const;

    UFUNCTION(BlueprintCallable, Category = "Voice System")
    void InitializeDefaultVoiceLines();

protected:
    UFUNCTION()
    void CheckPlayerProximity();

    UFUNCTION()
    FNarr_VoiceLine SelectBestVoiceLine(const TArray<FNarr_VoiceLine>& VoiceLines, ENarr_VoiceContextType ContextType) const;

    UFUNCTION()
    void CreateBiomeDiscoveryTriggers();

    UFUNCTION()
    void CreateSurvivalEventTriggers();
};