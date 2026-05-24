#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "SharedTypes.h"
#include "Quest_ComprehensiveIntegrationSystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_IntegrationZoneData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FString ZoneName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    FVector ZoneLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    EQuestType AssociatedQuestType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    int32 MaxCrowdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration")
    bool bIsActive;

    FQuest_IntegrationZoneData()
    {
        ZoneName = TEXT("DefaultZone");
        ZoneLocation = FVector::ZeroVector;
        ZoneRadius = 1000.0f;
        AssociatedQuestType = EQuestType::Exploration;
        MaxCrowdSize = 50;
        bIsActive = true;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_EnvironmentalStoryData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FString StoryElementName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FString StoryDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FVector ElementLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    float InteractionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    bool bRequiresSpecificQuest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    FString RequiredQuestID;

    FQuest_EnvironmentalStoryData()
    {
        StoryElementName = TEXT("Unknown Element");
        StoryDescription = TEXT("An ancient remnant of the past.");
        ElementLocation = FVector::ZeroVector;
        InteractionRadius = 500.0f;
        bRequiresSpecificQuest = false;
        RequiredQuestID = TEXT("");
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_ComprehensiveIntegrationSystem : public AActor
{
    GENERATED_BODY()

public:
    AQuest_ComprehensiveIntegrationSystem();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Integration Zones")
    TArray<FQuest_IntegrationZoneData> IntegrationZones;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environmental Story")
    TArray<FQuest_EnvironmentalStoryData> EnvironmentalStoryElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Quest Complexity")
    TMap<EQuestDifficulty, int32> ComplexityLayers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Settings")
    bool bEnableAdvancedIntegration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Settings")
    float SystemUpdateInterval;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Settings")
    int32 MaxSimultaneousQuests;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void InitializeIntegrationSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void RegisterIntegrationZone(const FQuest_IntegrationZoneData& ZoneData);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void RegisterEnvironmentalStory(const FQuest_EnvironmentalStoryData& StoryData);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    bool ValidateQuestIntegration(const FString& QuestID);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    TArray<FQuest_IntegrationZoneData> GetActiveIntegrationZones();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    FQuest_EnvironmentalStoryData GetNearestStoryElement(const FVector& PlayerLocation);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void UpdateIntegrationSystem();

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void ActivateIntegrationZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    void DeactivateIntegrationZone(const FString& ZoneName);

    UFUNCTION(BlueprintCallable, Category = "Quest Integration")
    bool IsPlayerInIntegrationZone(const FVector& PlayerLocation, FString& OutZoneName);

private:
    void SetupDefaultIntegrationZones();
    void SetupDefaultEnvironmentalStories();
    void ValidateSystemIntegrity();

    FTimerHandle SystemUpdateTimer;
    bool bSystemInitialized;
};