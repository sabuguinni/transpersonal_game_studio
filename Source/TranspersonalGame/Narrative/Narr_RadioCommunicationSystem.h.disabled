#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "../SharedTypes.h"
#include "Narr_RadioCommunicationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_RadioMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Message")
    FString SenderName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Message")
    FString MessageContent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Message")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Message")
    ENarr_MessagePriority Priority;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Message")
    ENarr_MessageCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Message")
    float MessageDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Message")
    bool bIsEmergency;

    FNarr_RadioMessage()
    {
        SenderName = TEXT("");
        MessageContent = TEXT("");
        AudioURL = TEXT("");
        Priority = ENarr_MessagePriority::Normal;
        Category = ENarr_MessageCategory::General;
        MessageDuration = 30.0f;
        bIsEmergency = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_RadioStation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    FString StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    FString StationCallsign;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    FVector StationLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    float TransmissionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    bool bIsActive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio Station")
    TArray<FNarr_RadioMessage> MessageQueue;

    FNarr_RadioStation()
    {
        StationName = TEXT("");
        StationCallsign = TEXT("");
        StationLocation = FVector::ZeroVector;
        TransmissionRange = 5000.0f;
        bIsActive = true;
        MessageQueue.Empty();
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNarr_RadioCommunicationSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UNarr_RadioCommunicationSystem();

    // Subsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    // Radio message management
    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    void BroadcastMessage(const FNarr_RadioMessage& Message, const FString& StationCallsign);

    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    void BroadcastEmergencyAlert(const FString& SenderName, const FString& AlertMessage, const FString& AudioURL);

    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    void QueueMessage(const FNarr_RadioMessage& Message, const FString& StationCallsign);

    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    bool IsPlayerInRange(const FString& StationCallsign) const;

    // Station management
    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    void RegisterRadioStation(const FNarr_RadioStation& Station);

    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    void SetStationActive(const FString& StationCallsign, bool bActive);

    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    FNarr_RadioStation GetStation(const FString& StationCallsign) const;

    // Audio playback
    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    void PlayRadioMessage(const FNarr_RadioMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    void StopCurrentTransmission();

    // Message filtering
    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    TArray<FNarr_RadioMessage> GetMessagesInRange() const;

    UFUNCTION(BlueprintCallable, Category = "Radio Communication")
    TArray<FNarr_RadioMessage> GetEmergencyMessages() const;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio System")
    TMap<FString, FNarr_RadioStation> RadioStations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio System")
    TArray<FNarr_RadioMessage> ActiveMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio System")
    UAudioComponent* RadioAudioComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio System")
    float DefaultTransmissionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio System")
    float MessagePlaybackDelay;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Radio System")
    bool bRadioSystemEnabled;

private:
    void ProcessMessageQueue();
    void UpdateActiveMessages();
    FVector GetPlayerLocation() const;
    float CalculateDistance(const FVector& Location1, const FVector& Location2) const;
    void InitializeDefaultStations();
};