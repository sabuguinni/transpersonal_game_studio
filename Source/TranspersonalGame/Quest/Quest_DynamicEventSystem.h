#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/TimerHandle.h"
#include "Quest_DynamicEventSystem.generated.h"

// Event types for dynamic quest generation
UENUM(BlueprintType)
enum class EQuest_DynamicEventType : uint8
{
    DinosaurAttack      UMETA(DisplayName = "Dinosaur Attack"),
    ResourceDepletion   UMETA(DisplayName = "Resource Depletion"),
    WeatherStorm        UMETA(DisplayName = "Weather Storm"),
    TerritorialDispute  UMETA(DisplayName = "Territorial Dispute"),
    InjuredTribeMember  UMETA(DisplayName = "Injured Tribe Member"),
    FoodShortage        UMETA(DisplayName = "Food Shortage")
};

// Event urgency levels
UENUM(BlueprintType)
enum class EQuest_EventUrgency : uint8
{
    Low         UMETA(DisplayName = "Low Priority"),
    Medium      UMETA(DisplayName = "Medium Priority"),
    High        UMETA(DisplayName = "High Priority"),
    Critical    UMETA(DisplayName = "Critical Emergency")
};

// Dynamic event data structure
USTRUCT(BlueprintType)
struct FQuest_DynamicEventData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_DynamicEventType EventType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EQuest_EventUrgency Urgency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector EventLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EventRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EventDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RewardPoints;

    FQuest_DynamicEventData()
    {
        EventType = EQuest_DynamicEventType::DinosaurAttack;
        Urgency = EQuest_EventUrgency::Medium;
        EventTitle = TEXT("Unknown Event");
        EventDescription = TEXT("A mysterious event has occurred");
        EventLocation = FVector::ZeroVector;
        EventRadius = 500.0f;
        EventDuration = 300.0f; // 5 minutes
        RewardPoints = 100;
    }
};

// Component for managing dynamic quest events
UCLASS(ClassGroup=(Quest), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UQuest_DynamicEventComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UQuest_DynamicEventComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Event management
    UFUNCTION(BlueprintCallable, Category = "Dynamic Events")
    void TriggerEvent(const FQuest_DynamicEventData& EventData);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Events")
    void CompleteEvent();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Events")
    bool IsEventActive() const { return bEventActive; }

    UFUNCTION(BlueprintCallable, Category = "Dynamic Events")
    FQuest_DynamicEventData GetCurrentEvent() const { return CurrentEventData; }

    // Event generation
    UFUNCTION(BlueprintCallable, Category = "Dynamic Events")
    FQuest_DynamicEventData GenerateRandomEvent(const FVector& PlayerLocation);

protected:
    // Event state
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event State")
    bool bEventActive;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event State")
    FQuest_DynamicEventData CurrentEventData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event State")
    float EventTimeRemaining;

    // Event configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Config")
    float EventGenerationInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Config")
    float EventDetectionRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Event Config")
    bool bAutoGenerateEvents;

    // Timers
    FTimerHandle EventGenerationTimer;
    FTimerHandle EventDurationTimer;

    // Helper functions
    void GenerateRandomEventNearPlayer();
    void OnEventTimeout();
    EQuest_DynamicEventType GetRandomEventType() const;
    EQuest_EventUrgency GetRandomUrgency() const;
    FString GetEventTitleForType(EQuest_DynamicEventType Type) const;
    FString GetEventDescriptionForType(EQuest_DynamicEventType Type) const;
};

// Actor for visual representation of dynamic events
UCLASS()
class TRANSPERSONALGAME_API AQuest_DynamicEventActor : public AActor
{
    GENERATED_BODY()

public:
    AQuest_DynamicEventActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // Event setup
    UFUNCTION(BlueprintCallable, Category = "Dynamic Event")
    void SetupEvent(const FQuest_DynamicEventData& EventData);

    UFUNCTION(BlueprintCallable, Category = "Dynamic Event")
    void ActivateEvent();

    UFUNCTION(BlueprintCallable, Category = "Dynamic Event")
    void DeactivateEvent();

    // Player interaction
    UFUNCTION()
    void OnPlayerEnterEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitEvent(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

protected:
    // Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* EventTrigger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EventMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UTextRenderComponent* EventText;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UQuest_DynamicEventComponent* DynamicEventComponent;

    // Event data
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event Data")
    FQuest_DynamicEventData EventData;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event State")
    bool bPlayerInRange;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Event State")
    bool bEventCompleted;

    // Visual effects
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects")
    float PulseSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual Effects")
    float PulseIntensity;

    float PulseTimer;

    // Helper functions
    void UpdateVisualEffects(float DeltaTime);
    FLinearColor GetColorForUrgency(EQuest_EventUrgency Urgency) const;
    void UpdateEventText();
};

#include "Quest_DynamicEventSystem.generated.h"