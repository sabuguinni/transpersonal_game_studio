#include "Eng_ComponentProtocols.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

UEng_ComponentProtocols::UEng_ComponentProtocols()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Update every 100ms
    
    ComponentId = FString::Printf(TEXT("Component_%d"), GetUniqueID());
}

void UEng_ComponentProtocols::BeginPlay()
{
    Super::BeginPlay();
    
    // Register this component
    RegisterComponent(ComponentId, TEXT("ComponentProtocols"));
    
    // Register default message types
    RegisterMessageHandler(TEXT("HEALTH_UPDATE"));
    RegisterMessageHandler(TEXT("POSITION_SYNC"));
    RegisterMessageHandler(TEXT("STATE_CHANGE"));
    RegisterMessageHandler(TEXT("INTERACTION_REQUEST"));
    RegisterMessageHandler(TEXT("PERFORMANCE_REPORT"));
    
    UE_LOG(LogTemp, Warning, TEXT("Component Protocols initialized: %s"), *ComponentId);
}

void UEng_ComponentProtocols::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    ProcessMessageQueue();
    UpdatePerformanceMetrics();
}

void UEng_ComponentProtocols::SendMessage(const FString& MessageType, const FString& TargetId, const FString& MessageData)
{
    FEng_ComponentMessage NewMessage;
    NewMessage.MessageType = MessageType;
    NewMessage.SenderId = ComponentId;
    NewMessage.TargetId = TargetId;
    NewMessage.MessageData = MessageData;
    NewMessage.Timestamp = GetCurrentTime();
    
    MessageQueue.Add(NewMessage);
    
    UE_LOG(LogTemp, Log, TEXT("Message sent: %s -> %s [%s]"), *ComponentId, *TargetId, *MessageType);
}

void UEng_ComponentProtocols::RegisterMessageHandler(const FString& MessageType)
{
    SupportedMessageTypes.AddUnique(MessageType);
    UE_LOG(LogTemp, Log, TEXT("Message handler registered: %s for %s"), *MessageType, *ComponentId);
}

TArray<FEng_ComponentMessage> UEng_ComponentProtocols::GetPendingMessages() const
{
    TArray<FEng_ComponentMessage> PendingMessages;
    
    for (const FEng_ComponentMessage& Message : MessageQueue)
    {
        if (Message.TargetId == ComponentId || Message.TargetId == TEXT("BROADCAST"))
        {
            PendingMessages.Add(Message);
        }
    }
    
    return PendingMessages;
}

void UEng_ComponentProtocols::ProcessMessage(const FEng_ComponentMessage& Message)
{
    if (SupportedMessageTypes.Contains(Message.MessageType))
    {
        UE_LOG(LogTemp, Log, TEXT("Processing message: %s from %s"), *Message.MessageType, *Message.SenderId);
        
        // Handle specific message types
        if (Message.MessageType == TEXT("HEALTH_UPDATE"))
        {
            // Process health update
        }
        else if (Message.MessageType == TEXT("POSITION_SYNC"))
        {
            // Process position synchronization
        }
        else if (Message.MessageType == TEXT("STATE_CHANGE"))
        {
            // Process state change notification
        }
        else if (Message.MessageType == TEXT("INTERACTION_REQUEST"))
        {
            // Process interaction request
            RequestComponentInteraction(Message.SenderId, TEXT("RESPONSE"));
        }
        else if (Message.MessageType == TEXT("PERFORMANCE_REPORT"))
        {
            // Process performance report
            float ProcessingTime = FCString::Atof(*Message.MessageData);
            ReportComponentPerformance(Message.SenderId, ProcessingTime);
        }
    }
}

void UEng_ComponentProtocols::RegisterComponent(const FString& InComponentId, const FString& ComponentType)
{
    RegisteredComponents.Add(InComponentId, ComponentType);
    UE_LOG(LogTemp, Warning, TEXT("Component registered: %s [%s]"), *InComponentId, *ComponentType);
}

bool UEng_ComponentProtocols::IsComponentRegistered(const FString& InComponentId) const
{
    return RegisteredComponents.Contains(InComponentId);
}

TArray<FString> UEng_ComponentProtocols::GetRegisteredComponents() const
{
    TArray<FString> ComponentIds;
    RegisteredComponents.GetKeys(ComponentIds);
    return ComponentIds;
}

void UEng_ComponentProtocols::RequestComponentInteraction(const FString& TargetComponent, const FString& InteractionType)
{
    if (IsComponentRegistered(TargetComponent))
    {
        SendMessage(TEXT("INTERACTION_REQUEST"), TargetComponent, InteractionType);
        UE_LOG(LogTemp, Log, TEXT("Interaction requested: %s with %s"), *InteractionType, *TargetComponent);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot request interaction: Component %s not registered"), *TargetComponent);
    }
}

void UEng_ComponentProtocols::ApproveInteraction(const FString& RequesterId, bool bApproved)
{
    FString ResponseData = bApproved ? TEXT("APPROVED") : TEXT("DENIED");
    SendMessage(TEXT("INTERACTION_RESPONSE"), RequesterId, ResponseData);
    
    UE_LOG(LogTemp, Log, TEXT("Interaction %s for %s"), *ResponseData, *RequesterId);
}

void UEng_ComponentProtocols::ReportComponentPerformance(const FString& InComponentId, float ProcessingTime)
{
    if (!ComponentPerformanceData.Contains(InComponentId))
    {
        ComponentPerformanceData.Add(InComponentId, TArray<float>());
    }
    
    TArray<float>& PerformanceHistory = ComponentPerformanceData[InComponentId];
    PerformanceHistory.Add(ProcessingTime);
    
    // Keep only last 100 samples
    if (PerformanceHistory.Num() > 100)
    {
        PerformanceHistory.RemoveAt(0);
    }
    
    UE_LOG(LogTemp, VeryVerbose, TEXT("Performance reported: %s = %.3fms"), *InComponentId, ProcessingTime);
}

float UEng_ComponentProtocols::GetAverageComponentPerformance(const FString& InComponentId) const
{
    if (ComponentPerformanceData.Contains(InComponentId))
    {
        const TArray<float>& PerformanceHistory = ComponentPerformanceData[InComponentId];
        if (PerformanceHistory.Num() > 0)
        {
            float Total = 0.0f;
            for (float Sample : PerformanceHistory)
            {
                Total += Sample;
            }
            return Total / PerformanceHistory.Num();
        }
    }
    return 0.0f;
}

void UEng_ComponentProtocols::ProcessMessageQueue()
{
    TArray<FEng_ComponentMessage> PendingMessages = GetPendingMessages();
    
    for (const FEng_ComponentMessage& Message : PendingMessages)
    {
        ProcessMessage(Message);
    }
    
    // Clean up old messages (older than 5 seconds)
    float CurrentTime = GetCurrentTime();
    MessageQueue.RemoveAll([CurrentTime](const FEng_ComponentMessage& Message)
    {
        return (CurrentTime - Message.Timestamp) > 5.0f;
    });
}

void UEng_ComponentProtocols::UpdatePerformanceMetrics()
{
    // Update performance metrics for registered components
    for (const auto& ComponentPair : RegisteredComponents)
    {
        const FString& CompId = ComponentPair.Key;
        float AvgPerformance = GetAverageComponentPerformance(CompId);
        
        if (AvgPerformance > 16.67f) // > 16.67ms indicates potential 60fps issues
        {
            UE_LOG(LogTemp, Warning, TEXT("Performance warning: %s averaging %.2fms"), *CompId, AvgPerformance);
        }
    }
}

float UEng_ComponentProtocols::GetCurrentTime() const
{
    if (UWorld* World = GetWorld())
    {
        return World->GetTimeSeconds();
    }
    return 0.0f;
}