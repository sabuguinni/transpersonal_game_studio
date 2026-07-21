#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Eng_ComponentProtocols.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_ComponentMessage
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "Component Protocol")
    FString MessageType;

    UPROPERTY(BlueprintReadWrite, Category = "Component Protocol")
    FString SenderId;

    UPROPERTY(BlueprintReadWrite, Category = "Component Protocol")
    FString TargetId;

    UPROPERTY(BlueprintReadWrite, Category = "Component Protocol")
    FString MessageData;

    UPROPERTY(BlueprintReadWrite, Category = "Component Protocol")
    float Timestamp;

    FEng_ComponentMessage()
    {
        MessageType = TEXT("");
        SenderId = TEXT("");
        TargetId = TEXT("");
        MessageData = TEXT("");
        Timestamp = 0.0f;
    }
};

UCLASS(BlueprintType, Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEng_ComponentProtocols : public UActorComponent
{
    GENERATED_BODY()

public:
    UEng_ComponentProtocols();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Message System
    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    void SendMessage(const FString& MessageType, const FString& TargetId, const FString& MessageData);

    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    void RegisterMessageHandler(const FString& MessageType);

    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    TArray<FEng_ComponentMessage> GetPendingMessages() const;

    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    void ProcessMessage(const FEng_ComponentMessage& Message);

    // Component Registration
    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    void RegisterComponent(const FString& ComponentId, const FString& ComponentType);

    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    bool IsComponentRegistered(const FString& ComponentId) const;

    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    TArray<FString> GetRegisteredComponents() const;

    // Interaction Protocols
    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    void RequestComponentInteraction(const FString& TargetComponent, const FString& InteractionType);

    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    void ApproveInteraction(const FString& RequesterId, bool bApproved);

    // Performance Monitoring
    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    void ReportComponentPerformance(const FString& ComponentId, float ProcessingTime);

    UFUNCTION(BlueprintCallable, Category = "Component Protocol")
    float GetAverageComponentPerformance(const FString& ComponentId) const;

protected:
    UPROPERTY(BlueprintReadOnly, Category = "Component Protocol")
    TArray<FEng_ComponentMessage> MessageQueue;

    UPROPERTY(BlueprintReadOnly, Category = "Component Protocol")
    TMap<FString, FString> RegisteredComponents;

    UPROPERTY(BlueprintReadOnly, Category = "Component Protocol")
    TArray<FString> SupportedMessageTypes;

    UPROPERTY(BlueprintReadOnly, Category = "Component Protocol")
    TMap<FString, TArray<float>> ComponentPerformanceData;

    UPROPERTY(BlueprintReadOnly, Category = "Component Protocol")
    FString ComponentId;

private:
    void ProcessMessageQueue();
    void UpdatePerformanceMetrics();
    float GetCurrentTime() const;
};