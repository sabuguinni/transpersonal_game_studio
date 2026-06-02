#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Narr_TribalCommunicationSystem.generated.h"

UENUM(BlueprintType)
enum class ENarr_CommunicationType : uint8
{
    Gesture         UMETA(DisplayName = "Hand Gesture"),
    Vocal           UMETA(DisplayName = "Vocal Sound"),
    Warning         UMETA(DisplayName = "Warning Call"),
    Hunting         UMETA(DisplayName = "Hunt Signal"),
    Gathering       UMETA(DisplayName = "Gather Call"),
    Danger          UMETA(DisplayName = "Danger Alert"),
    Fire            UMETA(DisplayName = "Fire Signal"),
    Water           UMETA(DisplayName = "Water Found"),
    Shelter         UMETA(DisplayName = "Shelter Call")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_TribalMessage
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Communication")
    ENarr_CommunicationType MessageType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Communication")
    FString MessageContent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Communication")
    FVector TargetLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Communication")
    float UrgencyLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Communication")
    float CommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tribal Communication")
    bool bRequiresResponse;

    FNarr_TribalMessage()
    {
        MessageType = ENarr_CommunicationType::Gesture;
        MessageContent = TEXT("");
        TargetLocation = FVector::ZeroVector;
        UrgencyLevel = 0.5f;
        CommunicationRange = 1000.0f;
        bRequiresResponse = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNarr_TribalCommunicationSystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNarr_TribalCommunicationSystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Core Communication Functions
    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    void SendTribalMessage(const FNarr_TribalMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    void ReceiveTribalMessage(const FNarr_TribalMessage& Message);

    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    void BroadcastEmergencySignal(ENarr_CommunicationType SignalType, const FVector& DangerLocation);

    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    void SendHuntingCoordination(const FVector& PreyLocation, float PreySize);

    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    void CallForGathering(const FVector& ResourceLocation, EResourceType ResourceType);

    // Response and Acknowledgment
    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    void AcknowledgeMessage(const FNarr_TribalMessage& OriginalMessage);

    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    bool CanHearCommunication(const FVector& SourceLocation, float MessageRange) const;

    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    void ProcessIncomingSignals();

    // Gesture and Vocal Systems
    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    void PerformGesture(ENarr_CommunicationType GestureType, const FVector& TargetDirection);

    UFUNCTION(BlueprintCallable, Category = "Tribal Communication")
    void MakeVocalSound(ENarr_CommunicationType SoundType, float Intensity);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    float MaxCommunicationRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    float GestureDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    float VocalSoundRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    TArray<FNarr_TribalMessage> PendingMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    TArray<FNarr_TribalMessage> ReceivedMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    bool bCanSendMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    bool bCanReceiveMessages;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    float MessageCooldownTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Communication Settings")
    float LastMessageTime;

private:
    void UpdateCommunicationRange();
    void ProcessMessageQueue();
    bool ValidateMessage(const FNarr_TribalMessage& Message) const;
    void TriggerCommunicationAnimation(ENarr_CommunicationType Type);
    void PlayCommunicationSound(ENarr_CommunicationType Type, float Intensity);
};